#include <Arduino.h>
#include "EEUnit.hpp"
#include "EEPROM.hpp"

#ifndef _eedispatcher_hpp
#define _eedispatcher_hpp

#define BLOWGUN_PRESET_CNT 4

class EEDispatcher
{
private:
    EEPROM ee24c64;

public:
    EEUnit **ee_blowgun_preset_arr;

    EEUnit ee_blowgun_preset_1 = EEUnit(0x0000, &ee24c64, false),
           ee_blowgun_preset_2 = EEUnit(0x0001, &ee24c64, false),
           ee_blowgun_preset_3 = EEUnit(0x0002, &ee24c64, false),
           ee_blowgun_preset_4 = EEUnit(0x0003, &ee24c64, false);
           
            
    bool init();
};

#endif