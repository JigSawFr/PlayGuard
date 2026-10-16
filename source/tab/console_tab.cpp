// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/console_tab.hpp"

#include "action/fw_gate.hpp"
#include "action/sys_notes_flow.hpp"
#include "action/sys_update_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/game_check_activity.hpp"
#include "app.hpp"
#include "tab/clock_tab.hpp"
#include "ui/ui.hpp"
#include "util/patches.hpp"

using namespace brls::literals;

ConsoleTab::ConsoleTab()
    : TabBase("xml/tab/console.xml")
{
    storage_note->setSingleLine(false);
    serial_note->setSingleLine(false);
    patches_note->setSingleLine(false);

    clock->setDetailText(ui::opens());
    clock->registerClickAction([](brls::View*) {
        ClockTab::open_screen();
        return true;
    });
    games->setDetailText(ui::opens());
    games->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new GameCheckActivity());
        return true;
    });

    // Ⓐ on the serial number shows / hides the masked digits.
    serial->registerAction("playguard/tools/serial_show"_i18n, brls::BUTTON_A, [this](brls::View*) {
        this->serial_revealed = !this->serial_revealed;
        this->refresh();
        return true;
    }, false, false, brls::SOUND_CLICK);

    // The release notes of each (installed and newer versions).
    const std::pair<brls::DetailCell*, sys_notes::Which> notes_cells[] = {
        { fw.getView(), sys_notes::Which::Firmware },
        { ams.getView(), sys_notes::Which::Atmosphere },
        { hekate.getView(), sys_notes::Which::Hekate },
    };
    for (const auto& n : notes_cells) {
        const sys_notes::Which which = n.second;
        n.first->registerAction("playguard/sys_notes/hint"_i18n, brls::BUTTON_A, [which](brls::View*) {
            sys_notes_flow::open(which);
            return true;
        });
    }
    sys_advice->setSingleLine(false);

    // On a firmware newer than the checked one, the firmware screen again.
    compat->registerClickAction([](brls::View*) {
        if (fw_gate::needed()) brls::Application::pushActivity(new FirmwareGateActivity());
        else ui::info("playguard/tools/compat_ok_info"_i18n);
        return true;
    });
}

void ConsoleTab::refresh()
{
    SysInfo si;
    sysinfo_get(&si);
    // With the newer release the last update check found (sys_update_flow),
    // and how many releases behind when more than one.
    bool newer = false;
    fw->setDetailText(ui::opens(sys_notes_flow::status_text(sys_notes::Which::Firmware, &newer)));
    fw->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    if (si.ams_valid) {
        ams->setDetailText(ui::opens(sys_notes_flow::status_text(sys_notes::Which::Atmosphere, &newer)));
        ams->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    } else {
        ams->setDetailText("playguard/common/unavailable"_i18n);
    }
    // hekate: only when its bootloader/update.bin is on the SD card.
    const std::string hk = sys_update_flow::installed_hekate();
    hekate->setDetailText(ui::opens(sys_notes_flow::status_text(sys_notes::Which::Hekate, &newer)));
    hekate->setDetailTextColor(newer ? ui::color_warn() : ui::color_neutral());
    ui::set_visible(hekate.getView(), !hk.empty());
    // A newer firmware Atmosphère or hekate does not support yet, or only in
    // a newer release (their notes say).
    const std::string advice = sys_notes_flow::firmware_advice();
    sys_advice->setText(advice);
    NVGcolor c = ui::color_neutral();
    compat->setDetailText(ui::compat_text(si, &c));
    compat->setDetailTextColor(c);

    storage->setDetailText(ui::storage_text(si));
    const std::string other = ui::other_storage_note(si);
    storage_note->setText(other);
    c = ui::color_neutral();
    blank->setDetailText(ui::blank_text(si, &c));
    blank->setDetailTextColor(c);
    const bool maskable = si.serial_valid && si.serial[0] && !(si.blank_valid && si.blank);
    serial->setDetailText(ui::serial_text(si, this->serial_revealed));
    serial->setDetailTextColor(ui::serial_warning(si) ? ui::color_warn() : ui::color_neutral());
    serial->setActionAvailable(brls::BUTTON_A, maskable);
    serial->updateActionHint(brls::BUTTON_A, this->serial_revealed ? "playguard/tools/serial_hide"_i18n
                                                                   : "playguard/tools/serial_show"_i18n);
    brls::Application::getGlobalHintsUpdateEvent()->fire();   // redraw the footer hints

    const patches::Report& report = ui::patch_report();
    c = ui::color_neutral();
    game_patches->setDetailText(ui::patches_text(report, &c));
    game_patches->setDetailTextColor(c);
    bool warn = false;
    const std::string note = ui::patches_note(report, si, &warn);
    patches_note->setText(note);
    patches_note->setTextColor(warn ? ui::color_warn() : ui::color_note());
    ui::set_visible_all({ { storage_note.getView(), !other.empty() },
                          { sys_advice.getView(), !advice.empty() },
                          { serial_note.getView(), ui::serial_warning(si) },
                          { patches_note.getView(), !note.empty() } });
}

brls::View* ConsoleTab::create()
{
    return new ConsoleTab();
}
