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
    mb_comm_solo_tempC_cancel.addTrigger([this]() -> void {  });
    mb_comm_solo_tempC_accept.addTrigger([this]() -> void {  });
    mb_comm_rtc_new_cancel.addTrigger([this]() -> void {  });
    mb_comm_rtc_new_accept.addTrigger([this]() -> void {  });
    mb_comm_blow_calib_inc.addTrigger([this]() -> void {  });
    mb_comm_blow_calib_dec.addTrigger([this]() -> void {  });
    mb_comm_self_cancel.addTrigger([this]() -> void {  });
    mb_comm_self_accept.addTrigger([this]() -> void {  });
    mb_comm_self_mode_up.addTrigger([this]() -> void {  });
    mb_comm_self_mode_down.addTrigger([this]() -> void {  });
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

    TaskManager::newTask(20,    [this]() -> void { poll(); });
    TaskManager::newTask(20,    [this]() -> void { commCheck(); });
    TaskManager::newTask(200,   [this]() -> void {
        mb_rtc_hh.writeValue((uint16_t)rtc.getHours());
        mb_rtc_mm.writeValue((uint16_t)rtc.getMinutes());
        mb_rtc_ss.writeValue((uint16_t)rtc.getSeconds());
    });

    delay(50);
    loadFromEE();

    delay(50);
    blowingSelectPreset(blowgun_preset_selected);
    blowingChangePrescaler(true);
}

void FXCore::loadFromEE()
{
    for (uint8_t index = 0; index < BLOWGUN_PRESET_CNT; index++)
        blowgun_preset_volume[index] = ee_blowgun_preset_arr[index]->readEE() * 5;
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

void FXCore::getSensorsVal()
{
    isConnectedTo380V = io_v380_s.readDigital();
    isWaterInWJacket = io_water_jacket_s.readDigital();
    isMixerError = io_mixer_crash_s.readDigital();
    isStopBtnPressed = isStopBtnPressed ? true : io_stop_btn_s.readDigital();
    isBlowgunCall = isBlowgunCall ? true : io_blowgun_s.readDigital();
    liquidTempC = io_liquid_temp_s.readAnalog() / 20.475 - 50;
    battChargeV = io_battery_s.readAnalog() / 255.9375;
}