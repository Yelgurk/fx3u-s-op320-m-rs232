#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"

MBDispatcher mb_disp;
IODispatcher io_monitor;
EEDispatcher ee_disp;
TaskManager tm_fx3u;

uint8_t testVal1 = 0;
uint8_t testVal2 = 0;
uint8_t testVal3 = 0;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    tm_fx3u.newTask([]() -> void { Serial.print("task 1 = "); Serial.println(++testVal1); }, 1000);
    tm_fx3u.newTask([]() -> void { Serial.print("task 2 = "); Serial.println(++testVal2); }, 700);
    tm_fx3u.newTask([]() -> void { Serial.print("task 3 = "); Serial.println(++testVal3); }, 300);
}

void loop()
{
    tm_fx3u.dispatcher();

    if (testVal1 > 0 && testVal1 % 20 == 0)
        delay(10000);
}