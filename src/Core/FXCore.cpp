#include "../include/Core/FXCore.hpp"

/* private */

void FXCore::checkIsHardReseted()
{
    if (ee_hard_reset_value_setted.readEE() == 0)
        this->hardReset();
}

bool FXCore::checkIsProgWasRunned()
{
    if (prog_running->getState())
    {
        uint8_t state_index = prog_state->getValue();
        if (state_index >= static_cast<uint8_t>(PROG_STATE::PasteurFinished))
            is_pasteur_part_finished_crutch = true;

        if (prog_need_in_freezing->getState())
        {
            if (state_index >= static_cast<uint8_t>(PROG_STATE::FreezingFinished))
                is_freezing_part_finished_crutch = true;
        }
        else
            is_freezing_part_finished_crutch = true;

        if (prog_need_in_heating->getState())
        {
            if (state_index >= static_cast<uint8_t>(PROG_STATE::HeatingFinished))
                is_heating_part_finished_crutch = true;
        }
        else
            is_heating_part_finished_crutch = true;

        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
        {
            if (rtc_general_current->getDiffSec(rtc_prog_pasteur_paused) >= PASTEUR_AWAIT_LIMIT_MM * 60)
            {
                taskFinishProg(!is_water_in_jacket ? FINISH_FLAG::WaterJacketError : FINISH_FLAG::Power380vError);
                return false;
            }
        }
        else if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
        {
            if (rtc_general_current->getDiffSec(rtc_general_last_time_point) >= PASTEUR_AWAIT_LIMIT_MM * 60)
            {
                taskFinishProg(FINISH_FLAG::Power380vError);
                return false;
            }
            else
            {
                uint64_t in_await_span_sec = rtc_prog_expected_finish->getDiffSec(rtc_general_last_time_point);
                rtc_prog_expected_finish->clone(rtc_general_current);
                rtc_prog_expected_finish->addMinutes((in_await_span_sec / 60) + 1);
            }
        }

        if (prog_preset_selected->getValue() == 0)
        {
            prog_pasteur_tempC = self_prog_pasteur_tempC->getValue();
            prog_heating_tempC = self_prog_heating_tempC->getValue();
            prog_freezing_tempC = self_prog_freezing_tempC->getValue();
            prog_selected_duration_mm = self_prog_duration_mm->getValue();
        }
        else
            auto_prog_presets.resumePreset(
                prog_preset_selected->getValue() - 1,
                prog_pasteur_tempC,
                prog_heating_tempC,
                prog_freezing_tempC,
                prog_selected_duration_mm
                );

        return true;
    }

    return false;
}

void FXCore::displayMainInfoVars()
{
    info_main_batt_charge->setValue(batt_chargeV);
    info_main_liq_tempC->setValue(liquid_tempC < 0 ? 0 : (liquid_tempC > 255 ? 255 : liquid_tempC));
}

void FXCore::displayTasksDeadline()
{
    scr_get_op320->setValueByModbus();
    if ((prog_running->getState() || is_task_flowing_running) && scr_get_op320->getValue() == SCR_MASTER_PAGE)
        scr_set_op320->setValue(SCR_MASTER_PAGE_TIME);
    else if (!(prog_running->getState() || is_task_flowing_running) && scr_get_op320->getValue() == SCR_MASTER_PAGE_TIME)
        scr_set_op320->setValue(SCR_MASTER_PAGE);
}

void FXCore::gotoMainScreen() {
    scr_set_op320->setValue(prog_running->getState() || is_task_flowing_running ? SCR_MASTER_PAGE_TIME : SCR_MASTER_PAGE);
}

void FXCore::checkDayFix()
{
    if (rtc_general_current->isAnotherDay(rtc_general_last_time_point))
    {
        if (prog_running->getState() &&
            rtc_prog_expected_finish->isBiggerThan(rtc_general_last_time_point) && 
            rtc_general_current->isBiggerThan(rtc_prog_expected_finish))
            rtc_prog_expected_finish->clone(rtc_general_current, CloneType::Date);

        if (is_task_flowing_running && 
            rtc_blowing_finish->isBiggerThan(rtc_general_last_time_point) &&
            rtc_general_current->isBiggerThan(rtc_blowing_finish))
            rtc_blowing_finish->clone(rtc_general_current, CloneType::Date);

        rtc_general_current->sendToEE(true);
        auto_prog_presets.resetCallFlags();
    }
}

void FXCore::setActivityPoint()
{
    checkDayFix();
    rtc_general_last_time_point->clone(rtc_general_current);
}

void FXCore::callTempCSensor()
{
    #if TEST_LOCAL == 0
        double response = (double)io_liquid_temp_s.readAnalog();
    #else
        double response = SENSOR_TEMPC_MINVAL + 10;
    #endif

    if (response < SENSOR_TEMPC_MINVAL || response > SENSOR_TEMPC_MAXVAL)
        sensor_tempC_error = true;
    else
    {
        sensor_tempC_error = false;
        sensor_call_tempC[sensor_call_index] =
            (response - SENSOR_TEMPC_MINVAL) /
            SENSOR_1TC_IN_MA_VAL((double)master_20ma_positive_limit->getUint16Value(), (double)master_4ma_negative_limit->getValue());
        
        if (sensor_call_tempC[sensor_call_index] > (double)master_4ma_negative_limit->getValue())
            sensor_call_tempC[sensor_call_index] -= (double)master_4ma_negative_limit->getValue();
        else
            sensor_call_tempC[sensor_call_index] = 0;

        ++sensor_call_index;

        #if TEST_LOCAL == 1
            liquid_tempC = (int16_t)(TEST_TEMPC_MAX - (((TEST_TEMPC_MAX / 3 * 2) / SENSOR_TEMPC_CALL_CNT) * (SENSOR_TEMPC_CALL_CNT - sensor_call_index - 1)));
        #endif 
    }
   
    if (sensor_call_index >= SENSOR_TEMPC_CALL_CNT)    
    {
        sensor_call_index = 0;
        double sum = 0;
        for (uint8_t index = 0; index < SENSOR_TEMPC_CALL_CNT; index++)
            sum += sensor_call_tempC[index];

        #if TEST_LOCAL == 0
            liquid_tempC = (int16_t)(sum / SENSOR_TEMPC_CALL_CNT);
        #endif
    }
}

