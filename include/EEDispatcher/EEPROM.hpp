#include <Arduino.h>
#include "SlowSoftI2CMaster.h"

#ifndef EEPROM_hpp
#define EEPROM_hpp

#define EE_I2C_ARRD 0x50
#define EE_FULL_BSIZE 8192
#define EE_SECTOR_START_ADDR 0x004D
#define EE_SECTOR_SIZE 3
#define EE_COUNTER_SIZE 4
#define EE_CYCLE_LIMIT 120000

class EEPROM
{
private:
    SlowSoftI2CMaster *ssitcm;
    uint32_t ee_curr_cycle_cnt = 0;
    uint16_t ee_sector_start_addr = EE_SECTOR_START_ADDR;

    uint32_t getEESectorCycles(uint16_t ee_sector_depth);
    void changeEESector();
    void incEEWriteCycleCnt();

protected:
    bool resetEE(uint16_t ee_begin, uint16_t ee_end);

public:
    EEPROM();
    bool init();
    void checkEECycle(bool boot_up);
    uint8_t readEE(unsigned long ee_addr);
    bool readEE(unsigned long ee_addr, uint8_t *byte);
    bool writeEE(unsigned long ee_addr, uint8_t byte, bool traceable = false);
    uint16_t* getSectorStartAddr();
};

#endif