#include "../include/EEDispatcher/EEPROM.hpp"

EEPROM::EEPROM() {
    this->ssitcm = new SlowSoftI2CMaster(PA13, PA15, true);
}

bool EEPROM::init() {
    return this->ssitcm->i2c_init();
}

uint8_t EEPROM::readEE(unsigned long ee_addr)
{
    uint8_t response = 0;
    if (!ssitcm->i2c_start((EE_I2C_ARRD << 1) | I2C_WRITE)) return false;
    if (!ssitcm->i2c_write((ee_addr >> 8) & 0xFF)) return false;
    if (!ssitcm->i2c_write(ee_addr & 0xFF)) return false;
    if (!ssitcm->i2c_rep_start((EE_I2C_ARRD << 1) | I2C_READ)) return false;
    //*byte = ssitcm->i2c_read(true);
    response = ssitcm->i2c_read(true);
    ssitcm->i2c_stop();

    delay(2);
    return response;
}

bool EEPROM::readEE(unsigned long ee_addr, uint8_t *byte)
{
    if (!ssitcm->i2c_start((EE_I2C_ARRD << 1) | I2C_WRITE)) return false;
    if (!ssitcm->i2c_write((ee_addr >> 8) & 0xFF)) return false;
    if (!ssitcm->i2c_write(ee_addr & 0xFF)) return false;
    if (!ssitcm->i2c_rep_start((EE_I2C_ARRD << 1) | I2C_READ)) return false;
    *byte = ssitcm->i2c_read(true);
    ssitcm->i2c_stop();
    delay(2);

    return true;
}

bool EEPROM::writeEE(unsigned long ee_addr, uint8_t byte, bool traceable)
{
    if (!ssitcm->i2c_start((EE_I2C_ARRD << 1) | I2C_WRITE)) return false;
    if (!ssitcm->i2c_write((ee_addr >> 8) & 0xFF)) return false;
    if (!ssitcm->i2c_write(ee_addr & 0xFF)) return false;
    if (!ssitcm->i2c_write(byte)) return false;
    ssitcm->i2c_stop();
    delay(6);

    if (traceable)
        incEEWriteCycleCnt();

    return true;
}

void EEPROM::incEEWriteCycleCnt()
{
    ++ee_curr_cycle_cnt;
    uint8_t ee_cycle_cnt_uint32[4];

    for (uint8_t range = 0; range < 4; range++)
    {
        ee_cycle_cnt_uint32[range] = ((uint8_t*)&ee_curr_cycle_cnt)[3 - range];
        writeEE(ee_sector_start_addr + EE_SECTOR_SIZE + range, ee_cycle_cnt_uint32[range], false);
    }
}

void EEPROM::checkEECycle(bool boot_up)
{
    if (boot_up)
        ee_curr_cycle_cnt = 0;

    uint16_t depth = -1; 
    uint32_t ee_cycle_cnt_old = 0;

    do
    {
        ++depth;
        ee_cycle_cnt_old = ee_curr_cycle_cnt;
        ee_curr_cycle_cnt = getEESectorCycles(depth);
    }
    while (ee_curr_cycle_cnt >= EE_CYCLE_LIMIT);

    if (ee_cycle_cnt_old >= EE_CYCLE_LIMIT && boot_up && ee_curr_cycle_cnt == 0)
        boot_up = false;

    if (depth > 0)
    {
        ee_sector_start_addr = ee_sector_start_addr + (EE_SECTOR_SIZE + EE_COUNTER_SIZE) * depth;
        if (!boot_up)
            changeEESector();
    }
}

void EEPROM::changeEESector()
{
    bool ee_val_readed = false,
         ee_val_writed = false;
    uint8_t ee_reloc_val;

    for (uint32_t point = ee_sector_start_addr - EE_SECTOR_SIZE - EE_COUNTER_SIZE; point < ee_sector_start_addr - EE_COUNTER_SIZE; point++)
    {
        ee_val_readed = readEE(point, &ee_reloc_val);
        ee_val_writed = writeEE(point + EE_SECTOR_SIZE + EE_COUNTER_SIZE, ee_reloc_val);
    
        if (!ee_val_readed || !ee_val_writed)
            break;
    }
}

bool EEPROM::resetEE(uint16_t ee_begin, uint16_t ee_end)
{
    ee_end = ee_end > EE_FULL_BSIZE ? EE_FULL_BSIZE : ee_end;

    for (uint16_t point = ee_begin; point < ee_end - 1; point++)
        if (!writeEE(point, 0))
            return false;

    return true;
}

uint32_t EEPROM::getEESectorCycles(uint16_t ee_sector_depth)
{
    uint8_t ee_cycle_cnt_uint32[4];
    for (uint8_t range = 0; range < 4; range++)
            readEE(ee_sector_start_addr + (EE_SECTOR_SIZE + EE_COUNTER_SIZE) * ee_sector_depth + EE_SECTOR_SIZE + range, &ee_cycle_cnt_uint32[3 - range]);
    
    return *((uint32_t*) ee_cycle_cnt_uint32);
}

uint16_t* EEPROM::getSectorStartAddr() {
    return &ee_sector_start_addr;
}