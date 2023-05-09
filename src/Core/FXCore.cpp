#include "../include/Core/FXCore.hpp"

FXCore::FXCore()
{
    this->MBDispatcher::init();
    this->EEDispatcher::init();
    this->IODispatcher::init();

    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    scr_set_op320 = new SettingUnit(NULL, &mb_set_op320_scr);
    scr_get_op320 = new SettingUnit(NULL, &mb_get_op320_scr);

    info_main_process = new SettingUnit(NULL, &mb_proc_list, (uint8_t)OP320_PROCESS::COUNT - 1);
    info_main_step_show_hide = new SettingUnit(NULL, &mb_step_name_list, 1);
    info_main_step = new SettingUnit(NULL, &mb_step_index_list, (uint8_t)OP320_STEP::COUNT - 1);
    info_error_notify = new SettingUnit(NULL, &mb_notification_list, (uint8_t)OP320_ERROR::COUNT - 1);
    
    solo_heating_tempC = new SettingUnit(&ee_solo_heating_tempC, &mb_solo_heating_tempC);
    solo_freezing_tempC = new SettingUnit(&ee_solo_freezing_tempC, &mb_solo_freezing_tempC);
    
    calibration_blowgun_main_dose = new SettingUnit(NULL, &mb_blow_calib_dosage, 0, 100);
    calibration_blowgun_main_dose->setValue(3);
    calibration_blowgun_dope_dose = new SettingUnit(&ee_blowgun_calib_range, &mb_blow_calib_range, 99, 100);
    
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

    rtc_general_current = new TimeUnit(false);
    rtc_general_set_new = new TimeUnit(false);
    rtc_general_last_point = new TimeUnit(true);
    rtc_prog_pasteur_started = new TimeUnit(true);
    rtc_prog_expected_finish = new TimeUnit(true);
    rtc_prog_pasteur_paused = new TimeUnit(true);
    rtc_prog_pasteur_finished = new TimeUnit(true);

    rtc_general_current->setMBPointer(&mb_rtc_ss, PointerType::Seconds);
    rtc_general_current->setMBPointer(&mb_rtc_mm, PointerType::Minutes);
    rtc_general_current->setMBPointer(&mb_rtc_hh, PointerType::Hours);
    rtc_general_current->setMBPointer(&mb_rtc_DD, PointerType::Days);
    rtc_general_current->setMBPointer(&mb_rtc_MM, PointerType::Months);
    rtc_general_current->setMBPointer(&mb_rtc_YY, PointerType::Years);

    rtc_general_set_new->setMBPointer(&mb_rtc_new_ss, PointerType::Seconds);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_mm, PointerType::Minutes);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_hh, PointerType::Hours);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_DD, PointerType::Days);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_MM, PointerType::Months);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_YY, PointerType::Years);
    rtc_general_set_new->setZeroDateTime();

    mb_comm_goto_scr_blowing.addTrigger([this]()->      void { scr_set_op320->setValue(SCR_BLOWING_PAGE); });
    mb_comm_blow_preset_1.addTrigger([this]()->         void { blowgun_presets->selectPreset(0); });
    mb_comm_blow_preset_2.addTrigger([this]()->         void { blowgun_presets->selectPreset(1); });
    mb_comm_blow_preset_3.addTrigger([this]()->         void { blowgun_presets->selectPreset(2); });
    mb_comm_blow_preset_4.addTrigger([this]()->         void { blowgun_presets->selectPreset(3); });
    mb_comm_blow_vInc.addTrigger([this]()->             void { blowgun_presets->incValue(); });
    mb_comm_blow_vDec.addTrigger([this]()->             void { blowgun_presets->decValue(); });
    mb_comm_blow_prescaler.addTrigger([this]()->        void { blowgun_presets->changeScaler(); });
    mb_comm_solo_tempC_cancel.addTrigger([this]()->     void { solo_heating_tempC->refreshValue(); solo_freezing_tempC->refreshValue(); });
    mb_comm_solo_tempC_accept.addTrigger([this]()->     void { solo_heating_tempC->setValueByModbus(); solo_freezing_tempC->setValueByModbus(); });
    mb_comm_rtc_new_cancel.addTrigger([this]()->        void { rtc_general_set_new->setZeroDateTime(); scr_set_op320->setValue(SCR_USER_MENU); });
    mb_comm_rtc_new_accept.addTrigger([this]()->        void { FXCore::setNewDateTime(); });
    mb_comm_blow_calib_inc.addTrigger([this]()->        void { calibration_blowgun_dope_dose->incValue(); });
    mb_comm_blow_calib_dec.addTrigger([this]()->        void { calibration_blowgun_dope_dose->decValue(); });
    mb_comm_self_accept.addTrigger([this]()-> void {
        self_prog_pasteur_tempC->setValueByModbus();
        self_prog_heating_tempC->setValueByModbus();
        self_prog_freezing_tempC->setValueByModbus();
        self_prog_duration_mm->setValueByModbus();
    });
    mb_comm_self_cancel.addTrigger([this]()-> void {
        self_prog_pasteur_tempC->refreshValue();
        self_prog_heating_tempC->refreshValue();
        self_prog_freezing_tempC->refreshValue();
        self_prog_duration_mm->refreshValue();
        scr_set_op320->setValue(SCR_USER_MENU);
    });
    mb_comm_self_mode_up.addTrigger([this]()->          void { self_prog_mode->incValue(); });
    mb_comm_self_mode_down.addTrigger([this]()->        void { self_prog_mode->decValue(); });
    mb_comm_auto_preset_1.addTrigger([this]()->         void { auto_prog_presets->selectPreset(0); });
    mb_comm_auto_preset_2.addTrigger([this]()->         void { auto_prog_presets->selectPreset(1); });
    mb_comm_auto_preset_3.addTrigger([this]()->         void { auto_prog_presets->selectPreset(2); });
    mb_comm_auto_toggle.addTrigger([this]()->           void { auto_prog_presets->togglePreset(); });
    mb_comm_auto_accept.addTrigger([this]()->           void { auto_prog_presets->acceptChanges(); });
    mb_comm_auto_cancel.addTrigger([this]()->           void { auto_prog_presets->refreshPreset(); scr_set_op320->setValue(SCR_USER_MENU); });
    mb_master_water_saving_toggle.addTrigger([this]()-> void { master_water_saving_toggle->incValue(); });
    mb_master_hysteresis_toggle.addTrigger([this]()->   void { master_hysteresis_toggle->incValue(); });
    mb_master_accept.addTrigger([this]()-> void {
        master_hysteresis_tempC->setValueByModbus();
        master_pump_LM_performance->setValueByModbus();
        master_4ma_negative_limit->setValueByModbus();
        master_20ma_positive_limit->setValueByModbus();
    });
    mb_master_cancel.addTrigger([this]()-> void {
        master_hysteresis_tempC->refreshValue();
        master_pump_LM_performance->refreshValue();
        master_4ma_negative_limit->refreshValue();
        master_20ma_positive_limit->refreshValue();
        scr_set_op320->setValue(SCR_USER_MENU);
    });
    mb_master_full_hard_reset.addTrigger([this]()->     void {  });
    mb_comm_self_pasteur_start.addTrigger([this]()->    void {  });
    mb_comm_solo_heating_toggle.addTrigger([this]()->   void {  });
    mb_comm_solo_freezing_toggle.addTrigger([this]()->  void {  });
    mb_comm_blowgun_run_btn.addTrigger([this]()->       void {  });

    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); });
    TaskManager::newTask(200,   [this]() -> void { rtc_general_current->setRealTime(); });
}

void FXCore::setNewDateTime()
{
    rtc_general_set_new->loadFromMB();
    rtc.setDate(rtc_general_set_new->getDays(), rtc_general_set_new->getMonths(), rtc_general_set_new->getYears());
    rtc.setTime(rtc_general_set_new->getHours(), rtc_general_set_new->getMins(), rtc_general_set_new->getSecs());
    rtc_general_set_new->setZeroDateTime();
}