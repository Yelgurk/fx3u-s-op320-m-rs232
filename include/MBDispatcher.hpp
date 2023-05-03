#include <Arduino.h>
#include "MBUnit.hpp"
#include "Modbus.hpp"

#define FX3U_ID 1

#ifndef _mbdispatcher_hpp
#define _mbdispatcher_hpp

#define COMM_COUNT (36 + 4)

class MBDispatcher
{
private:
    Modbus op320;
    uint16_t mb_au16data[MB_UNIT_BUFF_SIZE];

    void poll_error();

public:
    MBUnit **mb_comm_arr;

    MBUnit mb_set_op320_scr = MBUnit(mb_au16data, 0, type::Uint16),
           mb_get_op320_scr = MBUnit(mb_au16data, 1, type::Uint16),
           mb_batt_charge = MBUnit(mb_au16data, 2, type::Uint16),
           mb_liq_tempC = MBUnit(mb_au16data, 3, type::Uint16),
           mb_rtc_hh = MBUnit(mb_au16data, 4, type::Uint16),
           mb_rtc_mm = MBUnit(mb_au16data, 5, type::Uint16),
           mb_rtc_ss = MBUnit(mb_au16data, 6, type::Uint16),
           mb_proc_list = MBUnit(mb_au16data, 7, type::Uint16),
           mb_step_name_list = MBUnit(mb_au16data, 8, type::Uint16),
           mb_step_index_list = MBUnit(mb_au16data, 9, type::Uint16),
           mb_proc_start_hh = MBUnit(mb_au16data, 10, type::Uint16),
           mb_proc_start_mm = MBUnit(mb_au16data, 11, type::Uint16),
           mb_proc_end_hh = MBUnit(mb_au16data, 12, type::Uint16),
           mb_proc_end_mm = MBUnit(mb_au16data, 13, type::Uint16),
           mb_blowing_preset_list = MBUnit(mb_au16data, 14, type::Uint16),
           mb_blowing_volume = MBUnit(mb_au16data, 15, type::Uint16),
           mb_blowing_incV = MBUnit(mb_au16data, 16, type::Uint16),
           mb_blowing_decV = MBUnit(mb_au16data, 17, type::Uint16),
           mb_solo_heating_tempC = MBUnit(mb_au16data, 18, type::Uint16),
           mb_solo_freezing_tempC = MBUnit(mb_au16data, 19, type::Uint16),
           mb_rtc_new_hh = MBUnit(mb_au16data, 20, type::Uint16),
           mb_rtc_new_mm = MBUnit(mb_au16data, 21, type::Uint16),
           mb_rtc_new_ss = MBUnit(mb_au16data, 22, type::Uint16),
           mb_rtc_new_DD = MBUnit(mb_au16data, 23, type::Uint16),
           mb_rtc_new_MM = MBUnit(mb_au16data, 24, type::Uint16),
           mb_rtc_new_YY = MBUnit(mb_au16data, 25, type::Uint16),
           mb_rtc_DD = MBUnit(mb_au16data, 26, type::Uint16),
           mb_rtc_MM = MBUnit(mb_au16data, 27, type::Uint16),
           mb_rtc_YY = MBUnit(mb_au16data, 28, type::Uint16),
           mb_blow_calib_dosage = MBUnit(mb_au16data, 29, type::Uint16),
           mb_blow_calib_range = MBUnit(mb_au16data, 30, type::Uint16),
           mb_self_pasteur_tempC = MBUnit(mb_au16data, 31, type::Uint16),
           mb_self_heating_tempC = MBUnit(mb_au16data, 32, type::Uint16),
           mb_self_freezing_tempC = MBUnit(mb_au16data, 33, type::Uint16),
           mb_self_durat_mm = MBUnit(mb_au16data, 34, type::Uint16),
           mb_self_mode_list = MBUnit(mb_au16data, 35, type::Uint16),
           mb_auto_pasteur_tempC = MBUnit(mb_au16data, 36, type::Uint16),
           mb_auto_heating_tempC = MBUnit(mb_au16data, 37, type::Uint16),
           mb_auto_freezing_tempC = MBUnit(mb_au16data, 38, type::Uint16),
           mb_auto_durat_mm = MBUnit(mb_au16data, 39, type::Uint16),
           mb_auto_run_rtc_hh = MBUnit(mb_au16data, 40, type::Uint16),
           mb_auto_run_rtc_mm = MBUnit(mb_au16data, 41, type::Uint16),
           mb_auto_preset_list = MBUnit(mb_au16data, 42, type::Uint16),
           mb_auto_preset_toggle = MBUnit(mb_au16data, 43, type::Uint16),
           mb_4ma_adc_limit = MBUnit(mb_au16data, 44, type::Uint16),
           mb_20ma_adc_limit = MBUnit(mb_au16data, 45, type::Uint16),
           mb_hysteresis = MBUnit(mb_au16data, 46, type::Uint16),
           mb_blowing_performance_lm = MBUnit(mb_au16data, 47, type::Uint16),
           mb_notification_list = MBUnit(mb_au16data, 48, type::Uint16),

