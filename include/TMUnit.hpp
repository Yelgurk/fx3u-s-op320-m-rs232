#include <Arduino.h>

#ifndef _tmunit_hpp
#define _tmunit_hpp

typedef std::function<void()> CallbackFunction;

class TMUnit
{
private:
    CallbackFunction timerFunc;
    uint32_t *current_mill;
    uint32_t *max_span_mill;
    uint32_t trigger_mill = 0;
    uint32_t time_span = 0;
    bool lock = false;

public:
    TMUnit() {}
    TMUnit(uint32_t &current_mill, uint32_t &max_span_mill, uint32_t time_span, CallbackFunction timerFunc);
    bool tryCall();
    void unlock();
};

#endif




