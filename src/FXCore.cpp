#include "FXCore.hpp"

void FXCore::init()
{
    this->MBDispatcher::init();
    this->EEDispatcher::init();
    this->IODispatcher::init();
    relaysOff();
    
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    mb_comm_stop_proc.addTrigger([this]() -> void { stopAllFunc(); });
    mb_comm_blow_preset_1.addTrigger([this]() -> void { blowingSelectPreset(0); });
    mb_comm_blow_preset_2.addTrigger([this]() -> void { blowingSelectPreset(1); });
    mb_comm_blow_preset_3.addTrigger([this]() -> void { blowingSelectPreset(2); });
    mb_comm_blow_preset_4.addTrigger([this]() -> void { blowingSelectPreset(3); });
    mb_comm_blow_vInc.addTrigger([this]() -> void { blowingResetVolume(true); });
    mb_comm_blow_vDec.addTrigger([this]() -> void { blowingResetVolume(false); });
    mb_comm_blow_prescaler.addTrigger([this]() -> void { blowingChangePrescaler(); });
    mb_comm_pass_7.addTrigger([this]() -> void { readPassword(7); });
    mb_comm_pass_8.addTrigger([this]() -> void { readPassword(8); });
    mb_comm_pass_9.addTrigger([this]() -> void { readPassword(9); });
    mb_comm_solo_tempC_cancel.addTrigger([this]() -> void {
        mb_solo_heating_tempC.writeValue((uint16_t)ee_solo_heating_tempC.readEE());
        mb_solo_freezing_tempC.writeValue((uint16_t)ee_solo_freezing_tempC.readEE());
        mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE);
        });
    mb_comm_solo_tempC_accept.addTrigger([this]() -> void {
        ee_solo_heating_tempC.writeEE(solo_heating_tempC = mb_solo_heating_tempC.readValue());
        ee_solo_freezing_tempC.writeEE(solo_freezing_tempC = mb_solo_freezing_tempC.readValue());
        });
    mb_comm_rtc_new_cancel.addTrigger([this]() -> void {
        mb_rtc_new_hh.writeValue((uint16_t)0);
        mb_rtc_new_mm.writeValue((uint16_t)0);
        mb_rtc_new_ss.writeValue((uint16_t)0);
        mb_rtc_new_DD.writeValue((uint16_t)0);
        mb_rtc_new_MM.writeValue((uint16_t)0);
        mb_rtc_new_YY.writeValue((uint16_t)0);
        mb_set_op320_scr.writeValue((uint16_t)SCR_USER_MENU);
        });
    mb_comm_rtc_new_accept.addTrigger([this]() -> void { setNewRTC(); });
    mb_comm_blow_calib_inc.addTrigger([this]() -> void {
        if (++blowing_calib_range > 99)
            blowing_calib_range = 99;
        ee_blowgun_calib_range.writeEE(blowing_calib_range);
        mb_blow_calib_range.writeValue((uint16_t)(blowing_calib_range * 100));
        });
    mb_comm_blow_calib_dec.addTrigger([this]() -> void {
        if (blowing_calib_range > 0)
            --blowing_calib_range;
        ee_blowgun_calib_range.writeEE(blowing_calib_range);
        mb_blow_calib_range.writeValue((uint16_t)(blowing_calib_range * 100));      
    });
    mb_comm_self_cancel.addTrigger([this]() -> void {
        loadFromEE();
        mb_set_op320_scr.writeValue((uint16_t)SCR_USER_MENU);
        });
    mb_comm_self_accept.addTrigger([this]() -> void {
        ee_self_pasteur_tempC.writeEE(settings_self_pasteur_tempC = mb_self_pasteur_tempC.readValue());
        ee_self_heating_tempC.writeEE(settings_self_heating_tempC = mb_self_heating_tempC.readValue());
        ee_self_freezing_tempC.writeEE(settings_self_freezing_tempC = mb_self_freezing_tempC.readValue());
        ee_self_pasteur_durat.writeEE(settings_self_pasteur_durat = mb_self_durat_mm.readValue());
        ee_self_psteur_mode.writeEE(settings_self_pasteur_mode = mb_self_mode_list.readValue());
    });
    mb_comm_self_mode_up.addTrigger([this]() -> void { selfPasteurChangeMode(true); });
    mb_comm_self_mode_down.addTrigger([this]() -> void { selfPasteurChangeMode(false); });
    mb_comm_auto_preset_1.addTrigger([this]() -> void { autoPasteurSelectPreset(0); });
    mb_comm_auto_preset_2.addTrigger([this]() -> void { autoPasteurSelectPreset(1); });
    mb_comm_auto_preset_3.addTrigger([this]() -> void { autoPasteurSelectPreset(2); });
    mb_comm_auto_toggle.addTrigger([this]() -> void { autoPasteurToggleOnOff(); });
    mb_comm_auto_cancel.addTrigger([this]() -> void {
        loadFromEE();
        mb_set_op320_scr.writeValue((uint16_t)SCR_USER_MENU);
    });
    mb_comm_auto_accept.addTrigger([this]() -> void {
        ee_auto_pasteur_tempC_arr[pasteur_preset_selected]->writeEE(pasteur_preset_pasteur_tempC[pasteur_preset_selected] = mb_auto_pasteur_tempC.readValue());
        ee_auto_freezing_tempC_arr[pasteur_preset_selected]->writeEE(pasteur_preset_freezing_tempC[pasteur_preset_selected] = mb_auto_freezing_tempC.readValue());
        ee_auto_heating_tempC_arr[pasteur_preset_selected]->writeEE(pasteur_preset_heating_tempC[pasteur_preset_selected] = mb_auto_heating_tempC.readValue());
        ee_auto_pasteur_duratMM_arr[pasteur_preset_selected]->writeEE(pasteur_preset_durat_mm[pasteur_preset_selected] = mb_auto_durat_mm.readValue());
        ee_auto_run_on_hh_arr[pasteur_preset_selected]->writeEE(pasteur_preset_run_on_hh[pasteur_preset_selected] = mb_auto_run_rtc_hh.readValue());
        ee_auto_run_on_mm_arr[pasteur_preset_selected]->writeEE(pasteur_preset_run_on_mm[pasteur_preset_selected] = mb_auto_run_rtc_mm.readValue());
        pasteur_rtc_triggers[pasteur_preset_selected].setTime(
            pasteur_preset_run_on_hh[pasteur_preset_selected],
            pasteur_preset_run_on_mm[pasteur_preset_selected],
            0, 0, 0, 0
        );
        autoPasteurSelectPreset(pasteur_preset_selected);
    });
    mb_master_water_saving_toggle.addTrigger([this]() -> void {
        *water_saving_on = !*water_saving_on;
        ee_master_water_saving.writeEE(*water_saving_on);
        mb_master_water_saving_monitor.writeValue((uint16_t)*water_saving_on);
    });
    mb_master_hysteresis_toggle.addTrigger([this]() -> void {
        *hysteresis_is_on = !*hysteresis_is_on;
        ee_master_hysteresis_toggle.writeEE(*hysteresis_is_on);
        mb_master_hysteresis_monitor.writeValue((uint16_t)*hysteresis_is_on);
    });
    mb_master_cancel.addTrigger([this]() -> void {
        mb_hysteresis.writeValue((uint16_t)*hysteresis_tempC);
        mb_20ma_adc_limit.writeValue((uint16_t)*adc_20ma_positive_limit);
        mb_4ma_adc_limit.writeValue((uint16_t)*adc_4ma_negative_limit);
        mb_blowing_performance_lm.writeValue((uint16_t)*pumb_perform_litres_min);
        mb_set_op320_scr.writeValue((uint16_t)SCR_USER_MENU);
    });
    mb_master_accept.addTrigger([this]() -> void {
        ee_master_hysteresis_value.writeEE(*hysteresis_tempC = mb_hysteresis.readValue());
        ee_master_20ma_adc_value.writeEE(*adc_20ma_positive_limit = mb_20ma_adc_limit.readValue());
        ee_master_4ma_adc_value.writeEE(*adc_4ma_negative_limit = mb_4ma_adc_limit.readValue());
        ee_master_pump_perf_lm.writeEE(*pumb_perform_litres_min = mb_blowing_performance_lm.readValue());        
    });
    mb_master_full_hard_reset.addTrigger([this]() -> void {  });

    mb_comm_self_pasteur_start.addTrigger([this]() -> void { pasteurStart(true); });
    mb_comm_solo_heating_toggle.addTrigger([this]() -> void { heaterToggle(!is_solo_heating); });
    mb_comm_solo_freezing_toggle.addTrigger([this]() -> void { freezingToggle(!is_solo_freezing); });
    mb_comm_blowgun_run_btn.addTrigger([this]() -> void { blowgunStart(); });

    delay(50);

    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); });
    TaskManager::newTask(100,   [this]() -> void { readSensors(); mainThread(); checkAutoStartup(); });
    TaskManager::newTask(200,   [this]() -> void {
        rtc_current_time.setInstTime();
        mb_rtc_hh.writeValue((uint16_t)rtc_current_time.hour);
        mb_rtc_mm.writeValue((uint16_t)rtc_current_time.minute);
        mb_rtc_ss.writeValue((uint16_t)rtc_current_time.second);
        mb_rtc_DD.writeValue((uint16_t)rtc_current_time.day);
        mb_rtc_MM.writeValue((uint16_t)rtc_current_time.month);
        mb_rtc_YY.writeValue((uint16_t)rtc_current_time.year);

    });
    TaskManager::newTask(300,  [this]() -> void {
        if (is_pasteur_proc_running && !is_pasteur_proc_paused && mb_get_op320_scr.readValue() == SCR_MASTER_PAGE)
        {
            displayState();
            mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE_TIME);
        }

        if (!is_pasteur_proc_running && !is_pasteur_proc_paused && mb_get_op320_scr.readValue() == SCR_MASTER_PAGE_TIME)
        {
            displayState();
            mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE);
        }
    });
    TaskManager::newTask(10000, [this]() -> void { checkRTC(false); });
    TaskManager::newTask(30000, [this]() -> void { readTempCSensor(); });
    blowgun_washing_task = TaskManager::newTask(10, [this]() -> void { blowgunFinish(); }, true);
    blowgun_washing_task->stop();

    delay(50);
    mb_blow_calib_dosage.writeValue((uint16_t)(blow_calib_volume / 10));
    checkRTC(true);
    loadFromEE();

    delay(50);
    blowingSelectPreset(blowgun_preset_selected);
    blowingChangePrescaler(true);
    autoPasteurSelectPreset(pasteur_preset_selected);
}

