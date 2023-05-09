#include "TMUnit.hpp"

TMUnit::TMUnit(uint32_t &current_mill, uint32_t &max_span_mill, uint32_t time_span, CallbackFunction timerFunc, bool singleCall)
{
    this->current_mill = &current_mill;
    this->max_span_mill = &max_span_mill;
    this->time_span = controllable ? 0 : time_span;
    this->trigger_mill = this->time_span;
    this->timerFunc = timerFunc;
    this->controllable = singleCall;
    this->lock = controllable;
}

bool TMUnit::tryCall()
{
    if (controllable && time_span > 0 && !lock)
    {
        trigger_mill += *current_mill > ellapsed_mill ? (uint32_t)(*current_mill - ellapsed_mill) : (uint32_t)(*current_mill + max_span_mill - ellapsed_mill);
        ellapsed_mill = *current_mill;

        if (trigger_mill >= time_span)
        {
            timerFunc();
            this->stop();
        }
        return true;
    }
    else if (controllable && time_span <= 0 && !lock)
    {
        this->stop(); 
        return false;
    }
    else if (controllable)
        return false;

    if (!controllable && time_span == 0)
    {
        timerFunc();
        return true;
    }

    if (!controllable && *current_mill >= trigger_mill && !lock)
    {
        timerFunc();
        trigger_mill = *current_mill + time_span;
        //trigger_mill += time_span;

        if (trigger_mill > *max_span_mill)
        {
            trigger_mill -= *max_span_mill;
            this->lock = true;
        }

        return true;
    }

    return false;
}

void TMUnit::unlock()
{
    if (!controllable)
        this->lock = false;
}

void TMUnit::run(uint32_t seconds_span)
{
    if (controllable)
    {
        this->time_span = seconds_span > 0 ? seconds_span * 1000 : time_span;
        this->lock = false;
    }
}

void TMUnit::stop()
{
    if (controllable)
    {
        trigger_mill = 0;
        ellapsed_mill = 0;
        lock = true;
    }
}