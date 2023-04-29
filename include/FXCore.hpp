#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>

#ifndef _fxcore_hpp
#define _fxcore_hpp

#define BLOWGUN_PRESET_WASHING 3 
#define SCR_USER_MENU 5

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

    // solo temp c
    uint8_t solo_heating_tempC = 0;
    uint8_t solo_freezing_tempC = 0;

    // blowgun calibration
    uint16_t blow_calib_volume = 3000;
    uint8_t blowing_calib_range = 100;

    // self pasteur
    uint8_t self_pasteur_tempC = 0;
    uint8_t self_heating_tempC = 0;
    uint8_t self_freezing_tempC = 0;
    uint8_t self_pasteur_durat = 0;
    uint8_t self_pasteur_mode = 0;

    STM32RTC& rtc = STM32RTC::getInstance();
    
    void blowingSelectPreset(uint8_t preset_id);
    void blowingResetVolume(bool is_positive);
    void blowingChangePrescaler(bool boot_up = false);
    void selfPasteurChangeMode(bool is_positive);
    void readSlowSensors();
    void readMediumSensors();
    void readFastSensors();

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