void FXCore::checkRTC(bool is_plc_start_up)
{
    uint32_t rtc_fullDays = rtc.getYear() * 365 + rtc.getMonth() * 31 + rtc.getDay();
    uint32_t ee_fullDays = ee_rtc_curr_year.readEE() * 365 + ee_rtc_curr_month.readEE() * 31 + ee_rtc_curr_day.readEE();

    Serial.print("rtc = ");
    Serial.println(rtc_fullDays);
    Serial.print("ee = ");
    Serial.println(ee_fullDays);
    Serial.println(ee_rtc_curr_year.readEE());
    Serial.println(ee_rtc_curr_month.readEE());
    Serial.println(ee_rtc_curr_day.readEE());
    Serial.println();

    if (!is_plc_start_up)
    {
        ee_dynamic_rtc_curr_hh.writeEE(rtc.getHours());
        ee_dynamic_rtc_curr_mm.writeEE(rtc.getMinutes());
        ee_dynamic_rtc_curr_ss.writeEE(rtc.getSeconds());
    }

    if (rtc_fullDays > ee_fullDays)
    {
        ee_rtc_curr_day.writeEE(rtc.getDay());
        ee_rtc_curr_month.writeEE(rtc.getMonth());
        ee_rtc_curr_year.writeEE(rtc.getYear());
        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            ee_auto_is_runned_today_arr[index]->writeEE(pasteur_preset_is_runned_today[index] = 0);
    }
    else if (rtc_fullDays < ee_fullDays && is_plc_start_up)
    {
        rtc.setDate(ee_rtc_curr_day.readEE(), ee_rtc_curr_month.readEE(), ee_rtc_curr_year.readEE());
        rtc.setHours(ee_dynamic_rtc_curr_hh.readEE());
        rtc.setMinutes(ee_dynamic_rtc_curr_mm.readEE());
        rtc.setSeconds(ee_dynamic_rtc_curr_ss.readEE());

        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            pasteur_preset_is_runned_today[index] = ee_auto_is_runned_today_arr[index]->readEE();
    }
    else if (is_plc_start_up)
    {
        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            pasteur_preset_is_runned_today[index] = ee_auto_is_runned_today_arr[index]->readEE();
    }
}

