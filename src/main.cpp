#include <Arduino.h>
#include "FXCore.hpp"

MBDispatcher mb_disp;
IODispatcher io_monitor;
TaskManager tm_fx3u;

void testF();
void testR();

void setup()
{
    Serial.setRx(PA10);
    Serial.setTx(PA9);
    Serial.begin(38400);

    mb_disp.init();
    io_monitor.init();
    io_monitor.relaysOff();

    tm_fx3u.newTask([]() -> void { mb_disp.poll(); }, 0);
    tm_fx3u.newTask([]() -> void { mb_disp.mb_coil1.writeValue(!mb_disp.mb_coil1.readValue()); }, 2000);
    tm_fx3u.newTask([]() -> void { testF(); }, 1000);
    tm_fx3u.newTask([]() -> void { testR(); }, 50);
}

void testF()
{
    mb_disp.mb_val1.writeValue((uint16_t)(mb_disp.mb_val1.readValue() + 1));
}

void testR()
{
    io_monitor.io_mixer_r.write(mb_disp.mb_coil2.readValue());
    io_monitor.io_heater_r.write(mb_disp.mb_coil3.readValue());
    mb_disp.mb_val2.writeValue((uint16_t)(mb_disp.mb_val2.readValue() + 1));
}

void loop()
{
    tm_fx3u.dispatcher();
}