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
        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
        {
            if (rtc_general_current->getDiffMin(rtc_prog_pasteur_paused) >= PASTEUR_AWAIT_LIMIT_MM)
            {
                taskFinishProg(!is_water_in_jacket ? FINISH_FLAG::WaterJacketError : FINISH_FLAG::Power380vError);
                return false;
            }
        }
        else if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
        {
            if (rtc_general_current->getDiffMin(rtc_general_last_time_point) >= PASTEUR_AWAIT_LIMIT_MM)
            {
                taskFinishProg(FINISH_FLAG::Power380vError);
                return false;
            }
            else
            {
                uint8_t duration = rtc_prog_expected_finish->getDiffMin(rtc_general_last_time_point);
                rtc_prog_expected_finish->clone(rtc_general_current);
                rtc_prog_expected_finish->addMinutes(duration);
            }
        }

        if (prog_preset_selected->getValue() == 0)
        {
            prog_pasteur_tempC = self_prog_pasteur_tempC->getValue();
            prog_heating_tempC = self_prog_heating_tempC->getValue();
            prog_freezing_tempC = self_prog_freezing_tempC->getValue();
        }
        else
            auto_prog_presets->resumePreset(
                prog_preset_selected->getValue() - 1,
                prog_pasteur_tempC,
                prog_heating_tempC,
                prog_freezing_tempC
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
    if ((prog_running->getState() || is_task_washing_running) && scr_get_op320->getValue() == SCR_MASTER_PAGE)
        scr_set_op320->setValue(SCR_MASTER_PAGE_TIME);
    else if (!(prog_running->getState() || is_task_washing_running) && scr_get_op320->getValue() == SCR_MASTER_PAGE_TIME)
        scr_set_op320->setValue(SCR_MASTER_PAGE);
}

void FXCore::gotoMainScreen() {
    scr_set_op320->setValue(prog_running->getState() || is_task_washing_running ? SCR_MASTER_PAGE_TIME : SCR_MASTER_PAGE);
}

void FXCore::checkDayFix()
{
    if (rtc_general_current->isAnotherDay(rtc_general_last_time_point))
    {
        if (prog_running->getState() &&
            rtc_prog_expected_finish->isBiggerThan(rtc_general_last_time_point) && 
            rtc_general_current->isBiggerThan(rtc_prog_expected_finish))
            rtc_prog_expected_finish->clone(rtc_general_current, CloneType::Date);

        if (is_task_washing_running && 
            rtc_blowing_finish->isBiggerThan(rtc_general_last_time_point) &&
            rtc_general_current->isBiggerThan(rtc_blowing_finish))
            rtc_blowing_finish->clone(rtc_general_current, CloneType::Date);

        rtc_general_current->sendToEE(true);
        auto_prog_presets->newDay();
    }
}

