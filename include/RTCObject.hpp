#include <Arduino.h>
#include <STM32RTC.h>

#ifndef _rtcobject_hpp
#define _rtcobject_hpp

struct RTCObject
{
private:
    STM32RTC& rtc = STM32RTC::getInstance();

public:
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t year;
    uint8_t month;
    uint8_t day;

    RTCObject() {
        setInstTime();
    }

    RTCObject(uint8_t hour, uint8_t minute, uint8_t second, uint8_t year, uint8_t month, uint8_t day) {
        setTime(hour, minute, second, year, month, day);
    }

    void setInstTime() {
        setTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getYear(), rtc.getMonth(), rtc.getDay());
    }

    void setTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t year, uint8_t month, uint8_t day)
    {
        this->hour = hour;
        this->minute = minute;
        this->second = second;
        this->year = year;
        this->month = month;
        this->day = day;
    }

    bool inRange(uint8_t compMinutes, RTCObject &compare)
    {
        if (compare.day != day || compare.month != month || compare.year != year)
            return false;
        
        uint32_t currSec = ((hour * 60) + minute) * 60 + second;
        uint32_t compSec = ((compare.hour * 60) + compare.minute) * 60 + compare.second;
        uint32_t duration = (currSec > compSec ? currSec - compSec : compSec - currSec) / 60;

        if (duration < compMinutes)
            return true;
        else
            return false;
    }
};

#endif