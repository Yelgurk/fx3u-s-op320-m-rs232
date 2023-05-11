#include <Arduino.h>
#include "../include/EEDispatcher/EEUnit.hpp"
#include "../include/MBDispatcher/MBUnit.hpp"

#ifndef SettingUnit_hpp
#define SettingUnit_hpp

class SettingUnit
{
private:
    EEUnit *ee_var_pointer = NULL;
    MBUnit *mb_var_pointer = NULL;
    uint8_t workable_value = 0;
    uint8_t max_limit = 0;
    uint8_t display_scale = 1;
    uint8_t display_split = 1;

public:
    SettingUnit(EEUnit *ee_var_pointer, MBUnit *mb_var_pointer, uint8_t max_limit = 0, uint8_t display_scale = 1);
    void changeEEpointer(EEUnit *ee_var_pointer);
    void setScale(uint8_t display_scale);
    void setSplit(uint8_t display_split);
    void displayValue();
    void setValueByModbus();
    void setValue(uint8_t value);
    void acceptNewValue();
    void refreshValue();
    void incValue();
    void decValue();
    bool getState();
    uint8_t getValue();
    uint16_t getScaledValue();
};

#endif