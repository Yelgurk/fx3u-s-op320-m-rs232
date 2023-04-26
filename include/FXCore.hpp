#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

#ifndef _fxcore_hpp
#define _fxcore_hpp

#define BLOWGUN_PRESET_CNT 4

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

    // conf variables
    uint8_t blowgun_volume_preset[BLOWGUN_PRESET_CNT] { 0 };

    STM32RTC& rtc = STM32RTC::getInstance();
    
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