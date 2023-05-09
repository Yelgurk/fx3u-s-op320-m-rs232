#include "../include/IODispatcher/IOUnit.hpp"

IOUnit::IOUnit(uint8_t stm_pin, mode pin_mode, bool is_reverse)
{
    this->stm_pin = stm_pin;
    this->pin_mode = pin_mode;
    this->is_reverse = is_reverse;
}

bool IOUnit::init()
{
    if (pin_mode == mode::None)
        return false;

    switch (pin_mode)
    {
        case mode::Relay: {
            pinMode(stm_pin, OUTPUT);
            digitalWrite(stm_pin, LOW);
        } break;
        default: pinMode(stm_pin, INPUT); break;
    }
    return true;
}

bool IOUnit::write(bool value)
{
    if (pin_mode != mode::Relay)
        return false;
    
    digitalWrite(stm_pin, value ? HIGH : LOW);
    return true;
}

bool IOUnit::readDigital()
{
    if (pin_mode == mode::DigitalIN || pin_mode == mode::Relay)
        return is_reverse ? !digitalRead(stm_pin) : digitalRead(stm_pin);

    return false;
}

uint16_t IOUnit::readAnalog()
{
    if (pin_mode == mode::AnalogIN)
        return analogRead(stm_pin);
    
    return 0;
}

bool IOUnit::isAnalog() {
    return this->pin_mode == mode::AnalogIN ? true : false;
}