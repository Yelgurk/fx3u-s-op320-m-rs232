#include "MBDispatcher.hpp"

void MBDispatcher::init()
{
    this->op320 = Modbus(FX3U_ID, Serial, 0);
    this->op320.start();
}

void MBDispatcher::poll()
{
    int8_t result = op320.poll(mb_au16data, mb_datas_size);
    
    if (result > 0 && result <= 4)
        poll_error();

    mb_au16data[mb_datas_size - 3] = op320.getInCnt();
    mb_au16data[mb_datas_size - 2] = op320.getOutCnt();
    mb_au16data[mb_datas_size - 1] = op320.getErrCnt();
}

void MBDispatcher::poll_error() { }