#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

#ifndef _fxcore_hpp
#define _fxcore_hpp

#define BLOWGUN_PRESET_WASHING 3 

class FXCore : protected MBDispatcher,
               protected IODispatcher,
               protected EEDispatcher,
               public TaskManager
{
private:
    // sensors
    bool isConnectedTo380V = false;
    bool isWaterInWJacket = false;
    bool isMixerError = false;
    bool isStopBtnPressed = false;
    bool isBlowgunCall = false;
    int16_t liquidTempC = 0;
    uint16_t battChargeV = 0;
    
    // blowgun variables
    int16_t blowgun_preset_volume[BLOWGUN_PRESET_CNT] { 0 };
    uint8_t blowgun_prescaler_table[3] { 10, 25, 50 };
    uint8_t blowgun_pretime_table[3] { 15, 30, 60 };
    uint8_t blowgun_preset_selected = 0;
    uint8_t blowgun_prescaler_selected = 0;

    STM32RTC& rtc = STM32RTC::getInstance();
    
    void blowingSelectPreset(uint8_t preset_id);
    void blowingResetVolume(bool is_positive);
    void blowingChangePrescaler(bool boot_up = false);
    void getSensorsVal();

public:
    void init();
    void loadFromEE();
    void setNewTime();
    void setNewDate();
    void pasteurStart();
    void pasteurPause();
    void pasteurResume();
    void pasteurFinish();
    void blowgunStart();
    void blowgunFinish();
    void heaterToggle(bool toggle);
    void mixerToggle(bool toggle);
    void pasteurTask();
    void mainThread();
};

#endif