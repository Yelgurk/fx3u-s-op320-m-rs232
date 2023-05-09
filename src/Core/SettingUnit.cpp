#include "../include/Core/SettingUnit.hpp"

SettingUnit::SettingUnit(EEUnit *ee_var_pointer, MBUnit *mb_var_pointer, uint8_t max_limit, uint8_t display_scale)
{
    this->ee_var_pointer = ee_var_pointer;
    this->mb_var_pointer = mb_var_pointer;
    this->max_limit = max_limit;
    this->display_scale = display_scale;

    refreshValue();
}

void SettingUnit::changeEEpointer(EEUnit *ee_var_pointer)
{
    this->ee_var_pointer = ee_var_pointer;
    refreshValue();
}

void SettingUnit::setScale(uint8_t display_scale) {
    this->display_scale = display_scale;
    displayValue();
}

void SettingUnit::setSplit(uint8_t display_split) {
    this->display_split = display_split;
    displayValue();
}

void SettingUnit::displayValue()
{
    if (mb_var_pointer)
    {
        if (display_split <= 1)
            mb_var_pointer->writeValue((uint16_t)((uint16_t)this->workable_value * (uint16_t)this->display_scale));
        else
        {
            uint16_t final_value = (uint16_t)this->workable_value * (uint16_t)this->display_scale;
            mb_var_pointer->writeValue((uint16_t)((display_split == 60 ? final_value / display_split * 100 : final_value / display_split) + (final_value % display_split)));
        }
    }
}

void SettingUnit::setValueByModbus() {
    if (mb_var_pointer) setValue(mb_var_pointer->readValue() / display_scale);
}

void SettingUnit::setValue(uint8_t value)
{
    this->workable_value = (max_limit == 0 && value <= 255) || value <= max_limit ? value : (max_limit == 0 ? 255 : max_limit);
    acceptNewValue();
}

void SettingUnit::acceptNewValue()
{
    if (ee_var_pointer) ee_var_pointer->writeEE(this->workable_value);
    refreshValue();
}

void SettingUnit::refreshValue()
{
    if (ee_var_pointer) ee_var_pointer->readEE(&this->workable_value);
    displayValue();
}

void SettingUnit::incValue()
{
    this->workable_value = (max_limit > 0 && workable_value < max_limit) || workable_value < 255 ? ++workable_value : 0;
    acceptNewValue();
}

void SettingUnit::decValue()
{
    this->workable_value = workable_value > 0 ? --workable_value : (max_limit > 0 ? max_limit : 255);
    acceptNewValue();
}

uint8_t SettingUnit::getValue() {
    return this->workable_value;
}

uint16_t SettingUnit::getScaledValue(){
    return (uint16_t)getValue() * display_scale;
}