void FXCore::setActivityPoint()
{
    checkDayFix();
    rtc_general_last_time_point->clone(rtc_general_current);
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

    EEDispatcher::ee24c64.checkEECycle(true);
    delay(50);

    machine_type = new SettingUnit(&ee_machine_type_selected, &mb_machine_type, static_cast<uint8_t>(MACHINE_TYPE::COUNT) - 1);

    /* task var */
    delay(50);
    machine_state = new SettingUnit(NULL, NULL, static_cast<uint8_t>(MACHINE_STATE::COUNT));
    prog_running = new SettingUnit(&ee_proc_pasteur_running, NULL, 1);
    prog_state = new SettingUnit(&ee_proc_pasteur_state, NULL, static_cast<uint8_t>(PROG_STATE::COUNT) - 1);
    prog_preset_selected = new SettingUnit(&ee_proc_pasteur_preset, NULL, 3);
    prog_need_in_freezing = new SettingUnit(&ee_proc_need_in_freezing, NULL, 1);
    prog_freezing_part_finished = new SettingUnit(&ee_proc_freezing_finished, NULL, 1),
    prog_need_in_heating = new SettingUnit(&ee_proc_need_in_heating, NULL, 1);
    prog_heating_part_finished = new SettingUnit(&ee_proc_heating_finished, NULL, 1),
    prog_jacket_filled = new SettingUnit(&ee_proc_waterJacket_filled_yet, NULL, 1);
    prog_finished_flag = new SettingUnit(&ee_proc_finished_flag, NULL, static_cast<uint8_t>(FINISH_FLAG::COUNT) - 1);
    
    rtc_prog_pasteur_started = new TimeUnit(true);
    rtc_prog_pasteur_paused = new TimeUnit(true);
    rtc_prog_expected_finish = new TimeUnit(true);
    rtc_prog_finished = new TimeUnit(true);
    rtc_blowing_started = new TimeUnit(false);
    rtc_blowing_finish = new TimeUnit(false);

    delay(50);
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

    /* configs */
    delay(50);
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
    master_20ma_positive_limit = new SettingUnit(&ee_master_20ma_adc_value, &mb_20ma_adc_limit);
    
    delay(50);
    flowgun_presets = new BlowingPreset(
        &mb_blowing_preset_list,
        &mb_blowing_volume,
        &mb_blowing_incV,
        &mb_blowing_decV,
        ee_blowgun_preset_arr
    );
    
    delay(50);
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
    rtc_general_last_time_point = new TimeUnit(true);
    rtc_general_set_new = new TimeUnit(false);

    delay(50);
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

    /* op320 buttons trigger bind */
    delay(50);
    mb_comm_stop_proc.addTrigger([this]()->             void { stopAllTasks(); });
    mb_comm_goto_scr_blowing.addTrigger([this]()->      void { if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DM_flow) || machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow)) scr_set_op320->setValue(SCR_BLOWING_PAGE); });
    mb_comm_blow_preset_1.addTrigger([this]()->         void { flowgun_presets->selectPreset(0); });
    mb_comm_blow_preset_2.addTrigger([this]()->         void { flowgun_presets->selectPreset(1); });
    mb_comm_blow_preset_3.addTrigger([this]()->         void { flowgun_presets->selectPreset(2); });
    mb_comm_blow_preset_4.addTrigger([this]()->         void { flowgun_presets->selectPreset(3); });
    mb_comm_blow_vInc.addTrigger([this]()->             void { flowgun_presets->incValue(); });
    mb_comm_blow_vDec.addTrigger([this]()->             void { flowgun_presets->decValue(); });
    mb_comm_blow_prescaler.addTrigger([this]()->        void { flowgun_presets->changeScaler(); });
    mb_comm_solo_tempC_cancel.addTrigger([this]()->     void { solo_heating_tempC->refreshValue(); solo_freezing_tempC->refreshValue(); });
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
    mb_comm_auto_preset_1.addTrigger([this]()->         void { auto_prog_presets->selectPreset(0); });
    mb_comm_auto_preset_2.addTrigger([this]()->         void { auto_prog_presets->selectPreset(1); });
    mb_comm_auto_preset_3.addTrigger([this]()->         void { auto_prog_presets->selectPreset(2); });
    mb_comm_auto_toggle.addTrigger([this]()->           void { auto_prog_presets->togglePreset(); });
    mb_comm_auto_accept.addTrigger([this]()->           void { auto_prog_presets->acceptChanges(); });
    mb_comm_auto_cancel.addTrigger([this]()->           void { auto_prog_presets->refreshPreset(); scr_set_op320->setValue(SCR_USER_MENU); });
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
    mb_comm_blowgun_run_btn.addTrigger([this]()->       void { is_flowing_call = true; });
    mb_comm_goto_scr_master.addTrigger([this]()->       void { gotoMainScreen(); });
    mb_master_machine_type_up.addTrigger([this]()->     void { stopAllTasks(); machine_type->incValue(); });
    mb_master_machine_type_down.addTrigger([this]()->   void { stopAllTasks(); machine_type->decValue(); });

    /* self check after startup / before giving contorl to plc (tasks) */
    delay(50);
    readSensors();
    checkIsHardReseted();
    checkIsProgWasRunned();

    /* tasks: millis, func */
    delay(50);
    uwTick = 0;
    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); readSensors(); });
    TaskManager::newTask(200,   [this]() -> void { rtc_general_current->setRealTime(); checkDayFix(); });
    TaskManager::newTask(300,   [this]() -> void { threadMain(); checkAutoStartup(); });
    TaskManager::newTask(500,   [this]() -> void { displayTasksDeadline(); });
    TaskManager::newTask(1000,  [this]() -> void { displayMainInfoVars(); });
    TaskManager::newTask(5000,  [this]() -> void { if (scr_get_op320->getValue() == static_cast<uint8_t>(SCR_HELLO_PAGE)) gotoMainScreen(); });
    TaskManager::newTask(10000, [this]() -> void { setActivityPoint(); });
    TaskManager::newTask(30000, [this]() -> void { is_heaters_starters_available = true; });
    TaskManager::newTask(2 * 60 * 1000, [this]() -> void { EEDispatcher::ee24c64.checkEECycle(false); });
    TaskManager::newTask(5 * 60 * 1000, [this]() -> void { start_error_displayed_yet = false; });

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