/* public */
void FXCore::init()
{
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();
    delay(50);

    this->MBDispatcher::init();
    this->EEDispatcher::init();
    this->IODispatcher::init();
    delay(50);

    EEDispatcher::ee24c64.checkEECycle(true);

    machine_type = new SettingUnit(&ee_machine_type_selected, &mb_machine_type, static_cast<uint8_t>(MACHINE_TYPE::COUNT) - 1);
    delay(50);

    /* task var */
    machine_state = new SettingUnit(NULL, NULL, static_cast<uint8_t>(MACHINE_STATE::COUNT));
    prog_running = new SettingUnit(&ee_proc_pasteur_running, NULL, 1);
    prog_state = new SettingUnit(&ee_proc_pasteur_state, NULL, static_cast<uint8_t>(PROG_STATE::COUNT) - 1);
    prog_preset_selected = new SettingUnit(&ee_proc_pasteur_preset, NULL, 3);
    prog_need_in_freezing = new SettingUnit(&ee_proc_need_in_freezing, NULL, 1);
    prog_freezing_part_finished = new SettingUnit(&ee_proc_freezing_finished, NULL, 1),
    prog_need_in_heating = new SettingUnit(&ee_proc_need_in_heating, NULL, 1);
    prog_heating_part_finished = new SettingUnit(&ee_proc_heating_finished, NULL, 1),
    prog_jacket_filled = new SettingUnit(&ee_proc_waterJacket_filled_yet, NULL, 1);
    prog_pasteur_tempC_reached = new SettingUnit(&ee_proc_pasteurTempC_reached_yet, NULL, 1);
    prog_finished_flag = new SettingUnit(&ee_proc_finished_flag, NULL, static_cast<uint8_t>(FINISH_FLAG::COUNT) - 1);
    
    rtc_prog_pasteur_started = new TimeUnit(true);
    rtc_prog_pasteur_paused = new TimeUnit(true);
    rtc_prog_expected_finish = new TimeUnit(true);
    rtc_prog_finished = new TimeUnit(true);
    rtc_blowing_started = new TimeUnit(false);
    rtc_blowing_finish = new TimeUnit(false);

    rtc_prog_pasteur_started->setEEPointer(&ee_proc_started_hh, PointerType::Hours);
    rtc_prog_pasteur_started->setEEPointer(&ee_proc_started_mm, PointerType::Minutes);
    rtc_prog_pasteur_started->setEEPointer(&ee_proc_started_DD, PointerType::Days);
    rtc_prog_pasteur_started->setEEPointer(&ee_proc_started_MM, PointerType::Months);
    rtc_prog_pasteur_started->setEEPointer(&ee_proc_started_YY, PointerType::Years);
    rtc_prog_pasteur_started->setMBPointer(&mb_proc_start_hh, PointerType::Hours);
    rtc_prog_pasteur_started->setMBPointer(&mb_proc_start_mm, PointerType::Minutes);
    rtc_prog_pasteur_started->loadFromEE();

    rtc_prog_pasteur_paused->setEEPointer(&ee_proc_paused_hh, PointerType::Hours);
    rtc_prog_pasteur_paused->setEEPointer(&ee_proc_paused_mm, PointerType::Minutes);
    rtc_prog_pasteur_paused->setEEPointer(&ee_proc_paused_DD, PointerType::Days);
    rtc_prog_pasteur_paused->setEEPointer(&ee_proc_paused_MM, PointerType::Months);
    rtc_prog_pasteur_paused->setEEPointer(&ee_proc_paused_YY, PointerType::Years);
    rtc_prog_pasteur_paused->loadFromEE();

    rtc_prog_finished->setEEPointer(&ee_proc_finished_hh, PointerType::Hours);
    rtc_prog_finished->setEEPointer(&ee_proc_finished_mm, PointerType::Minutes);
    rtc_prog_finished->setEEPointer(&ee_proc_finished_DD, PointerType::Days);
    rtc_prog_finished->setEEPointer(&ee_proc_finished_MM, PointerType::Months);
    rtc_prog_finished->setEEPointer(&ee_proc_finished_YY, PointerType::Years);
    rtc_prog_finished->setMBPointer(&mb_proc_end_hh, PointerType::Hours);
    rtc_prog_finished->setMBPointer(&mb_proc_end_mm, PointerType::Minutes);
    prog_running->getState() ?
        rtc_prog_finished->setZeroDateTime() : 
        rtc_prog_finished->loadFromEE();

    rtc_prog_expected_finish->setEEPointer(&ee_proc_exp_finish_hh, PointerType::Hours);
    rtc_prog_expected_finish->setEEPointer(&ee_proc_exp_finish_mm, PointerType::Minutes);
    rtc_prog_expected_finish->setEEPointer(&ee_proc_exp_finish_DD, PointerType::Days);
    rtc_prog_expected_finish->setEEPointer(&ee_proc_exp_finish_MM, PointerType::Months);
    rtc_prog_expected_finish->setEEPointer(&ee_proc_exp_finish_YY, PointerType::Years);
    rtc_prog_expected_finish->setMBPointer(&mb_proc_end_hh, PointerType::Hours);
    rtc_prog_expected_finish->setMBPointer(&mb_proc_end_mm, PointerType::Minutes);
    rtc_prog_expected_finish->loadFromEE();

    rtc_blowing_started->setMBPointer(&mb_proc_start_hh, PointerType::Hours);
    rtc_blowing_started->setMBPointer(&mb_proc_start_mm, PointerType::Minutes);

    rtc_blowing_finish->setMBPointer(&mb_proc_end_hh, PointerType::Hours);
    rtc_blowing_finish->setMBPointer(&mb_proc_end_mm, PointerType::Minutes);
    delay(50);

    /* configs */
    scr_set_op320 = new SettingUnit(NULL, &mb_set_op320_scr, 0, 1, false);
    scr_get_op320 = new SettingUnit(NULL, &mb_get_op320_scr, 0, 1, false);

    info_main_liq_tempC = new SettingUnit(NULL, &mb_liq_tempC);
    info_main_batt_charge = new SettingUnit(NULL, &mb_batt_charge);
    info_main_process = new SettingUnit(NULL, &mb_proc_list, static_cast<uint8_t>(OP320_PROCESS::COUNT) - 1);
    info_main_step_show_hide = new SettingUnit(NULL, &mb_step_name_list, 1);
    info_main_step = new SettingUnit(NULL, &mb_step_index_list, static_cast<uint8_t>(OP320_STEP::COUNT) - 1);
    info_error_notify = new SettingUnit(NULL, &mb_notification_list, static_cast<uint8_t>(OP320_ERROR::COUNT) - 1);
    
    solo_heating_tempC = new SettingUnit(&ee_solo_heating_tempC, &mb_solo_heating_tempC);
    solo_freezing_tempC = new SettingUnit(&ee_solo_freezing_tempC, &mb_solo_freezing_tempC);
    
    calibration_flowgun_main_dose = new SettingUnit(NULL, &mb_blow_calib_dosage, 0, 100);
    calibration_flowgun_main_dose->setValue(3);
    calibration_flowgun_dope_dose = new SettingUnit(&ee_blowgun_calib_range, &mb_blow_calib_range, 99, 100);
    
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
    master_20ma_positive_limit = new SettingUnit(&ee_uint16_master_20ma_adc_value, &mb_20ma_adc_limit);
    master_calibr_side_toggle = new SettingUnit(&ee_master_calibr_side_toggle, &mb_master_calibr_display, 1);
    delay(50);
    
    flowgun_presets.init(
        &mb_blowing_preset_list,
        &mb_blowing_volume,
        &mb_blowing_incV,
        &mb_blowing_decV,
        ee_blowgun_preset_arr
    );
    delay(50);
    
    auto_prog_presets.init(
        ee_auto_pasteur_tempC_arr,
        ee_auto_heating_tempC_arr,
        ee_auto_freezing_tempC_arr,
        ee_auto_pasteur_duratMM_arr,
        ee_auto_run_on_hh_arr,
        ee_auto_run_on_mm_arr,
        ee_auto_run_toggle_arr,
        ee_auto_is_runned_today_arr,
        ee_auto_extra_tempC_arr,
        ee_auto_extra_toggle_arr,
        ee_auto_extra_run_hh_arr,
        ee_auto_extra_run_mm_arr,
        ee_auto_extra_runned_arr,
        &mb_auto_preset_list,
        &mb_auto_pasteur_tempC,
        &mb_auto_heating_tempC,
        &mb_auto_freezing_tempC,
        &mb_auto_durat_mm,
        &mb_auto_run_rtc_hh,
        &mb_auto_run_rtc_mm,
        &mb_auto_preset_toggle,
        &mb_auto_extra_heat_tempC,
        &mb_auto_extra_heat_toggle,
        &mb_auto_extra_heat_run_hh,
        &mb_auto_extra_heat_run_mm
    );
    delay(50);

    rtc_general_current = new TimeUnit(false);
    rtc_general_last_time_point = new TimeUnit(true);
    rtc_general_set_new = new TimeUnit(false);

    rtc_general_current->setMBPointer(&mb_rtc_ss, PointerType::Seconds);
    rtc_general_current->setMBPointer(&mb_rtc_mm, PointerType::Minutes);
    rtc_general_current->setMBPointer(&mb_rtc_hh, PointerType::Hours);
    rtc_general_current->setMBPointer(&mb_rtc_DD, PointerType::Days);
    rtc_general_current->setMBPointer(&mb_rtc_MM, PointerType::Months);
    rtc_general_current->setMBPointer(&mb_rtc_YY, PointerType::Years);
    rtc_general_current->setEEPointer(&ee_rtc_curr_day, PointerType::Days);
    rtc_general_current->setEEPointer(&ee_rtc_curr_month, PointerType::Months);
    rtc_general_current->setEEPointer(&ee_rtc_curr_year, PointerType::Years);
    rtc_general_current->setRealTime();

    rtc_general_last_time_point->setEEPointer(&ee_dynamic_rtc_curr_ss, PointerType::Seconds);
    rtc_general_last_time_point->setEEPointer(&ee_dynamic_rtc_curr_mm, PointerType::Minutes);
    rtc_general_last_time_point->setEEPointer(&ee_dynamic_rtc_curr_hh, PointerType::Hours);
    rtc_general_last_time_point->setEEPointer(&ee_rtc_curr_day, PointerType::Days);
    rtc_general_last_time_point->setEEPointer(&ee_rtc_curr_month, PointerType::Months);
    rtc_general_last_time_point->setEEPointer(&ee_rtc_curr_year, PointerType::Years);
    rtc_general_last_time_point->loadFromEE();
    rtc_general_last_time_point->setEEPointer(NULL, PointerType::Days);
    rtc_general_last_time_point->setEEPointer(NULL, PointerType::Months);
    rtc_general_last_time_point->setEEPointer(NULL, PointerType::Years);

    rtc_general_set_new->setMBPointer(&mb_rtc_new_ss, PointerType::Seconds);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_mm, PointerType::Minutes);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_hh, PointerType::Hours);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_DD, PointerType::Days);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_MM, PointerType::Months);
    rtc_general_set_new->setMBPointer(&mb_rtc_new_YY, PointerType::Years);
    rtc_general_set_new->setZeroDateTime();
    delay(50);

    /* op320 buttons trigger bind */
    mb_comm_stop_proc.addTrigger([this]()->             void { stopAllTasks(false); });
    mb_comm_goto_scr_blowing.addTrigger([this]()->      void { if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DM_flow) || machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow)) scr_set_op320->setValue(SCR_BLOWING_PAGE); });
    mb_comm_blow_preset_1.addTrigger([this]()->         void { flowgun_presets.selectPreset(0); });
    mb_comm_blow_preset_2.addTrigger([this]()->         void { flowgun_presets.selectPreset(1); });
    mb_comm_blow_preset_3.addTrigger([this]()->         void { flowgun_presets.selectPreset(2); });
    mb_comm_blow_preset_4.addTrigger([this]()->         void { flowgun_presets.selectPreset(3); });
    mb_comm_blow_vInc.addTrigger([this]()->             void { flowgun_presets.incValue(); });
    mb_comm_blow_vDec.addTrigger([this]()->             void { flowgun_presets.decValue(); });
    mb_comm_blow_prescaler.addTrigger([this]()->        void { flowgun_presets.changeScaler(); });
    mb_comm_solo_tempC_cancel.addTrigger([this]()->     void { solo_heating_tempC->refreshValue(); solo_freezing_tempC->refreshValue(); gotoMainScreen(); });
    mb_comm_solo_tempC_accept.addTrigger([this]()->     void { solo_heating_tempC->setValueByModbus(); solo_freezing_tempC->setValueByModbus(); });
    mb_comm_rtc_new_cancel.addTrigger([this]()->        void { rtc_general_set_new->setZeroDateTime(); scr_set_op320->setValue(SCR_USER_MENU); });
    mb_comm_rtc_new_accept.addTrigger([this]()->        void { FXCore::setNewDateTime(); });
    mb_comm_blow_calib_inc.addTrigger([this]()->        void { calibration_flowgun_dope_dose->incValue(); });
    mb_comm_blow_calib_dec.addTrigger([this]()->        void { calibration_flowgun_dope_dose->decValue(); });
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
    mb_comm_auto_preset_1.addTrigger([this]()->         void { auto_prog_presets.selectPreset(0); });
    mb_comm_auto_preset_2.addTrigger([this]()->         void { auto_prog_presets.selectPreset(1); });
    mb_comm_auto_preset_3.addTrigger([this]()->         void { auto_prog_presets.selectPreset(2); });
    mb_comm_auto_toggle.addTrigger([this]()->           void { auto_prog_presets.togglePreset(); });
    mb_comm_auto_accept.addTrigger([this]()->           void { auto_prog_presets.acceptChanges(); });
    mb_comm_auto_cancel.addTrigger([this]()->           void { auto_prog_presets.refreshPreset(); scr_set_op320->setValue(SCR_USER_MENU); });
    mb_comm_pass_7.addTrigger([this]()->                void { if (inputCode(7)) scr_set_op320->setValue(SCR_MASTER_SETTINGS); });
    mb_comm_pass_8.addTrigger([this]()->                void { if (inputCode(8)) scr_set_op320->setValue(SCR_MASTER_SETTINGS); });
    mb_comm_pass_9.addTrigger([this]()->                void { if (inputCode(9)) scr_set_op320->setValue(SCR_MASTER_SETTINGS); });
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
    mb_master_full_hard_reset.addTrigger([this]()->     void { hardReset(); });
    mb_comm_self_pasteur_start.addTrigger([this]()->    void { taskStartProg(0); });
    mb_comm_solo_heating_toggle.addTrigger([this]()->   void { taskTryToggleHeating(!is_task_heating_running); });
    mb_comm_solo_freezing_toggle.addTrigger([this]()->  void { taskTryToggleFreezing(!is_task_freezing_running); });
    //mb_comm_blowgun_run_btn.addTrigger([this]()->       void { is_flowing_call = true; });
    mb_comm_goto_scr_master.addTrigger([this]()->       void { gotoMainScreen(); });
    mb_master_machine_type_up.addTrigger([this]()->     void { stopAllTasks(false); machine_type->incValue(); });
    mb_master_machine_type_down.addTrigger([this]()->   void { stopAllTasks(false); machine_type->decValue(); });
    mb_comm_auto_extra_heat_toggle.addTrigger([this]()->void { auto_prog_presets.toggleExtraH(); });
    mb_master_calibr_toggle.addTrigger([this]()->       void { master_calibr_side_toggle->setValue(master_calibr_side_toggle->getValue() == 0 ? 1 : 0); });
    delay(50);

    /* self check after startup / before giving contorl to plc (tasks) */
    for (uint8_t index = 0; index < SENSOR_TEMPC_CALL_CNT; index++)
    {
        callTempCSensor();
        delay(10);
    }
    readSensors();
    readWaterInJacket();
    checkIsHardReseted();
    checkIsProgWasRunned();
    delay(50);

    /* tasks: millis, func. TaskManagerTick(uwTick) = 0 => set millis() to 0 (for Arduino) */
    uwTick = 0;
    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); readSensors(); });
    TaskManager::newTask(200,   [this]() -> void { rtc_general_current->setRealTime(); checkDayFix(); });
    TaskManager::newTask(300,   [this]() -> void { threadMain(); checkAutoStartup(); });
    TaskManager::newTask(500,   [this]() -> void { displayTasksDeadline(); });
    TaskManager::newTask(1000,  [this]() -> void { callTempCSensor(); displayMainInfoVars(); });
    TaskManager::newTask(5000,  [this]() -> void { if (scr_get_op320->getValue() == static_cast<uint8_t>(SCR_HELLO_PAGE)) gotoMainScreen(); readWaterInJacket(); flow_error_displayed_yet = false; });
    TaskManager::newTask(10000, [this]() -> void { setActivityPoint(); });
    TaskManager::newTask(30000, [this]() -> void { is_heaters_starters_state = !is_heaters_starters_state; start_error_displayed_yet = false; });
    TaskManager::newTask(60000, [this]() -> void { water_saving_toggle = !water_saving_toggle; });
    TaskManager::newTask(2 * 60 * 1000, [this]() -> void { EEDispatcher::ee24c64.checkEECycle(false); });

    delay(50);
    scr_set_op320->setValue(SCR_HELLO_PAGE);
}

