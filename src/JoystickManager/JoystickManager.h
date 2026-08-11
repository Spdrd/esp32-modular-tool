#pragma once
#include <Arduino.h>
#include "../ButtonManager/ButtonManager.h"

struct JoystickPinConfig {
    int  xPin;
    int  yPin;
    bool invertX;
    bool invertY;
};

enum JoyDirection : uint8_t {
    JOY_NONE = 0,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT
};

// Joystick analogico de dos potenciometros (0-3.3V por eje).
//
// Expone dos interfaces independientes sobre la misma lectura:
//
//   1. DISCRETA - se comporta como la cruceta: eventos de flanco con
//      repeticion al mantener, y estado sostenido por eje (isUpDown()...).
//      Con mirror() dispara los mismos callbacks que ya usa ButtonManager,
//      asi que las herramientas existentes funcionan sin tocarlas.
//
//   2. ANALOGICA - posicion continua del stick: getX()/getY() en -1..1,
//      magnitud y angulo. Para lo que necesite proporcionalidad real.
//
// IMPORTANTE: usar pines de ADC1 (GPIO 32-39). El ADC2 queda inutilizable
// mientras el WiFi esta encendido, y este proyecto usa ESP-NOW.
class JoystickManager {
public:
    JoystickManager(JoystickPinConfig config);

    void begin();    // configura los pines y calibra el centro en reposo
    void update();    // llamar en cada loop

    // Recalibra el centro. El stick debe estar suelto al llamarla.
    void calibrateCenter(int samples = 64);

    // --- Interfaz analogica ---
    int   getRawX() const { return rawX; }          // 0-4095
    int   getRawY() const { return rawY; }
    float getX()    const { return normX; }         // -1.0 (izq) .. 1.0 (der)
    float getY()    const { return normY; }         // -1.0 (abajo) .. 1.0 (arriba)
    float getMagnitude() const;                     // 0.0 .. ~1.0
    float getAngleDeg()  const;                     // 0-360, 0 = derecha
    bool  isCentered()   const { return normX == 0.0f && normY == 0.0f; }

    int getCenterX() const { return centerX; }
    int getCenterY() const { return centerY; }

    // --- Interfaz discreta ---
    // Por eje e independientes entre si: en diagonal, dos quedan activos.
    bool isUpDown()    const { return heldUp; }
    bool isDownDown()  const { return heldDown; }
    bool isLeftDown()  const { return heldLeft; }
    bool isRightDown() const { return heldRight; }

    // Direccion dominante (un solo eje). JOY_NONE si esta centrado.
    JoyDirection getDirection() const { return dominant; }

    // --- Callbacks ---
    // Espeja los de ButtonManager: el stick dispara las mismas acciones de
    // direccion que la cruceta sin duplicar la configuracion de cada tool.
    void mirror(ButtonManager* buttons) { mirrored = buttons; }
    void setCallbacks(ButtonActionCallbacks callbacks);

private:
    JoystickPinConfig config;

    int   rawX, rawY;
    int   centerX, centerY;
    float normX, normY;
    float emaX, emaY;
    bool  emaReady;

    bool heldUp, heldDown, heldLeft, heldRight;
    JoyDirection dominant;
    JoyDirection lastFired;

    unsigned long repeatStartMs;
    unsigned long repeatLastMs;

    ButtonManager* mirrored;
    ButtonActionCallbacks own;

    int   sampleAxis(int pin) const;
    float normalize(int raw, int center, bool invert) const;
    void  fire(JoyDirection dir);
};