void FXCore::stopAllTasks()
{
    is_stop_btn_pressed = false;
    if (prog_running->getState())
        taskFinishProg(FINISH_FLAG::UserCall);
        
    taskFinishFlowing();
    taskTryToggleHeating(false);
    taskTryToggleFreezing(false);
    taskToggleMixer(false);
}

void FXCore::taskTryToggleFlowing()
{
    is_flowing_call = false;

    if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DM_flow) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow))
    {
        if (!flowgun_presets->isWashingSelected())
        {
            if (is_task_washing_running)
            {
                taskFinishFlowing();
                delay(500);
            }

            io_blowgun_r.write(true);

            delay(60000 * flowgun_presets->getValue() / (master_pump_LM_performance->getValue() * 1000));
            taskFinishFlowing();
        }
        else
        {
            if (!is_task_washing_running)
            {
                is_task_washing_running = true;
                rtc_blowing_started->clone(rtc_general_current);
                rtc_blowing_finish->clone(rtc_general_current);
                rtc_blowing_finish->addMinutes(flowgun_presets->getValue() / 60);
                io_blowgun_r.write(true);
            }
        }
    }
}

void FXCore::taskTryToggleFreezing(bool turn_on)
{
    if (machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::PM) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP) ||
        machine_type->getValue() == static_cast<uint8_t>(MACHINE_TYPE::DMP_flow))
    {
        if (!prog_running->getState() && is_connected_380V)
        {
            is_task_freezing_running = turn_on;

            if (turn_on && is_task_heating_running)
                taskTryToggleHeating(false);

            if (!is_task_freezing_running && !is_task_heating_running)
            {
                io_heater_r.write(false);
                io_water_jacket_r.write(false);
                taskToggleMixer(false);
            }
        }
        else if (!is_connected_380V && turn_on) 
        {
            is_task_freezing_running = false;
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            taskToggleMixer(false);

            info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vOut));
            scr_set_op320->setValue(SCR_ERROR_NOTIFY);
        }
    }
}

void FXCore::taskTryToggleHeating(bool turn_on)
{
    if (machine_type->getValue() != static_cast<uint8_t>(MACHINE_TYPE::DM_flow))
    {
        if (!prog_running->getState() && is_connected_380V)
        {
            is_task_heating_running = turn_on;
            if (turn_on && is_task_freezing_running)
                taskTryToggleFreezing(false);

            if (!turn_on)
                taskHeating(0);

            if (!is_task_heating_running && !is_task_freezing_running)
            {
                io_heater_r.write(false);
                io_water_jacket_r.write(false);
                taskToggleMixer(false);
            }
        }
        else if (!is_connected_380V && turn_on) 
        {
            is_task_heating_running = false;
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            taskToggleMixer(false);

            info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vOut));
            scr_set_op320->setValue(SCR_ERROR_NOTIFY);
        }
    }
}

void FXCore::taskToggleMixer(bool turn_on)
{
    if (turn_on && (machine_state->getValue() > static_cast<uint8_t>(MACHINE_STATE::Flowing) && is_connected_380V))
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
        if (!is_connected_380V)
        {
            info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vOut));
            if (!start_error_displayed_yet)
                scr_set_op320->setValue(SCR_ERROR_NOTIFY);
            start_error_displayed_yet = true;
            return false;
        }

        if (prog_running->getState())
            return false;
        else
            stopAllTasks();

        start_error_displayed_yet = false;
        prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurRunning));
        prog_freezing_part_finished->setValue(0);
        prog_heating_part_finished->setValue(0);
        prog_preset_selected->setValue(pasteur_preset);
        prog_jacket_filled->setValue(0);
        prog_running->setValue(1);

        gotoMainScreen();
        
        rtc_prog_pasteur_started->clone(rtc_general_current);
        rtc_prog_finished->setZeroDateTime();
        rtc_prog_expected_finish->clone(rtc_general_current);

        if (prog_preset_selected->getValue() == 0)
        {
            prog_pasteur_tempC = self_prog_pasteur_tempC->getValue();
            prog_heating_tempC = self_prog_heating_tempC->getValue();
            prog_freezing_tempC = self_prog_freezing_tempC->getValue();
            prog_need_in_freezing->setValue(self_prog_mode->getValue() > 0 ? 1 : 0);
            prog_need_in_heating->setValue(self_prog_mode->getValue() > 1 ? 1 : 0);
            rtc_prog_expected_finish->addMinutes(self_prog_duration_mm->getValue());
        }
        else
        {
            uint8_t duration = 0;
            auto_prog_presets->startPreset(
                prog_preset_selected->getValue() - 1,
                prog_pasteur_tempC,
                prog_heating_tempC,
                prog_freezing_tempC,
                duration
                );
            prog_need_in_freezing->setValue(1);
            prog_need_in_heating->setValue(1);
            rtc_prog_expected_finish->addMinutes(duration);
        }

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
        uint8_t estimated = rtc_prog_expected_finish->getDiffMin(rtc_prog_pasteur_paused);
        rtc_prog_expected_finish->clone(rtc_general_current);
        rtc_prog_expected_finish->addMinutes(estimated);

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

