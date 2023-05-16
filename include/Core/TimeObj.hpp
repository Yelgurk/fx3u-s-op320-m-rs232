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
    uint64_t hours = 0;
    uint64_t minutes = 0;
    uint64_t seconds = 0;
    uint64_t days = 0;
    uint64_t months = 0;
    uint64_t years = 0;

public:
    uint64_t getSecs();
    uint64_t getMins();
    uint64_t getHours();
    uint64_t getDays();
    uint64_t getMonths();
    uint64_t getYears();
    void clone(TimeObj *parent, CloneType type = CloneType::All);
    void addMinutes(uint64_t minutes);
    bool isAnotherDay(TimeObj *reference);
    bool isBiggerThan(TimeObj *reference, bool relative_today = false);
    bool isTimeEqual(TimeObj *reference);
    uint64_t getDiffSec(TimeObj *reference, bool relative_today = false);
};

#endif