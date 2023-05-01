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
    mb_comm_pass_7.addTrigger([this]() -> void {  });
    mb_comm_pass_8.addTrigger([this]() -> void {  });
    mb_comm_pass_9.addTrigger([this]() -> void {  });
    mb_comm_solo_tempC_cancel.addTrigger([this]() -> void {
        mb_solo_heating_tempC.writeValue((uint16_t)ee_solo_heating_tempC.readEE());
        mb_solo_freezing_tempC.writeValue((uint16_t)solo_freezing_tempC);
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
    mb_comm_rtc_new_accept.addTrigger([this]() -> void {
        rtc.setHours(mb_rtc_new_hh.readValue());
        rtc.setMinutes(mb_rtc_new_mm.readValue());
        rtc.setSeconds(mb_rtc_new_ss.readValue());
        rtc.setDay(mb_rtc_new_DD.readValue());
        rtc.setMonth(mb_rtc_new_MM.readValue());
        rtc.setYear(mb_rtc_new_YY.readValue());
    });
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
        loadFromEE();
    });
    mb_comm_self_mode_up.addTrigger([this]() -> void { selfPasteurChangeMode(true); });
    mb_comm_self_mode_down.addTrigger([this]() -> void { selfPasteurChangeMode(false); });
    mb_comm_auto_preset_1.addTrigger([this]() -> void {  });
    mb_comm_auto_preset_2.addTrigger([this]() -> void {  });
    mb_comm_auto_preset_3.addTrigger([this]() -> void {  });
    mb_comm_auto_toggle.addTrigger([this]() -> void {  });
    mb_comm_auto_cancel.addTrigger([this]() -> void {  });
    mb_comm_auto_accept.addTrigger([this]() -> void {  });
    mb_master_water_saving_toggle.addTrigger([this]() -> void {  });
    mb_master_hysteresis_toggle.addTrigger([this]() -> void {  });
    mb_master_cancel.addTrigger([this]() -> void {  });
    mb_master_accept.addTrigger([this]() -> void {  });
    mb_master_full_hard_reset.addTrigger([this]() -> void {  });

    TaskManager::newTask(20,    [this]() -> void { poll(); commCheck(); });
    TaskManager::newTask(100,   [this]() -> void { readSensors(); mainThread(); });
    TaskManager::newTask(200,   [this]() -> void {
        mb_rtc_hh.writeValue((uint16_t)rtc.getHours());
        mb_rtc_mm.writeValue((uint16_t)rtc.getMinutes());
        mb_rtc_ss.writeValue((uint16_t)rtc.getSeconds());
        mb_rtc_DD.writeValue((uint16_t)rtc.getDay());
        mb_rtc_MM.writeValue((uint16_t)rtc.getMonth());
        mb_rtc_YY.writeValue((uint16_t)rtc.getYear());
    });
    TaskManager::newTask(30000, [this]() -> void { readTempCSensor(); });
    blowgun_washing_task = TaskManager::newTask(0, [this]() -> void { blowgunFinish(); }, true);

    delay(50);
    mb_blow_calib_dosage.writeValue((uint16_t)(blow_calib_volume / 10));
    loadFromEE();

    delay(50);
    blowingSelectPreset(blowgun_preset_selected);
    blowingChangePrescaler(true);

    //pateur preset select func
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
}

bool FXCore::pasteurStart(bool is_user_call, uint8_t preset_index = 0)
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

    if (is_user_call)
    {
        is_pasteur_need_in_freezing = settings_self_pasteur_mode >= 1 ? true : false;
        is_pasteur_need_in_heating = settings_self_pasteur_mode >= 2 ? true : false;
        pasteur_proc_time_span_mm = settings_self_pasteur_durat;
        pasteur_proc_pasteur_tempC = settings_self_pasteur_tempC;
        pasteur_proc_heeting_tempC = settings_self_heating_tempC;
        pasteur_proc_freezing_tempC = settings_self_freezing_tempC;
        pasteur_preset_runned = 0;
    }
    else
    {
        is_pasteur_need_in_freezing = true;
        is_pasteur_need_in_heating = true;
        pasteur_proc_time_span_mm = pasteur_preset_pasteur_tempC[preset_index];
        pasteur_proc_pasteur_tempC = pasteur_preset_pasteur_tempC[preset_index];
        pasteur_proc_heeting_tempC = pasteur_preset_pasteur_tempC[preset_index];
        pasteur_proc_freezing_tempC = pasteur_preset_pasteur_tempC[preset_index];
        pasteur_preset_runned = preset_index + 1;
    }
    rtc_pasteur_started.setInstTime();

    // save to ee rtc of running

    return true;
}

void FXCore::pasteurPause()
{
    if (is_pasteur_proc_running)
    {
        is_pasteur_proc_paused = true;
        rtc_pasteur_paused.setInstTime();

        io_heater_r.write(false);
        io_mixer_r.write(false);
        if (!is_water_in_jacket)
            io_water_jacket_r.write(true);

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
    }

    // check resume response to pause rtc and finish or save to ee rtc of resume
}

