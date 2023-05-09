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
    TimeUnit rtc_general_current = TimeUnit(false),
             rtc_general_new = TimeUnit(false),
             rtc_general_last_point = TimeUnit(true),
             rtc_prog_pasteur_started = TimeUnit(true),
             rtc_prog_expected_finish = TimeUnit(true),
             rtc_prog_pasteur_paused = TimeUnit(true),
             rtc_prog_pasteur_finished = TimeUnit(true);
    SettingUnit info_main_process = SettingUnit(NULL, &mb_proc_list, (uint8_t)OP320Process::COUNT - 1),
                info_main_step_show_hide = SettingUnit(NULL, &mb_step_name_list, 1),
                info_main_step = SettingUnit(NULL, &mb_step_index_list, (uint8_t)OP320Step::COUNT - 1),
                info_error_notify = SettingUnit(NULL, &mb_notification_list, (uint8_t)OP320Error::COUNT - 1),
                solo_heating_tempC = SettingUnit(&ee_solo_heating_tempC, &mb_solo_heating_tempC),
                solo_freezing_tempC = SettingUnit(&ee_solo_freezing_tempC, &mb_self_freezing_tempC),
                calibration_blowgun_main_dose = SettingUnit(NULL, &mb_blow_calib_dosage, 0, 1000),
                calibration_blowgun_dope_dose = SettingUnit(&ee_blowgun_calib_range, &mb_blow_calib_range, 0, 10),
                self_prog_pasteur_tempC = SettingUnit(&ee_self_pasteur_tempC, &mb_self_pasteur_tempC),
                self_prog_heating_tempC = SettingUnit(&ee_self_heating_tempC, &mb_self_heating_tempC),
                self_prog_freezing_tempC = SettingUnit(&ee_self_freezing_tempC, &mb_self_freezing_tempC),
                self_prog_duration_mm = SettingUnit(&ee_self_pasteur_durat, &mb_self_durat_mm),
                self_prog_mode = SettingUnit(&ee_self_pasteur_mode, &mb_self_mode_list, 2),
                master_water_saving_toggle = SettingUnit(&ee_master_water_saving, &mb_master_water_saving_monitor, 1),
                master_hysteresis_toggle = SettingUnit(&ee_master_hysteresis_toggle, &mb_master_hysteresis_monitor, 1),
                master_hysteresis_tempC = SettingUnit(&ee_master_hysteresis_value, &mb_hysteresis),
                master_pump_LM_performance = SettingUnit(&ee_master_pump_perf_lm, &mb_blowing_performance_lm),
                master_4ma_negative_limit = SettingUnit(&ee_master_4ma_adc_value, &mb_4ma_adc_limit),
                master_20ma_positive_limit = SettingUnit(&ee_master_20ma_adc_value, &mb_20ma_adc_limit);
    BlowingPreset blowgun_presets;
    AutoPasteurPreset auto_prog_presets;
public:
    FXCore();
};

#endif