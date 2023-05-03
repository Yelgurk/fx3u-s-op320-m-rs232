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

    ee_auto_pasteur_tempC_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_pasteur_tempC,
        &ee_auto2_pasteur_tempC,
        &ee_auto3_pasteur_tempC
    };

    ee_auto_heating_tempC_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_heating_tempC,
        &ee_auto2_heating_tempC,
        &ee_auto3_heating_tempC
    };

    ee_auto_freezing_tempC_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_freezing_tempC,
        &ee_auto2_freezing_tempC,
        &ee_auto3_freezing_tempC
    };

    ee_auto_pasteur_duratMM_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_pasteur_durat,
        &ee_auto2_pasteur_durat,
        &ee_auto3_pasteur_durat
    };

    ee_auto_run_on_hh_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_run_on_hh,
        &ee_auto2_run_on_hh,
        &ee_auto3_run_on_hh
    };

    ee_auto_run_on_mm_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_run_on_mm,
        &ee_auto2_run_on_mm,
        &ee_auto3_run_on_mm
    };

    ee_auto_run_toggle_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_toggle,
        &ee_auto2_toggle,
        &ee_auto3_toggle
    };

    ee_auto_is_runned_today_arr = new EEUnit*[PASTEUR_PRESET_CNT] {
        &ee_auto1_is_runned_today,
        &ee_auto2_is_runned_today,
        &ee_auto3_is_runned_today
    };

    return response;
}