#include "IODispatcher.hpp"

bool IODispatcher::init()
{
    io_array = new IOUnit[c_io_array_size] {
        heater_r,
        mixer_r,
        water_jacket_r,
        blowgun_r,
        water_jacket_s,
        stop_btn_s,
        mixer_crash_s,
        v380_s,
        blowgun_s,
        battery_s,
        liquid_temp_s
    };

    for (uint8_t i = 0; i < c_io_array_size; i++)
        if (!this->io_array[i].init())
            return false;

    return true;
}

void IODispatcher::relaysOff()
{
    for (uint8_t i = 0; i < c_io_array_size; i++)
        this->io_array[i].write(false);
}