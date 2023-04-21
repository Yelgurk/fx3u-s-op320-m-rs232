#include "IOUnit.hpp"

IOUnit::IOUnit(uint8_t stm_pin, mode pin_mode)
{
    this->stm_pin = stm_pin;
    this->pin_mode = pin_mode;
}

void IOUnit::init()
{
    if (pin_mode != mode::None)
    switch (pin_mode)
    {
        case mode::Relay: pinMode(stm_pin, OUTPUT); break;
        default: pinMode(stm_pin, INPUT); break;
    }
}

bool IOUnit::write(bool value)
{
    if (pin_mode == mode::Relay)
        digitalWrite(stm_pin, value ? HIGH : LOW);
}

uint8_t IOUnit::read()
{
    if (pin_mode == mode::AnalogIN)
        return analogRead(stm_pin);
    else
        return digitalRead(stm_pin);
}