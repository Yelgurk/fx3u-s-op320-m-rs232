#include "FXCore.hpp"

void FXCore::init()
{
    this->MBDispatcher::init();
    this->EEDispatcher::init();
    this->IODispatcher::init();
    relaysOff();
    
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.begin();

    loadFromEE();

    newTask(25,     [this]() -> void { poll(); });
    newTask(200,    [this]() -> void {
        mb_rtc_hh.writeValue((uint16_t)rtc.getHours());
        mb_rtc_mm.writeValue((uint16_t)rtc.getMinutes());
        mb_rtc_ss.writeValue((uint16_t)rtc.getSeconds());
    });

    //newTask(50,     [this]() -> void { getSensorsVal(); });
    //newTask(200,    [this]() -> void { mainThread(); });
}

void FXCore::loadFromEE()
{

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

void FXCore::pasteurTask()
{

}

void FXCore::mainThread()
{
    if (mb_comm_stop_proc.readValue() == 1)
        io_blowgun_r.write(true);
    else
        io_blowgun_r.write(false);
}