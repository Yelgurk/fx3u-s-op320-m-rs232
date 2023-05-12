#include "../include/MBDispatcher/MBDispatcher.hpp"

void MBDispatcher::init()
{
    mb_comm_arr = new MBUnit*[COMM_COUNT] {
        &mb_comm_stop_proc,
        &mb_comm_blow_preset_1,
        &mb_comm_blow_preset_2,
        &mb_comm_blow_preset_3,
        &mb_comm_blow_preset_4,
        &mb_comm_blow_vInc,
        &mb_comm_blow_vDec,
        &mb_comm_blow_prescaler,
        &mb_comm_pass_7,
        &mb_comm_pass_8,
        &mb_comm_pass_9,
        &mb_comm_solo_tempC_cancel,
        &mb_comm_solo_tempC_accept,
        &mb_comm_rtc_new_cancel,
        &mb_comm_rtc_new_accept,
        &mb_comm_blow_calib_inc,
        &mb_comm_blow_calib_dec,
        &mb_comm_self_cancel,
        &mb_comm_self_accept,
        &mb_comm_self_mode_up,
        &mb_comm_self_mode_down,
        &mb_comm_auto_preset_1,
        &mb_comm_auto_preset_2,
        &mb_comm_auto_preset_3,
        &mb_comm_auto_toggle,
        &mb_comm_auto_cancel,
        &mb_comm_auto_accept,
        &mb_master_water_saving_toggle,
        &mb_master_hysteresis_toggle,
        &mb_master_cancel,
        &mb_master_accept,
        &mb_master_full_hard_reset,
        &mb_comm_self_pasteur_start,
        &mb_comm_solo_heating_toggle,
        &mb_comm_solo_freezing_toggle,
        &mb_comm_blowgun_run_btn,
        &mb_comm_goto_scr_blowing,
        &mb_comm_goto_scr_master
    };

    this->op320 = Modbus(FX3U_ID, Serial, 0);
    this->op320.start();
}

void MBDispatcher::poll()
{
    int8_t result = op320.poll(mb_au16data, MB_UNIT_BUFF_SIZE);
    
    if (result > 0 && result <= 4)
        poll_error();

    mb_au16data[MB_UNIT_BUFF_SIZE - 3] = op320.getInCnt();
    mb_au16data[MB_UNIT_BUFF_SIZE - 2] = op320.getOutCnt();
    mb_au16data[MB_UNIT_BUFF_SIZE - 1] = op320.getErrCnt();
}

void MBDispatcher::poll_error() { }

void MBDispatcher::commCheck()
{
    for (uint8_t index = 0; index < COMM_COUNT; index++)
        mb_comm_arr[index]->triggerFired();

}