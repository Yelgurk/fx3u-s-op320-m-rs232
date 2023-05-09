#include <Arduino.h>
#include "./Core/FXCore.hpp"

FXCore fx_core;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    analogReadResolution(12);

    //fx_core.FXCore::init();
}

void loop()
{
    //fx_core.dispatcher();
}