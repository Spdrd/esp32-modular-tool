#include "JoystickManager.h"
#include <math.h>

// Zona muerta: por debajo de esto el eje se reporta como 0. Los potenciometros
// nunca vuelven exactamente al centro y el ADC del ESP32 tiene ruido propio.
static const float DEADZONE = 0.18f;

// Umbrales con histeresis para los eventos discretos: hace falta pasar de
// PRESS para activar una direccion, pero cae solo al bajar de RELEASE. Sin
// esa separacion el stick tiembla en el borde y dispara eventos en rafaga.
static const float DIR_PRESS   = 0.55f;
static const float DIR_RELEASE = 0.38f;

// Repeticion al mantener, igual criterio que el DAS de los juegos
static const unsigned long REPEAT_DELAY_MS  = 420;
static const unsigned long REPEAT_PERIOD_MS = 140;

// Suavizado exponencial de la lectura cruda
static const float EMA_ALPHA = 0.35f;

static const int SAMPLES_PER_UPDATE = 4;

// Antirrebote del boton del stick, mismo criterio que ButtonManager
static const unsigned long SW_DEBOUNCE_MS = 50;

JoystickManager::JoystickManager(JoystickPinConfig config)
    : config(config),
      rawX(0), rawY(0), centerX(2048), centerY(2048),
      normX(0), normY(0), emaX(0), emaY(0), emaReady(false),
      heldUp(false), heldDown(false), heldLeft(false), heldRight(false),
      dominant(JOY_NONE), lastFired(JOY_NONE),
      swHeld(false), swLastRead(false), swDebounceMs(0), swPressMs(0),
      pendingGesture(JOY_GESTURE_NONE),
      repeatStartMs(0), repeatLastMs(0),
      mirrored(nullptr) {}

void JoystickManager::begin() {
    // 11 dB = fondo de escala ~3.3V, que es el rango del joystick.
    analogReadResolution(12);
    analogSetPinAttenuation(config.xPin, ADC_11db);
    analogSetPinAttenuation(config.yPin, ADC_11db);

    // Los GPIO 34-39 son solo de entrada y no admiten pinMode con pull.
    pinMode(config.xPin, INPUT);
    pinMode(config.yPin, INPUT);

    // El switch del stick cierra a masa: necesita pull-up. El GPIO 33 si lo
    // tiene (los 34-39 no), por eso el boton va ahi y no en otro ADC1.
    pinMode(config.swPin, INPUT_PULLUP);

    delay(10);
    calibrateCenter();

    Serial.print("[JOY] centro X=");
    Serial.print(centerX);
    Serial.print(" Y=");
    Serial.println(centerY);
}

int JoystickManager::sampleAxis(int pin) const {
    long sum = 0;
    for (int i = 0; i < SAMPLES_PER_UPDATE; i++) sum += analogRead(pin);
    return (int)(sum / SAMPLES_PER_UPDATE);
}

void JoystickManager::calibrateCenter(int samples) {
    long sx = 0, sy = 0;
    for (int i = 0; i < samples; i++) {
        sx += analogRead(config.xPin);
        sy += analogRead(config.yPin);
        delayMicroseconds(200);
    }
    centerX = (int)(sx / samples);
    centerY = (int)(sy / samples);

    emaReady = false;   // el filtro arranca de cero tras recalibrar
    normX = normY = 0;
    heldUp = heldDown = heldLeft = heldRight = false;
    dominant = lastFired = JOY_NONE;
}

// Normaliza a -1..1 tratando cada mitad por separado: el centro real casi
// nunca cae en 2048, asi que dividir por un solo rango daria un eje sesgado.
float JoystickManager::normalize(int raw, int center, bool invert) const {
    float v;
    if (raw >= center) {
        int span = 4095 - center;
        v = (span > 0) ? (float)(raw - center) / (float)span : 0.0f;
    } else {
        v = (center > 0) ? (float)(raw - center) / (float)center : 0.0f;
    }

    if (invert) v = -v;
    if (v >  1.0f) v =  1.0f;
    if (v < -1.0f) v = -1.0f;

    // Zona muerta con reescalado, para que el valor util siga yendo de 0 a 1
    // en vez de saltar de golpe al salir de la zona muerta.
    float mag = fabsf(v);
    if (mag < DEADZONE) return 0.0f;
    float scaled = (mag - DEADZONE) / (1.0f - DEADZONE);
    return (v < 0) ? -scaled : scaled;
}

float JoystickManager::getMagnitude() const {
    float m = sqrtf(normX * normX + normY * normY);
    return (m > 1.0f) ? 1.0f : m;
}

float JoystickManager::getAngleDeg() const {
    if (isCentered()) return 0.0f;
    float a = atan2f(normY, normX) * 180.0f / (float)M_PI;
    if (a < 0) a += 360.0f;
    return a;
}

void JoystickManager::setCallbacks(ButtonActionCallbacks callbacks) {
    own = callbacks;
}

// Los callbacks espejados ganan: son los que la herramienta activa acaba de
// configurar en ButtonManager.
ButtonActionCallbacks JoystickManager::activeCallbacks() const {
    return mirrored ? mirrored->getCallbacks() : own;
}

