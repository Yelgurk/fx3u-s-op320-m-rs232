#include "FXCore.hpp"

void FXCore::init()
{
    this->MBDispatcher::init();
    this->EEDispatcher::init();
    this->IODispatcher::init();
    relaysOff();
    
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    mb_comm_stop_proc.addTrigger([this]() -> void { pasteurFinish(); blowgunFinish(); heaterToggle(false); mixerToggle(false); });
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
        ee_self_pasteur_tempC.writeEE(self_pasteur_tempC = mb_self_pasteur_tempC.readValue());
        ee_self_heating_tempC.writeEE(self_heating_tempC = mb_self_heating_tempC.readValue());
        ee_self_freezing_tempC.writeEE(self_freezing_tempC = mb_self_freezing_tempC.readValue());
        ee_self_pasteur_durat.writeEE(self_pasteur_durat = mb_self_durat_mm.readValue());
        ee_self_psteur_mode.writeEE(self_pasteur_mode = mb_self_mode_list.readValue());
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
    TaskManager::newTask(100,   [this]() -> void { readFastSensors(); mainThread(); });
    TaskManager::newTask(200,   [this]() -> void {
        mb_rtc_hh.writeValue((uint16_t)rtc.getHours());
        mb_rtc_mm.writeValue((uint16_t)rtc.getMinutes());
        mb_rtc_ss.writeValue((uint16_t)rtc.getSeconds());
        mb_rtc_DD.writeValue((uint16_t)rtc.getDay());
        mb_rtc_MM.writeValue((uint16_t)rtc.getMonth());
        mb_rtc_YY.writeValue((uint16_t)rtc.getYear());
    });
    TaskManager::newTask(2000,  [this]() -> void { readMediumSensors(); });
    TaskManager::newTask(30000, [this]() -> void { readSlowSensors(); });

    delay(50);
    mb_blow_calib_dosage.writeValue((uint16_t)(blow_calib_volume / 10));
    loadFromEE();

    delay(50);
    blowingSelectPreset(blowgun_preset_selected);
    blowingChangePrescaler(true);

    //readFastSensors();
    //readMediumSensors();
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
    mb_self_pasteur_tempC.writeValue((uint16_t)(self_pasteur_tempC = ee_self_pasteur_tempC.readEE()));
    mb_self_heating_tempC.writeValue((uint16_t)(self_heating_tempC = ee_self_heating_tempC.readEE()));
    mb_self_freezing_tempC.writeValue((uint16_t)(self_freezing_tempC = ee_self_freezing_tempC.readEE()));
    mb_self_durat_mm.writeValue((uint16_t)(self_pasteur_durat = ee_self_pasteur_durat.readEE()));
    mb_self_mode_list.writeValue((uint16_t)(self_pasteur_mode = ee_self_psteur_mode.readEE()));
}

void FXCore::setNewTime()
{

}

void FXCore::setNewDate()
{

}

void FXCore::pasteurStart()
{

}

void FXCore::pasteurPause()
{

}

void FXCore::pasteurResume()
{

}

void FXCore::pasteurFinish()
{

}

void FXCore::blowgunStart()
{

}

void FXCore::blowgunFinish()
{

}

void FXCore::heaterToggle(bool toggle)
{

}

void FXCore::mixerToggle(bool toggle)
{

}

void FXCore::pasteurTask()
{

}

void FXCore::mainThread()
{
    if (isBlowgunCall) //(!isConnectedTo380V && isBlowgunCall)
    {
        io_blowgun_r.write(true);
        delay(60000 / 38000 / (blowgun_preset_volume[blowgun_preset_selected] * 10));
        io_blowgun_r.write(false);
        isBlowgunCall = false;
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
    self_pasteur_mode = is_positive ? ++self_pasteur_mode : --self_pasteur_mode;

    if (self_pasteur_mode > 2)
        self_pasteur_mode = 2;

    ee_self_psteur_mode.writeEE(self_pasteur_mode);
    mb_self_mode_list.writeValue((uint16_t)self_pasteur_mode);
}

void FXCore::readSlowSensors()
{
    isConnectedTo380V = io_v380_s.readDigital();
    isWaterInWJacket = io_water_jacket_s.readDigital();
}

void FXCore::readMediumSensors()
{
    mb_batt_charge.writeValue(battChargeV = io_battery_s.readAnalog() / 40.95);
    liquidTempC = io_liquid_temp_s.readAnalog() / 20.475 - 50;
    mb_liq_tempC.writeValue((uint16_t)liquidTempC);
}

void FXCore::readFastSensors()
{
    isStopBtnPressed = isStopBtnPressed ? true : io_stop_btn_s.readDigital();
    
    if (io_blowgun_s.readDigital())
    {
        io_blowgun_r.write(true);
        delay(60000 * (blowgun_preset_volume[blowgun_preset_selected] * 20) / 38000);
        io_blowgun_r.write(false);
    }
    else
        io_blowgun_r.write(false);


    isMixerError = io_mixer_crash_s.readDigital();
}