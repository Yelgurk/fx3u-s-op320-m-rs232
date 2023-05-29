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
#include "../include/Data/PasswordAccess.hpp"

#ifndef FXCore_hpp
#define FXCore_hpp

#define TEST_LOCAL 0
#define TEST_TEMPC_MAX 70

#define SCR_HELLO_PAGE 1
#define SCR_MASTER_PAGE 2
#define SCR_MASTER_PAGE_TIME 3
#define SCR_BLOWING_PAGE 4
#define SCR_USER_MENU 5
#define SCR_MASTER_SETTINGS 11
#define SCR_ERROR_NOTIFY 12

#define HEAT_MM_FOR_1CELSIUS_UP 3
#define WATER_JACKED_FILL_EST_MM 5
#define PASTEUR_AWAIT_LIMIT_MM 60
#define BLOWGUN_PRESET_WASHING 3

#define SENSOR_TEMPC_CALL_CNT 15
#define SENSOR_TEMPC_MAXVAL   (double)(4095 / 22 * 20)
#define SENSOR_TEMPC_MINVAL   (double)(4095 / 22 * 4)
#define SENSOR_1TC_IN_MA_VAL(max_tempC, min_tempC) ((SENSOR_TEMPC_MAXVAL - SENSOR_TEMPC_MINVAL) / (max_tempC + min_tempC))

#define SENSOR_CHARGE_MAXVAL (double)2320
#define SENSOR_CHARGE_MINVAL (double)(SENSOR_CHARGE_MAXVAL / 26 * 23)
#define SENSOR_CHARGE_RANGE  (double)(SENSOR_CHARGE_MAXVAL - SENSOR_CHARGE_MINVAL)

enum class MACHINE_TYPE  : uint8_t { DMP_flow, DM_flow, DMP, PM, HM, COUNT };
enum class MACHINE_STATE : uint8_t { Await, Flowing, Heating, Freezing, Pasteurizing, COUNT };
enum class FINISH_FLAG   : uint8_t { Success, UserCall, MixerError, Power380vError, WaterJacketError, COUNT };
enum class PROG_STATE    : uint8_t { PasteurRunning, PasteurPaused, PasteurFinished, FreezingFinished, HeatingFinished, CycleFinished, COUNT };
enum class OP320_PROCESS : uint8_t { Await, Washing, Heating, Freezing, Chargering, PasteurSelf, PasteurP1, PasteurP2, PasteurP3, AutoHeating, COUNT };
enum class OP320_STEP    : uint8_t { Await, PasteurFinish, WaterJacket, PasteurHeating, PasteurProc, FreezingTo, HeatingTo, WaterJCirculation, ErrSolveAwait, COUNT };
enum class OP320_ERROR   : uint8_t { Power380vOut, Mixer, Power380vIn, PowerMoreHour, WaterMoreHour, WaterAwait, SlowHeating, PasteurAlready, TempCSensorError, COUNT };

class FXCore : protected MBDispatcher, protected IODispatcher, protected EEDispatcher, protected PasswordAccess, public TaskManager
{
private:
    STM32RTC& rtc = STM32RTC::getInstance();
    
    SettingUnit *machine_type;

    /* sensors var */
    bool is_water_in_jacket = false,
         is_flowing_call = false,
         is_flowing_uncalled = false,
         is_stop_btn_pressed = false,
         is_connected_380V = false,
         is_heaters_starters_state = false,
         is_mixer_error = false;
    int16_t liquid_tempC = 0;
    uint8_t batt_chargeV = 0;
    double sensor_call_tempC[SENSOR_TEMPC_CALL_CNT] { 0 };
    uint8_t sensor_call_index = 0;

    /* tasks var */
    bool is_pasteur_part_finished_crutch = false,
         is_heating_part_finished_crutch = false,
         is_freezing_part_finished_crutch = false,
         is_task_freezing_running = false,
         is_task_heating_running = false,
         is_task_flowing_running = false,
         is_task_heating_extra = false,
         start_error_displayed_yet = false,
         heat_error_displayed_yet = false,
         flow_error_displayed_yet = false,
         water_saving_toggle = false,
         flowing_task_paused = false,
         sensor_tempC_error = false;
    uint8_t prog_pasteur_tempC = 0,
            prog_heating_tempC = 0,
            prog_freezing_tempC = 0,
            prog_selected_duration_mm = 0,
            extra_heating_tempC = 0;
    uint32_t flowing_current_ms = 0,
             flowing_trigger_ms = 0;
    SettingUnit *machine_state,
                *prog_running,
                *prog_state,
                *prog_preset_selected,
                *prog_need_in_freezing,
                *prog_freezing_part_finished,
                *prog_need_in_heating,
                *prog_heating_part_finished,
                *prog_jacket_filled,
                *prog_pasteur_tempC_reached,
                *prog_finished_flag;
    TimeUnit    *rtc_prog_pasteur_started,
                *rtc_prog_pasteur_paused,
                *rtc_prog_finished,
                *rtc_prog_expected_finish,
                *rtc_blowing_started,
                *rtc_blowing_finish;

    /* configs */
    TimeUnit    *rtc_general_current,
                *rtc_general_last_time_point,
                *rtc_general_set_new;
    SettingUnit *scr_set_op320,
                *scr_get_op320,
                *info_main_liq_tempC,
                *info_main_batt_charge,
                *info_main_process,
                *info_main_step_show_hide,
                *info_main_step,
                *info_error_notify,
                *solo_heating_tempC,
                *solo_freezing_tempC,
                *calibration_flowgun_main_dose,
                *calibration_flowgun_dope_dose,
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
                *master_20ma_positive_limit,
                *master_calibr_side_toggle;
    BlowingPreset flowgun_presets = BlowingPreset();
    AutoPasteurPreset auto_prog_presets = AutoPasteurPreset();

    void checkIsHardReseted();
    bool checkIsProgWasRunned();
    void displayMainInfoVars();
    void displayTasksDeadline();
    void gotoMainScreen();
    void checkDayFix();
    void setActivityPoint();
    void callTempCSensor();

public:
    void init();
    void setNewDateTime();
    void stopAllTasks(bool stop_presets = true);
    void taskTryToggleFlowing();
    void taskTryToggleHeating(bool turn_on);
    void taskTryToggleFreezing(bool turn_on);
    void taskToggleMixer(bool turn_on);
    bool taskStartProg(uint8_t pasteur_preset = 0);
    void taskPauseProg(OP320_ERROR flag);
    void taskResumeProg();
    void taskFinishProg(FINISH_FLAG flag);
    bool taskFinishFlowing(bool forced = true);
    void flowgunOff();
    bool taskHeating(uint8_t expected_tempC);
    bool taskFreezing(uint8_t expected_tempC);
    bool checkAutoStartup(bool force_off = false);
    bool threadProg();
    void threadMain();
    void readSensors();
    void readWaterInJacket();
    void displayOP320States();
    void hardReset();
};

#endif