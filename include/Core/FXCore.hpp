#include <Arduino.h>
#include <STM32RTC.h>

#include "../include/MBDispatcher/MBDispatcher.hpp"
#include "../include/IODispatcher/IODispatcher.hpp"
#include "../include/EEDispatcher/EEDispatcher.hpp"
#include "../include/TaskManager/TaskManager.hpp"

#include "../include/Core/SettingUnit.hpp"
#include "../include/Core/TimeUnit.hpp"

#include "../include/Data/AutoPasteurPreset.hpp"
#include "../include/Data/BlowingPreset.hpp"

#ifndef FXCore_hpp
#define FXCore_hpp
 
#define SCR_HELLO_PAGE 1
#define SCR_MASTER_PAGE 2
#define SCR_MASTER_PAGE_TIME 3
#define SCR_BLOWING_PAGE 4
#define SCR_USER_MENU 5
#define SCR_MASTER_SETTINGS 11
#define SCR_ERROR_NOTIFY 12

#define PASTEUR_AWAIT_LIMIT_MM 60
#define BLOWGUN_PRESET_WASHING 3

enum class FinishFlag   { Success, UserCall, MixerError, Power380vError, WaterJacketError, COUNT };
enum class OP320Process { Await, Washing, Heating, Freezing, Chargering, PasteurSelf, PasteurP1, PasteurP2, PasteurP3, COUNT };
enum class OP320Step    { Await, PasteurFinish, WaterJacket, PasteurHeating, PasteurProc, FreezingTo, HeatingTo, WaterJCirculation, COUNT };
enum class OP320Error   { Power380vOut, Mixer, Power380vIn, PowerMoreHour, WaterMoreHour, WaterAwait, COUNT };

class FXCore : protected MBDispatcher, protected IODispatcher, protected EEDispatcher, public TaskManager
{
private:
    TimeUnit    *rtc_general_current,
                *rtc_general_set_new,
                *rtc_general_last_point,
                *rtc_prog_pasteur_started,
                *rtc_prog_expected_finish,
                *rtc_prog_pasteur_paused,
                *rtc_prog_pasteur_finished;
    SettingUnit *info_main_process,
                *info_main_step_show_hide,
                *info_main_step,
                *info_error_notify,
                *solo_heating_tempC,
                *solo_freezing_tempC,
                *calibration_blowgun_main_dose,
                *calibration_blowgun_dope_dose,
                *self_prog_pasteur_tempC,
                *self_prog_heating_tempC,
                *self_prog_freezing_tempC,
                *self_prog_duration_mm,
                *self_prog_mode,
                *master_water_saving_toggle,
                *master_hysteresis_toggle,
                *master_hysteresis_tempC,
                *master_pump_LM_performance,
                *master_4ma_negative_limit,
                *master_20ma_positive_limit;
    BlowingPreset *blowgun_presets;
    AutoPasteurPreset *auto_prog_presets;
public:
    FXCore();
};

#endif