#include "Arduino.h"

#ifndef _mbunit_hpp
#define _mbunit_hpp

#define MB_UNIT_BUFF_MAIN 52
#define MB_UNIT_OP_COMM 3
#define MB_UNIT_BUFF_SIZE (MB_UNIT_BUFF_MAIN + MB_UNIT_OP_COMM)

typedef std::function<void()> CallbackFunction;

enum class type { None, Coil, Uint16 };

class MBUnit
{
private:
    uint16_t *array_unit_ptr = nullptr;
    uint16_t op320_index;
    uint8_t buff_bit = -1;
    type value_type = type::None;

    CallbackFunction triggerFunc;
    bool is_trigger_func = false; 

public:
    MBUnit(uint16_t (&mb_array)[MB_UNIT_BUFF_SIZE], uint16_t op320_index, type value_type);
    void addTrigger(CallbackFunction triggerFunc);
    bool writeValue(uint16_t value);
    bool writeValue(bool value);
    uint16_t readValue();
    bool triggerFired();
};

#endif