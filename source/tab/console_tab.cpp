// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/console_tab.hpp"

#include "action/fw_gate.hpp"
#include "action/sys_notes_flow.hpp"
#include "action/sys_update_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/game_check_activity.hpp"
#include "core/sysinfo.h"
#include "tab/clock_tab.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/ui.hpp"
#include "util/patches.hpp"
#include "view/row_icon.hpp"

using namespace brls::literals;

ConsoleTab::ConsoleTab()
    : TabBase("xml/tab/console.xml"), list(this)
{

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

    // What the notification bell says is to do here, on its row.
    this->mark(clock.getView(), { "clock" });
    this->mark(games.getView(), { "games" });
    this->mark(storage.getView(), { "sd_space" });
    this->mark(game_patches.getView(), { "patches" });

    // The other system's parental controls: an info icon on the row, the text a
    // press away (a paragraph under it took half the screen).
    storage_info = RowIcon::on_row(storage.getView(), RowIcon::Kind::Info);
    storage->registerClickAction([this](brls::View*) {
        if (!this->storage_note.empty()) ui::info(this->storage_note);
        return true;
    });
    // A warning icon for the serial number not blanked on emuMMC, on the row that
    // says so (A on the serial number's row shows / hides its digits).
    blank_warn = RowIcon::on_row(blank.getView(), RowIcon::Kind::Warning);
    blank->registerClickAction([](brls::View*) {
        ui::info("playguard/tools/serial_note_emummc"_i18n);
        return true;
    });

    // A newer firmware Atmosphère or hekate does not support yet: a warning
    // icon on the firmware's row, the advice in its release notes (A).
    fw_warn = RowIcon::on_row(fw.getView(), RowIcon::Kind::Warning);
    // What the game patches found: an icon (a warning when something needs a
    // look), the text by A.
    patches_icon = RowIcon::on_row(game_patches.getView(), RowIcon::Kind::Info);
    game_patches->registerClickAction([this](brls::View*) {
        if (!this->patches_note.empty()) ui::info(this->patches_note);
        return true;
    });

    // On a firmware newer than the checked one, the firmware screen again.
    compat->registerClickAction([](brls::View*) {
        if (fw_gate::needed()) brls::Application::pushActivity(new FirmwareGateActivity());
        else ui::info("playguard/tools/compat_ok_info"_i18n);
        return true;
    });
    // What the console runs, as tiles above the rest (first in the XML).
    if (brls::Box* tiles = this->list.tiles({ { "cn_fw", "chip" }, { "cn_ams", "bolt" }, { "cn_hekate", "flag" } }, true))
        tiles->setMarginBottom(20);
    this->list.apply({ { "cn_clock", "globe" }, { "cn_games", "gamepad" }, { "cn_compat", "check" }, { "cn_storage", "folder" },
                       { "cn_blank", "code" }, { "cn_serial", "tag" }, { "cn_patches", "shield" } });
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
    fw_warn->set_shown(!sys_notes_flow::firmware_advice().empty());
    NVGcolor c = ui::color_neutral();
    const std::string compat_value = ui::compat_text(si, &c);
    // Checked: the line's icon says it (a teal check) and no word is needed;
    // anything else, a warning in the value's colour and the words.
    const SysCompat compat_state = sysinfo_compat(&si);
    const bool compat_ok = compat_state == SysCompat_Ok;
    compat->setDetailText(compat_ok ? "" : compat_value);
    compat->setDetailTextColor(c);
    const bool compat_warns = compat_state == SysCompat_UntestedNewer || compat_state == SysCompat_PlayTimerUnsupported;
    cadran::List::recolor_icon(compat, compat_ok ? "check" : "alert",
                           compat_ok ? "cadran/teal" : compat_warns ? "cadran/amber" : "cadran/coral");

    storage->setDetailText(ui::storage_text(si));
    this->storage_note = ui::other_storage_note(si);
    storage->setFocusable(!this->storage_note.empty());
    storage_info->set_shown(!this->storage_note.empty());
    c = ui::color_neutral();
    blank->setDetailText(ui::blank_text(si, &c));
    blank->setDetailTextColor(c);
    blank->setFocusable(ui::serial_warning(si));
    blank_warn->set_shown(ui::serial_warning(si));
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
    this->patches_note = ui::patches_note(report, si, &warn);
    patches_icon->set_kind(warn ? RowIcon::Kind::Warning : RowIcon::Kind::Info);
    patches_icon->set_shown(!this->patches_note.empty());
    this->list.sync();
    cadran::set_pill("");
}

brls::View* ConsoleTab::create()
{
    return new ConsoleTab();
}
