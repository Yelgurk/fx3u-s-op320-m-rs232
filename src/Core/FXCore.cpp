#include "../include/Core/FXCore.hpp"

FXCore::FXCore()
{
    MBDispatcher::init();
    EEDispatcher::init();
    IODispatcher::init();

    rtc_general_current = new TimeUnit(false);
    rtc_general_set_new = new TimeUnit(false);
    rtc_general_last_point = new TimeUnit(true);
    rtc_prog_pasteur_started = new TimeUnit(true);
    rtc_prog_expected_finish = new TimeUnit(true);
    rtc_prog_pasteur_paused = new TimeUnit(true);
    rtc_prog_pasteur_finished = new TimeUnit(true);

    info_main_process = new SettingUnit(NULL, &mb_proc_list, (uint8_t)OP320Process::COUNT - 1);
    info_main_step_show_hide = new SettingUnit(NULL, &mb_step_name_list, 1);
    info_main_step = new SettingUnit(NULL, &mb_step_index_list, (uint8_t)OP320Step::COUNT - 1);
    info_error_notify = new SettingUnit(NULL, &mb_notification_list, (uint8_t)OP320Error::COUNT - 1);
    
    solo_heating_tempC = new SettingUnit(&ee_solo_heating_tempC, &mb_solo_heating_tempC);
    solo_freezing_tempC = new SettingUnit(&ee_solo_freezing_tempC, &mb_self_freezing_tempC);
    
    calibration_blowgun_main_dose = new SettingUnit(NULL, &mb_blow_calib_dosage, 0, 1000);
    calibration_blowgun_dope_dose = new SettingUnit(&ee_blowgun_calib_range, &mb_blow_calib_range, 0, 10);
    
    self_prog_pasteur_tempC = new SettingUnit(&ee_self_pasteur_tempC, &mb_self_pasteur_tempC);
    self_prog_heating_tempC = new SettingUnit(&ee_self_heating_tempC, &mb_self_heating_tempC);
    self_prog_freezing_tempC = new SettingUnit(&ee_self_freezing_tempC, &mb_self_freezing_tempC);
    self_prog_duration_mm = new SettingUnit(&ee_self_pasteur_durat, &mb_self_durat_mm);
    self_prog_mode = new SettingUnit(&ee_self_pasteur_mode, &mb_self_mode_list, 2);
    
    master_water_saving_toggle = new SettingUnit(&ee_master_water_saving, &mb_master_water_saving_monitor, 1);
    master_hysteresis_toggle = new SettingUnit(&ee_master_hysteresis_toggle, &mb_master_hysteresis_monitor, 1);
    master_hysteresis_tempC = new SettingUnit(&ee_master_hysteresis_value, &mb_hysteresis);
    master_pump_LM_performance = new SettingUnit(&ee_master_pump_perf_lm, &mb_blowing_performance_lm);
    master_4ma_negative_limit = new SettingUnit(&ee_master_4ma_adc_value, &mb_4ma_adc_limit);
    master_20ma_positive_limit = new SettingUnit(&ee_master_20ma_adc_value, &mb_20ma_adc_limit);
    
    blowgun_presets = new BlowingPreset(
        &mb_blowing_preset_list,
        &mb_blowing_volume,
        &mb_blowing_incV,
        &mb_blowing_decV,
        ee_blowgun_preset_arr
    );
    
    auto_prog_presets = new AutoPasteurPreset(
        ee_auto_pasteur_tempC_arr,
        ee_auto_heating_tempC_arr,
        ee_auto_freezing_tempC_arr,
        ee_auto_pasteur_duratMM_arr,
        ee_auto_run_on_hh_arr,
        ee_auto_run_on_mm_arr,
        ee_auto_run_toggle_arr,
        ee_auto_is_runned_today_arr,
        &mb_auto_preset_list,
        &mb_auto_pasteur_tempC,
        &mb_auto_heating_tempC,
        &mb_auto_freezing_tempC,
        &mb_auto_durat_mm,
        &mb_auto_run_rtc_hh,
        &mb_auto_run_rtc_mm,
        &mb_auto_preset_toggle
    );

    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); });
}