void FXCore::setNewRTC()
{
    rtc.setHours(mb_rtc_new_hh.readValue());
    rtc.setMinutes(mb_rtc_new_mm.readValue());
    rtc.setSeconds(mb_rtc_new_ss.readValue());
    rtc.setDay(mb_rtc_new_DD.readValue());
    rtc.setMonth(mb_rtc_new_MM.readValue());
    rtc.setYear(mb_rtc_new_YY.readValue());
    
    ee_rtc_curr_day.writeEE(rtc.getDay());
    ee_rtc_curr_month.writeEE(rtc.getMonth());
    ee_rtc_curr_year.writeEE(rtc.getYear());
    ee_dynamic_rtc_curr_hh.writeEE(rtc.getHours());
    ee_dynamic_rtc_curr_mm.writeEE(rtc.getMinutes());
    ee_dynamic_rtc_curr_ss.writeEE(rtc.getSeconds());        

    for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
        ee_auto_is_runned_today_arr[index]->writeEE(pasteur_preset_is_runned_today[index] = 0);
}

void FXCore::loadFromEE()
{
    // blowgun
    for (uint8_t index = 0; index < BLOWGUN_PRESET_CNT; index++)
        blowgun_preset_volume[index] = ee_blowgun_preset_arr[index]->readEE() * 5;

    // solo heat + freez
    mb_solo_heating_tempC.writeValue((uint16_t)(solo_heating_tempC = ee_solo_heating_tempC.readEE()));
    mb_solo_freezing_tempC.writeValue((uint16_t)(solo_freezing_tempC = ee_solo_freezing_tempC.readEE()));

    // blowing calibration
    blowing_calib_range = ee_blowgun_calib_range.readEE();
    mb_blow_calib_range.writeValue((uint16_t)(blowing_calib_range * 100));

    // self pasteur
    mb_self_pasteur_tempC.writeValue((uint16_t)(settings_self_pasteur_tempC = ee_self_pasteur_tempC.readEE()));
    mb_self_heating_tempC.writeValue((uint16_t)(settings_self_heating_tempC = ee_self_heating_tempC.readEE()));
    mb_self_freezing_tempC.writeValue((uint16_t)(settings_self_freezing_tempC = ee_self_freezing_tempC.readEE()));
    mb_self_durat_mm.writeValue((uint16_t)(settings_self_pasteur_durat = ee_self_pasteur_durat.readEE()));
    mb_self_mode_list.writeValue((uint16_t)(settings_self_pasteur_mode = ee_self_psteur_mode.readEE()));

    // auto pasteur
    for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
    {
        pasteur_preset_pasteur_tempC[index] = ee_auto_pasteur_tempC_arr[index]->readEE();
        pasteur_preset_heating_tempC[index] = ee_auto_heating_tempC_arr[index]->readEE();
        pasteur_preset_freezing_tempC[index] = ee_auto_freezing_tempC_arr[index]->readEE();
        pasteur_preset_durat_mm[index] = ee_auto_pasteur_duratMM_arr[index]->readEE();
        pasteur_preset_run_on_hh[index] = ee_auto_run_on_hh_arr[index]->readEE();
        pasteur_preset_run_on_mm[index] = ee_auto_run_on_mm_arr[index]->readEE();
        pasteur_preset_run_toggle[index] = ee_auto_run_toggle_arr[index]->readEE();
        pasteur_preset_is_runned_today[index] = ee_auto_is_runned_today_arr[index]->readEE();
        pasteur_rtc_triggers[index].setTime(pasteur_preset_run_on_hh[index], pasteur_preset_run_on_mm[index], 0, 0, 0, 0);
    }

    //master
        *water_saving_on = ee_master_water_saving.readEE();
        mb_master_water_saving_toggle.writeValue((uint16_t)*water_saving_on);
        mb_master_water_saving_monitor.writeValue((uint16_t)*water_saving_on);
        *hysteresis_is_on = ee_master_hysteresis_toggle.readEE();
        mb_master_hysteresis_toggle.writeValue((uint16_t)*hysteresis_is_on);
        mb_master_hysteresis_monitor.writeValue((uint16_t)*hysteresis_is_on);
        mb_hysteresis.writeValue((uint16_t)(*hysteresis_tempC = ee_master_hysteresis_value.readEE()));
        mb_20ma_adc_limit.writeValue((uint16_t)(*adc_20ma_positive_limit = ee_master_20ma_adc_value.readEE()));
        mb_4ma_adc_limit.writeValue((uint16_t)(*adc_4ma_negative_limit = ee_master_4ma_adc_value.readEE()));
        mb_blowing_performance_lm.writeValue((uint16_t)(*pumb_perform_litres_min = ee_master_pump_perf_lm.readEE()));
}

