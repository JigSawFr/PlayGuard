// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_library.hpp"

#include <algorithm>
#include <utility>

#include <aurore/cadran/util/text.hpp>

namespace forwarder_library
{

namespace
{

constexpr const char* PREFIX = "lib:";

// Each one's name is playguard/forwarders/icon_lib/<id> (tools/
// check_resources.py reads this list), its image img/forwarder/<id>.png.
const char* const IDS[] = {
    "emulation", "games",    "saves",       "files",    "install", "store",    "launcher",   "overlays", "cheats",
    "mods",      "themes",   "network",     "wifi",     "download", "update",  "streaming",  "video",    "music",
    "reader",    "web",      "chat",        "camera",   "drawing", "learning", "controllers", "monitor", "system",
    "tools",     "dump",     "keys",        "reboot",   "dev",     "clock",    "calculator", "star",
};

// The words that say a homebrew's kind, in a lower-case name or file name:
// the first kind with one wins, so a name before a word it holds
// ("toolbox" before "tool", "nx-shell" before "shell").
const std::pair<const char*, std::vector<const char*>> WORDS[] = {
    { "overlays", { "tesla", "ovlloader", "ultrahand", "overlay" } },
    { "cheats", { "edizon", "breeze", "cheat" } },
    { "mods", { "modmanager", "mod manager", "mods" } },
    { "saves", { "jksv", "checkpoint", "save" } },
    { "install", { "goldleaf", "tinwoo", "awoo", "tinfoil", "dbi", "install" } },
    { "store", { "appstore", "app store", "hb-store", "shop" } },
    { "dump", { "nxdumptool", "dump" } },
    { "keys", { "lockpick", "keys" } },
    { "update", { "updater", "update", "daybreak" } },
    { "streaming", { "moonlight", "chiaki", "stream" } },
    { "video", { "nxmp", "pplay", "video", "movie", "youtube", "film" } },
    { "music", { "music", "audio", "radio", "mp3" } },
    { "reader", { "ebook", "reader", "comic", "manga", "book", "pdf" } },
    { "themes", { "nxtheme", "theme" } },
    { "files", { "nx-shell", "nxshell", "files", "file manager", "file browser", "explorer", "commander" } },
    { "network", { "ftpd", "ftp", "mtp", "smb", "nfs", "transfer" } },
    { "wifi", { "ldn", "lan play", "lanplay", "lan-play", "wifi", "wlan" } },
    { "controllers", { "missioncontrol", "mission control", "sys-con", "controller", "joycon", "bluetooth" } },
    { "monitor", { "sys-clk", "sysclk", "monitor", "stats", "fps" } },
    { "reboot", { "reboot", "payload", "hekate", "fusee", "rcm", "power" } },
    { "launcher", { "hbmenu", "sphaira", "menu", "launcher" } },
    { "emulation", { "retroarch", "emulat", "mgba", "ppsspp", "dolphin", "citra", "melonds", "mupen", "pcsx", "snes",
                     "scummvm", "dosbox" } },
    { "camera", { "album", "screenshot", "capture", "camera", "photo", "gallery" } },
    { "chat", { "chat", "discord", "messag", "social", "reddit" } },
    { "web", { "browser", "web", "internet" } },
    { "download", { "download", "torrent", "cloud", "sync" } },
    { "dev", { "nxlink", "terminal", "debug", "shell", "python", "devkit" } },
    { "drawing", { "draw", "paint", "sketch", "pixel" } },
    { "clock", { "clock", "timer", "alarm" } },
    { "calculator", { "calc" } },
    { "learning", { "learn", "quiz", "school", "math" } },
    { "system", { "toolbox", "setting", "config", "sys-", "system" } },
    { "tools", { "tool", "util" } },
    { "games", { "game", "doom", "tetris", "chess", "puzzle", "quake", "2048" } },
};

}   // namespace

const std::vector<std::string>& ids()
{
    static const std::vector<std::string> all(std::begin(IDS), std::end(IDS));
    return all;
}

bool is_ref(const std::string& icon)
{
    return icon.rfind(PREFIX, 0) == 0;
}

std::string id_of(const std::string& icon)
{
    if (!is_ref(icon)) return "";
    const std::string id = icon.substr(std::char_traits<char>::length(PREFIX));
    const auto& all = ids();
    return std::find(all.begin(), all.end(), id) != all.end() ? id : "";
}

std::string ref_of(const std::string& id)
{
    return PREFIX + id;
}

std::string asset_of(const std::string& id)
{
    return "img/forwarder/" + id + ".png";
}

std::string suggest(const std::string& name, const std::string& nro)
{
    // The file's name without its folders: "/switch/JKSV/JKSV.nro" -> "jksv.nro".
    const size_t slash = nro.find_last_of("/:");
    const std::string seen = text::lower_ascii(name + " " + (slash == std::string::npos ? nro : nro.substr(slash + 1)));
    for (const auto& kind : WORDS)
        for (const char* word : kind.second)
            if (seen.find(word) != std::string::npos) return kind.first;
    return "";
}

}   // namespace forwarder_library
