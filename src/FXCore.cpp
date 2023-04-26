#include "FXCore.hpp"

void FXCore::init()
{
    this->MBDispatcher::init();
    this->IODispatcher::init();
    this->EEDispatcher::init();
    relaysOff();

    newTask([this]() -> void { poll(); }, 0);
    newTask([this]() -> void { mb_coil1.writeValue(!mb_coil1.readValue()); }, 2000);
}