bool FXCore::pasteurStart(bool is_user_call, uint8_t preset_index)
{
    if (is_pasteur_proc_running)
        return false;
    else
        stopAllFunc();

    mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE_TIME);
    is_pasteur_proc_running = true;
    is_pasteur_proc_paused = false;
    is_pasteur_part_finished = false;
    is_freezing_part_finished = false;
    is_heating_part_finished = false;
    is_waterJ_filled_yet = false;

    ee_proc_pasteur_running.writeEE(1);
    ee_proc_pasteur_paused.writeEE(0);
    ee_proc_pasteur_part_finished.writeEE(0);
    ee_proc_need_in_freezing.writeEE(0);
    ee_proc_need_in_heating.writeEE(0);
    ee_proc_waterJacket_filled_yet.writeEE(0);

    if (is_user_call)
    {
        is_pasteur_need_in_freezing = settings_self_pasteur_mode >= 1 ? true : false;
        is_pasteur_need_in_heating = settings_self_pasteur_mode >= 2 ? true : false;
        ee_proc_need_in_freezing.writeEE(settings_self_pasteur_mode >= 1 ? 1 : 0);
        ee_proc_need_in_heating.writeEE(settings_self_pasteur_mode >= 2 ? 1 : 0);

        pasteur_proc_time_span_mm = settings_self_pasteur_durat;
        pasteur_proc_pasteur_tempC = settings_self_pasteur_tempC;
        pasteur_proc_heeting_tempC = settings_self_heating_tempC;
        pasteur_proc_freezing_tempC = settings_self_freezing_tempC;
        ee_proc_time_span.writeEE(pasteur_proc_time_span_mm);
        ee_proc_pasteur_tempC.writeEE(pasteur_proc_pasteur_tempC);
        ee_proc_freezing_tempC.writeEE(pasteur_proc_heeting_tempC);
        ee_proc_heating_tempC.writeEE(pasteur_proc_freezing_tempC);

        pasteur_preset_runned = 0;
    }
    else
    {
        is_pasteur_need_in_freezing = true;
        is_pasteur_need_in_heating = true;
        ee_proc_need_in_freezing.writeEE(1);
        ee_proc_need_in_heating.writeEE(1);

        pasteur_proc_time_span_mm = pasteur_preset_durat_mm[preset_index];
        pasteur_proc_pasteur_tempC = pasteur_preset_pasteur_tempC[preset_index];
        pasteur_proc_heeting_tempC = pasteur_preset_heating_tempC[preset_index];
        pasteur_proc_freezing_tempC = pasteur_preset_freezing_tempC[preset_index];
        ee_proc_time_span.writeEE(pasteur_preset_durat_mm[preset_index]);
        ee_proc_pasteur_tempC.writeEE(pasteur_preset_pasteur_tempC[preset_index]);
        ee_proc_freezing_tempC.writeEE(pasteur_preset_heating_tempC[preset_index]);
        ee_proc_heating_tempC.writeEE(pasteur_preset_freezing_tempC[preset_index]);

        pasteur_preset_runned = preset_index + 1;
        ee_auto_is_runned_today_arr[preset_index]->writeEE(pasteur_preset_is_runned_today[preset_index] = 1);
    }
    ee_proc_preset_runned_index.writeEE(pasteur_preset_runned);

    rtc_pasteur_started.setInstTime();
    ee_proc_started_hh.writeEE(rtc_pasteur_started.hour);
    ee_proc_started_mm.writeEE(rtc_pasteur_started.minute);

    // save to ee rtc of running

    return true;
}