void JoystickManager::fire(JoyDirection dir) {
    ButtonActionCallbacks cbs = activeCallbacks();

    switch (dir) {
        case JOY_UP:    Serial.println("[JOY] UP");    if (cbs.onUp)    cbs.onUp();    break;
        case JOY_DOWN:  Serial.println("[JOY] DOWN");  if (cbs.onDown)  cbs.onDown();  break;
        case JOY_LEFT:  Serial.println("[JOY] LEFT");  if (cbs.onLeft)  cbs.onLeft();  break;
        case JOY_RIGHT: Serial.println("[JOY] RIGHT"); if (cbs.onRight) cbs.onRight(); break;
        default: break;
    }
}

const char* joyGestureName(JoyGesture g) {
    switch (g) {
        case JOY_GESTURE_OK:   return "OK";
        case JOY_GESTURE_A:    return "A";
        case JOY_GESTURE_B:    return "B";
        case JOY_GESTURE_MENU: return "MENU";
        default:               return "--";
    }
}

JoyGesture JoystickManager::classifyHold(unsigned long heldMs) const {
    if (heldMs >= JOY_GESTURE_MENU_MS) return JOY_GESTURE_MENU;
    if (heldMs >= JOY_GESTURE_B_MS)    return JOY_GESTURE_B;
    if (heldMs >= JOY_GESTURE_A_MS)    return JOY_GESTURE_A;
    return JOY_GESTURE_OK;
}

void JoystickManager::fireGesture(JoyGesture g) {
    ButtonActionCallbacks cbs = activeCallbacks();
    switch (g) {
        case JOY_GESTURE_OK:   Serial.println("[JOY] OK");   if (cbs.onOk)   cbs.onOk();   break;
        case JOY_GESTURE_A:    Serial.println("[JOY] A");    if (cbs.onA)    cbs.onA();    break;
        case JOY_GESTURE_B:    Serial.println("[JOY] B");    if (cbs.onB)    cbs.onB();    break;
        case JOY_GESTURE_MENU: Serial.println("[JOY] MENU"); if (cbs.onMenu) cbs.onMenu(); break;
        default: break;
    }
}

// Antirrebote por flanco. El gesto se resuelve al soltar, clasificado por
// cuanto duro la pulsacion; asi no hay repeticion ni disparos en cascada.
void JoystickManager::updateButton() {
    bool reading = (digitalRead(config.swPin) == LOW);
    unsigned long now = millis();

    if (reading != swLastRead) {
        swDebounceMs = now;
        swLastRead   = reading;
    }

    if (now - swDebounceMs > SW_DEBOUNCE_MS && reading != swHeld) {
        swHeld = reading;
        if (swHeld) {
            swPressMs = now;
        } else {
            // fireGesture puede cambiar los callbacks (MENU sale de la
            // herramienta), asi que el estado se deja limpio antes.
            JoyGesture g = classifyHold(now - swPressMs);
            pendingGesture = JOY_GESTURE_NONE;
            fireGesture(g);
            return;
        }
    }

    pendingGesture = swHeld ? classifyHold(now - swPressMs) : JOY_GESTURE_NONE;
}

void JoystickManager::update() {
    // Antes que nada: mas abajo hay un return temprano cuando el stick esta
    // centrado, y el boton tiene que responder tambien en reposo.
    updateButton();

    rawX = sampleAxis(config.xPin);
    rawY = sampleAxis(config.yPin);

    if (!emaReady) {
        emaX = (float)rawX;
        emaY = (float)rawY;
        emaReady = true;
    } else {
        emaX += EMA_ALPHA * ((float)rawX - emaX);
        emaY += EMA_ALPHA * ((float)rawY - emaY);
    }

    normX = normalize((int)emaX, centerX, config.invertX);
    normY = normalize((int)emaY, centerY, config.invertY);

    // --- Estado sostenido, por eje e independiente (permite diagonales) ---
    // Cada direccion usa su propio umbral con histeresis.
    heldRight = (normX >  (heldRight ? DIR_RELEASE : DIR_PRESS));
    heldLeft  = (-normX > (heldLeft  ? DIR_RELEASE : DIR_PRESS));
    heldUp    = (normY >  (heldUp    ? DIR_RELEASE : DIR_PRESS));
    heldDown  = (-normY > (heldDown  ? DIR_RELEASE : DIR_PRESS));

    // --- Direccion dominante para los eventos de flanco ---
    // Un solo eje a la vez, para que navegar menus se sienta como la cruceta.
    JoyDirection dir = JOY_NONE;
    if (fabsf(normX) >= fabsf(normY)) {
        if      (heldRight) dir = JOY_RIGHT;
        else if (heldLeft)  dir = JOY_LEFT;
    } else {
        if      (heldUp)   dir = JOY_UP;
        else if (heldDown) dir = JOY_DOWN;
    }
    dominant = dir;

    unsigned long now = millis();

    if (dir == JOY_NONE) {
        lastFired = JOY_NONE;
        return;
    }

    if (dir != lastFired) {
        // Flanco: dispara ya y arranca la cuenta para la repeticion
        fire(dir);
        lastFired     = dir;
        repeatStartMs = now;
        repeatLastMs  = now;
    } else if (now - repeatStartMs >= REPEAT_DELAY_MS &&
               now - repeatLastMs  >= REPEAT_PERIOD_MS) {
        fire(dir);
        repeatLastMs = now;
    }
}