void FXCore::setNewDateTime()
{
    rtc_general_set_new->loadFromMB();
    rtc.setDate(rtc_general_set_new->getDays(), rtc_general_set_new->getMonths(), rtc_general_set_new->getYears());
    rtc.setTime(rtc_general_set_new->getHours(), rtc_general_set_new->getMins(), rtc_general_set_new->getSecs());
    rtc_general_set_new->setZeroDateTime();
    rtc_general_current->sendToEE(true);
    rtc_general_last_time_point->clone(rtc_general_current);
}

void FXCore::stopAllTasks(bool stop_presets)
{
    is_stop_btn_pressed = false;
    if (prog_running->getState())
        taskFinishProg(FINISH_FLAG::UserCall);

    if (stop_presets)
        checkAutoStartup(true);
        
    taskFinishFlowing();
    is_task_heating_extra = false;
    taskTryToggleHeating(false);
    taskTryToggleFreezing(false);
    taskToggleMixer(false);
}

void FXCore::taskTryToggleFlowing()
{
    if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DM_flow) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow))
    {
        if (!is_task_flowing_running)
        {
            rtc_blowing_started->setRealTime();
            rtc_blowing_finish->setRealTime();
            FlowgunTick = 0;
            flowing_current_ms = 0;

            if (!flowgun_presets.isWashingSelected())
            {
                uint16_t flowing_dose =
                    flowgun_presets.getValue() +
                    ((double)flowgun_presets.getValue() / (double)((uint16_t)calibration_flowgun_main_dose->getValue() * 1000)) *
                    (double)(calibration_flowgun_main_dose->getValue() * 100);
                flowing_trigger_ms = 60000 * flowing_dose / ((uint16_t)master_pump_LM_performance->getValue() * 1000);
            }
            else if (flowgun_presets.isWashingSelected())
                flowing_trigger_ms = flowgun_presets.getValue() * 1000;

            is_task_flowing_running = true;
            rtc_blowing_finish->addSeconds(flowing_trigger_ms / 1000); 
            io_blowgun_r.write(true);
        }
    }
}

