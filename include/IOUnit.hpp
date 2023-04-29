#include <Arduino.h>

#ifndef _iounit_hpp
#define _iounit_hpp

enum class mode { None, Relay, DigitalIN, AnalogIN };

class IOUnit
{
private:
    uint8_t stm_pin = 0;
    mode pin_mode = mode::None;
    bool is_reverse = true;

public:
    IOUnit(uint8_t stm_pin, mode pin_mode, bool is_reverse = true);
    bool init();
    bool write(bool value);
    bool readDigital();
    uint16_t readAnalog();
    bool isAnalog();
};

#endif