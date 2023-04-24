#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"

MBDispatcher mb_disp;
IODispatcher io_monitor;
EEDispatcher ee_disp;

/*
    0 = set eeprom bytes to zero
    1 = start test r/w
    2 = show eeprom variables
*/
#define RUN_TEST 1

void testEEReadWriteRelloc();
void testEEDisplayVars(uint16_t size);
uint8_t testVal = -1;

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    ee_disp.init();

    Serial.println("[Tepmlate]");
    Serial.println("[ val1 | val2 | val3 | cyclCnt b1 | cyclCnt b2 | cyclCnt b3 | cyclCnt b4 | cyclCnt in Uint32 ]");

    if (RUN_TEST <= 0)
        ee_disp.ee24c64.resetEE(0x0000, 0x0500);
    else if (RUN_TEST == 1)
    {
        testVal = ee_disp.ee24c64.readEE(0x0000);
        ee_disp.ee24c64.checkEECycle(true);
    }
    else if (RUN_TEST > 1)
        testEEDisplayVars(30);

    Serial.print("testVal = ");
    Serial.println(testVal);
    Serial.println("to loop");
    //mb_disp.init();
}

uint32_t currMill = 0;
uint32_t task1Mill = 0;
uint32_t task2Mill = 0;

void loop()
{
    if (RUN_TEST <= 0 || RUN_TEST > 1)
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

    if (currMill - task1Mill > 1000)
    {
        task1Mill = millis();
        ee_disp.testVal1.writeEE(++testVal);
        ee_disp.testVal2.writeEE(ee_disp.testVal2.readEE() + 2);
        ee_disp.testVal3.writeEE(ee_disp.testVal3.readEE() + 1);

        Serial.print("[ ");
        Serial.print(ee_disp.testVal1.readEE());
        Serial.print(" | ");
        Serial.print(ee_disp.testVal2.readEE());
        Serial.print(" = 0x");
        Serial.print(ee_disp.testVal2.getVarAddr());
        Serial.print(" | ");
        Serial.print(ee_disp.testVal3.readEE());
        Serial.print(" = 0x");
        Serial.print(ee_disp.testVal3.getVarAddr());
        Serial.print(" | ");
        Serial.print(ee_disp.ee24c64.readEE(ee_disp.ee24c64.ee_sector_start_addr + EE_SECTOR_SIZE));
        Serial.print(" | ");
        Serial.print(ee_disp.ee24c64.readEE(ee_disp.ee24c64.ee_sector_start_addr + EE_SECTOR_SIZE + 1));
        Serial.print(" | ");
        Serial.print(ee_disp.ee24c64.readEE(ee_disp.ee24c64.ee_sector_start_addr + EE_SECTOR_SIZE + 2));
        Serial.print(" | ");
        Serial.print(ee_disp.ee24c64.readEE(ee_disp.ee24c64.ee_sector_start_addr + EE_SECTOR_SIZE + 3));
        Serial.print(" | ");
        Serial.print(ee_disp.ee24c64.getEESectorCycles(0));
        Serial.println(" ]");
    }

    if (currMill - task2Mill > 10000)
    {
        task2Mill = millis();
        ee_disp.ee24c64.checkEECycle(false);
    }
}

void testEEDisplayVars(uint16_t size)
{
    for (uint16_t point = 0; point < size; point++)
    {
        Serial.print("0x");
        Serial.print(point);
        Serial.print(" = ");
        Serial.println(ee_disp.ee24c64.readEE(point));
    }
}