void FXCore::taskTryToggleFreezing(bool turn_on)
{
    if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::PM) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow))
    {
        if (!sensor_tempC_error)
        {
            is_task_heating_extra = false;

            if (!prog_running->getState() && is_connected_380V)
            {
                is_task_freezing_running = turn_on;

                if (turn_on && is_task_heating_running)
                    taskTryToggleHeating(false);

                if (!is_task_freezing_running && !is_task_heating_running && is_task_heating_extra)
                {
                    io_heater_r.write(false);
                    io_water_jacket_r.write(false);
                    taskToggleMixer(false);
                }
            }
        }
        
        if (!prog_running->getState() && (!is_connected_380V || sensor_tempC_error)) 
        {
            is_task_freezing_running = false;
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            taskToggleMixer(false);

            if (turn_on)
            {
                if (!sensor_tempC_error)
                    info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vOut));
                else
                    info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::TempCSensorError));
                scr_set_op320->setValue(SCR_ERROR_NOTIFY);
            }
        }
        
    }
}

void FXCore::taskTryToggleHeating(bool turn_on)
{
    if (machine_type->getValue() != static_cast<uint8_t>(MACHINE_TYPE::DM_flow))
    {
        if (!sensor_tempC_error)
        {
            is_task_heating_extra = false;

            if (!prog_running->getState() && is_connected_380V)
            {
                is_task_heating_running = turn_on;
                if (turn_on && is_task_freezing_running)
                    taskTryToggleFreezing(false);

                if (!turn_on)
                    taskHeating(0);

                if (!is_task_heating_running && !is_task_freezing_running && !is_task_heating_extra)
                {
                    io_heater_r.write(false);
                    io_water_jacket_r.write(false);
                    taskToggleMixer(false);
                }
            }
        }
        
        if (!prog_running->getState() && (!is_connected_380V || sensor_tempC_error)) 
        {
            is_task_heating_running = false;
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            taskToggleMixer(false);

            if (turn_on)
            {
                if (!sensor_tempC_error)
                    info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vOut));
                else
                    info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::TempCSensorError));
                scr_set_op320->setValue(SCR_ERROR_NOTIFY);
            }
        }
    }
}

