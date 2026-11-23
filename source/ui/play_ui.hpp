// ui — what PlayGuard adds to the kit's ui/ui.hpp (helpers shared by every
// app), in the same namespace: the PIN and minutes dialogs, the text of the
// console's compatibility, serial and patches, the main screen's tab
// positions, the unlock the title says, and PlayGuard's palette.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <iterator>
#include <string>

#include "ui/play_text.hpp"
#include "ui/ui.hpp"
#include "util/patches.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/prodinfo.hpp"

namespace ui
{

// Reads the stored PIN (pctl_get_pin) and shows it in large digits, or a
// dialog with the error. The app's copies are wiped as soon as the label
// holds the text. True when it was shown; `on_close` then runs once it is
// closed (OK or B). Used by PIN › Show the PIN.
bool show_pin_dialog(std::function<void()> on_close = nullptr);

// A duration set with the stick (15 min a step, 5 with L and R, up to
// 24:00; "no limit" starts at 1 h). The callback only on confirm.
void prompt_minutes(const std::string& header, uint16_t current, std::function<void(uint16_t)> on_value);

// Compatibility with the firmware, shown beside the firmware text.
std::string compat_text(const SysInfo& info, const char** color = nullptr);

// The other system (sysMMC from emuMMC, or an emuMMC set up on the card from
// sysMMC) has its own parental controls, out of PlayGuard's sight; "" if none.
std::string other_storage_note(const SysInfo& info);
// The serial-number warning only applies on emuMMC: on sysMMC the real serial
// is normal (online play), blanking it would cut Nintendo's services.
bool        serial_warning(const SysInfo& info);
std::string blank_text(const SysInfo& info, const char** color = nullptr);
// The console's real serial number: the system's when Atmosphère does not
// blank it, else read from its PRODINFO backup on the SD card
// (util/prodinfo.hpp). `note` says where it comes from, the whole number in
// it ("" serial: not found, and why). Looked up once per run.
struct RealSerial
{
    std::string      serial;
    std::string      note;
    prodinfo::Source source = prodinfo::Source::None;   // when blanked
};
const RealSerial& real_serial(const SysInfo& info);
// real_serial() with the middle masked ("XAW1000•••••01"), or "Unavailable".
std::string serial_text(const SysInfo& info);
bool        patches_warning(const patches::Report& report);
std::string patches_text(const patches::Report& report, const char** color = nullptr);
// Explanation under the Game patches line ("" when there is nothing to say);
// *warn tells whether it is a warning (amber) or a plain note.
std::string patches_note(const patches::Report& report, const SysInfo& info, bool* warn);

// Sidebar positions of the main screen tabs, in the order of
// resources/xml/activity/main.xml (separators count as positions).
namespace tab
{
constexpr int dashboard    = 0;
constexpr int play_timer   = 2;
constexpr int activity     = 3;   // play time per game
constexpr int restrictions = 4;
constexpr int security     = 5;   // PIN, unlock, protecting PlayGuard, companion app, delete
constexpr int history      = 6;   // the change history and settings backups
constexpr int console      = 8;   // the network clock, installed games, forwarders, firmware, storage, serial, patches
constexpr int preferences  = 10;  // the app's settings and its updates
constexpr int about        = 11;  // version, help, credits
// The n-th tab (0 = Today) as a sidebar position (separators count).
constexpr int of(int n)
{
    constexpr int positions[] = { dashboard, play_timer, activity, restrictions, security, history, console, preferences, about };
    return n >= 0 && n < (int)std::size(positions) ? positions[n] : dashboard;
}
}   // namespace tab

// Temporary unlock as last read by any tab (`valid` false: the read failed,
// keep the previous value). The main screen's title says it while unlocked.
void note_unlocked(bool valid, bool unlocked);
bool known_unlocked();
// That unlock is PlayGuard's own pause while the parent is in it
// (action/session_pause), not one to turn back on: said calmly.
void note_paused(bool paused);
bool known_paused();

// PlayGuard's own theme keys (ui/palette.hpp) in both variants, with the
// pairs they keep readable: before the window, whose theme file then
// changes them like any other key.
void install_palette();

}   // namespace ui
