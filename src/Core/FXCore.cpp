#include "../include/Core/FXCore.hpp"

FXCore::FXCore()
{
    EEDispatcher::init();
    MBDispatcher::init();
    IODispatcher::init();
}