void FXCore::taskToggleMixer(bool turn_on)
{
    if (turn_on && (machine_state->getValue() > static_cast<uint8_t>(MACHINE_STATE::Flowing) || prog_running->getState()) && is_connected_380V)
        io_mixer_r.write(true);
    else
        io_mixer_r.write(false);
}

bool FXCore::taskStartProg(uint8_t pasteur_preset)
{
    if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::PM))
    {
        if (!is_connected_380V || sensor_tempC_error)
        {
            if (!start_error_displayed_yet || pasteur_preset == 0)
            {
                info_error_notify->setValue(static_cast<uint8_t>(!is_connected_380V ? OP320_ERROR::Power380vOut : OP320_ERROR::TempCSensorError));
                scr_set_op320->setValue(SCR_ERROR_NOTIFY);
            }
            start_error_displayed_yet = true;
            return false;
        }

        if (prog_running->getState())
            return false;
        else
            stopAllTasks(false);

        is_pasteur_part_finished_crutch = false;
        start_error_displayed_yet = false;
        heat_error_displayed_yet = false;
        prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurRunning));
        prog_freezing_part_finished->setValue(0);
        prog_heating_part_finished->setValue(0);
        prog_preset_selected->setValue(pasteur_preset);
        prog_jacket_filled->setValue(0);
        prog_pasteur_tempC_reached->setValue(0);
        prog_running->setValue(1);

        gotoMainScreen();

        rtc_prog_pasteur_started->clone(rtc_general_current);
        rtc_prog_expected_finish->clone(rtc_general_current);
        rtc_prog_finished->setZeroDateTime();
        rtc_prog_pasteur_paused->setZeroDateTime();

        if (prog_preset_selected->getValue() == 0)
        {
            prog_pasteur_tempC = self_prog_pasteur_tempC->getValue();
            prog_heating_tempC = self_prog_heating_tempC->getValue();
            prog_freezing_tempC = self_prog_freezing_tempC->getValue();
            prog_need_in_freezing->setValue(self_prog_mode->getValue() > 0 ? 1 : 0);
            prog_need_in_heating->setValue(self_prog_mode->getValue() > 1 ? 1 : 0);
            prog_selected_duration_mm = self_prog_duration_mm->getValue();
        }
        else
        {
            auto_prog_presets.startPreset(
                prog_preset_selected->getValue() - 1,
                prog_pasteur_tempC,
                prog_heating_tempC,
                prog_freezing_tempC,
                prog_selected_duration_mm
                );
            prog_need_in_freezing->setValue(1);
            prog_need_in_heating->setValue(1);
        }
        uint16_t heating_up_estimated_mm = (prog_pasteur_tempC > liquid_tempC ? prog_pasteur_tempC - liquid_tempC : 1) * HEAT_MM_FOR_1CELSIUS_UP;
        rtc_prog_expected_finish->addMinutes(
            WATER_JACKED_FILL_EST_MM +
            heating_up_estimated_mm + 
            prog_selected_duration_mm
        );

        is_freezing_part_finished_crutch = !prog_need_in_freezing->getState();
        is_heating_part_finished_crutch = !prog_need_in_heating->getState();
        taskToggleMixer(true);

        return true;
    }
    return false;
}