void FXCore::pasteurPause(OP320Error to_op320)
{
    if (is_pasteur_proc_running)
    {
        is_pasteur_proc_paused = true;
        rtc_pasteur_paused.setInstTime();

        io_heater_r.write(false);
        io_mixer_r.write(false);
        if (!is_water_in_jacket)
            io_water_jacket_r.write(true);

        mb_notification_list.writeValue((uint16_t)to_op320);
        mb_set_op320_scr.writeValue((uint16_t)SCR_ERROR_NOTIFY);

        // save to ee rtc of pause + pause error code
    }
}

void FXCore::pasteurResume()
{
    if (is_pasteur_proc_running)
    {
        is_pasteur_proc_paused = false;

        io_mixer_r.write(true);
        io_water_jacket_r.write(false);
        
        // check resume response to pause rtc and finish or save to ee rtc of resume
    }
}

void FXCore::pasteurFinish(FinishFlag flag)
{
    if (is_pasteur_proc_running)
    {
        is_pasteur_proc_running = false;
        is_pasteur_proc_paused = false;
        is_pasteur_part_finished = true;
        is_freezing_part_finished = false;
        is_heating_part_finished = false;
        is_waterJ_filled_yet = false;
        is_pasteur_need_in_freezing = false;
        is_pasteur_need_in_heating = false;
        mixerToggle(false);
        rtc_pasteur_finished.setInstTime();

        if (flag != FinishFlag::Success && flag != FinishFlag::UserCall)
        {
            OP320Error to_op320;
            switch(flag)
            {
                case FinishFlag::MixerError: to_op320 = OP320Error::Mixer; break;
                case FinishFlag::Power380vError: to_op320 = OP320Error::PowerMoreHour; break;
                case FinishFlag::WaterJacketError: to_op320 = OP320Error::WaterMoreHour; break;
            }
            mb_notification_list.writeValue((uint16_t)to_op320);
            mb_set_op320_scr.writeValue((uint16_t)SCR_ERROR_NOTIFY);
        }

        // save to ee rtc of finish + code ok or mixer error
    }
}

void FXCore::blowgunStart()
{
    is_blowgun_call = false;

    if (blowgun_preset_selected != BLOWGUN_PRESET_WASHING && !is_connected_380V)
    {
        if (is_blowgun_washing_runned)
        {
            blowgunFinish(true);
            delay(1000);
        }

        io_blowgun_r.write(true);
        // litres * expected volume / litres in minute of pump
        delay(60000 * (blowgun_preset_volume[blowgun_preset_selected] * 10) / 38000);
        blowgunFinish(true);
    }
    else if (blowgun_preset_selected == BLOWGUN_PRESET_WASHING && !is_connected_380V)
    {
        if (!is_blowgun_washing_runned)
        {
            is_blowgun_washing_runned = true;
            blowgun_start_washing_time.setInstTime();
            io_blowgun_r.write(true);
        }
    }
    else if (!is_connected_380V)
    {
        mb_notification_list.writeValue((uint16_t)OP320Error::Power380vIn);
        mb_set_op320_scr.writeValue((uint16_t)SCR_ERROR_NOTIFY);
    }
}

void FXCore::blowgunFinish(bool forced) {
    
    if (forced || blowgun_start_washing_time.outRange(blowgun_preset_volume[BLOWGUN_PRESET_WASHING], rtc_current_time, true))
    {
        is_blowgun_washing_runned = false;
        io_blowgun_r.write(false);
    }
}

void FXCore::heaterToggle(bool toggle)
{
    if (!is_pasteur_proc_running && is_connected_380V)
    {
        is_solo_heating = toggle;
        if (toggle && is_solo_freezing)
            freezingToggle(false);

        if (!toggle)
            heatersPID(0);

        if (!is_solo_heating && !is_solo_freezing)
        {
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            mixerToggle(false);
        }
    }
    else if (!is_connected_380V && toggle)
    {
        is_solo_heating = false;
        io_heater_r.write(false);
        io_water_jacket_r.write(false);
        mb_notification_list.writeValue((uint16_t)OP320Error::Power380vOut);
        mb_set_op320_scr.writeValue((uint16_t)SCR_ERROR_NOTIFY);
    }
}

