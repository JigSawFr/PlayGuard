// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/console_tab.hpp"

#include "action/fw_gate.hpp"
#include "action/sys_notes_flow.hpp"
#include "action/sigpatch_flow.hpp"
#include "action/sys_update_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/forwarders_activity.hpp"
#include "activity/game_check_activity.hpp"
#include "core/sysinfo.h"
#include "tab/clock_tab.hpp"
#include "ui/cadran_frame.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/patches.hpp"
#include "view/row_icon.hpp"
#include "view/tab_badge.hpp"

using namespace brls::literals;

ConsoleTab::ConsoleTab()
    : TabBase("xml/tab/console.xml"), list(this)
{
    clock->registerClickAction([](brls::View*) {
        ClockTab::open_screen();
        return true;
    });
    games->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new GameCheckActivity());
        return true;
    });
    forwarders->registerClickAction([](brls::View*) {
        brls::Application::pushActivity(new ForwardersActivity());
        return true;
    });

    // The real serial number, whole, and where it comes from: a press away
    // (the row masks its middle).
    serial_info = RowIcon::on_row(serial.getView(), RowIcon::Kind::Info);
    serial->registerClickAction([this](brls::View*) {
        SysInfo si;
        sysinfo_get(&si);
        ui::info(ui::real_serial(si).note);
        return true;
    });

    // The release notes of each (installed and newer versions).
    const std::pair<brls::DetailCell*, sys_notes::Which> notes_cells[] = {
        { fw.getView(), sys_notes::Which::Firmware },
        { ams.getView(), sys_notes::Which::Atmosphere },
        { hekate.getView(), sys_notes::Which::Hekate },
        { syspatch.getView(), sys_notes::Which::SysPatch },
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
    // says so.
    blank_warn = RowIcon::on_row(blank.getView(), RowIcon::Kind::Warning);
    blank->registerClickAction([](brls::View*) {
        ui::info("playguard/tools/serial_note_emummc"_i18n);
        return true;
    });

    // What the game patches found: an icon (a warning when something needs a
    // look), the text by A.
    patches_icon = RowIcon::on_row(game_patches.getView(), RowIcon::Kind::Info);
    game_patches->registerClickAction([this](brls::View*) {
        // sigpatch files beside sys-patch: what was found and, in the same
        // question, whether to set them aside (action/sigpatch_flow.hpp).
        if (!sigpatch_flow::offered()) {
            if (!this->patches_note.empty()) ui::info(this->patches_note);
            return true;
        }
        sigpatch_flow::set_aside([this](bool) { this->refresh(); }, this->patches_note);
        return true;
    });

    // On a firmware newer than the checked one, the firmware screen again.
    compat->registerClickAction([](brls::View*) {
        if (fw_gate::needed()) brls::Application::pushActivity(new FirmwareGateActivity());
        else ui::info("playguard/tools/compat_ok_info"_i18n);
        return true;
    });
    // What the console runs, as tiles above the rest (first in the XML).
    if (brls::Box* tiles = this->list.tiles(
            { { "cn_fw", "chip" }, { "cn_ams", "bolt" }, { "cn_hekate", "flag" }, { "cn_syspatch", "shield" } }, true))
        tiles->setMarginBottom(20);
    // A newer firmware Atmosphère or hekate does not support yet: a warning
    // in the firmware tile's corner, the advice in its release notes (A).
    fw_warn = new RowIcon(RowIcon::Kind::Warning);
    if (brls::Box* head = this->list.head("cn_fw")) {
        auto* push = new brls::Box();
        push->setGrow(1.0f);
        head->addView(push);
        head->addView(fw_warn);
    }
    // The screens of their own as tiles under them, each one's count of
    // things to do (mark()) in its corner rather than under its title.
    if (brls::Box* tools = this->list.tiles({ { "cn_clock", "globe" }, { "cn_games", "gamepad" }, { "cn_forwarders", "home" } }, false)) {
        tools->setMarginBottom(20);
        for (const char* tile : { "cn_clock", "cn_games" }) {
            auto* cell = (brls::DetailCell*)this->getView(tile);
            TabBadge* badge = TabBadge::on_row(cell);
            brls::Box* head = this->list.head(tile);
            if (!badge || !head) continue;
            cell->removeView(badge, false);
            auto* push = new brls::Box();
            push->setGrow(1.0f);
            head->addView(push);
            head->addView(badge);
        }
    }
    this->list.apply({ { "cn_compat", "check" }, { "cn_storage", "folder" }, { "cn_blank", "code" },
                       { "cn_serial", "tag" }, { "cn_patches", "shield" } });
}

void ConsoleTab::refresh()
{
    SysInfo si;
    sysinfo_get(&si);
    // Each tile: the installed version, then a pill with the newer release
    // the last update check found (sys_update_flow) and how many releases
    // behind, in amber, or "Up to date" once a check knows the newest.
    const auto version_tile = [this](brls::DetailCell* cell, const char* tile_id, sys_notes::Which which) {
        const sys_notes_flow::Status s = sys_notes_flow::status(which);
        cell->setDetailText(s.installed.empty() ? "playguard/common/unavailable"_i18n : s.installed);
        cell->setDetailTextColor(cadran::kit::color(s.installed.empty() ? "cadran/text2" : "cadran/text"));
        std::string tag, note;
        if (s.newer) {
            tag = brls::getStr("playguard/tools/tile_newer", s.latest);
            if (s.behind > 1) note = brls::getStr("playguard/tools/tile_behind", (int)s.behind);
        } else if (!s.installed.empty() && !s.latest.empty()) {
            tag = "playguard/tools/tile_current"_i18n;
        }
        this->list.tag(tile_id, tag, s.newer, note);
    };
    version_tile(fw, "cn_fw", sys_notes::Which::Firmware);
    version_tile(ams, "cn_ams", sys_notes::Which::Atmosphere);
    // hekate: only when its bootloader/update.bin is on the SD card.
    version_tile(hekate, "cn_hekate", sys_notes::Which::Hekate);
    ui::set_visible(hekate.getView(), !sys_update_flow::installed_hekate().empty());
    // sys-patch: when it is installed; its version from its log.ini
    // (unavailable when it does not log).
    const patches::Report& report = ui::patch_report();
    version_tile(syspatch, "cn_syspatch", sys_notes::Which::SysPatch);
    ui::set_visible(syspatch.getView(), report.syspatch_installed);
    // A newer firmware Atmosphère or hekate does not support yet, or only in
    // a newer release (their notes say): its corner's warning, the tile in amber.
    const bool fw_advice = !sys_notes_flow::firmware_advice().empty();
    fw_warn->set_shown(fw_advice);
    if (fw_advice) fw->setBorderColor(cadran::kit::color("cadran/amber_line"));
    NVGcolor c = cadran::kit::color("cadran/text2");
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
    c = cadran::kit::color("cadran/text2");
    blank->setDetailText(ui::blank_text(si, &c));
    blank->setDetailTextColor(c);
    blank->setFocusable(ui::serial_warning(si));
    blank_warn->set_shown(ui::serial_warning(si));
    serial->setDetailText(ui::serial_text(si));
    serial->setDetailTextColor(ui::serial_warning(si) ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    serial_info->set_shown(!ui::real_serial(si).note.empty());

    c = cadran::kit::color("cadran/text2");
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
