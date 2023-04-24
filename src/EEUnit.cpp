#include "EEUnit.hpp"

EEUnit::EEUnit() { }

EEUnit::EEUnit(uint16_t main_ee_addr, EEPROM *ee24c64, bool is_tracable, bool is_reloc_var, uint16_t *ee_sector_addr)
{
    this->main_ee_addr = main_ee_addr;
    this->ee24c64 = ee24c64;
    this->is_tracable = is_tracable;
    this->is_reloc_var = is_reloc_var;

    if (is_reloc_var && ee_sector_addr != nullptr)
        this->ee_sector_addr = ee_sector_addr;
    else
        this->is_reloc_var = false;
}

uint8_t EEUnit::readEE()
{
    if (ee24c64 != nullptr)
        return ee24c64->readEE(getVarAddr());
    else
        return 0;
}

bool EEUnit::readEE(uint8_t *byte)
{
    if (ee24c64 != nullptr)
        return ee24c64->readEE(getVarAddr(), byte);
    else
        return false;
}

bool EEUnit::writeEE(uint8_t byte)
{
    if (ee24c64 != nullptr)
        return ee24c64->writeEE(getVarAddr(), byte, this->is_tracable);
    else
        return false;
}

uint16_t EEUnit::getVarAddr()
{
    if (!this->is_reloc_var)
        return main_ee_addr;
    else
        return *ee_sector_addr + main_ee_addr - EE_SECTOR_START_ADDR; 
}