void FXCore::freezingToggle(bool toggle)
{
    if (!is_pasteur_proc_running && is_connected_380V)
    {
        is_solo_freezing = toggle;
        if (toggle && is_solo_heating)
            heaterToggle(false);

        if (!is_solo_freezing && !is_solo_heating)
        {
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            mixerToggle(false);
        }
    }
    else if (!is_connected_380V && toggle)
    {
        is_solo_freezing = false;
        io_heater_r.write(false);
        io_water_jacket_r.write(false);
        mixerToggle(false);
        mb_notification_list.writeValue((uint16_t)OP320Error::Power380vOut);
        mb_set_op320_scr.writeValue((uint16_t)SCR_ERROR_NOTIFY);
    }
}

void FXCore::mixerToggle(bool toggle)
{
    if (toggle && (is_solo_freezing || is_solo_heating || is_pasteur_proc_running) && is_connected_380V)
        io_mixer_r.write(true);
    else
        io_mixer_r.write(false);
}

void FXCore::heatersPID(uint8_t tempC)
{
    if ((is_pasteur_proc_running || is_solo_heating) && is_heaters_available)
    {
        is_heaters_available = false;

        if (liquid_tempC > tempC)
            io_heater_r.write(true);
        else
            io_heater_r.write(false);
    }
    else
        io_heater_r.write(false);
}

void FXCore::stopAllFunc()
{
    is_stop_btn_pressed = false;
    if (is_pasteur_proc_running)
        pasteurFinish(FinishFlag::UserCall);
        
    blowgunFinish(true);
    heaterToggle(false);
    freezingToggle(false);
    mixerToggle(false);
}

bool FXCore::pasteurTask()
{
    rtc_pasteur_in_proc.setInstTime();

    if (!is_pasteur_part_finished)
    {
        if (is_mixer_error)
        {
            pasteurFinish(FinishFlag::MixerError);
            return false;
        }

        if (is_pasteur_proc_paused)
        {
            rtc_pasteur_in_await.setInstTime();
            if (!rtc_pasteur_paused.inRange(PASTEUR_AWAIT_LIMIT_MM, rtc_pasteur_in_await))
            {
                pasteurFinish(!is_connected_380V ? FinishFlag::Power380vError : FinishFlag::WaterJacketError);
                return false;
            }
        }

        if ((!is_connected_380V || (is_waterJ_filled_yet ? !is_water_in_jacket : false)) && !is_pasteur_proc_paused)
        {
            pasteurPause(!is_connected_380V ? OP320Error::Power380vOut : OP320Error::WaterAwait);
            return false;
        }
        else if ((!is_connected_380V || (is_waterJ_filled_yet ? !is_water_in_jacket : false)) && is_pasteur_proc_paused)
            return false;

        if (is_connected_380V && (is_waterJ_filled_yet ? is_water_in_jacket : true) && is_pasteur_proc_paused)
            pasteurResume();

        if (!is_water_in_jacket)
        {
            io_water_jacket_r.write(true);
            return true;
        }
        else
        {
            if (!is_waterJ_filled_yet)
                is_waterJ_filled_yet = true;
            
            io_water_jacket_r.write(false);
        }

        heatersPID(pasteur_proc_pasteur_tempC);
        mixerToggle(true);

        if (!rtc_pasteur_started.inRange(pasteur_proc_time_span_mm, rtc_pasteur_in_proc))
            is_pasteur_part_finished = true;
    }

    if (is_pasteur_need_in_freezing ? !is_freezing_part_finished : false)
    {
        freezingTask(pasteur_proc_freezing_tempC);
        if (liquid_tempC >= pasteur_proc_freezing_tempC)
            is_freezing_part_finished = true;
        return true;
    }

    if (is_pasteur_need_in_heating ? !is_heating_part_finished : false)
    {
        heatingTask(pasteur_proc_heeting_tempC);
        if (liquid_tempC <= pasteur_proc_heeting_tempC)
            is_heating_part_finished = true;
        return true;
    }

    if ((is_pasteur_need_in_freezing ? is_freezing_part_finished : true) &&
        (is_pasteur_need_in_heating ? is_heating_part_finished : true))
        pasteurFinish(FinishFlag::Success);

    return true;
}

void FXCore::heatingTask(uint8_t expectedTempC)
{
    mixerToggle(true);
    if (!is_water_in_jacket)
        io_water_jacket_r.write(true);
    else
    {
        io_water_jacket_r.write(false);
        heatersPID(expectedTempC);
    }
}

void FXCore::freezingTask(uint8_t expectedTempC)
{
    mixerToggle(true);
    if (liquid_tempC > expectedTempC)
        io_water_jacket_r.write(true);
    else
        io_water_jacket_r.write(false);
}

