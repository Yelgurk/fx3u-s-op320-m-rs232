#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

#ifndef _fxcore_hpp
#define _fxcore_hpp

class FXCore
{

};

#endif

/*
MBDispatcher mb_disp;
IODispatcher io_monitor;
EEDispatcher ee_disp;
TaskManager tm_fx3u;

STM32RTC& rtc = STM32RTC::getInstance();
*/

/*
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    tm_fx3u.newTask([]() -> void {
        Serial.print(rtc.getHours());
        Serial.print(":");
        Serial.print(rtc.getMinutes());
        Serial.print(":");
        Serial.println(rtc.getSeconds());
    }, 1000);
    */