#include <Arduino.h>
#include <STM32RTC.h>

#ifndef _rtcobject_hpp
#define _rtcobject_hpp

struct RTCObject
{
private:
    STM32RTC& rtc = STM32RTC::getInstance();
    bool isRelative()
    {
        if (day == 0 && month == 0 && year == 0)
            return true;
        else
            return false;
    }

public:
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    bool successComparedToday = false;

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

    bool inRange(uint8_t compMinutes, RTCObject &compare, bool is_sec_not_min = false)
    {
        if (!isRelative() && (compare.day != day || compare.month != month || compare.year != year))
            return false;
        
        uint32_t currSec = ((hour * 60) + minute) * 60 + second;
        uint32_t compSec = ((compare.hour * 60) + compare.minute) * 60 + compare.second;
        
        if (currSec > compSec)
            return false;
        else
        {
            uint32_t duration = is_sec_not_min ? (compSec - currSec) : (compSec - currSec) / 60;
            if (duration < compMinutes)
                return true;
            else
                return false;
        }
    }

    bool inRangeOnce(uint8_t compMinutes, RTCObject &compare)
    {
        if (!successComparedToday)
        {
            bool response = inRange(compMinutes, compare);
            if (response)
                successComparedToday = true;
            return response;
        }
        else
            return false;
    }

    bool outRange(uint8_t compMinutes, RTCObject &compare, bool is_sec_not_min = false) {
        return !inRange(compMinutes, compare, is_sec_not_min);
    }

    bool outRangeOnce(uint8_t compMinutes, RTCObject &compare)
    {
        if (!successComparedToday)
        {
            bool response = outRange(compMinutes, compare);
            if (response)
                successComparedToday = true;
            return response;
        }
    }
};

#endif