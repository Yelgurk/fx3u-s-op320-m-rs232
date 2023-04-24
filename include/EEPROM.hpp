#include <Arduino.h>
#include "SlowSoftI2CMaster.h"

#ifndef _eeprom_hpp
#define _eeprom_hpp

#define EE_BYTE_SIZE 8192
#define EE_I2C_ARRD 0x50
#define EE_CYCLE_UINT32_ADDR 0x0003
#define EE_CYCLE_LIMIT 70000
#define EE_SECTOR_SIZE 0x0004

class EEPROM
{
public:
    SlowSoftI2CMaster *ssitcm;
    uint32_t ee_curr_cycle_cnt = 0;
    uint16_t ee_sector_start_addr = 0x0000;

    uint32_t getEESectorCycles(uint16_t ee_sector_depth);

public:
    bool init();
    uint8_t readEE(unsigned long ee_addr);
    bool readEE(unsigned long ee_addr, uint8_t *byte);
    bool writeEE(unsigned long ee_addr, uint8_t byte, bool traceable = false);
    void incEEWriteCycleCnt();
    void checkEECycle(bool boot_up);
    void changeEESector();
    bool resetEE(uint16_t ee_begin, uint16_t ee_end);

public:
    EEPROM();
};

#endif