#include "../include/Core/TimeUnit.hpp"

TimeUnit::TimeUnit(bool into_ee_after_edit) {
    this->into_ee_after_edit = into_ee_after_edit;
}

void TimeUnit::setEEPointer(EEUnit* pointer, PointerType type)
{
    switch (type)
    {
        case Seconds: this->ee_time_ss_obj = pointer; break;
        case Minutes: this->ee_time_mm_obj = pointer; break;
        case Hours: this->ee_time_hh_obj = pointer; break;
        case Days: this->ee_date_DD_obj = pointer; break;
        case Months: this->ee_date_MM_obj = pointer; break;
        case Years: this->ee_date_YY_obj = pointer; break;
    }
}

void TimeUnit::setMBPointer(MBUnit* pointer, PointerType type)
{
    switch (type)
    {
        case Seconds: this->mb_time_ss_obj = pointer; break;
        case Minutes: this->mb_time_mm_obj = pointer; break;
        case Hours: this->mb_time_hh_obj = pointer; break;
        case Days: this->mb_date_DD_obj = pointer; break;
        case Months: this->mb_date_MM_obj = pointer; break;
        case Years: this->mb_date_YY_obj = pointer; break;
    }
}

void TimeUnit::clone(TimeObj parent)
{
    this->TimeObj::clone(parent);
    sendToMB();
    sendToEE(into_ee_after_edit);
}

void TimeUnit::setRealTime()
{
    this->seconds = rtc.getSeconds();
    this->minutes = rtc.getMinutes();
    this->hours = rtc.getHours();
    this->days = rtc.getDay();
    this->months = rtc.getMonth();
    this->years = rtc.getYear();

    sendToMB();
    sendToEE(into_ee_after_edit);
}

void TimeUnit::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    this->hours = hours;
    this->minutes = minutes;
    this->seconds = seconds;

    sendToMB();
    sendToEE(into_ee_after_edit);
}

void TimeUnit::setDate(uint8_t days, uint8_t months, uint8_t years)
{
    this->days = days;
    this->months = months;
    this->years = years;

    sendToMB();
    sendToEE(into_ee_after_edit);
}

void TimeUnit::setDateTime(uint8_t hours, uint8_t mionutes, uint8_t days, uint8_t months, uint8_t years)
{
    if (into_ee_after_edit)
    {
        into_ee_after_edit = false;
        this->setTime(hours, minutes);
        into_ee_after_edit = true;
    } 
    else
        this->setTime(hours, minutes);
    this->setDate(days, months, years);
}

void TimeUnit::setHour(uint8_t hours)
{
    this->setTime(hours, this->minutes, this->seconds);
}

void TimeUnit::setMinute(uint8_t minutes)
{
    this->setTime(this->hours, minutes, this->seconds);
}

void TimeUnit::setSecond(uint8_t seconds)
{
    this->setTime(this->hours, this->minutes, seconds);
}

void TimeUnit::setYear(uint8_t years)
{
    this->setDate(this->days, this->months, years);
}

void TimeUnit::setMonth(uint8_t months)
{
    this->setDate(this->days, months, this->years);
}

void TimeUnit::setDay(uint8_t days)
{
    this->setDate(days, this->months, this->years);
}

void TimeUnit::setZeroDateTime()
{
    this->seconds = 0;
    this->minutes = 0;
    this->hours = 0;
    this->days = 0;
    this->months = 0;
    this->years = 0;
    sendToMB();
}

void TimeUnit::addMinutes(uint8_t minutes)
{
    TimeObj::addMinutes(minutes);
    sendToMB();
    sendToEE(into_ee_after_edit);
}

void TimeUnit::sendToEE(bool call_flag)
{
    if (call_flag)
    {
        if (ee_time_ss_obj) ee_time_ss_obj->writeEE(this->seconds);
        if (ee_time_mm_obj) ee_time_mm_obj->writeEE(this->minutes);
        if (ee_time_hh_obj) ee_time_hh_obj->writeEE(this->hours);
        if (ee_date_DD_obj) ee_date_DD_obj->writeEE(this->days);
        if (ee_date_MM_obj) ee_date_MM_obj->writeEE(this->months);
        if (ee_date_YY_obj) ee_date_YY_obj->writeEE(this->years);
    }
}

void TimeUnit::sendToMB()
{
    if (mb_time_ss_obj) mb_time_ss_obj->writeValue((uint16_t)this->seconds);
    if (mb_time_mm_obj) mb_time_mm_obj->writeValue((uint16_t)this->minutes);
    if (mb_time_hh_obj) mb_time_hh_obj->writeValue((uint16_t)this->hours);
    if (mb_date_DD_obj) mb_date_DD_obj->writeValue((uint16_t)this->days);
    if (mb_date_MM_obj) mb_date_MM_obj->writeValue((uint16_t)this->months);
    if (mb_date_YY_obj) mb_date_YY_obj->writeValue((uint16_t)this->years);
}

void TimeUnit::loadFromEE()
{
    if(ee_time_ss_obj) ee_time_ss_obj->readEE(&this->seconds);
    if(ee_time_mm_obj) ee_time_mm_obj->readEE(&this->minutes);
    if(ee_time_hh_obj) ee_time_hh_obj->readEE(&this->hours);
    if(ee_date_DD_obj) ee_date_DD_obj->readEE(&this->days);
    if(ee_date_MM_obj) ee_date_MM_obj->readEE(&this->months);
    if(ee_date_YY_obj) ee_date_YY_obj->readEE(&this->years);
    sendToMB();
}

void TimeUnit::loadFromMB()
{
    if (mb_time_ss_obj) mb_time_ss_obj->readValue(&this->seconds);
    if (mb_time_mm_obj) mb_time_mm_obj->readValue(&this->minutes);
    if (mb_time_hh_obj) mb_time_hh_obj->readValue(&this->hours);
    if (mb_date_DD_obj) mb_date_DD_obj->readValue(&this->days);
    if (mb_date_MM_obj) mb_date_MM_obj->readValue(&this->months);
    if (mb_date_YY_obj) mb_date_YY_obj->readValue(&this->years);
}