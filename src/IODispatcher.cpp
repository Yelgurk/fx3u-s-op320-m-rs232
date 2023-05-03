#include "IODispatcher.hpp"

bool IODispatcher::init()
{
    io_array = new IOUnit*[IO_COUNT] {
        &io_heater_r,
        &io_mixer_r,
        &io_water_jacket_r,
        &io_blowgun_r,
        &io_water_jacket_s,
        &io_stop_btn_s,
        &io_mixer_crash_s,
        &io_v380_s,
        &io_blowgun_s,
        &io_battery_s,
        &io_liquid_temp_s
    };

    for (uint8_t i = 0; i < IO_COUNT; i++)
        if (!this->io_array[i]->init())
            return false;

    return true;
}

void IODispatcher::relaysOff()
{
    for (uint8_t i = 0; i < IO_COUNT; i++)
        this->io_array[i]->write(false);
}