void FXCore::pasteurFinish(FinishFlag flag)
{
    is_pasteur_proc_running = false;
    is_pasteur_proc_paused = false;
    is_pasteur_part_finished = false;
    is_freezing_part_finished = false;
    is_heating_part_finished = false;
    is_waterJ_filled_yet = false;
    is_pasteur_need_in_freezing = false;
    is_pasteur_need_in_heating = false;
    mixerToggle(false);
    rtc_pasteur_finished.setInstTime();

    // save to ee rtc of finish + code ok or mixer error
}

void FXCore::blowgunStart()
{
    is_blowgun_call = false;
    if (blowgun_preset_selected != BLOWGUN_PRESET_WASHING)
    {
        if (is_blowgun_washing_runned)
        {
            blowgunFinish();
            delay(1000);
        }

        io_blowgun_r.write(true);
        // litres * expected volume / litres in minute of pump
        for (uint32_t blowing_delay = 60000 * (blowgun_preset_volume[blowgun_preset_selected] * 10) / 38000; blowing_delay > 0;)
        {
            if (blowing_delay > 100)
            {
                blowing_delay -= 100;
                delay(100);
            }
            else
            {
                delay(blowing_delay);
                blowing_delay = 0;
            }

            if (mb_get_op320_scr.readValue() != SCR_BLOWING_PAGE)
                mb_set_op320_scr.writeValue((uint16_t)SCR_BLOWING_PAGE);
        }
        io_blowgun_r.write(false);
    }
    else
    {
        if (!is_blowgun_washing_runned)
        {
            is_blowgun_washing_runned = true;
            blowgun_washing_task->run(blowgun_preset_volume[BLOWGUN_PRESET_WASHING]);
            io_blowgun_r.write(true);
        }
    }
}

void FXCore::blowgunFinish() {
    is_blowgun_washing_runned = false;
    blowgun_washing_task->stop();
    io_blowgun_r.write(false);
}

void FXCore::heaterToggle(bool toggle)
{
    if (!is_pasteur_proc_running)
    {
        if (is_solo_heating = toggle && is_solo_freezing)
            freezingToggle(false);

        if (!is_solo_heating && !is_solo_freezing)
        {
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            mixerToggle(false);
        }
    }
}

void FXCore::freezingToggle(bool toggle)
{
    if (!is_pasteur_proc_running)
    {
        if (is_solo_freezing = toggle && is_solo_heating)
            heaterToggle(false);

        if (!is_solo_freezing && !is_solo_heating)
        {
            io_heater_r.write(false);
            io_water_jacket_r.write(false);
            mixerToggle(false);
        }
    }
}

void FXCore::mixerToggle(bool toggle)
{
    if (toggle && (is_solo_freezing || is_solo_heating || is_pasteur_proc_running))
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
        
    blowgunFinish();
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
            // go to notify scr + set mb_notify val
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
            pasteurPause();
            return false;
        }
        else if ((!is_connected_380V || (is_waterJ_filled_yet ? !is_water_in_jacket : false)) && is_pasteur_proc_paused)
            return false;

        if (is_connected_380V && (is_waterJ_filled_yet ? is_water_in_jacket : true) && is_pasteur_proc_paused)
            pasteurResume();

        if (!is_water_in_jacket)
        {
            io_water_jacket_r.write(true);
            // write step to op320
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
    {
        io_water_jacket_r.write(true);
        // write step to op320
    }
    else
    {
        io_water_jacket_r.write(false);
        heatersPID(expectedTempC);
        // write step to op320
    }
}

void FXCore::freezingTask(uint8_t expectedTempC)
{
    mixerToggle(true);
    if (liquid_tempC > expectedTempC)
    {
        io_water_jacket_r.write(true);
        // write step to op320
    }
    else
    {
        io_water_jacket_r.write(false);
        // write step to op320
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

    if (is_pasteur_proc_running && mb_get_op320_scr.readValue() == SCR_MASTER_PAGE)
        mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE_TIME);

    if (!is_pasteur_proc_running && mb_get_op320_scr.readValue() == SCR_MASTER_PAGE_TIME)
        mb_set_op320_scr.writeValue((uint16_t)SCR_MASTER_PAGE);

    if (is_pasteur_proc_running)
        pasteurTask();

    if (is_solo_heating)
        heatingTask(solo_heating_tempC);

    if (is_solo_freezing)
        freezingTask(solo_freezing_tempC);

    displayState();
    //here alarm check for auto calling pasteur preset
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
        mb_proc_list.writeValue((uint16_t)OP320Process::Heating);

    if (is_solo_freezing)
        mb_proc_list.writeValue((uint16_t)OP320Process::Freezing);

    if (is_pasteur_proc_running)
    {
        switch(pasteur_preset_runned)
        {
            case 0: mb_proc_list.writeValue((uint16_t)OP320Process::PasteurSelf);
            case 1: mb_proc_list.writeValue((uint16_t)OP320Process::PasteurP1);
            case 2: mb_proc_list.writeValue((uint16_t)OP320Process::PasteurP2);
            case 3: mb_proc_list.writeValue((uint16_t)OP320Process::PasteurP3);
        }

        mb_step_name_list.writeValue((uint16_t)0);

        
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