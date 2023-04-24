#include <Arduino.h>
#include "MBDispatcher.hpp"
MBDispatcher mb_disp;

uint16_t value = 0;
uint16_t value2 = 0;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    pinMode(PC9, OUTPUT);
    pinMode(PC8, OUTPUT);
    pinMode(PA8, OUTPUT);

    mb_disp.init();
}

void loop()
{

    mb_disp.poll();
    //RW data after MB poll
} 