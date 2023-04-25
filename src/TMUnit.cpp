#include "TMUnit.hpp"

TMUnit::TMUnit(uint32_t &current_mill, uint32_t &max_span_mill, uint32_t time_span, CallbackFunction timerFunc)
{
    this->current_mill = &current_mill;
    this->max_span_mill = &max_span_mill;
    this->trigger_mill = time_span;
    this->time_span = time_span;
    this->timerFunc = timerFunc;
}

bool TMUnit::tryCall()
{
    if (*current_mill >= trigger_mill && !lock)
    {
        timerFunc();
        trigger_mill = trigger_mill + time_span;

        if (trigger_mill > *max_span_mill)
        {
            trigger_mill -= *max_span_mill;
            this->lock = true;
        }

        return true;
    }

    return false;
}

void TMUnit::unlock() {
    this->lock = false;
}