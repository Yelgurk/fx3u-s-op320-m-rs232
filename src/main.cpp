#include <Arduino.h>
#include "FXCore.hpp"

FXCore fx_core;

void testF();
void testR();

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    fx_core.FXCore::init();
    fx_core.newTask([]() -> void { fx_core.testR(); }, 50);
    fx_core.newTask([]() -> void { fx_core.testF(); }, 1000);
}

void loop()
{
    fx_core.dispatcher();
}