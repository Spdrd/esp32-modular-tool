#include <Arduino.h>
#include "GlobalConfig/GlobalConfig.h"

void setup() {
    Serial.begin(115200);

    randomSeed(micros());

    screen.begin();
    delay(500);

    buttons.begin();
    buttons.setCallbacks(getMenuCallbacks());

    renderMenu();
}

void loop() {
    buttons.update();
    itemLoopUpdate();
}
