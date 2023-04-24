#include <Arduino.h>
#include "EEUnit.hpp"
#include "EEPROM.hpp"

#ifndef _eedispatcher_hpp
#define _eedispatcher_hpp

class EEDispatcher
{
public:
    EEPROM ee24c64;

public:
    EEUnit  testVal1 = EEUnit(0x0000, &ee24c64, true),
            testVal2 = EEUnit(0x0001, &ee24c64, false, true, ee24c64.getSectorStartAddr()),
            testVal3 = EEUnit(0x0002, &ee24c64, false, true, ee24c64.getSectorStartAddr());

public:
    bool init();
};

#endif