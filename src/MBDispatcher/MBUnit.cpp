#include "MBUnit.hpp"

MBUnit::MBUnit(uint16_t (&mb_array)[MB_UNIT_BUFF_SIZE], uint16_t op320_index, type value_type)
{
    this->op320_index = op320_index;
    this->value_type = value_type;

    if (value_type == type::Uint16)
        this->array_unit_ptr = &mb_array[op320_index];
    else
    {
        this->array_unit_ptr = &mb_array[op320_index / 16];
        this->buff_bit = op320_index % 16;
    }
}

void MBUnit::addTrigger(CallbackFunction triggerFunc)
{
    this->triggerFunc = triggerFunc;
    this->is_trigger_func = true;
}

bool MBUnit::writeValue(uint16_t value)
{
    if (value_type == type::None)
        return false;

    if (value_type == type::Uint16)
        *array_unit_ptr = value;
    else
        bitWrite(*array_unit_ptr, buff_bit, value == 0 ? 0 : 1);
    
    return true;
}

bool MBUnit::writeValue(bool value)
{
    if (value_type == type::None)
        return false;

    if (value_type == type::Uint16)
        *array_unit_ptr = value;
    else
        bitWrite(*array_unit_ptr, buff_bit, value);

    return true;
}

uint16_t MBUnit::readValue()
{
    if (value_type == type::None)
        return 0;
    else if (value_type == type::Uint16)
        return *array_unit_ptr;
    else
        return bitRead(*array_unit_ptr, buff_bit);
}

void MBUnit::readValue(uint8_t* var) {
    var = readValue();
}

bool MBUnit::triggerFired()
{
    if (value_type == type::Coil && is_trigger_func && bitRead(*array_unit_ptr, buff_bit) == 1)
    {
        bitWrite(*array_unit_ptr, buff_bit, 0);
        triggerFunc();
        return true;
    }

    return false;
}