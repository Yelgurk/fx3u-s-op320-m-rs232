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
    TimeUnit *run_rtc_trigger[PASTEUR_PRESET_CNT],
             *run_rtc_extra[PASTEUR_PRESET_CNT];

    SettingUnit *preset_selected,
                *preset_pasteur_tempC,
                *preset_heating_tempC,
                *preset_freezing_tempC,
                *preset_duration_mm,
                *preset_toggle,
                *preset_runned_today,
                *preset_extra_tempC,
                *preset_extra_toggle,
                *preset_extra_runned;

    EEUnit *ee_pasteur_tempC,
           *ee_heating_tempC,
           *ee_freezing_tempC,
           *ee_pasteur_duratMM,
           *ee_run_on_hh,
           *ee_run_on_mm,
           *ee_run_toggle,
           *ee_is_runned_today,
           *ee_auto_extra_tempC,
           *ee_auto_extra_toggle,
           *ee_auto_extra_run_hh,
           *ee_auto_extra_run_mm,
           *ee_auto_extra_runned;

public:
    void init(
        EEUnit *ee_pasteur_tempC,
        EEUnit *ee_heating_tempC,
        EEUnit *ee_freezing_tempC,
        EEUnit *ee_pasteur_duratMM,
        EEUnit *ee_run_on_hh,
        EEUnit *ee_run_on_mm,
        EEUnit *ee_run_toggle,
        EEUnit *ee_is_runned_today,
        EEUnit *ee_auto_extra_tempC,
        EEUnit *ee_auto_extra_toggle,
        EEUnit *ee_auto_extra_run_hh,
        EEUnit *ee_auto_extra_run_mm,
        EEUnit *ee_auto_extra_runned,
        MBUnit *mb_auto_preset_list,
        MBUnit *mb_auto_pasteur_tempC,
        MBUnit *mb_auto_heating_tempC,
        MBUnit *mb_auto_freezing_tempC,
        MBUnit *mb_auto_durat_mm,
        MBUnit *mb_auto_run_rtc_hh,
        MBUnit *mb_auto_run_rtc_mm,
        MBUnit *mb_auto_preset_toggle,
        MBUnit *mb_auto_extra_heat_tempC,
        MBUnit *mb_auto_extra_heat_toggle,
        MBUnit *mb_auto_extra_heat_run_hh,
        MBUnit *mb_auto_extra_heat_run_mm
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
        this->ee_auto_extra_tempC = ee_auto_extra_tempC;
        this->ee_auto_extra_toggle = ee_auto_extra_toggle;
        this->ee_auto_extra_run_hh = ee_auto_extra_run_hh;
        this->ee_auto_extra_run_mm = ee_auto_extra_run_mm;
        this->ee_auto_extra_runned = ee_auto_extra_runned;
        delay(2);

        preset_selected = new SettingUnit(NULL, mb_auto_preset_list, PASTEUR_PRESET_CNT - 1, 1, false);
        preset_pasteur_tempC = new SettingUnit(NULL, mb_auto_pasteur_tempC, 0, 1, false);
        preset_heating_tempC = new SettingUnit(NULL, mb_auto_heating_tempC, 0, 1, false);
        preset_freezing_tempC = new SettingUnit(NULL, mb_auto_freezing_tempC, 0, 1, false);
        preset_duration_mm = new SettingUnit(NULL, mb_auto_durat_mm, 0, 1, false);
        preset_toggle = new SettingUnit(NULL, mb_auto_preset_toggle, 1, 1, false);
        preset_runned_today = new SettingUnit(NULL, NULL, 0, 1, false);
        preset_extra_tempC = new SettingUnit(NULL, mb_auto_extra_heat_tempC, 0, 1, false);
        preset_extra_toggle = new SettingUnit(NULL, mb_auto_extra_heat_toggle, 1, 1, false);
        preset_extra_runned = new SettingUnit(NULL, NULL, 0, 1, false);

        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
        {
            run_rtc_trigger[index] = new TimeUnit(false);
            run_rtc_trigger[index]->setEEPointer(&ee_run_on_mm[index], PointerType::Minutes);
            run_rtc_trigger[index]->setEEPointer(&ee_run_on_hh[index], PointerType::Hours);
            run_rtc_trigger[index]->setMBPointer(mb_auto_run_rtc_mm, PointerType::Minutes);
            run_rtc_trigger[index]->setMBPointer(mb_auto_run_rtc_hh, PointerType::Hours);
            run_rtc_trigger[index]->loadFromEE();

            run_rtc_extra[index] = new TimeUnit(false);
            run_rtc_extra[index]->setEEPointer(&ee_auto_extra_run_mm[index], PointerType::Minutes);
            run_rtc_extra[index]->setEEPointer(&ee_auto_extra_run_hh[index], PointerType::Hours);
            run_rtc_extra[index]->setMBPointer(mb_auto_extra_heat_run_mm, PointerType::Minutes);
            run_rtc_extra[index]->setMBPointer(mb_auto_extra_heat_run_hh, PointerType::Hours);
            run_rtc_extra[index]->loadFromEE();
        }

        delay(2);
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
        preset_extra_tempC->changeEEpointer(&ee_auto_extra_tempC[preset_selected->getValue()]);
        preset_extra_toggle->changeEEpointer(&ee_auto_extra_toggle[preset_selected->getValue()]);
        run_rtc_extra[preset_selected->getValue()]->loadFromEE();
        preset_extra_runned->changeEEpointer(&ee_auto_extra_runned[preset_selected->getValue()]);
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
        preset_extra_tempC->setValueByModbus();
        run_rtc_extra[preset_selected->getValue()]->loadFromMB();
        run_rtc_extra[preset_selected->getValue()]->sendToEE(true);
        resetCallFlags();
    }

    void togglePreset() {
        preset_toggle->setValue(preset_toggle->getValue() == 0 ? 1 : 0);
    }

    void toggleExtraH() {
        preset_extra_toggle->setValue(preset_extra_toggle->getValue() == 0 ? 1 : 0);
    }

    uint8_t isTimeToRunPreset(TimeUnit &current_time, bool &in_range)
    {
        in_range = false;

        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            if (current_time.isBiggerThan(run_rtc_trigger[index], true) &&
                ee_run_toggle[index].readEE() == 1 &&
                ee_is_runned_today[index].readEE() == 0)
            {
                if (current_time.getDiffSec(run_rtc_trigger[index], true) / 60 <= 60)
                    in_range = true;
                //else
                    //ee_is_runned_today[index].writeEE(1);
                return index + 1;
            }

        return 0;
    }

    uint8_t isTimeToRunExtra(TimeUnit &current_time, bool &in_range)
    {
        in_range = false;

        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
            if (current_time.isBiggerThan(run_rtc_extra[index], true) &&
                ee_auto_extra_toggle[index].readEE() == 1 &&
                ee_auto_extra_runned[index].readEE() == 0)
            {
                if (current_time.getDiffSec(run_rtc_extra[index], true) / 60 <= 60)
                    in_range = true;
                //else
                    //ee_auto_extra_runned[index].writeEE(1);
                return index + 1;
            }

        return 0;
    }

    bool startPreset(
        uint8_t preset_index,
        uint8_t &pasteur_tempC,
        uint8_t &heating_tempC,
        uint8_t &freezing_tempC,
        uint8_t &preset_duration
    )
    {
        if (ee_is_runned_today[preset_index].readEE() == 1)
            return false;

        resumePreset(
            preset_index,
            pasteur_tempC,
            heating_tempC,
            freezing_tempC,
            preset_duration
        );

        ee_is_runned_today[preset_index].writeEE(1);
        return true;
    }

    void resumePreset(
        uint8_t preset_index,
        uint8_t &pasteur_tempC,
        uint8_t &heating_tempC,
        uint8_t &freezing_tempC,
        uint8_t &preset_duration
    )
    {
        delay(50);
        pasteur_tempC = ee_pasteur_tempC[preset_index].readEE();
        heating_tempC = ee_heating_tempC[preset_index].readEE();
        freezing_tempC = ee_freezing_tempC[preset_index].readEE();
        preset_duration = ee_pasteur_duratMM[preset_duration].readEE();
    }

    uint8_t startExtraHeat(
        uint8_t preset_index
    )
    {
        if (ee_auto_extra_runned[preset_index].readEE() == 1)
            return 0;

        ee_auto_extra_runned[preset_index].writeEE(1);
        return ee_auto_extra_tempC[preset_index].readEE();
    }

    void resetCallFlags()
    {
        for (uint8_t index = 0; index < PASTEUR_PRESET_CNT; index++)
        {
            ee_is_runned_today[index].writeEE(0);
            ee_auto_extra_runned[index].writeEE(0);
        }
        preset_runned_today->refreshValue();
        preset_extra_runned->refreshValue();
    }

    void setDefault()
    {
        for (uint8_t index = 0; index < 3; index++)
        {
            ee_pasteur_tempC[index].writeEE(65);
            ee_heating_tempC[index].writeEE(45);
            ee_freezing_tempC[index].writeEE(42);
            ee_pasteur_duratMM[index].writeEE(30);
            ee_run_on_hh[index].writeEE(8 + 5 * index);
            ee_run_on_mm[index].writeEE(0);
            ee_run_toggle[index].writeEE(0);
            ee_is_runned_today[index].writeEE(0);
            ee_auto_extra_tempC[index].writeEE(45);
            ee_auto_extra_run_hh[index].writeEE(13 + 5 * index);
            ee_auto_extra_run_mm[index].writeEE(0);
            ee_auto_extra_toggle[index].writeEE(0);
            ee_auto_extra_runned[index].writeEE(0);
        }
    }
};

#endif