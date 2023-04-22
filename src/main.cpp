#include <Arduino.h>
#include "MBUnit.hpp"

#include "Modbus.hpp"
#define ID   1

Modbus slave;
unsigned long relayDelay = 0;
uint16_t au16data[9];
uint16_t value = 0;
uint16_t value2 = 0;

MBUnit MB_val1(au16data, 3, type::Uint16);
MBUnit MB_val2(au16data, 4, type::Uint16); 

void setup()
{
    //io_setup(); //configura las entradas y salidas

    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    slave = Modbus(ID, Serial, 0);

    pinMode(PC9, OUTPUT);
    pinMode(PC8, OUTPUT);
    pinMode(PA8, OUTPUT);

    slave.start();
    relayDelay = millis() + 100;
    digitalWrite(PC9, HIGH);
}

uint32_t curr = 0;
uint32_t old = 0;
bool display = false;

void io_poll();

void loop()
{
    curr = millis();
    if (curr - old > 2000)
    {
        old = millis();
        display = !display;
    }

    delay(2);
    ++value;
    value2 += 2;

    if (slave.poll( au16data, 9 ) > 4)
    {
        relayDelay = millis() + 500;
        digitalWrite(PC9, HIGH);
    }
    if (millis() > relayDelay) digitalWrite(PC9, LOW );
  
    io_poll();
} 


void io_poll() {
  bitWrite( au16data[0], 2, display ? 1 : 0);

  digitalWrite( PC8, bitRead( au16data[1], 1 ));
  digitalWrite( PA8, bitRead( au16data[1], 2 ));

  //au16data[3] = value;
  //au16data[4] = value2;

  MB_val1.writeValue(value);
  MB_val2.writeValue(value2);

  au16data[6] = slave.getInCnt();
  au16data[7] = slave.getOutCnt();
  au16data[8] = slave.getErrCnt();
}