#include <Arduino.h>
#include "TMUnit.hpp"

#ifndef TaskManager_hpp
#define TaskManager_hpp

//template <typename interruptFunc>

class TaskManager
{
private:
    uint32_t current_mill = 0;
    uint32_t max_span_mill = 0;
    uint8_t tasks_cnt = 0;
    TMUnit *tasks_arr = new TMUnit[0];

public:
    TMUnit* newTask(uint32_t time_span_mill, CallbackFunction timerFuncbool, bool singleCall = false);
    void dispatcher();
};

#endif