void FXCore::taskFinishFlowing(bool forced)
{
    if (forced || rtc_general_current->isTimeEqual(rtc_blowing_finish) || rtc_general_current->isBiggerThan(rtc_blowing_finish))
    {
        is_task_washing_running = false;
        io_blowgun_r.write(false);
    }
}

void FXCore::taskHeating(uint8_t expected_tempC)
{
    if ((prog_running->getState() || is_task_heating_running))
    {
        is_heaters_starters_available = false;
        taskToggleMixer(true);

        if (!is_water_in_jacket)
            io_water_jacket_r.write(true);
        else
        {
            io_water_jacket_r.write(false);
            if (liquid_tempC < expected_tempC && is_heaters_starters_available)
                io_heater_r.write(true);
            else if (is_heaters_starters_available)
                io_heater_r.write(false);
        }
    }
    else if (is_heaters_starters_available)
    {
        is_heaters_starters_available = false;
        taskToggleMixer(false);
        io_heater_r.write(false);
    }
}

void FXCore::taskFreezing(uint8_t expected_tempC)
{
    if (is_task_freezing_running && !(prog_running->getState() || is_task_heating_running))
    {
        taskToggleMixer(true);

        if (liquid_tempC > expected_tempC)
            io_water_jacket_r.write(true);
        else
            io_water_jacket_r.write(false);
    }
}

void FXCore::checkAutoStartup()
{
    uint8_t preset_id = 0;
    if (preset_id = auto_prog_presets->isTimeToRunPreset(*rtc_general_current) > 0)
        taskStartProg(preset_id);
}

bool FXCore::threadProg()
{
    if (prog_state->getValue() < static_cast<uint8_t>(PROG_STATE::PasteurFinished))
    {
        if (is_mixer_error)
        {
            taskFinishProg(FINISH_FLAG::MixerError);
            return false;
        }

        if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
            if (rtc_general_current->getDiffMin(rtc_prog_pasteur_paused) >= PASTEUR_AWAIT_LIMIT_MM)
            {
                taskFinishProg(!is_connected_380V ? FINISH_FLAG::Power380vError : FINISH_FLAG::WaterJacketError);
                return false;
            }

        if (!is_connected_380V || prog_jacket_filled->getState() ? !is_water_in_jacket : false)
        {
            if (prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurRunning))
                taskPauseProg(!is_connected_380V ? OP320_ERROR::Power380vOut : OP320_ERROR::WaterAwait);

            return false;
        }

        if (is_connected_380V && (prog_jacket_filled->getState() ? is_water_in_jacket : true) &&
            prog_state->getValue() == static_cast<uint8_t>(PROG_STATE::PasteurPaused))
            taskResumeProg();

        if (!is_water_in_jacket)
        {
            io_water_jacket_r.write(true);
            return true;
        }
        else
        {
            if (!prog_jacket_filled->getState())
                prog_jacket_filled->setValue(1);
            
            io_water_jacket_r.write(false);
        }

        taskHeating(prog_pasteur_tempC);

        if (rtc_general_current->isTimeEqual(rtc_prog_expected_finish) ||
            rtc_general_current->isBiggerThan(rtc_prog_expected_finish))
            prog_state->setValue(static_cast<uint8_t>(PROG_STATE::PasteurFinished));
        else
            return true;
    }

    if (prog_need_in_freezing->getState() ? !prog_freezing_part_finished->getState() : false)
    {
        taskFreezing(prog_freezing_tempC);
        if (liquid_tempC <= prog_freezing_tempC)
        {
            prog_freezing_part_finished->setValue(1);
            prog_state->setValue(static_cast<uint8_t>(PROG_STATE::FreezingFinished));
        }
        return true;
    }

    if (prog_need_in_heating->getValue() ? !prog_heating_part_finished->getState() : false)
    {
        taskHeating(prog_heating_tempC);
        if (liquid_tempC >= prog_heating_tempC)
        {
            prog_heating_part_finished->setValue(1);
            prog_state->setValue(static_cast<uint8_t>(PROG_STATE::HeatingFinished));
        }
        return true;
    }

    if ((prog_need_in_freezing->getState() ? prog_freezing_part_finished->getState() : true) &&
        (prog_need_in_heating->getValue() ? prog_heating_part_finished->getState() : true))
        taskFinishProg(FINISH_FLAG::Success);

    return true;
}

