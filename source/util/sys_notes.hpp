// sys_notes — the release notes of what the console runs on, for the
// Console tab (activity/sys_notes_activity.hpp), and of PlayGuard itself, for
// About: Atmosphère's, hekate's, sys-patch's and PlayGuard's from their GitHub releases (GET /repos/…/releases: one request, each release's
// notes included), the firmware's from switchbrew (its System_Versions table,
// then one page per version, as wikitext). Each release keeps the newest
// firmware its notes say it supports, so the Console tab can say when a firmware is
// out before Atmosphère or hekate support it. Parsing is plain C++
// (host-tested, tests/sys_notes); fetch() goes to the network
// (sys_notes_net.cpp). Everything read is text from outside: cleaned
// (util/safe_text.hpp) and bounded, never run or opened.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <iterator>
#include <string>
#include <vector>

namespace sys_notes
{

enum class Which
{
    Atmosphere,
    Hekate,
    Firmware,
    PlayGuard,
    SysPatch,
};
constexpr Which ALL[] = { Which::Atmosphere, Which::Hekate, Which::Firmware, Which::PlayGuard, Which::SysPatch };
constexpr size_t COUNT = std::size(ALL);

// "atmosphere", "hekate", "firmware", "playguard", "syspatch" (the keys of
// the cache and, but for PlayGuard, of sys_updates).
const char* key(Which which);
// "Atmosphère", "hekate", "", "PlayGuard", "sys-patch" (the firmware's name
// is translated).
const char* product(Which which);

// The releases kept per product, the newer ones a screen shows at most
// beside the installed one, and the most of a release's notes kept.
constexpr size_t MAX_RELEASES = 20;
constexpr size_t MAX_NEWER = 5;
constexpr size_t MAX_NOTES = 32 * 1024;

struct Release
{
    std::string version;    // "1.12.1" (sys-patch's: "1.6.3.0")
    std::string date;       // "2026-10-16" ("" unknown)
    std::string notes;      // GitHub Markdown, cleaned; "" not read yet (a firmware's page)
    std::string page;       // its page, https ("" none)
    std::string supports;   // the newest firmware its notes say it supports ("" none named)
    bool security = false;  // firmware: fixes security flaws
    bool pctl = false;      // firmware: updates pctl, the parental-control service
};

struct List
{
    std::vector<Release> releases;   // newest first
    std::string fetched;             // "2026-10-16" when they were fetched ("" never)
};

struct Cache
{
    List atmosphere, hekate, firmware, playguard, syspatch;
    List&       of(Which which);
    const List& of(Which which) const;
};

// --- GitHub -------------------------------------------------------------------

// The repository of Atmosphère, hekate, sys-patch or PlayGuard
// ("Atmosphere-NX/Atmosphere").
const char* repo(Which which);

// A release list (GET /repos/…/releases) into `out`, newest first as listed:
// at most `keep`, drafts and pre-releases left out, and releases whose tag is
// not a version ("v6.5.4", "1.12.1"). False when it is not a release list.
bool parse_github(const std::string& json, Which which, std::vector<Release>* out, size_t keep = MAX_RELEASES);

// A release's notes as they are worth reading on the console: images that
// stand alone (hekate's banner) go, and so do what does not describe the
// release (hekate's FAQ and empty Download section; Atmosphère's notes of
// the previous release, repeated under "And the following was changed in").
// PlayGuard's: the console's own lines read as changelog.hpp reads them
// ("<!-- console: … -->", "console:only", "console:hide"), its heading (the
// screen's title says it) and the download and count lines dropped, and
// what follows the notes (screenshots, install, build) left out; each
// change without its pull-request and commit references.
std::string trim_notes(Which which, const std::string& md);

// The newest firmware a release's notes say it supports ("" when none is
// named): Atmosphère's "Support was added for 23.0.0", "Basic support was
// added for 22.5.0"; hekate's "Latest HOS supported: 23.0.1", "HOS 23.0.1
// support". Only a version of three numbers, after "for", "HOS",
// "firmware" or "supported:", on a line that speaks of support.
std::string supported_firmware(const std::string& notes);

// --- switchbrew ---------------------------------------------------------------

// Its System_Versions table: each version and its release date ("" when
// the table has none), newest first, at most `keep`.
struct Version
{
    std::string version, date;
};
std::vector<Version> parse_versions(const std::string& wikitext, size_t keep = MAX_RELEASES);

// "October 1, 2026 (UTC)" -> "2026-10-01"; "" when it is not a date.
std::string wiki_date(const std::string& text);

// A version's page (https://switchbrew.org/wiki/23.0.1, as wikitext) into
// `out`: its change-log as Markdown, whether it fixes security flaws, its
// release date when the page has one and whether pctl was updated. False
// when it has no change-log.
bool parse_firmware_page(const std::string& wikitext, Release* out);

// Wikitext as GitHub Markdown: bullets (*, **), bold and italics, links
// (their text), templates and references dropped.
std::string wiki_to_markdown(const std::string& wikitext);

// The page of a version on switchbrew.
std::string firmware_page(const std::string& version);

// --- What a screen shows ------------------------------------------------------

// The releases a screen shows as tabs, as indexes into `list.releases`, in
// the order of the tabs, from the oldest: older ones (in the tabs the newer
// ones leave, the closest), the installed one (when listed), then the newer
// ones, at most MAX_NEWER of them (the newest). *hidden: the newer releases
// left out; *installed_at: the installed one's index (-1 when it is not
// listed); *newer_from: the first newer one's tab (the count when none).
std::vector<size_t> tabs(const List& list, const std::string& installed, size_t* hidden, int* installed_at,
                         size_t* newer_from = nullptr);

// How many listed releases are newer than `installed`.
size_t newer_count(const List& list, const std::string& installed);

// --- Support ------------------------------------------------------------------

// The newest firmware `version` of Atmosphère or hekate supports, as far as
// the notes listed up to it say ("" when none of them names one).
std::string supports_up_to(const List& list, const std::string& version);

// Whether support up to `max` covers firmware `fw`: `max` is as new, or the
// same major and minor version (a hotfix such as 23.0.1 needs nothing new).
bool covers(const std::string& max, const std::string& fw);

// The oldest listed release whose notes (up to it) cover `fw`; "" none.
std::string first_supporting(const List& list, const std::string& fw);

// What updating the firmware to `fw` needs of Atmosphère or hekate.
enum class Need
{
    Nothing,    // supported by the installed one (or unknown: nothing said)
    Update,     // supported by a newer release: update it first (*version)
    Wait,       // not supported by any release yet: *version is the newest
};
Need need_for(const List& list, const std::string& installed, const std::string& fw, std::string* version);

// --- Cache --------------------------------------------------------------------

// The cache file's JSON, and back (false when it is not one; what is not
// valid in it is left out, as the file can be edited).
std::string encode(const Cache& cache);
bool        decode(const std::string& text, Cache* out);

// Merges a fresh list into a kept one: the fresh releases, with the notes a
// firmware page gave before when the fresh list has none for it.
void merge(List* kept, const List& fresh);

// --- Network (sys_notes_net.cpp) ----------------------------------------------

// The release list of `which`. For the firmware: switchbrew's table, then,
// with `pages`, the page of each version a screen shows (tabs() against
// `installed`: each one's notes; a page that cannot be read is left
// without, and named in *error). Blocking: run it off the main thread.
// On the desktop build PLAYGUARD_SIM_SYS_NOTES names a folder holding
// atmosphere.json, hekate.json, System_Versions.wiki and <version>.wiki
// ("offline": no network); without it nothing is fetched.
bool fetch(Which which, const std::string& installed, bool pages, List* out, std::string* error);

}   // namespace sys_notes