void FXCore::taskPauseProg(OP320_ERROR flag)
{
    if (prog_running->getState())
    {
        prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurPaused));
        rtc_prog_finished->loadFromMB();
        rtc_prog_pasteur_paused->clone(rtc_general_current);

        io_heater_r.write(false);
        io_mixer_r.write(false);
        if (!is_water_in_jacket)
            io_water_jacket_r.write(true);

        info_error_notify->setValue(static_cast<uint8_t>(flag));
        scr_set_op320->setValue(SCR_ERROR_NOTIFY);
    }
}

void FXCore::taskResumeProg()
{
    if (prog_running->getState())
    {
        prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurRunning));
        uint64_t estimated_sec = rtc_prog_expected_finish->getDiffSec(rtc_prog_pasteur_paused);
        rtc_prog_expected_finish->clone(rtc_general_current);
        rtc_prog_expected_finish->addMinutes(estimated_sec / 60);
        rtc_prog_pasteur_paused->setZeroDateTime();

        io_mixer_r.write(true);
        io_water_jacket_r.write(false);
    }
}

void FXCore::taskFinishProg(FINISH_FLAG flag)
{
    if (prog_running->getState())
    {
        prog_running->setValue(0);
        prog_state->setValue(static_cast<uint8_t>(PROG_STATE::CycleFinished));

        is_pasteur_part_finished_crutch = false;
        is_heating_part_finished_crutch = false;
        is_freezing_part_finished_crutch = false;
        
        taskTryToggleHeating(false);
        taskTryToggleFreezing(false);
        taskToggleMixer(false);
        
        rtc_prog_finished->clone(rtc_general_current);

        if (flag > FINISH_FLAG::UserCall)
        {
            OP320_ERROR to_op320;
            switch(flag)
            {
                case FINISH_FLAG::MixerError: to_op320 = OP320_ERROR::Mixer; break;
                case FINISH_FLAG::Power380vError: to_op320 = OP320_ERROR::PowerMoreHour; break;
                case FINISH_FLAG::WaterJacketError: to_op320 = OP320_ERROR::WaterMoreHour; break;
            }
            info_error_notify->setValue(static_cast<uint8_t>(to_op320));
            scr_set_op320->setValue(SCR_ERROR_NOTIFY);
        }
    }
}

bool FXCore::taskFinishFlowing(bool forced)
{
    if (forced)
    {
        flowgunOff();
        return true;
    }

    is_flowing_call = io_blowgun_s.readDigital();

    if (is_task_flowing_running && !is_flowing_call && !flowing_task_paused)
    {
        io_blowgun_r.write(false);
        flowing_current_ms = FlowgunTick;
        flowing_task_paused = true;
        return false;
    }
    else if (is_task_flowing_running && !is_flowing_call && flowing_task_paused)
    {
        if ((FlowgunTick - flowing_current_ms) / 1000 >= 10)
            flowgunOff();

        return false;
    }
    else if (is_task_flowing_running && is_flowing_call && flowing_task_paused)
    {
        FlowgunTick = flowing_current_ms;
        rtc_blowing_finish->setRealTime();
        rtc_blowing_finish->addSeconds((flowing_trigger_ms - flowing_current_ms) / 1000);
        flowing_task_paused = false;
        io_blowgun_r.write(true);
    }

    if (is_task_flowing_running && is_flowing_call && !flowing_task_paused)
        flowing_current_ms = FlowgunTick;

    if (flowing_current_ms >= flowing_trigger_ms)
    {
        flowgunOff();
        return true;
    }
    return false;
}

void FXCore::flowgunOff()
{
    is_task_flowing_running = false;
    io_blowgun_r.write(false);

    if (is_flowing_call = io_blowgun_s.readDigital())
        is_flowing_uncalled = true;
}

bool FXCore::taskHeating(uint8_t expected_tempC)
{
    if (expected_tempC == 0)
    {
        if (!is_heaters_starters_state)
            io_heater_r.write(false);
        return false;
    }

    if (is_task_heating_running || is_task_heating_extra || prog_running->getState())
    {
        taskToggleMixer(true);

        if (!is_water_in_jacket)
            io_water_jacket_r.write(true);
        else
        {
            io_water_jacket_r.write(false);
          
            if (liquid_tempC + (master_hysteresis_toggle->getState() ? master_hysteresis_tempC->getValue() : 0) < expected_tempC)
            {
                if (is_heaters_starters_state)
                    io_heater_r.write(true);
            }
            else if (is_task_heating_extra || prog_running->getState())
            {
                if (!is_heaters_starters_state)
                    io_heater_r.write(false);
            }
            else
            {
                if (!is_heaters_starters_state)
                    io_heater_r.write(false);
                taskTryToggleHeating(false);
                return true;
            }
        }
    }
    else
    {
        taskToggleMixer(false);
        if (!is_heaters_starters_state)
            io_heater_r.write(false);
    }

    return true;
}

bool FXCore::taskFreezing(uint8_t expected_tempC)
{
    if (expected_tempC == 0)
    {
        io_water_jacket_r.write(false);
        return false;
    }

    if (prog_running->getState() || is_task_freezing_running)
    {
        taskToggleMixer(true);
        if (!is_heaters_starters_state)
            io_heater_r.write(false);

        if (liquid_tempC > expected_tempC)
        {
            if (master_water_saving_toggle->getState())
                io_water_jacket_r.write(water_saving_toggle);
            else
                io_water_jacket_r.write(true);
        }
        else if (prog_running->getState())
        {
            io_water_jacket_r.write(false);
        }
        else
        {
            io_water_jacket_r.write(false);
            taskTryToggleFreezing(false);
        }
    }
    else
    {
        io_water_jacket_r.write(false);
        taskToggleMixer(false);
    }
    
    return true;
}

