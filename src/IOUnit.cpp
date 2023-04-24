#include "IOUnit.hpp"

IOUnit::IOUnit(uint8_t stm_pin, mode pin_mode)
{
    this->stm_pin = stm_pin;
    this->pin_mode = pin_mode;
}

bool IOUnit::init()
{
    Serial.print(stm_pin);
    Serial.print(", ");
    Serial.println((uint8_t)pin_mode);

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

uint32_t IOUnit::read()
{
    if (pin_mode == mode::AnalogIN)
        return analogRead(stm_pin);
    else
        return digitalRead(stm_pin) == HIGH ? 1 : 0;
}

bool IOUnit::isAnalog() {
    return this->pin_mode == mode::AnalogIN ? true : false;
}