#include <Arduino.h>
#include "EEUnit.hpp"
#include "EEPROM.hpp"

#ifndef _eedispatcher_hpp
#define _eedispatcher_hpp

#define BLOWGUN_PRESET_CNT 4
#define PASTEUR_PRESET_CNT 3

class EEDispatcher
{
private:
    EEPROM ee24c64;

public:
    EEUnit **ee_blowgun_preset_arr;

    EEUnit ee_blowgun_preset_1 = EEUnit(0x0000, &ee24c64, false),
           ee_blowgun_preset_2 = EEUnit(0x0001, &ee24c64, false),
           ee_blowgun_preset_3 = EEUnit(0x0002, &ee24c64, false),
           ee_blowgun_preset_4 = EEUnit(0x0003, &ee24c64, false),
           ee_solo_heating_tempC = EEUnit(0x0004, &ee24c64, false),
           ee_solo_freezing_tempC = EEUnit(0x0005, &ee24c64, false),
           ee_rtc_curr_day = EEUnit(0x0006, &ee24c64, false),
           ee_rtc_curr_month = EEUnit(0x0007, &ee24c64, false),
           ee_rtc_curr_year = EEUnit(0x0008, &ee24c64, false),
           ee_blowgun_calib_range = EEUnit(0x0009, &ee24c64, false),
           ee_self_pasteur_tempC = EEUnit(0x0010, &ee24c64, false),
           ee_self_heating_tempC = EEUnit(0x0011, &ee24c64, false),
           ee_self_freezing_tempC = EEUnit(0x0012, &ee24c64, false),
           ee_self_pasteur_durat = EEUnit(0x0013, &ee24c64, false),
           ee_self_psteur_mode = EEUnit(0x0014, &ee24c64, false),
           ee_auto1_pasteur_tempC = EEUnit(0x0015, &ee24c64, false),
           ee_auto2_pasteur_tempC = EEUnit(0x0016, &ee24c64, false),
           ee_auto3_pasteur_tempC = EEUnit(0x0017, &ee24c64, false),
           ee_auto1_heating_tempC = EEUnit(0x0018, &ee24c64, false),
           ee_auto2_heating_tempC = EEUnit(0x0019, &ee24c64, false),
           ee_auto3_heating_tempC = EEUnit(0x0020, &ee24c64, false),
           ee_auto1_freezing_tempC = EEUnit(0x0021, &ee24c64, false),
           ee_auto2_freezing_tempC = EEUnit(0x0022, &ee24c64, false),
           ee_auto3_freezing_tempC = EEUnit(0x0023, &ee24c64, false),
           ee_auto1_pasteur_durat = EEUnit(0x0024, &ee24c64, false),
           ee_auto2_pasteur_durat = EEUnit(0x0025, &ee24c64, false),
           ee_auto3_pasteur_durat = EEUnit(0x0026, &ee24c64, false),
           ee_auto1_run_on_hh = EEUnit(0x0027, &ee24c64, false),
           ee_auto1_run_on_mm = EEUnit(0x0028, &ee24c64, false),
           ee_auto2_run_on_hh = EEUnit(0x0029, &ee24c64, false),
           ee_auto2_run_on_mm = EEUnit(0x0030, &ee24c64, false),
           ee_auto3_run_on_hh = EEUnit(0x0031, &ee24c64, false),
           ee_auto3_run_on_mm = EEUnit(0x0032, &ee24c64, false),
           ee_auto1_toggle = EEUnit(0x0033, &ee24c64, false),
           ee_auto2_toggle = EEUnit(0x0034, &ee24c64, false),
           ee_auto3_toggle = EEUnit(0x0035, &ee24c64, false),
           ee_master_water_saving = EEUnit(0x0036, &ee24c64, false),
           ee_master_hysteresis_toggle = EEUnit(0x0037, &ee24c64, false),
           ee_master_hysteresis_value = EEUnit(0x0038, &ee24c64, false),
           ee_master_4ma_adc_value = EEUnit(0x0039, &ee24c64, false),
           ee_master_20ma_adc_value = EEUnit(0x0040, &ee24c64, false),
           ee_master_pump_perf_lm = EEUnit(0x0041, &ee24c64, false);
           
            
    bool init();
};

#endif