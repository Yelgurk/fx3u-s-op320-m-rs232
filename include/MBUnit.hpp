#include "Arduino.h"

#ifndef _mbunit_hpp
#define _mbunit_hpp

enum class type { None, Coil, Uint16 };

class MBUnit
{
private:
    //void (*interruptFunc)(int ) = NULL;
    uint16_t *array_unit_ptr = nullptr;
    uint8_t op320_index;
    uint8_t buff_bit = -1;
    type value_type = type::None;

public:
    MBUnit(uint16_t (&mb_array)[9], uint8_t op320_index, type value_type);
    bool writeValue(uint16_t value);
    bool writeValue(bool value);
    uint16_t readValue();
};

#endif