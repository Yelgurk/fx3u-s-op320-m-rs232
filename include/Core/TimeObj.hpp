#include <Arduino.h>
#include <STM32RTC.h>

#ifndef TimeObj_hpp
#define TimeObj_hpp

#define IN_YEAR 372 // calculation crutch
#define IN_MONTH 31 // calculation crutch

enum CloneType { All, Date, Time };

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
    void clone(TimeObj *parent, CloneType type = CloneType::All);
    void addMinutes(uint8_t minutes);
    bool isAnotherDay(TimeObj *reference);
    bool isBiggerThan(TimeObj *reference, bool relative_today = false);
    bool isTimeEqual(TimeObj *reference);
    uint32_t getDiffMin(TimeObj *reference, bool relative_today = false);
    uint32_t getTotalMin(bool relative_today = false);
};

#endif