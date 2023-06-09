#include <Arduino.h>
#include "../include/EEDispatcher/EEUnit.hpp"
#include "../include/MBDispatcher/MBUnit.hpp"
#include "../include/Core/SettingUnit.hpp"
#include "../include/Core/TimeUnit.hpp"

#ifndef BlowingPreset_hpp
#define BlowingPreset_hpp

#define DISPLAY_SCALE_VALUE 5
#define DISPLAY_SCALE_SEC DISPLAY_SCALE_VALUE
#define DISPLAY_SCALE_ML 50 
#define DISPLAY_SPLIT_SEC 60
#define MAX_VALUE 180

class BlowingPreset
{
private:
    EEUnit *ee_blowgun_preset_arr;

    SettingUnit *preset_selected,
                *preset_value,
                *preset_inc_value,
                *preset_dec_value;

    /* 0..2 => if (preset_selected < 3) 100ml..500ml/DISPLAY_SCALE || if (preset_selected == 3) 15sec..60sec/DISPLAY_SCALE  */
    uint8_t scale_selected = 0;
    uint8_t scale_ml_arr[3] { 10, 20, 30 };
    uint8_t scale_sec_arr[3] { 12, 12, 12 };//{ 3, 6, 12 };

    void displaySelectedScaler()
    {
        if (preset_selected->getValue() < BLOWGUN_PRESET_CNT - 1)
        {
            preset_inc_value->setValue(scale_ml_arr[scale_selected]);
            preset_inc_value->setScale(DISPLAY_SCALE_ML);
            preset_dec_value->setValue(scale_ml_arr[scale_selected]);
            preset_dec_value->setScale(DISPLAY_SCALE_ML);
        }
        else
        {
            preset_inc_value->setValue(scale_sec_arr[scale_selected]);
            preset_inc_value->setScale(DISPLAY_SCALE_SEC);
            preset_dec_value->setValue(scale_sec_arr[scale_selected]);
            preset_dec_value->setScale(DISPLAY_SCALE_SEC);
        }
        delay(2);
    }

public:
    void init(MBUnit *mb_blowing_preset_list, MBUnit *mb_blowing_volume, MBUnit *mb_blowing_incV, MBUnit *mb_blowing_decV, EEUnit *ee_blowgun_preset_arr)
    {
        this->ee_blowgun_preset_arr = ee_blowgun_preset_arr;  
        delay(2);  

        preset_selected = new SettingUnit(NULL, mb_blowing_preset_list, BLOWGUN_PRESET_CNT - 1, 1, false);
        preset_value = new SettingUnit(NULL, mb_blowing_volume, MAX_VALUE, DISPLAY_SCALE_VALUE, false);
        preset_inc_value = new SettingUnit(NULL, mb_blowing_incV, 0, DISPLAY_SCALE_ML, false);
        preset_dec_value = new SettingUnit(NULL, mb_blowing_decV, 0, DISPLAY_SCALE_ML, false);
        
        delay(2);
        selectPreset(preset_selected->getValue());
    }

    void selectPreset(uint8_t index)
    {
        preset_selected->setValue(index);
        preset_value->changeEEpointer(&ee_blowgun_preset_arr[preset_selected->getValue()]);
        preset_value->setSplit(preset_selected->getValue() < BLOWGUN_PRESET_CNT - 1 ? 1 : DISPLAY_SPLIT_SEC);

        delay(2);
        displaySelectedScaler();
    }

    void changeScaler()
    {
        scale_selected = scale_selected < 2 ? ++scale_selected : 0;
        displaySelectedScaler();
    }

    void incValue() {
        preset_value->setValue(MAX_VALUE - preset_inc_value->getValue() > preset_value->getValue() ? preset_inc_value->getValue() + preset_value->getValue() : MAX_VALUE);
    }

    void decValue()
    {
        preset_value->setValue(
            preset_value->getValue() >= preset_dec_value->getValue() ?
            preset_value->getValue() - preset_dec_value->getValue() :
            0
        );
    }

    uint16_t getValue() {
        return preset_selected->getValue() == BLOWGUN_PRESET_CNT - 1 ?
            preset_value->getValue() * 5 :
            preset_value->getValue() * 50;
    }

    bool isWashingSelected() {
        return preset_selected->getValue() == 3 ? true : false;
    }

    void setDefault() {
        ee_blowgun_preset_arr[0].writeEE(60);
        ee_blowgun_preset_arr[1].writeEE(90);
        ee_blowgun_preset_arr[2].writeEE(120);
        ee_blowgun_preset_arr[3].writeEE(60);
    }
};

#endif