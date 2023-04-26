#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

#ifndef _fxcore_hpp
#define _fxcore_hpp

class FXCore : protected MBDispatcher,
               protected IODispatcher,
               protected EEDispatcher,
               public TaskManager
{
private:

public:
    void init();

    void testF() {
        mb_val1.writeValue((uint16_t)(mb_val1.readValue() + 1));
    }

    void testR()
    {
        io_mixer_r.write(mb_coil2.readValue());
        io_heater_r.write(mb_coil3.readValue());
        mb_val2.writeValue((uint16_t)(mb_val2.readValue() + 1));
    }
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

/*
    tm_fx3u.newTask([]() -> void { mb_disp.poll(); }, 0);
    tm_fx3u.newTask([]() -> void { mb_disp.mb_coil1.writeValue(!mb_disp.mb_coil1.readValue()); }, 2000);
    tm_fx3u.newTask([]() -> void { testF(); }, 1000);
    tm_fx3u.newTask([]() -> void { testR(); }, 50);
    */