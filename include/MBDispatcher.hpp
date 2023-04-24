#include <Arduino.h>
#include "MBUnit.hpp"
#include "Modbus.hpp"

#define FX3U_ID 1


#ifndef _mbdispatcher_hpp
#define _mbdispatcher_hpp

class MBDispatcher
{
public:
    static const uint8_t mb_datas_size = 9;
    Modbus op320;
    uint16_t mb_au16data[mb_datas_size];

    void poll_error();

public:
    MBUnit  MB_val1 = MBUnit(mb_au16data, 3, type::Uint16),
            MB_val2 = MBUnit(mb_au16data, 4, type::Uint16),
            MB_coil1 = MBUnit(mb_au16data, 2, type::Coil),
            MB_coil2 = MBUnit(mb_au16data, 17, type::Coil),
            MB_coil3 = MBUnit(mb_au16data, 18, type::Coil);

public:
    void init();
    void poll();

};

#endif