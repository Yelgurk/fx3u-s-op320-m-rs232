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
 
#define SCR_HELLO_PAGE 1
#define SCR_MASTER_PAGE 2
#define SCR_MASTER_PAGE_TIME 3
#define SCR_BLOWING_PAGE 4
#define SCR_USER_MENU 5
#define SCR_MASTER_SETTINGS 11
#define SCR_ERROR_NOTIFY 12

#define PASTEUR_AWAIT_LIMIT_MM 60
#define BLOWGUN_PRESET_WASHING 3

enum class MACHINE_STATE : uint8_t { Nothing, Heating, Freezing, Pasteurizing, COUNT };
enum class FINISH_FLAG   : uint8_t { Success, UserCall, MixerError, Power380vError, WaterJacketError, COUNT };
enum class PROG_TYPE     : uint8_t { OnlyPasteur, WithFreezing, FullCycle, COUNT };
enum class PROG_STATE    : uint8_t { PasteurRunning, PasteurPaused, PasteurFinished, FreezingFinished, HeatingFinished, COUNT };
enum class OP320_PROCESS : uint8_t { Await, Washing, Heating, Freezing, Chargering, PasteurSelf, PasteurP1, PasteurP2, PasteurP3, COUNT };
enum class OP320_STEP    : uint8_t { Await, PasteurFinish, WaterJacket, PasteurHeating, PasteurProc, FreezingTo, HeatingTo, WaterJCirculation, COUNT };
enum class OP320_ERROR   : uint8_t { Power380vOut, Mixer, Power380vIn, PowerMoreHour, WaterMoreHour, WaterAwait, COUNT };

/* int to enum => uint8_t X = 1; ENUM Y = static_cast<ENUM>(x); */

class FXCore : protected MBDispatcher, protected IODispatcher, protected EEDispatcher, protected PasswordAccess, public TaskManager
{
private:
    STM32RTC& rtc = STM32RTC::getInstance();
    
    /* task var */
    bool is_task_freezing_running = false,
         is_task_heating_running = false;
    SettingUnit *prog_running,
                *prog_state,
                *prog_preset_selected,
                *prog_need_in_freezing,
                *prog_need_in_heating,
                *prog_jacket_filled,
                *prog_finished_flag;
    TimeUnit    *rtc_prog_pasteur_started,
                *rtc_prog_pasteur_paused,
                *rtc_prog_finished,
                *rtc_prog_expected_finish,
                *rtc_prog_duration_mm_span;

    /* configs */
    TimeUnit    *rtc_general_current,
                *rtc_general_last_time_point,
                *rtc_general_set_new;
    SettingUnit *scr_set_op320,
                *scr_get_op320,
                *info_main_process,
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

    void checkIsHardReseted();
    void checkIsProgWasRunned();

public:
    FXCore();
    void setNewDateTime();
    void stopAllTasks();
    void taskTryBlowing();
    void taskTryToggleHeating(bool turn_on);
    void taskTryToggleFreezing(bool turn_on);
    void taskToggleMixer(bool turn_on);
    void taskStartProg(uint8_t pasteur_preset = 0);
    void taskPauseProg(OP320_ERROR flag);
    void taskResumeProg();
    void taskFinishProg(FINISH_FLAG flag);
    void taskHeaters(uint8_t expected_tempC);
    void threadProg();
    void threadMain();
    void readSensors();
    void unlockHeatStarter();
    void hardReset();
};

#endif