#include <Arduino.h>
#include "EEPROM.hpp"

#ifndef _eeunit_hpp
#define _eeunit_hpp

class EEUnit
{
private:
    uint16_t main_ee_addr = 0x0000;
    uint16_t *ee_sector_addr = nullptr;
    bool is_reloc_var = false,
         is_tracable = false;
    EEPROM *ee24c64 = nullptr;

    uint16_t getVarAddr();

public:
    EEUnit(uint16_t main_ee_addr, EEPROM *ee24c64, bool is_tracable, uint16_t *ee_sector_addr = nullptr);
    uint8_t readEE();
    bool readEE(uint8_t *byte);
    bool writeEE(uint8_t byte);
};

#endif