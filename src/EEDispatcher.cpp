#include "EEDispatcher.hpp"

bool EEDispatcher::init()
{
    bool response = ee24c64.init();

    ee_blowgun_preset_arr = new EEUnit*[BLOWGUN_PRESET_CNT] {
        &ee_blowgun_preset_1,
        &ee_blowgun_preset_2,
        &ee_blowgun_preset_3,
        &ee_blowgun_preset_4
    };

    return response;
}