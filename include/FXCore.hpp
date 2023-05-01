#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>
#include "RTCObject.hpp"

#ifndef _fxcore_hpp
#define _fxcore_hpp

#define BLOWGUN_PRESET_WASHING 3 
#define SCR_HELLO_PAGE 1
#define SCR_MASTER_PAGE 2
#define SCR_MASTER_PAGE_TIME 3
#define SCR_BLOWING_PAGE 4
#define SCR_USER_MENU 5
#define PASTEUR_AWAIT_LIMIT_MM 60

enum class FinishFlag { Success, UserCall, MixerError, Power380vError, WaterJacketError };

class FXCore : protected MBDispatcher,
               protected IODispatcher,
               protected EEDispatcher,
               public TaskManager
{
private:
    // pasteur variables
    bool is_pasteur_proc_running = false;
    bool is_pasteur_proc_paused = false;
    bool is_pasteur_part_finished = false;
    bool is_waterJ_filled_yet = false;
    RTCObject rtc_pasteur_started;
    RTCObject rtc_pasteur_finished;
    RTCObject rtc_pasteur_paused;
    RTCObject rtc_pasteur_finish_time;
    RTCObject rtc_pasteur_in_proc;
    RTCObject rtc_pasteur_in_await;

    // solo func variables
    bool is_solo_heating = false;
    bool is_solo_freezing = false;

    // sensors
    bool is_connected_380V = false;
    bool is_water_in_jacket = false;
    bool is_mixer_error = false;
    bool is_stop_btn_pressed = false;
    bool is_blowgun_call = false;
    bool is_heaters_available = false;
    int16_t liquid_tempC = 0;
    uint16_t batt_chargeV = 0;
    
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
    void readSensors();
    void readTempCSensor();

public:
    void init();
    void loadFromEE();
    bool pasteurStart();
    void pasteurPause();
    void pasteurResume();
    void pasteurFinish(FinishFlag flag);
    void blowgunStart();
    void blowgunFinish();
    void heaterToggle(bool toggle);
    void freezingToggle(bool toggle);
    void mixerToggle(bool toggle);
    void heatersPID(uint8_t tempC);
    void stopAllFunc();
    bool pasteurTask();
    void heatingTask();
    void freezingTask();
    void mainThread();
};

#endif