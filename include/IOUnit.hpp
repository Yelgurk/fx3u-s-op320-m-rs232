#include <Arduino.h>

#ifndef _iounit_hpp
#define _iounit_hpp

enum class mode { None, Relay, DigitalIN, AnalogIN };

class IOUnit
{
private:
    uint8_t stm_pin = 0;
    mode pin_mode = mode::None;

public:
    IOUnit(uint8_t stm_pin, mode pin_mode);
    void init();
    bool write(bool value);
    uint8_t read();
};

#endif