#include "IOUnit.hpp"

#ifndef _iodispatcher_hpp
#define _iodispatcher_hpp

/* Relay */
#define Y00 PC9
#define Y01 PC8
#define Y02 PA8
#define Y03 PA0
#define Y04 PB3
#define Y05 PD12

/* Digital IN */
#define X00 PB13
#define X01 PB14
#define X02 PB11
#define X03 PB12
#define X04 PE15
#define X05 PB10
#define X06 PE13
#define X07 PE14

/* Analog IN */
#define AD0 PA1
#define AD1 PA3 
#define AD2 PC4
#define AD3 PC5
#define AD4 PC0
#define AD5 PC1

/* Physical mapping */
#define HEATER_PIN              Y00
#define MIXER_PIN               Y01
#define WJACKET_VALVE_PIN       Y02
#define BLOWGUN_PUMP_PIN        Y03

#define WJACKET_SENSOR_PIN      X00
#define STOPBTN_PIN             X01
#define MIXER_CRASH_SIGNAL_PIN  X02
#define V380_SENSOR_PIN         X03
#define BLOWGUN_SENSOR          X04

#define BATTERY_CHARGE_PIN      AD0
#define LIQUID_TEMPC_PIN        AD3

#define IO_COUNT 11

class IODispatcher
{
private:
    IOUnit **io_array;

public:
    IOUnit io_heater_r = IOUnit(HEATER_PIN, mode::Relay),
           io_mixer_r = IOUnit(MIXER_PIN, mode::Relay),
           io_water_jacket_r = IOUnit(WJACKET_VALVE_PIN, mode::Relay),
           io_blowgun_r = IOUnit(BLOWGUN_PUMP_PIN, mode::Relay),
           io_water_jacket_s = IOUnit(WJACKET_SENSOR_PIN, mode::DigitalIN),
           io_stop_btn_s = IOUnit(STOPBTN_PIN, mode::DigitalIN),
           io_mixer_crash_s = IOUnit(MIXER_CRASH_SIGNAL_PIN, mode::DigitalIN),
           io_v380_s = IOUnit(V380_SENSOR_PIN, mode::DigitalIN),
           io_blowgun_s = IOUnit(BLOWGUN_SENSOR, mode::DigitalIN),
           io_battery_s = IOUnit(BATTERY_CHARGE_PIN, mode::AnalogIN),
           io_liquid_temp_s = IOUnit(LIQUID_TEMPC_PIN, mode::AnalogIN);

    bool init();
    void relaysOff();
};

#endif