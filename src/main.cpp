#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"

MBDispatcher mb_disp;
IODispatcher io_monitor;
EEDispatcher ee_disp;
TaskManager tm_fx3u;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);
}

void loop()
{
    tm_fx3u.dispatcher();
}