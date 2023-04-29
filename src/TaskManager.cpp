#include "TaskManager.hpp"

void TaskManager::newTask(uint32_t time_span_mill, CallbackFunction timerFunc)
{
    TMUnit *resize = new TMUnit[++tasks_cnt];
    for(uint8_t index = 0; index < tasks_cnt - 1; index++)
        resize[index] = tasks_arr[index];

    resize[tasks_cnt - 1] = TMUnit(current_mill, max_span_mill, time_span_mill, timerFunc);
    max_span_mill = time_span_mill > max_span_mill ? time_span_mill : max_span_mill; 

    delete[] tasks_arr;
    tasks_arr = resize;
}

void TaskManager::dispatcher()
{
    current_mill = millis();

    for (uint8_t index = 0; index < tasks_cnt; index++)
        tasks_arr[index].tryCall();
    
    if (current_mill >= max_span_mill)
    {
        for (uint8_t index = 0; index < tasks_cnt; index++)
            tasks_arr[index].unlock();

        uwTick -= current_mill;
    }
}