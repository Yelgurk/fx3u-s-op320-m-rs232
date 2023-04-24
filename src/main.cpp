#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEPROM.hpp"

MBDispatcher mb_disp;
IODispatcher io_monitor;
EEPROM ee24c64;

#define RUN_TEST 1

void testEEReadWriteRelloc();
uint8_t testVal = 0;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    ee24c64.init();
    Serial.println("[Tepmlate]");
    Serial.println("[ val1 | val2 | val3 | cyclCnt b1 | cyclCnt b2 | cyclCnt b3 | cyclCnt b4 | cyclCnt in Uint32 ]");

    if (RUN_TEST == 0)
        ee24c64.resetEE(0x0000, 0x0050);
    else
    {
        testVal = ee24c64.readEE(0x0000);
        ee24c64.checkEECycle(true);
    }

    //mb_disp.init();
}

uint32_t currMill = 0;
uint32_t task1Mill = 0;
uint32_t task2Mill = 0;

void loop()
{
    if (RUN_TEST == 0)
    {
        Serial.println("await...");
        delay(2000);
    }
    else
        testEEReadWriteRelloc();
}

void testEEReadWriteRelloc()
{
    //mb_disp.poll();
    //RW data after mb_disp.poll()

    currMill = millis();

    if (currMill - task1Mill > 500)
    {
        task1Mill = millis();
        ee24c64.writeEE(0x0000, ++testVal, true);
        ee24c64.writeEE(0x0001, testVal * 2, true);
        ee24c64.writeEE(0x0002, testVal * 3, true);

        Serial.print("[ ");
        Serial.print(ee24c64.readEE(0x0000));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(0x0001));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(0x0002));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(ee24c64.ee_sector_start_addr + EE_CYCLE_UINT32_ADDR));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(ee24c64.ee_sector_start_addr + EE_CYCLE_UINT32_ADDR + 1));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(ee24c64.ee_sector_start_addr + EE_CYCLE_UINT32_ADDR + 2));
        Serial.print(" | ");
        Serial.print(ee24c64.readEE(ee24c64.ee_sector_start_addr + EE_CYCLE_UINT32_ADDR + 3));
        Serial.print(" | ");
        Serial.print(ee24c64.getEESectorCycles(0));
        Serial.println(" ]");
    }

    if (currMill - task2Mill > 10000)
    {
        task2Mill = millis();
        ee24c64.checkEECycle(false);
    }
}