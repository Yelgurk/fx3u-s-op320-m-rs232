#include <Arduino.h>
#include <STM32RTC.h>

#ifndef TimeObj_hpp
#define TimeObj_hpp

class TimeObj
{
protected:
    STM32RTC& rtc = STM32RTC::getInstance();
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t days = 0;
    uint8_t months = 0;
    uint8_t years = 0;

public:
    uint8_t getSecs();
    uint8_t getMins();
    uint8_t getHours();
    uint8_t getDays();
    uint8_t getMonths();
    uint8_t getYears();
    void clone(TimeObj parent);
    bool isBiggerThan(TimeObj &reference, bool relative_today = false);
    uint16_t getDiffMin(TimeObj &reference, bool relative_today = false);
};

#endif