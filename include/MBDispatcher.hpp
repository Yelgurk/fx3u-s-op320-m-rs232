#include <Arduino.h>
#include "MBUnit.hpp"
#include "Modbus.hpp"

#define FX3U_ID 1

#ifndef _mbdispatcher_hpp
#define _mbdispatcher_hpp

class MBDispatcher
{
private:
    Modbus op320;
    static const uint8_t mb_datas_size = 9;
    uint16_t mb_au16data[mb_datas_size];

    void poll_error();

public:
    MBUnit  mb_val1 = MBUnit(mb_au16data, 3, type::Uint16),
            mb_val2 = MBUnit(mb_au16data, 4, type::Uint16),
            mb_coil1 = MBUnit(mb_au16data, 2, type::Coil),
            mb_coil2 = MBUnit(mb_au16data, 17, type::Coil),
            mb_coil3 = MBUnit(mb_au16data, 18, type::Coil);

    void init();
    void poll();
};

#endif