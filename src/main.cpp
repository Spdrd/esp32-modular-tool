#include <Arduino.h>
#include "GlobalConfig/GlobalConfig.h"

// Combo boton del stick + A sostenido 2s para recuperar la pantalla si se
// desconecto y reconecto. Reinicia solo el controlador del display y repinta
// el estado actual; no reinicia el ESP32 ni cambia de menu.
static bool          s_reinitArmed = false;
static unsigned long s_reinitStart = 0;
static const unsigned long SCREEN_REINIT_HOLD_MS = 2000;

static void checkScreenReinit() {
    // Lectura cruda del estado sostenido: no pasa por los callbacks de flanco,
    // asi que no interfiere con lo que el boton del stick o A hagan en cada
    // pantalla.
    if (buttons.isOkDown() && buttons.isADown()) {
        if (!s_reinitArmed) {
            s_reinitArmed = true;
            s_reinitStart = millis();
        } else if (millis() - s_reinitStart >= SCREEN_REINIT_HOLD_MS) {
            Serial.println("[MAIN] combo stick+A: reiniciando pantalla...");
            screen.reinit(TFT_RST);
            redrawCurrentState();
            s_reinitArmed = false;   // no repetir hasta soltar y volver a pulsar
        }
    } else {
        s_reinitArmed = false;
    }
}

void setup() {
    Serial.begin(115200);

    randomSeed(micros());

    screen.begin();
    delay(500);

    speaker.begin();
    linterna.begin();
    buttons.begin();
    buttons.setCallbacks(getMenuCallbacks());

    // El stick reproduce los callbacks de direccion que cada herramienta ya
    // registra en buttons, asi que no hay que configurarlo en cada una.
    joystick.begin();
    joystick.mirror(&buttons);

    renderMenu();
}

void loop() {
    checkScreenReinit();
    buttons.update();
    joystick.update();
    itemLoopUpdate();
}
