#include <Arduino.h>
#include "EEUnit.hpp"
#include "EEPROM.hpp"

#ifndef EEDispatcher_hpp
#define EEDispatcher_hpp

#define BLOWGUN_PRESET_CNT 4
#define PASTEUR_PRESET_CNT 3

class EEDispatcher
{
private:
    EEPROM ee24c64;

public:
    EEUnit *ee_blowgun_preset_arr;
    EEUnit *ee_auto_pasteur_tempC_arr;
    EEUnit *ee_auto_heating_tempC_arr;
    EEUnit *ee_auto_freezing_tempC_arr;
    EEUnit *ee_auto_pasteur_duratMM_arr;    
    EEUnit *ee_auto_run_on_hh_arr;
    EEUnit *ee_auto_run_on_mm_arr;
    EEUnit *ee_auto_run_toggle_arr;
    EEUnit *ee_auto_is_runned_today_arr;  

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
           ee_self_pasteur_tempC = EEUnit(0x000A, &ee24c64, false),
           ee_self_heating_tempC = EEUnit(0x000B, &ee24c64, false),
           ee_self_freezing_tempC = EEUnit(0x000C, &ee24c64, false),
           ee_self_pasteur_durat = EEUnit(0x000D, &ee24c64, false),
           ee_self_pasteur_mode = EEUnit(0x000E, &ee24c64, false),
           ee_auto1_pasteur_tempC = EEUnit(0x000F, &ee24c64, false),
           ee_auto2_pasteur_tempC = EEUnit(0x0010, &ee24c64, false),
           ee_auto3_pasteur_tempC = EEUnit(0x0011, &ee24c64, false),
           ee_auto1_heating_tempC = EEUnit(0x0012, &ee24c64, false),
           ee_auto2_heating_tempC = EEUnit(0x0013, &ee24c64, false),
           ee_auto3_heating_tempC = EEUnit(0x0014, &ee24c64, false),
           ee_auto1_freezing_tempC = EEUnit(0x0015, &ee24c64, false),
           ee_auto2_freezing_tempC = EEUnit(0x0016, &ee24c64, false),
           ee_auto3_freezing_tempC = EEUnit(0x0017, &ee24c64, false),
           ee_auto1_pasteur_durat = EEUnit(0x0018, &ee24c64, false),
           ee_auto2_pasteur_durat = EEUnit(0x0019, &ee24c64, false),
           ee_auto3_pasteur_durat = EEUnit(0x001A, &ee24c64, false),
           ee_auto1_run_on_hh = EEUnit(0x001B, &ee24c64, false),
           ee_auto2_run_on_hh = EEUnit(0x001C, &ee24c64, false),
           ee_auto3_run_on_hh = EEUnit(0x001D, &ee24c64, false),
           ee_auto1_run_on_mm = EEUnit(0x001E, &ee24c64, false),
           ee_auto2_run_on_mm = EEUnit(0x001F, &ee24c64, false),
           ee_auto3_run_on_mm = EEUnit(0x0020, &ee24c64, false),
           ee_auto1_toggle = EEUnit(0x0021, &ee24c64, false),
           ee_auto2_toggle = EEUnit(0x0022, &ee24c64, false),
           ee_auto3_toggle = EEUnit(0x0023, &ee24c64, false),
           ee_auto1_is_runned_today = EEUnit(0x0024, &ee24c64, false),
           ee_auto2_is_runned_today = EEUnit(0x0025, &ee24c64, false),
           ee_auto3_is_runned_today = EEUnit(0x0026, &ee24c64, false),
           ee_master_water_saving = EEUnit(0x0027, &ee24c64, false),
           ee_master_hysteresis_toggle = EEUnit(0x0028, &ee24c64, false),
           ee_master_hysteresis_value = EEUnit(0x0029, &ee24c64, false),
           ee_master_4ma_adc_value = EEUnit(0x002A, &ee24c64, false),
           ee_master_20ma_adc_value = EEUnit(0x002B, &ee24c64, false),
           ee_master_pump_perf_lm = EEUnit(0x002C, &ee24c64, false),

           ee_proc_pasteur_running = EEUnit(0x002D, &ee24c64, false),
           ee_proc_pasteur_paused = EEUnit(0x002E, &ee24c64, false),
           ee_proc_pasteur_part_finished = EEUnit(0x002F, &ee24c64, false),
           ee_proc_need_in_freezing = EEUnit(0x0030, &ee24c64, false),
           ee_proc_need_in_heating = EEUnit(0x0031, &ee24c64, false),
           ee_proc_freezing_part_finished = EEUnit(0x0032, &ee24c64, false),
           ee_proc_heating_part_finished = EEUnit(0x0033, &ee24c64, false),
           ee_proc_waterJacket_filled_yet = EEUnit(0x0034, &ee24c64, false),
           ee_proc_time_span = EEUnit(0x0035, &ee24c64, false),
           ee_proc_pasteur_tempC = EEUnit(0x0036, &ee24c64, false),
           ee_proc_freezing_tempC = EEUnit(0x0037, &ee24c64, false),
           ee_proc_heating_tempC = EEUnit(0x0038, &ee24c64, false),
           ee_proc_preset_runned_index = EEUnit(0x0039, &ee24c64, false),
           ee_proc_started_hh = EEUnit(0x003A, &ee24c64, false),
           ee_proc_started_mm = EEUnit(0x003B, &ee24c64, false),
           ee_proc_estimated_mm = EEUnit(0x003C, &ee24c64, false),

           ee_dynamic_proc_last_point_hh = EEUnit(0x003D, &ee24c64, false, ee24c64.getSectorStartAddr()),
           ee_dynamic_proc_last_point_mm = EEUnit(0x003E, &ee24c64, false, ee24c64.getSectorStartAddr()),

           ee_dynamic_rtc_curr_hh = EEUnit(0x003F, &ee24c64, false, ee24c64.getSectorStartAddr()),
           ee_dynamic_rtc_curr_mm = EEUnit(0x0040, &ee24c64, false, ee24c64.getSectorStartAddr()),
           ee_dynamic_rtc_curr_ss = EEUnit(0x0041, &ee24c64, true, ee24c64.getSectorStartAddr());
            
    bool init();
};

#endif