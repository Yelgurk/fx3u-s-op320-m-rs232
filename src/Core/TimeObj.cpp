#include "../Include/Core/TimeObj.hpp"

uint8_t TimeObj::getSecs() {
    return this->seconds;
}

uint8_t TimeObj::getMins() {
    return this->minutes;
}

uint8_t TimeObj::getHours() {
    return this->hours;
}

uint8_t TimeObj::getDays() {
    return this->days;
}

uint8_t TimeObj::getMonths() {
    return this->months;
}

uint8_t TimeObj::getYears() {
    return this->years;
}

void TimeObj::clone(TimeObj *parent, CloneType type)
{
    if (type == CloneType::All || type == CloneType::Time)
    {
        this->seconds = parent->seconds;
        this->minutes = parent->minutes;
        this->hours = parent->hours;
    }

    if (type == CloneType::All || type == CloneType::Date)
    {
        this->days = parent->days;
        this->months = parent->months;
        this->years = parent->years;
    }
}

void TimeObj::addMinutes(uint8_t minutes)
{
    this->minutes += minutes % 60;
    this->hours += (minutes / 60) + (this->minutes / 60);
    this->minutes %= 60;

    this->days += hours / 24;
    this->hours %= 24;

    this->months += days / (IN_MONTH + 1);
    this->days %= (IN_MONTH + 1);
    this->days = this->days == 0 ? 1 : this->days;
    
    this->years += months / 13;
    this->months %= 13;
    this->months = this->months == 0 ? 1 : this->months;
}

bool TimeObj::isAnotherDay(TimeObj *reference) {
    return  this->days != reference->days ||
            this->months != reference->months ||
            this->years != reference->years ? true : false;
}

bool TimeObj::isBiggerThan(TimeObj *reference, bool relative_today) {
    return getDiffSec(reference, relative_today) > 0 ? true : false;
}

bool TimeObj::isTimeEqual(TimeObj *reference) {
    return this->minutes == reference->minutes && this->hours == reference->hours && this->seconds == reference->seconds ? true : false;
}

uint64_t TimeObj::getDiffSec(TimeObj *reference, bool relative_today)
{
    uint64_t current =
                ((uint64_t)this->hours * 60 + (uint64_t)this->minutes +
                ((uint64_t)relative_today ? 0 :
                ((uint64_t)this->years * (uint64_t)IN_YEAR * 24 * 60) +
                ((uint64_t)this->months * (uint64_t)IN_MONTH * 24 * 60) +
                ((uint64_t)this->days * 24 * 60))) * 60 +
                (uint64_t)this->seconds;
                    

    uint64_t refDT =
                ((uint64_t)reference->hours * 60 + (uint64_t)reference->minutes +
                ((uint64_t)relative_today ? 0 :
                ((uint64_t)reference->years * (uint64_t)IN_YEAR * 24 * 60) +
                ((uint64_t)reference->months * (uint64_t)IN_MONTH * 24 * 60) +
                ((uint64_t)reference->days * 24 * 60))) * 60 +
                (uint64_t)reference->seconds;

    return current > refDT ? current - refDT : 0;
}