bool FXCore::checkAutoStartup(bool force_off)
{
    uint8_t preset_id = 0;
    bool in_range = false;
    if (auto_prog_presets.isTimeToRunPreset(preset_id, *rtc_general_current, in_range))
    {
        if (in_range && !force_off && !prog_running->getState())
        {
            taskStartProg(preset_id);
            return true;
        }
        else
        {
            auto_prog_presets.markPresetAsCompleted(preset_id - 1);
            return false;
        }
    }

    preset_id = 0;
    in_range = false;
    if (auto_prog_presets.isTimeToRunExtra(preset_id, *rtc_general_current, in_range))
    {
        if (in_range && !force_off && !prog_running->getState() && !is_task_heating_running && !is_task_freezing_running && !is_task_heating_extra)
        {
            is_task_heating_extra = true;
            extra_heating_tempC = auto_prog_presets.startExtraHeat(preset_id - 1);
            return true;
        }
        else
        {
            auto_prog_presets.markExtraAsCompleted(preset_id - 1);
            return false;
        }
    }

    return false;
}

bool FXCore::threadProg()
{
    if (!is_pasteur_part_finished_crutch)
    {
        if (is_mixer_error)
        {
            taskFinishProg(FINISH_FLAG::MixerError);
            return false;
        }

        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
            if (rtc_general_current->getDiffSec(rtc_prog_pasteur_paused) >= PASTEUR_AWAIT_LIMIT_MM * 60)
            {
                taskFinishProg(!is_connected_380V ? FINISH_FLAG::Power380vError : FINISH_FLAG::WaterJacketError);
                return false;
            }

        bool WJacketErr = prog_jacket_filled->getState() ? !is_water_in_jacket : false;
        if (!is_connected_380V || WJacketErr)
        {
            if (!is_connected_380V)
            {
                if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
                    taskPauseProg(OP320_ERROR::Power380vOut);
                return false;
            }
            else if (WJacketErr)
            {
                if (rtc_prog_pasteur_paused->isZeroTime())
                    rtc_prog_pasteur_paused->clone(rtc_general_current);

                if (rtc_general_current->getDiffSec(rtc_prog_pasteur_paused) / 60 >= 5 &&
                    prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
                    taskPauseProg(OP320_ERROR::WaterAwait);

                if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
                    return false;
            }
        }

        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
        {
            if (is_connected_380V && !WJacketErr)
                taskResumeProg();
            else
                return false;
        }

        if (!WJacketErr && prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
            rtc_prog_pasteur_paused->setZeroDateTime();

        if (!is_water_in_jacket)
        {
            io_water_jacket_r.write(true);

            if (!prog_jacket_filled->getState())
                return true;
        }
        else
        {
            if (!prog_jacket_filled->getState())
                prog_jacket_filled->setValue(1);

            io_water_jacket_r.write(false);
        }

        taskHeating(prog_pasteur_tempC);

        if (liquid_tempC >= prog_pasteur_tempC && !prog_pasteur_tempC_reached->getState())
        {
            prog_pasteur_tempC_reached->setValue(1);
            rtc_prog_expected_finish->clone(rtc_general_current);
            rtc_prog_expected_finish->addMinutes(prog_selected_duration_mm);
        }

        if (rtc_general_current->isBiggerThan(rtc_prog_expected_finish))
        {
            if (prog_pasteur_tempC_reached->getState())
            {
                prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurFinished));
                is_pasteur_part_finished_crutch = true;
                rtc_prog_expected_finish->setZeroDateTime();
            }
            else
            {
                rtc_prog_expected_finish->addMinutes((prog_pasteur_tempC - liquid_tempC) * HEAT_MM_FOR_1CELSIUS_UP);
                if (!heat_error_displayed_yet)
                {
                    heat_error_displayed_yet = true;
                    info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::SlowHeating));
                    scr_set_op320->setValue(SCR_ERROR_NOTIFY);
                }
            }
        }

        return true;
    }

    if (!is_freezing_part_finished_crutch)
    {
        taskHeating(0);
        taskFreezing(prog_freezing_tempC);
        if (liquid_tempC <= prog_freezing_tempC)
        {
            prog_freezing_part_finished->setValue(1);
            is_freezing_part_finished_crutch = true;
            prog_state->setValue(static_cast<uint8_t>(PROG_STATE::FreezingFinished));
        }
        else
            return true;
    }

    if (!is_heating_part_finished_crutch)
    {
        taskFreezing(0);
        taskHeating(prog_heating_tempC);
        
        if (liquid_tempC >= prog_heating_tempC)
        {
            prog_heating_part_finished->setValue(1);
            is_heating_part_finished_crutch = true;
            prog_state->setValue(static_cast<uint8_t>(PROG_STATE::HeatingFinished));
        }
        else
            return true;
    }

    taskFinishProg(FINISH_FLAG::Success);

    if (prog_need_in_heating->getState())
    {
        is_task_heating_extra = true;
        extra_heating_tempC = prog_heating_tempC;
    }

    return true;
}

