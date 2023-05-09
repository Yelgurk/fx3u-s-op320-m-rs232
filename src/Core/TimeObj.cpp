#include "../Include/Core/TimeObj.hpp"

void TimeObj::clone(TimeObj parent)
{
    this->seconds = parent.seconds;
    this->minutes = parent.minutes;
    this->hours = parent.hours;
    this->days = parent.days;
    this->months = parent.months;
    this->years = parent.years;
}

bool TimeObj::isBiggerThan(TimeObj reference) {
    return getDiffMin(reference) > 0 ? true : false;
}

uint16_t TimeObj::getDiffMin(TimeObj reference)
{
    uint64_t THIS = (years * 365 * 24 * 60) +
                    (months * 31 * 24 * 60) +
                    (days * 24 * 60) +
                    hours * 60 + minutes;

    uint64_t Ref = (reference.years * 365 * 24 * 60) +
                   (reference.months * 31 * 24 * 60) +
                   (reference.days * 24 * 60) +
                   reference.hours * 60 + reference.minutes;

    return THIS > Ref ? THIS - Ref : 0;
}

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
