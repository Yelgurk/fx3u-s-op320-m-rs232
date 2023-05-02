#include <Arduino.h>
#include "MBDispatcher.hpp"
#include "IODispatcher.hpp"
#include "EEDispatcher.hpp"
#include "TaskManager.hpp"
#include <STM32RTC.h>
#include "RTCObject.hpp"

#ifndef _fxcore_hpp
#define _fxcore_hpp
 
#define SCR_HELLO_PAGE 1
#define SCR_MASTER_PAGE 2
#define SCR_MASTER_PAGE_TIME 3
#define SCR_BLOWING_PAGE 4
#define SCR_USER_MENU 5
#define SCR_ERROR_NOTIFY 12
#define PASTEUR_AWAIT_LIMIT_MM 60
#define BLOWGUN_PRESET_WASHING 3

enum class FinishFlag { Success, UserCall, MixerError, Power380vError, WaterJacketError };
enum class OP320Process { Await, Washing, Heating, Freezing, Chargering, PasteurSelf, PasteurP1, PasteurP2, PasteurP3 };
enum class OP320Step { Await, PasteurFinish, WaterJacket, PasteurHeating, PasteurProc, FreezingTo, HeatingTo, WaterJCirculation };
enum class OP320Error { Power380vOut, Mixer, Power380vIn, PowerMoreHour, WaterMoreHour, WaterAwait };

class FXCore : protected MBDispatcher,
               protected IODispatcher,
               protected EEDispatcher,
               public TaskManager
{
private:
    // pasteur proc variables
    bool is_pasteur_proc_running = false;
    bool is_pasteur_proc_paused = false;
    bool is_pasteur_part_finished = false;
    bool is_freezing_part_finished = false;
    bool is_heating_part_finished = false;
    bool is_waterJ_filled_yet = false;
    bool is_pasteur_need_in_freezing = false;
    bool is_pasteur_need_in_heating = false;
    uint8_t pasteur_proc_time_span_mm = 0;
    uint8_t pasteur_proc_pasteur_tempC = 0;
    uint8_t pasteur_proc_heeting_tempC = 0;
    uint8_t pasteur_proc_freezing_tempC = 0;
    RTCObject rtc_pasteur_started;
    RTCObject rtc_pasteur_finished;
    RTCObject rtc_pasteur_paused;
    RTCObject rtc_pasteur_finish_time;
    RTCObject rtc_pasteur_in_proc;
    RTCObject rtc_pasteur_in_await;
    RTCObject rtc_current_time;
    uint8_t pasteur_preset_runned = 0;

    // pasteur preset variables
    RTCObject pasteur_rtc_triggers[PASTEUR_PRESET_CNT];
    uint8_t pasteur_preset_pasteur_tempC[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_heating_tempC[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_freezing_tempC[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_durat_mm[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_run_on_hh[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_run_on_mm[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_run_toggle[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_is_runned_today[PASTEUR_PRESET_CNT] { 0 };
    uint8_t pasteur_preset_selected = 0;

    // blowgun preset variables
    int16_t blowgun_preset_volume[BLOWGUN_PRESET_CNT] { 0 };
    uint8_t blowgun_prescaler_table[3] { 10, 25, 50 };
    uint8_t blowgun_pretime_table[3] { 15, 30, 60 };
    uint8_t blowgun_preset_selected = 0;
    uint8_t blowgun_prescaler_selected = 0;
    TMUnit *blowgun_washing_task;
    bool is_blowgun_washing_runned = false;

    // self pasteur
    uint8_t settings_self_pasteur_tempC = 0;
    uint8_t settings_self_heating_tempC = 0;
    uint8_t settings_self_freezing_tempC = 0;
    uint8_t settings_self_pasteur_durat = 0;
    uint8_t settings_self_pasteur_mode = 0;

    // solo func variables
    bool is_solo_heating = false;
    bool is_solo_freezing = false;

    // solo temp c
    uint8_t solo_heating_tempC = 0;
    uint8_t solo_freezing_tempC = 0;

    // blowgun calibration
    uint16_t blow_calib_volume = 3000;
    uint8_t blowing_calib_range = 100;

    // sensors
    bool is_connected_380V = false;
    bool is_water_in_jacket = false;
    bool is_mixer_error = false;
    bool is_stop_btn_pressed = false;
    bool is_blowgun_call = false;
    bool is_heaters_available = false;
    int16_t liquid_tempC = 0;
    uint16_t batt_chargeV = 0;

    STM32RTC& rtc = STM32RTC::getInstance();
    
    void blowingSelectPreset(uint8_t preset_id);
    void blowingResetVolume(bool is_positive);
    void blowingChangePrescaler(bool boot_up = false);
    void selfPasteurChangeMode(bool is_positive);
    void autoPasteurSelectPreset(uint8_t preset_id);
    void readSensors();
    void readTempCSensor();

public:
    void init();
    void loadFromEE();
    bool pasteurStart(bool is_user_call, uint8_t preset_index = 0);
    void pasteurPause(OP320Error to_op320);
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
    void heatingTask(uint8_t expectedTempC);
    void freezingTask(uint8_t expectedTempC);
    void mainThread();
    void displayState();
};

#endif