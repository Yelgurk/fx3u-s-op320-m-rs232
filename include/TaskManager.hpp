#include <Arduino.h>
#include "TMUnit.hpp"

#ifndef _taskmanager_hpp
#define _taskmanager_hpp

//template <typename interruptFunc>

class TaskManager
{
private:
    uint32_t current_mill = 0;
    uint32_t max_span_mill = 0;
    uint8_t tasks_cnt = 0;
    TMUnit *tasks_arr = new TMUnit[0];

public:
    void newTask(CallbackFunction timerFunc, uint32_t time_span);
    void dispatcher();
};

#endif