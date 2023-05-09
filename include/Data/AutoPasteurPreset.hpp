#include <Arduino.h>
#include "../include/EEDispatcher/EEUnit.hpp"
#include "../include/MBDispatcher/MBUnit.hpp"
#include "../include/Core/SettingUnit.hpp"
#include "../include/Core/TimeUnit.hpp"

#ifndef AutoPasteurPreset_hpp
#define AutoPasteurPreset_hpp

class AutoPasteurPreset
{
private:
    TimeUnit *run_rtc_trigger[PASTEUR_PRESET_CNT];

    SettingUnit *preset_selected,
                *preset_pasteur_tempC,
                *preset_heating_tempC,
                *preset_freezing_tempC,
                *preset_duration_mm,
                *preset_toggle,
                *preset_runned_today;

    EEUnit *ee_pasteur_tempC,
           *ee_heating_tempC,
           *ee_freezing_tempC,
           *ee_pasteur_duratMM,
           *ee_run_on_hh,
           *ee_run_on_mm,
           *ee_run_toggle,
           *ee_is_runned_today;

public:
    AutoPasteurPreset(
        EEUnit *ee_pasteur_tempC,
        EEUnit *ee_heating_tempC,
        EEUnit *ee_freezing_tempC,
        EEUnit *ee_pasteur_duratMM,
        EEUnit *ee_run_on_hh,
        EEUnit *ee_run_on_mm,
        EEUnit *ee_run_toggle,
        EEUnit *ee_is_runned_today,
        MBUnit *mb_auto_preset_list,
        MBUnit *mb_auto_pasteur_tempC,
        MBUnit *mb_auto_heating_tempC,
        MBUnit *mb_auto_freezing_tempC,
        MBUnit *mb_auto_durat_mm,
        MBUnit *mb_auto_run_rtc_hh,
        MBUnit *mb_auto_run_rtc_mm,
        MBUnit *mb_auto_preset_toggle
    )
    {
        this->ee_pasteur_tempC = ee_pasteur_tempC;
        this->ee_heating_tempC = ee_heating_tempC;
        this->ee_freezing_tempC = ee_freezing_tempC;
        this->ee_pasteur_duratMM = ee_pasteur_duratMM;
        this->ee_run_on_hh = ee_run_on_hh;
        this->ee_run_on_mm = ee_run_on_mm;
        this->ee_run_toggle = ee_run_toggle;
        this->ee_is_runned_today = ee_is_runned_today;

        preset_selected = new SettingUnit(NULL, mb_auto_preset_list, PASTEUR_PRESET_CNT - 1);
        preset_pasteur_tempC = new SettingUnit(NULL, mb_auto_pasteur_tempC);
        preset_heating_tempC = new SettingUnit(NULL, mb_auto_heating_tempC);
        preset_freezing_tempC = new SettingUnit(NULL, mb_auto_freezing_tempC);
        preset_duration_mm = new SettingUnit(NULL, mb_auto_durat_mm);
        preset_toggle = new SettingUnit(NULL, mb_auto_preset_toggle, 1);
        preset_runned_today = new SettingUnit(NULL, NULL);

        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
        {
            run_rtc_trigger[index] = new TimeUnit(false);
            run_rtc_trigger[index]->setEEPointer(&ee_run_on_mm[index], PointerType::Minutes);
            run_rtc_trigger[index]->setEEPointer(&ee_run_on_hh[index], PointerType::Hours);
            run_rtc_trigger[index]->setMBPointer(mb_auto_run_rtc_mm, PointerType::Minutes);
            run_rtc_trigger[index]->setMBPointer(mb_auto_run_rtc_hh, PointerType::Hours);
            run_rtc_trigger[index]->loadFromEE();
        }

        selectPreset(0);
    }

    void selectPreset(uint8_t index)
    {
        preset_selected->setValue(index);
        preset_pasteur_tempC->changeEEpointer(&ee_pasteur_tempC[preset_selected->getValue()]);
        preset_heating_tempC->changeEEpointer(&ee_heating_tempC[preset_selected->getValue()]);
        preset_freezing_tempC->changeEEpointer(&ee_freezing_tempC[preset_selected->getValue()]);
        preset_duration_mm->changeEEpointer(&ee_pasteur_duratMM[preset_selected->getValue()]);
        preset_toggle->changeEEpointer(&ee_run_toggle[preset_selected->getValue()]);
        preset_runned_today->changeEEpointer(&ee_is_runned_today[preset_selected->getValue()]);
        run_rtc_trigger[preset_selected->getValue()]->loadFromEE();
    }

    void refreshPreset() {
        selectPreset(preset_selected->getValue());
    }

    void acceptChanges()
    {
        preset_pasteur_tempC->setValueByModbus();
        preset_heating_tempC->setValueByModbus();
        preset_freezing_tempC->setValueByModbus();
        preset_duration_mm->setValueByModbus();
        run_rtc_trigger[preset_selected->getValue()]->loadFromMB();
        run_rtc_trigger[preset_selected->getValue()]->sendToEE(true);
    }

    void togglePreset() {
        preset_toggle->setValue(preset_toggle->getValue() == 0 ? 1 : 0);
    }

    uint8_t isTimeToRunPreset(TimeUnit &current_time)
    {
        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            if (!run_rtc_trigger[index]->isBiggerThan(current_time, true) &&
                preset_toggle->getValue() == 1 &&
                preset_runned_today->getValue() == 0)
                return index;

        return 255;
    }

    bool startPreset(
        uint8_t preset_index,
        uint8_t *pasteur_tempC,
        uint8_t *heating_tempC,
        uint8_t *freezing_tempC,
        uint8_t *duration_mm,
        TimeUnit *current_dateTime,
        TimeUnit *finish_dateTime
    )
    {
        if (preset_runned_today->getValue() == 1)
            return false;

        selectPreset(preset_index);
        *pasteur_tempC = preset_pasteur_tempC->getValue();
        *heating_tempC = preset_heating_tempC->getValue();
        *freezing_tempC = preset_freezing_tempC->getValue();
        *duration_mm = preset_duration_mm->getValue();
        finish_dateTime->clone(*current_dateTime);
        finish_dateTime->addMinutes(preset_duration_mm->getValue());
        preset_runned_today->setValue(1);

        return true;
    }

    void resumePreset(
        uint8_t preset_index,
        uint8_t *pasteur_tempC,
        uint8_t *heating_tempC,
        uint8_t *freezing_tempC,
        uint8_t *duration_mm,
        TimeUnit *last_point_dateTime,
        TimeUnit *current_dateTime,
        TimeUnit *old_finish_dateTime
    )
    {
        selectPreset(preset_index);
        uint8_t left_mm = old_finish_dateTime->getDiffMin(*last_point_dateTime);
        *pasteur_tempC = preset_pasteur_tempC->getValue();
        *heating_tempC = preset_heating_tempC->getValue();
        *freezing_tempC = preset_freezing_tempC->getValue();
        *duration_mm = preset_duration_mm->getValue();
        old_finish_dateTime->clone(*current_dateTime);
        old_finish_dateTime->addMinutes(left_mm);
    }

    void newDay()
    {
        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            ee_is_runned_today[index].writeEE(0);
        preset_runned_today->refreshValue();
    }
};

#endif