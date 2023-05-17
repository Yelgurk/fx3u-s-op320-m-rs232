#include <Arduino.h>
#include "EEPROM.hpp"

#ifndef EEUnit_hpp
#define EEUnit_hpp

enum class ValueType : uint8_t { UINT8, UINT16 };

class EEUnit
{
private:
    uint16_t main_ee_addr = 0x0000;
    uint16_t *ee_sector_addr = nullptr;
    bool is_reloc_var = false,
         is_tracable = false;
    EEPROM *ee24c64 = nullptr;
    ValueType type = ValueType::UINT8;

    uint16_t getVarAddr();

public:
    EEUnit(uint16_t main_ee_addr, EEPROM *ee24c64, bool is_tracable, uint16_t *ee_sector_addr = nullptr, ValueType type = ValueType::UINT8);
    uint8_t readEE();
    bool readEE(uint8_t *byte);
    bool writeEE(uint8_t byte);
    uint16_t readUint16EE();
    bool writeUint16EE(uint16_t value);
    ValueType getType();
};

#endif