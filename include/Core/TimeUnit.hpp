#include "TimeObj.hpp"
#include "../include/EEDispatcher/EEUnit.hpp"
#include "../include/MBDispatcher/MBUnit.hpp"

#ifndef TimeUnit_hpp
#define TimeUnit_hpp

enum PointerType { Seconds, Minutes, Hours, Days, Months, Years };

class TimeUnit : public TimeObj
{
private:
    bool into_ee_after_edit = false;
    EEUnit *ee_time_ss_obj = NULL;
    EEUnit *ee_time_mm_obj = NULL;
    EEUnit *ee_time_hh_obj = NULL;
    EEUnit *ee_date_DD_obj = NULL;
    EEUnit *ee_date_MM_obj = NULL;
    EEUnit *ee_date_YY_obj = NULL;

    MBUnit *mb_time_ss_obj = NULL;
    MBUnit *mb_time_mm_obj = NULL;
    MBUnit *mb_time_hh_obj = NULL;
    MBUnit *mb_date_DD_obj = NULL;
    MBUnit *mb_date_MM_obj = NULL;
    MBUnit *mb_date_YY_obj = NULL;

public:
    TimeUnit() { }
    TimeUnit(bool into_ee_after_edit);
    void setEEPointer(EEUnit* pointer, PointerType type);
    void setMBPointer(MBUnit* pointer, PointerType type);
    void clone(TimeObj *parent, CloneType type = CloneType::All);
    void setRealTime();
    void setDateTime(uint8_t hours, uint8_t mionutes, uint8_t days, uint8_t months, uint8_t years);
    void setDate(uint8_t days, uint8_t months, uint8_t years);
    void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds = 0);
    void setHour(uint8_t hours);
    void setMinute(uint8_t minutes);
    void setSecond(uint8_t seconds);
    void setYear(uint8_t years);
    void setMonth(uint8_t months);
    void setDay(uint8_t days);
    void setZeroDateTime();
    bool isZeroTime();
    void addMinutes(uint8_t minutes);
    void sendToEE(bool static_flag = true);
    void sendToMB();
    void loadFromEE();
    void loadFromMB();
};

#endif