#include <Arduino.h>
#include "MBDispatcher.hpp"
MBDispatcher mb_disp;

unsigned long relayDelay = 0;
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
    
    relayDelay = millis() + 100;
    digitalWrite(PC9, HIGH);
}

uint32_t curr = 0;
uint32_t old = 0;
bool display = false;

void poll_test();

void loop()
{
    curr = millis();
    if (curr - old > 2000)
    {
        old = millis();
        display = !display;
    }

    delay(2);
    ++value;
    value2 += 2;

    mb_disp.poll();
    poll_test();
} 

void poll_test()
{
    mb_disp.MB_coil1.writeValue(display);

    digitalWrite(PC8, bitRead(mb_disp.mb_au16data[1], 1));
    digitalWrite(PA8, bitRead(mb_disp.mb_au16data[1], 2));

    mb_disp.MB_val1.writeValue(value);
    mb_disp.MB_val2.writeValue(value2);
}