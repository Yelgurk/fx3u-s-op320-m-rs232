#include "../include/EEDispatcher/EEUnit.hpp"

EEUnit::EEUnit(uint16_t main_ee_addr, EEPROM *ee24c64, bool is_tracable, uint16_t *ee_sector_addr)
{
    this->main_ee_addr = main_ee_addr;
    this->ee24c64 = ee24c64;
    this->is_tracable = is_tracable;
    if (ee_sector_addr != nullptr)
    {
        this->ee_sector_addr = ee_sector_addr;
        this->is_reloc_var = true;
    }
}

uint8_t EEUnit::readEE()
{
    if (ee24c64 == nullptr)
        return 0;
    
    return ee24c64->readEE(getVarAddr());
}

bool EEUnit::readEE(uint8_t *byte)
{
    if (ee24c64 == nullptr)
        return false;
    
    return ee24c64->readEE(getVarAddr(), byte);
}

bool EEUnit::writeEE(uint8_t byte)
{
    if (ee24c64 == nullptr)
        return false;

    return ee24c64->writeEE(getVarAddr(), byte, this->is_tracable);
}

uint16_t EEUnit::getVarAddr() {
    return this->is_reloc_var ? *ee_sector_addr + main_ee_addr - EE_SECTOR_START_ADDR : main_ee_addr;
}