void FXCore::threadMain()
{
    scr_get_op320->setValueByModbus();

    if (prog_running->getState())
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Pasteurizing));
    else if (is_task_freezing_running)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Freezing));
    else if (is_task_heating_running)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Heating));
    else if (is_task_washing_running)
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Flowing));
    else
        machine_state->setValue(static_cast<uint8_t>(MACHINE_STATE::Await));

    if (is_flowing_call && !prog_running->getState() && !is_stop_btn_pressed && !is_connected_380V && scr_get_op320->getValue() == SCR_BLOWING_PAGE)
        taskTryToggleFlowing();
    else if (is_flowing_call && is_connected_380V)
    {
        is_flowing_call = false;
        info_error_notify->setValue(static_cast<uint8_t>(OP320_ERROR::Power380vIn));
        scr_set_op320->setValue(SCR_ERROR_NOTIFY);
    }

    if (is_stop_btn_pressed)
        stopAllTasks();

    if (prog_running->getState())
        threadProg();

    if (is_task_heating_running)
        taskHeating(solo_heating_tempC->getValue());
    else if (!prog_running->getState())
        taskHeating(0);

    if (is_task_freezing_running)
        taskFreezing(solo_freezing_tempC->getValue());

    if (is_task_washing_running)
        taskFinishFlowing(false);

    displayOP320States();
}

void FXCore::readSensors()
{
    is_stop_btn_pressed = is_stop_btn_pressed ? true : io_stop_btn_s.readDigital();
    is_flowing_call = is_flowing_call ? true : io_blowgun_s.readDigital();
    is_mixer_error = io_mixer_crash_s.readDigital();

    batt_chargeV = io_battery_s.readAnalog() / (4095 / 100);

    mb_batt_charge.writeValue((uint16_t)batt_chargeV);
    mb_liq_tempC.writeValue((uint16_t)liquid_tempC);
    
    is_connected_380V = io_v380_s.readDigital();
    is_water_in_jacket = io_water_jacket_s.readDigital();

    liquid_tempC = io_liquid_temp_s.readAnalog() /
        (4095 / (master_20ma_positive_limit->getValue() + master_4ma_negative_limit->getValue())) -
        master_4ma_negative_limit->getValue();
}

void FXCore::displayOP320States()
{
    if (!prog_running->getState() &&
        info_main_step->getValue() == static_cast<uint8_t>(OP320_STEP::PasteurFinish) &&
        rtc_general_current->getDiffMin(rtc_prog_finished) >= 5 &&
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
        info_main_process->setValue(static_cast<uint8_t>(OP320_PROCESS::Heating));
        info_main_step_show_hide->setValue(1);
        if (!is_water_in_jacket)
            info_main_step->setValue(static_cast<uint8_t>(OP320_STEP::WaterJacket));
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
    stopAllTasks();
    ee_hard_reset_value_setted.writeEE(1);
    rtc.setTime(8, 0, 0);
    rtc.setDate(12, 5, 23);
    rtc_general_current->setRealTime();
    rtc_general_current->sendToEE(true);
    rtc_general_last_time_point->clone(rtc_general_current);
    solo_heating_tempC->setValue(45);
    solo_freezing_tempC->setValue(15);
    calibration_flowgun_dope_dose->setValue(0);
    self_prog_pasteur_tempC->setValue(65);
    self_prog_heating_tempC->setValue(45);
    self_prog_freezing_tempC->setValue(15);
    self_prog_duration_mm->setValue(30);
    self_prog_mode->setValue(0);
    master_water_saving_toggle->setValue(1);
    master_hysteresis_toggle->setValue(1);
    master_hysteresis_tempC->setValue(2);
    master_pump_LM_performance->setValue(38);
    master_4ma_negative_limit->setValue(50);
    master_20ma_positive_limit->setValue(200);
    flowgun_presets->setDefault();
    flowgun_presets->selectPreset(0);
    auto_prog_presets->setDefault();
    auto_prog_presets->selectPreset(0);
}