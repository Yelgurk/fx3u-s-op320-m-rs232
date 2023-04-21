#include <Arduino.h>
#include "IODispatcher.hpp"

IODispatcher io_monitor;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);
}

void loop()
{
}