void FXCore::checkAutoStartup()
{
    for (uint8_t templ_id = 0; templ_id < PASTEUR_PRESET_CNT; templ_id++)
    {
        if (pasteur_preset_run_toggle[templ_id] == 1 &&
            pasteur_preset_is_runned_today[templ_id] == 0 &&
            pasteur_rtc_triggers[templ_id].inRange(2, rtc_current_time))
            pasteurStart(false, templ_id);
    }
}

void FXCore::mainThread()
{
    if (is_blowgun_call &&
        !is_pasteur_proc_running &&
        !is_stop_btn_pressed &&
        !is_connected_380V &&
        mb_get_op320_scr.readValue() == SCR_BLOWING_PAGE)
        blowgunStart();
    
    if (is_stop_btn_pressed)
        stopAllFunc();

    if (is_pasteur_proc_running)
        pasteurTask();

    if (is_solo_heating)
        heatingTask(solo_heating_tempC);

    if (is_solo_freezing)
        freezingTask(solo_freezing_tempC);

    if (is_blowgun_washing_runned)
        blowgunFinish(false);

    displayState();
}

void FXCore::displayState()
{
    mb_proc_list.writeValue((uint16_t)OP320Process::Await);
    mb_step_name_list.writeValue((uint16_t)0);
    mb_step_index_list.writeValue((uint16_t)OP320Step::Await);

    if (is_connected_380V)
        mb_proc_list.writeValue((uint16_t)OP320Process::Chargering);

    if (is_blowgun_washing_runned)
        mb_proc_list.writeValue((uint16_t)OP320Process::Washing);

    if (is_solo_heating)
    {
        mb_proc_list.writeValue((uint16_t)OP320Process::Heating);
        mb_step_name_list.writeValue((uint16_t)1);

        if (!is_water_in_jacket)
            mb_step_index_list.writeValue((uint16_t)OP320Step::WaterJacket);
        else
            mb_step_index_list.writeValue((uint16_t)OP320Step::HeatingTo);
    }

    if (is_solo_freezing)
    {
        mb_proc_list.writeValue((uint16_t)OP320Process::Freezing);
        mb_step_name_list.writeValue((uint16_t)1);

        if (!is_water_in_jacket)
            mb_step_index_list.writeValue((uint16_t)OP320Step::WaterJacket);
        else
            mb_step_index_list.writeValue((uint16_t)OP320Step::WaterJCirculation);
    }

    if (is_pasteur_proc_running)
    {

        mb_proc_list.writeValue((uint16_t)(5 + pasteur_preset_runned));
        mb_step_name_list.writeValue((uint16_t)1);

        if (!is_waterJ_filled_yet)
            mb_step_index_list.writeValue((uint16_t)OP320Step::WaterJacket);
        
        if (!is_pasteur_part_finished && is_waterJ_filled_yet && liquid_tempC < pasteur_proc_pasteur_tempC) //-[hysteresis val])
            mb_step_index_list.writeValue((uint16_t)OP320Step::PasteurHeating);

        if (!is_pasteur_part_finished && is_waterJ_filled_yet && liquid_tempC >= pasteur_proc_pasteur_tempC)
            mb_step_index_list.writeValue((uint16_t)OP320Step::PasteurProc);

        if (is_pasteur_part_finished && is_pasteur_need_in_freezing ? is_freezing_part_finished : false)
            mb_step_index_list.writeValue((uint16_t)OP320Step::FreezingTo);
        
        if (is_pasteur_part_finished && is_pasteur_need_in_heating ? is_heating_part_finished : false)
            mb_step_index_list.writeValue((uint16_t)OP320Step::HeatingTo);
    }

    if (!is_pasteur_proc_running && is_pasteur_part_finished)
    {
        mb_step_name_list.writeValue((uint16_t)1);
        mb_step_index_list.writeValue((uint16_t)OP320Step::PasteurFinish);

        if (!rtc_pasteur_finished.inRange(20, rtc_current_time))
            is_pasteur_part_finished = false;
    }
}

void FXCore::readPassword(uint8_t number)
{
    for(uint8_t index = 5; index > 0; index--)
        master_password[index] = master_password[index - 1];
    master_password[0] = number;

    if (master_password[0] == 7 &&
        master_password[1] == 8 &&
        master_password[2] == 9 &&
        master_password[3] == 8 &&
        master_password[4] == 9 &&
        master_password[5] == 7)
    {
        mb_set_op320_scr.writeValue((uint16_t)11);
        master_password[0] = 0;
    }
}

/* PRIVATE + PROTECTED */

void FXCore::blowingSelectPreset(uint8_t preset_id)
{
    blowgun_preset_selected = preset_id;
    mb_blowing_preset_list.writeValue((uint16_t)preset_id);

    if (blowgun_preset_selected != BLOWGUN_PRESET_WASHING)
        mb_blowing_volume.writeValue((uint16_t)blowgun_preset_volume[blowgun_preset_selected]);
    else
        mb_blowing_volume.writeValue((uint16_t)(blowgun_preset_volume[blowgun_preset_selected] / 60 * 100 + blowgun_preset_volume[blowgun_preset_selected] % 60));

    blowingChangePrescaler(true);
}

