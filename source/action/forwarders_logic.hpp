// forwarders_logic — the UI-free part of Console › Forwarders: one row per
// forwarder, from what the HOME menu has (core/forwarder.h) and what
// PlayGuard remembers making (util/forwarders.hpp); whether the console can
// start one (its signature patches, util/patches.hpp); the name a new one
// gets. Plain C++ for the host tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "util/forwarders.hpp"
#include "util/patches.hpp"

namespace forwarders_logic
{

// A forwarder the HOME menu has (core/forwarder.h's ForwarderRecord).
struct Installed
{
    uint64_t    id = 0;
    std::string name, author, version;
};

enum class State
{
    Ready,         // on the HOME menu, its .nro there
    NroMissing,    // on the HOME menu, its .nro gone: it would not start
    GateMissing,   // protected with the PIN, PlayGuard's .nro it starts first gone: install it again
    NotInstalled,  // made here, no longer on the HOME menu (deleted in System Settings)
    Elsewhere,     // on the HOME menu, made by another tool: PlayGuard knows not its .nro
};

struct Row
{
    uint64_t    id = 0;
    std::string name, author, version;
    std::string nro;    // "sdmc:/…"; empty for one made elsewhere
    State       state = State::Ready;
    bool        self = false;   // PlayGuard's own
    bool        pin = false;    // made here, protected with the PIN
    bool        gone_seen = false;   // NotInstalled, and already told (forwarders::Made::gone)
    bool        verified = false;    // protected, and started through the gate once (forwarders::Made::verified)
    int         free_days = 0, free_from = 0, free_to = 0;   // protected: when the PIN is not asked
    int         free2_days = 0, free2_from = 0, free2_to = 0;   // a second span of them (the weekend's)
    int         closed_days = 0, closed_from = 0, closed_to = 0;   // protected: when it never opens
    bool        limit = false;     // protected: closed once the day's play time is used up
    bool        carried = false;   // protected: its rules in it (forwarders::Made::carried)
    uint64_t    paused_until = 0;   // paused after refusals until then (POSIX; 0: not paused), read by forwarder_flow
    bool        custom_icon = false;   // its icon from the album or the library (forwarders::Made::icon)
    std::string icon;                  // that screenshot, "sdmc:/Nintendo/Album/…", or "lib:<id>"
    int         icon_x = forwarders::ICON_X_MIDDLE;   // where its square sits (forwarders::Made::icon_x)
    // How the HOME menu starts it (forwarders::Made::ask_user, …).
    bool        ask_user = false, screenshots = true, video = true;
};

// One row per forwarder on the HOME menu and per one remembered that is no
// longer there, by name (ignoring case), then ID. The name the console reads
// first, then the one remembered, then the .nro's file name, then the ID.
// `nro_exists` says whether an "sdmc:/…" path is on the SD card;
// `self_nro` is PlayGuard's own ("sdmc:/…", may be empty).
std::vector<Row> merge(const std::vector<forwarders::Made>& made, const std::vector<Installed>& installed,
                       const std::function<bool(const std::string&)>& nro_exists, const std::string& self_nro);

// The row of PlayGuard's own forwarder, or nullptr.
const Row* self_row(const std::vector<Row>& rows);

// The .nro's file name without its folder and ".nro" ("ftpd" for
// "sdmc:/switch/ftpd/ftpd.nro").
std::string file_title(const std::string& path);

// The name a new forwarder gets: the .nro's own (its NACP), else its file
// name; cut to forwarders::MAX_NAME.
std::string default_name(const std::string& nacp_name, const std::string& path);

// Whether the console can start a forwarder: its fs and loader signature
// checks patched (sys-patch at this boot, or sigpatch files for both, or
// hekate's).
enum class Patches
{
    Ready,
    Unknown,      // sys-patch is there, its log does not say for this boot
    Missing,
    Incomplete,   // sys-patch ran, but left a pattern of fs or ldr for this firmware unpatched (patch_gaps)
};
Patches patches_state(const patches::Report& report);
// What sys-patch left unpatched that a forwarder needs (fs and ldr, not es):
// "fs:nocntchk_19.0.0+=Unpatched, ldr:…"; a whole module when it logged
// none of its patterns as patched ("ldr"). Empty when nothing is known missing.
std::string patch_gaps(const patches::Report& report);

// Whether a forwarder built with the loader `made` ("" for one made before
// PlayGuard kept it) is out of date against this build's `current` (""
// when unknown, as on the desktop: then none is). sphaira's own forwarders
// had to be made again after a loader update to start their .nro well.
bool stale_loader(const std::string& made, const std::string& current);
// Whether a forwarder protected with the PIN (`pin`), made with the gate
// `made` ("sdmc:/…"), is out of date against the gate this build writes,
// `current`: only when that one is pinned (its own file, gate_nro), as a
// new PlayGuard's gate is another file the forwarder does not start.
bool stale_gate(bool pin, const std::string& made, const std::string& current, bool pinned);

// Whether `nro` ("sdmc:/…") is a homebrew menu, which starts any other
// .nro: hbmenu (sdmc:/hbmenu.nro, the one the album starts, or a copy of
// it elsewhere) or sphaira, by their file names.
bool is_menu(const std::string& nro);

// Whether `nro` is a homebrew known to write anywhere on the SD card (a
// file manager, an installer, an FTP server…), by its file name.
bool writes_sd(const std::string& nro);

// "Sat–Sun 09:00–20:00"-like: the days (`days`, bit 0 Sunday) as the
// names in `day_names` (Sunday first, 7 of them), and the span.
std::string hours_text(int days, int from, int to, const std::vector<std::string>& day_names);

// What locking the homebrew menu (core/hbl_lock.h) needs: two ways back to
// PlayGuard, each starting from the HOME menu: its own forwarder, and a menu
// of homebrew protected with the PIN, which starts PlayGuard's .nro too (and
// keeps a homebrew store usable). And what it should warn of.
struct LockCheck
{
    bool pin = false;       // a PIN on the console (to ask for)
    bool patches = false;   // forwarders can start: signature patches neither missing nor incomplete
    bool self = false;      // PlayGuard's own forwarder ready
    std::vector<std::string> menus;   // the protected menus ready, by name
    // Those of them started from the HOME menu through the gate once: a way
    // back seen working, not only made (the PIN screen opened, the menu after).
    std::vector<std::string> verified;
    // Forwarders that would open the homebrew menu without the PIN: a menu
    // not protected, and those another app made (what they start is unknown).
    std::vector<std::string> open_doors;
    // Made here, not protected (and not PlayGuard's own): each starts only
    // its homebrew, but one that writes to the SD card (sd_writers, among
    // them: a file manager, an installer) could change the lock's file.
    std::vector<std::string> unprotected, sd_writers;
    bool ready() const { return this->pin && this->patches && this->self && !this->verified.empty(); }
    // While locked: both ways still there (else the parent is told).
    bool routes() const { return this->self && !this->menus.empty(); }
};
LockCheck lock_check(const std::vector<Row>& rows, bool pin_set, Patches patches);

// While the homebrew menu is locked, what taking `row` away (deleting it, or
// its PIN) does to the ways back: nothing (Allowed); one of several
// protected menus (Confirm: the others remain); PlayGuard's own forwarder,
// or the last protected menu (Refused: unlock first).
enum class Guard
{
    Allowed,
    Confirm,
    Refused,
};
Guard way_back_guard(const std::vector<Row>& rows, const Row& row, bool locked);
// The protected menus ready other than `row`, by name (Confirm's).
std::vector<std::string> other_menus(const std::vector<Row>& rows, const Row& row);

// Refused at the gate: the name and the day ("2026-10-19", the first ten
// characters of the log's time) of each. Those refused at least `at_least`
// times on one day, with their most on a day and that day, in the order first
// seen: someone trying the PIN.
struct Refusals
{
    std::string name, day;
    int         count = 0;
};
std::vector<Refusals> repeated_refusals(const std::vector<std::pair<std::string, std::string>>& refused, int at_least = 3);

// Screenshots of the album (`paths`, "…/Nintendo/Album/2026/10/20/
// 2026102018300000-<id>.jpg"), the newest first, at most `max`; anything
// but a .jpg is left out. And "2026-10-20 18:30" from such a name ("" when
// it is not one).
std::vector<std::string> newest_shots(std::vector<std::string> paths, size_t max);
std::string shot_label(const std::string& path);

// A screenshot's time, from its name's first 14 digits; false when it is not
// one of the album's.
struct ShotTime
{
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
};
bool shot_time(const std::string& path, ShotTime& out);
// Its name's first 16 digits (its time, then the number of that second), as
// caps:a names it (core/album.h's AlbumShot); "" when it is not one.
std::string shot_stamp(const std::string& path);
// How many days the day of `t` is before `year`-`month`-`day`: 0 that day,
// 1 the day before; below 0 after it.
int days_before(const ShotTime& t, int year, int month, int day);
// For each of `paths`, whether another of them was taken the same minute:
// its seconds then tell them apart.
std::vector<bool> same_minute(const std::vector<std::string>& paths);
// Where an icon's square goes next along its screenshot (0 its left edge,
// 100 its right one, by quarters): `step` -1 to the left, 1 to the right;
// it stays at an edge.
int icon_x_step(int x, int step);

// Why writing to the SD card failed, from what a test write there said
// (`err`, an errno; 0 when it worked) and the room left: full, or written
// to by nothing (a card locked, or its folders' archive bit, which hekate's
// Tools › Arch bit fixes); None when neither shows.
enum class WriteHint
{
    None,
    Full,
    ReadOnly,
};
WriteHint write_hint(int err, uint64_t free_bytes);

}   // namespace forwarders_logic