void FXCore::threadMain()
{
    if (!is_task_flowing_running)
        FlowgunTick = 0;

    if (sensor_tempC_error && (prog_running->getState() || is_task_heating_running || is_task_freezing_running || is_task_heating_extra))
    {
        stopAllTasks(true);
        info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::TempCSensorError));
        scr_set_op320->setValue(SCR_ERROR_NOTIFY);
    }

    scr_get_op320->setValueByModbus();

    if (prog_running->getState())
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Pasteurizing));
    else if (is_task_freezing_running)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Freezing));
    else if (is_task_heating_running || is_task_heating_extra)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Heating));
    else if (is_task_flowing_running)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Flowing));
    else
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Await));

    if (is_flowing_call && scr_get_op320->getValue() == SCR_BLOWING_PAGE)
    {
        if (!is_connected_380V && !prog_running->getState() && !is_task_flowing_running && !is_flowing_uncalled &&!is_stop_btn_pressed)
            taskTryToggleFlowing();
        else if (!flow_error_displayed_yet && !is_flowing_uncalled && !is_task_flowing_running)
        {
            flow_error_displayed_yet = true;
            if (is_connected_380V)
                info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vIn));
            scr_set_op320->setValue(SCR_ERROR_NOTIFY);
        }
    }

    if (is_stop_btn_pressed)
        stopAllTasks(true);

    if (prog_running->getState())
        threadProg();

    if (is_task_heating_running)
        taskHeating(solo_heating_tempC->getValue());
    else if (is_task_heating_extra)
        taskHeating(extra_heating_tempC);
    else if (!prog_running->getState())
        taskHeating(0);

    if (is_task_freezing_running)
        taskFreezing(solo_freezing_tempC->getValue());

    if (!is_task_heating_running &&
        !is_task_heating_extra &&
        !is_task_freezing_running &&
        !prog_running->getState())
    {
        taskToggleMixer(false);
        io_water_jacket_r.write(false);
    }

    if (is_task_flowing_running)
        taskFinishFlowing(false);

    displayOP320States();
}

void FXCore::readSensors()
{
    is_flowing_call = io_blowgun_s.readDigital();//is_flowing_call ? true : io_blowgun_s.readDigital();
    if (is_flowing_uncalled && !is_flowing_call)
        is_flowing_uncalled = false;

    uint16_t response = io_battery_s.readAnalog();
    batt_chargeV = response <= SENSOR_CHARGE_MINVAL ? 0 :
        (response >= SENSOR_CHARGE_MAXVAL ? 100 :
        ((SENSOR_CHARGE_RANGE - (SENSOR_CHARGE_MAXVAL - (double)response)) / (SENSOR_CHARGE_RANGE / 100)));
    
    #if TEST_LOCAL == 0
        is_stop_btn_pressed = is_stop_btn_pressed ? true : io_stop_btn_s.readDigital();
        is_connected_380V = io_v380_s.readDigital();
        is_mixer_error = io_mixer_crash_s.readDigital();
    #else
        is_stop_btn_pressed = false;
        is_connected_380V = true;
        is_mixer_error = false;
    #endif

    /* readed by readWaterInJacket() in new task every 5 sec
    is_water_in_jacket = io_water_jacket_s.readDigital();*/
}

void FXCore::readWaterInJacket() {
    #if TEST_LOCAL == 0
        is_water_in_jacket = io_water_jacket_s.readDigital();
    #else
        is_water_in_jacket = true;
    #endif
}

void FXCore::displayOP320States()
{
    if (!prog_running->getState() &&
        info_main_step->getValue() == static_cast<uint8_t>(OP320_STEP::PasteurFinish) &&
        rtc_general_current->getDiffSec(rtc_prog_finished) >= 5 * 60 &&
        !rtc_prog_finished->isZeroTime())
    {
        info_main_step_show_hide->setValue(0);
        info_main_step->setValue(0);
    }

    switch (static_cast<MACHINE_STATE>(machine_state->getValue()))
    {
    case MACHINE_STATE::Flowing: {
        info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::Washing));
    } break;

    case MACHINE_STATE::Freezing: {
        info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::Freezing));
        info_main_step_show_hide->setValue(1);
        info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::WaterJCirculation));
    } break;

    case MACHINE_STATE::Heating: {
        info_main_process->setValue(static_cast<uint8_t>(is_task_heating_running ? OP320_PROCESS::Heating : OP320_PROCESS::AutoHeating));
        info_main_step_show_hide->setValue(1);
        if (!is_water_in_jacket)
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::WaterJacket));
        else if (is_task_heating_running ? liquid_tempC < solo_heating_tempC->getValue() : liquid_tempC < extra_heating_tempC)
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::PasteurHeating));
        else
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::HeatingTo));
    } break;

    case MACHINE_STATE::Pasteurizing: {
        info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::PasteurSelf) + prog_preset_selected->getValue());
        info_main_step_show_hide->setValue(1);
        
        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
        {
            if (!prog_jacket_filled->getState())
                info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::WaterJacket));
            else if (prog_jacket_filled->getState() && liquid_tempC < prog_pasteur_tempC)
                info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::PasteurHeating));
            else
                info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::PasteurProc));
        }
        
        else if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::ErrSolveAwait));
        
        else if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurFinished))
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::FreezingTo));
        
        else if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::FreezingFinished))
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::HeatingTo));
        
        else 
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::PasteurFinish));
    } break;

    default: {
        if (info_main_step->getValue() != static_cast<uint8_t>(OP320_STEP::PasteurFinish))
        {
            info_main_step_show_hide->setValue(0);
            info_main_step->setValue(0);
        }

        if (is_connected_380V)
            info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::Chargering));
        else
            info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::Await));
    } break;
    }
}

void FXCore::hardReset()
{
    stopAllTasks(true);
    ee_hard_reset_value_setted.writeEE(1);
    rtc_general_current->sendToEE(true);
    rtc_general_last_time_point->clone(rtc_general_current);
    machine_type->setValue(static_cast<uint8_t>(MACHINE_TYPE::DMP_flow));
    solo_heating_tempC->setValue(45);
    solo_freezing_tempC->setValue(40);
    calibration_flowgun_dope_dose->setValue(0);
    self_prog_pasteur_tempC->setValue(65);
    self_prog_heating_tempC->setValue(45);
    self_prog_freezing_tempC->setValue(42);
    self_prog_duration_mm->setValue(30);
    self_prog_mode->setValue(1);
    master_water_saving_toggle->setValue(0);
    master_hysteresis_toggle->setValue(1);
    master_hysteresis_tempC->setValue(1);
    master_calibr_side_toggle->setValue(1);
    master_pump_LM_performance->setValue(38);
    master_4ma_negative_limit->setValue(50);
    master_20ma_positive_limit->setValue(150);
    flowgun_presets.setDefault();
    flowgun_presets.selectPreset(0);
    auto_prog_presets.setDefault();
    auto_prog_presets.selectPreset(0);
}