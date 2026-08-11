#include <Arduino.h>
#include "GlobalConfig/GlobalConfig.h"

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
    buttons.update();
    joystick.update();
    itemLoopUpdate();
}
