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

void TimeObj::clone(TimeObj &parent)
{
    this->seconds = parent.seconds;
    this->minutes = parent.minutes;
    this->hours = parent.hours;
    this->days = parent.days;
    this->months = parent.months;
    this->years = parent.years;
}

void TimeObj::addMinutes(uint8_t minutes)
{
    this->minutes += minutes % 60;
    this->hours += (minutes / 60) + (this->minutes / 60);
    this->minutes %= 60;

    this->days += hours / 24;
    this->hours %= 24;

    this->months += days / IN_MONTH + 1;
    this->days %= IN_MONTH + 1;
    
    this->years += months / 13;
    this->months %= 13;
}

bool TimeObj::isAnotherDay(TimeObj &reference) {
    return days != reference.days || months != reference.months || years != reference.years ? true : false;
}

bool TimeObj::isBiggerThan(TimeObj &reference, bool relative_today) {
    return getDiffMin(reference, relative_today) > 0 ? true : false;
}

bool TimeObj::isTimeEqual(TimeObj &reference) {
    return this->minutes == reference.minutes && this->hours == reference.hours ? true : false;
}

uint16_t TimeObj::getDiffMin(TimeObj &reference, bool relative_today)
{
    uint64_t THIS = hours * 60 + minutes +
                    relative_today ? 0 :
                    (years * IN_YEAR * 24 * 60) +
                    (months * IN_MONTH * 24 * 60) +
                    (days * 24 * 60);
                    

    uint64_t Ref = reference.hours * 60 + reference.minutes +
                   relative_today ? 0 :
                   (reference.years * 365 * 24 * 60) +
                   (reference.months * 31 * 24 * 60) +
                   (reference.days * 24 * 60);

    return THIS > Ref ? THIS - Ref : 0;
}