           mb_comm_stop_proc = MBUnit(mb_au16data, 784, type::Coil),
           mb_comm_blow_preset_1 = MBUnit(mb_au16data, 785, type::Coil),
           mb_comm_blow_preset_2 = MBUnit(mb_au16data, 786, type::Coil),
           mb_comm_blow_preset_3 = MBUnit(mb_au16data, 787, type::Coil),
           mb_comm_blow_preset_4 = MBUnit(mb_au16data, 788, type::Coil),
           mb_comm_blow_vInc = MBUnit(mb_au16data, 789, type::Coil),
           mb_comm_blow_vDec = MBUnit(mb_au16data, 790, type::Coil),
           mb_comm_blow_prescaler = MBUnit(mb_au16data, 791, type::Coil),
           mb_comm_pass_7 = MBUnit(mb_au16data, 792, type::Coil),
           mb_comm_pass_8 = MBUnit(mb_au16data, 793, type::Coil),
           mb_comm_pass_9 = MBUnit(mb_au16data, 794, type::Coil),
           mb_comm_solo_tempC_cancel = MBUnit(mb_au16data, 795, type::Coil),
           mb_comm_solo_tempC_accept = MBUnit(mb_au16data, 796, type::Coil),
           mb_comm_rtc_new_cancel = MBUnit(mb_au16data, 797, type::Coil),
           mb_comm_rtc_new_accept = MBUnit(mb_au16data, 798, type::Coil),
           mb_comm_blow_calib_inc = MBUnit(mb_au16data, 799, type::Coil),
           
           mb_comm_blow_calib_dec = MBUnit(mb_au16data, 800, type::Coil),
           mb_comm_self_cancel = MBUnit(mb_au16data, 801, type::Coil),
           mb_comm_self_accept = MBUnit(mb_au16data, 802, type::Coil),
           mb_comm_self_mode_up = MBUnit(mb_au16data, 803, type::Coil),
           mb_comm_self_mode_down = MBUnit(mb_au16data, 804, type::Coil),
           mb_comm_auto_preset_1 = MBUnit(mb_au16data, 805, type::Coil),
           mb_comm_auto_preset_2 = MBUnit(mb_au16data, 806, type::Coil),
           mb_comm_auto_preset_3 = MBUnit(mb_au16data, 807, type::Coil),
           mb_comm_auto_toggle = MBUnit(mb_au16data, 808, type::Coil),
           mb_comm_auto_cancel = MBUnit(mb_au16data, 809, type::Coil),
           mb_comm_auto_accept = MBUnit(mb_au16data, 810, type::Coil),
           mb_master_water_saving_toggle = MBUnit(mb_au16data, 811, type::Coil),
           mb_master_hysteresis_toggle = MBUnit(mb_au16data, 812, type::Coil),
           mb_master_cancel = MBUnit(mb_au16data, 815, type::Coil),
           
           mb_master_water_saving_monitor = MBUnit(mb_au16data, 813, type::Coil),
           mb_master_hysteresis_monitor = MBUnit(mb_au16data, 814, type::Coil),

           mb_master_accept = MBUnit(mb_au16data, 816, type::Coil),
           mb_master_full_hard_reset = MBUnit(mb_au16data, 817, type::Coil),
           mb_comm_self_pasteur_start = MBUnit(mb_au16data, 818, type::Coil),
           mb_comm_solo_heating_toggle = MBUnit(mb_au16data, 819, type::Coil),
           mb_comm_solo_freezing_toggle = MBUnit(mb_au16data, 820, type::Coil),
           mb_comm_blowgun_run_btn = MBUnit(mb_au16data, 821, type::Coil);

    void init();
    void poll();
    void commCheck();
};

#endif