void FXCore::blowingResetVolume(bool is_positive)
{
    if (blowgun_preset_selected != BLOWGUN_PRESET_WASHING)
    {
        blowgun_preset_volume[blowgun_preset_selected] += is_positive ?
            blowgun_prescaler_table[blowgun_prescaler_selected] :
            -blowgun_prescaler_table[blowgun_prescaler_selected];

        if (blowgun_preset_volume[blowgun_preset_selected] > 995)
        blowgun_preset_volume[blowgun_preset_selected] = 995;
    }
    else
    {
        blowgun_preset_volume[blowgun_preset_selected] += is_positive ?
            blowgun_pretime_table[blowgun_prescaler_selected] :
            -blowgun_pretime_table[blowgun_prescaler_selected];

        if (blowgun_preset_volume[blowgun_preset_selected] > 585)
        blowgun_preset_volume[blowgun_preset_selected] = 585;
    }

    if (blowgun_preset_volume[blowgun_preset_selected] < 0 )
        blowgun_preset_volume[blowgun_preset_selected] = 0;

    ee_blowgun_preset_arr[blowgun_preset_selected]->writeEE(blowgun_preset_volume[blowgun_preset_selected] / 5);
    blowingSelectPreset(blowgun_preset_selected);
}

void FXCore::blowingChangePrescaler(bool boot_up)
{
    if (!boot_up)
    {
        switch (blowgun_prescaler_selected)
        {
            case 0: blowgun_prescaler_selected = 1; break;
            case 1: blowgun_prescaler_selected = 2; break;
            case 2: blowgun_prescaler_selected = 0; break;
        }
    }

    if (blowgun_preset_selected != BLOWGUN_PRESET_WASHING)
    {
        mb_blowing_incV.writeValue((uint16_t)(blowgun_prescaler_table[blowgun_prescaler_selected] * 10));
        mb_blowing_decV.writeValue((uint16_t)(blowgun_prescaler_table[blowgun_prescaler_selected] * 10));
    }
    else
    {
        mb_blowing_incV.writeValue((uint16_t)(blowgun_pretime_table[blowgun_prescaler_selected]));
        mb_blowing_decV.writeValue((uint16_t)(blowgun_pretime_table[blowgun_prescaler_selected]));
    }
}

void FXCore::selfPasteurChangeMode(bool is_positive)
{
    settings_self_pasteur_mode = is_positive ? ++settings_self_pasteur_mode : --settings_self_pasteur_mode;

    if (settings_self_pasteur_mode > 2)
        settings_self_pasteur_mode = 2;

    ee_self_psteur_mode.writeEE(settings_self_pasteur_mode);
    mb_self_mode_list.writeValue((uint16_t)settings_self_pasteur_mode);
}

void FXCore::autoPasteurSelectPreset(uint8_t preset_id)
{
    pasteur_preset_selected = preset_id;
    mb_auto_pasteur_tempC.writeValue((uint16_t)pasteur_preset_pasteur_tempC[preset_id]);
    mb_auto_freezing_tempC.writeValue((uint16_t)pasteur_preset_freezing_tempC[preset_id]);
    mb_auto_heating_tempC.writeValue((uint16_t)pasteur_preset_heating_tempC[preset_id]);
    mb_auto_durat_mm.writeValue((uint16_t)pasteur_preset_durat_mm[preset_id]);
    mb_auto_run_rtc_hh.writeValue((uint16_t)pasteur_preset_run_on_hh[preset_id]);
    mb_auto_run_rtc_mm.writeValue((uint16_t)pasteur_preset_run_on_mm[preset_id]);
    mb_auto_preset_list.writeValue((uint16_t)preset_id);
    mb_auto_preset_toggle.writeValue((uint16_t)pasteur_preset_run_toggle[preset_id]);
}

void FXCore::autoPasteurToggleOnOff()
{
    if (pasteur_preset_run_toggle[pasteur_preset_selected] == 0)
        pasteur_preset_run_toggle[pasteur_preset_selected] = 1;
    else
        pasteur_preset_run_toggle[pasteur_preset_selected] = 0;
    ee_auto_run_toggle_arr[pasteur_preset_selected]->writeEE(pasteur_preset_run_toggle[pasteur_preset_selected]);
    autoPasteurSelectPreset(pasteur_preset_selected);
}

void FXCore::readSensors()
{
    is_stop_btn_pressed = is_stop_btn_pressed ? true : io_stop_btn_s.readDigital();
    is_blowgun_call = is_blowgun_call ? true : io_blowgun_s.readDigital();
    is_mixer_error = io_mixer_crash_s.readDigital();

    mb_batt_charge.writeValue(batt_chargeV = io_battery_s.readAnalog() / 40.95);
    mb_liq_tempC.writeValue((uint16_t)liquid_tempC);
    
    is_connected_380V = io_v380_s.readDigital();
    is_water_in_jacket = io_water_jacket_s.readDigital();
}

void FXCore::readTempCSensor()
{
    liquid_tempC = io_liquid_temp_s.readAnalog() / 20.475 - 50;
    is_heaters_available = true;
}