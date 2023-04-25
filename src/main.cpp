#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

MBDispatcher mb_disp;
IODispatcher io_monitor;
EEDispatcher ee_disp;
TaskManager tm_fx3u;

STM32RTC& rtc = STM32RTC::getInstance();

/* Declare it volatile since it's incremented inside an interrupt */
volatile int alarmMatch_counter = 0;

/* Change this value to set alarm match offset in millisecond */
/* Note that only mcu with RTC_SSR_SS defined managed subsecond else only second */
static uint32_t atime = 1000;

/* Change these values to set the current initial time */
const byte seconds = 30;
const byte minutes = 48;
const byte hours = 23;

/* Change these values to set the current initial date */
const byte day = 25;
const byte month = 4;
const byte year = 23;

#define USE_SET_RTC 0

void alarmMatch(void *data);

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    #if USE_SET_RTC == 1
    if (!rtc.isTimeSet())
    {
        rtc.setTime(hours, minutes, seconds);
        rtc.setDate(day, month, year);

        rtc.attachInterrupt(alarmMatch, &atime);
        rtc.setAlarmDay(day);
        rtc.setAlarmTime(23, 55, 00);
        rtc.enableAlarm(rtc.MATCH_DHHMMSS);
    }
    #endif

    tm_fx3u.newTask([]() -> void {
        Serial.print(rtc.getHours());
        Serial.print(":");
        Serial.print(rtc.getMinutes());
        Serial.print(":");
        Serial.println(rtc.getSeconds());
    }, 1000);
}

void loop()
{
    tm_fx3u.dispatcher();
}

void alarmMatch(void *data)
{
    uint32_t epoc;
    uint32_t epoc_ms;
    uint32_t sec = 0;
    uint32_t _millis = 1000;

    if (data != NULL) {
        _millis = *(uint32_t*)data;
    }

    sec = _millis / 1000;

    if (sec == 0) {
        sec = 1;
    }
    epoc = rtc.getEpoch(&epoc_ms);

    Serial.printf("Alarm Match %i\n", ++alarmMatch_counter);
    rtc.setAlarmEpoch(epoc + sec, STM32RTC::MATCH_SS, epoc_ms);
}