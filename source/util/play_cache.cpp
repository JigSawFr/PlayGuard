// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/play_cache.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace play_cache
{

namespace
{
constexpr char MAGIC[4] = { 'P', 'G', 'P', 'C' };
constexpr uint32_t VERSION = 1;

struct Header
{
    char magic[4];
    uint32_t version;
    uint32_t stats_size;   // sizeof(PlayStats), sizeof(GameStat): another
    uint32_t game_size;    // layout (another build) is not read
};

// The stats without their games array, then `count` games.
constexpr size_t HEAD_SIZE = offsetof(PlayStats, games);

// A damaged file can hold a bool that is neither 0 nor 1, and reading one is
// undefined: its byte is read and written back as one.
void fix_bool(bool& b)
{
    unsigned char v;
    std::memcpy(&v, &b, 1);
    v = v != 0;
    std::memcpy(&b, &v, 1);
}
}   // namespace

std::string encode(const PlayStats& stats)
{
    const uint32_t count = stats.count > PLAYSTATS_MAX ? PLAYSTATS_MAX : stats.count;
    Header h;
    std::memcpy(h.magic, MAGIC, sizeof MAGIC);
    h.version    = VERSION;
    h.stats_size = sizeof(PlayStats);
    h.game_size  = sizeof(GameStat);
    std::string out;
    out.append(reinterpret_cast<const char*>(&h), sizeof h);
    out.append(reinterpret_cast<const char*>(&stats), HEAD_SIZE);
    out.append(reinterpret_cast<const char*>(stats.games), count * sizeof(GameStat));
    return out;
}

bool decode(const std::string& bytes, PlayStats& out)
{
    Header h;
    if (bytes.size() < sizeof h + HEAD_SIZE) return false;
    bytes.copy(reinterpret_cast<char*>(&h), sizeof h, 0);
    if (std::memcmp(h.magic, MAGIC, sizeof MAGIC) != 0 || h.version != VERSION ||
        h.stats_size != sizeof(PlayStats) || h.game_size != sizeof(GameStat))
        return false;
    std::memset(&out, 0, sizeof out);
    bytes.copy(reinterpret_cast<char*>(&out), HEAD_SIZE, sizeof h);
    if (out.count > PLAYSTATS_MAX || bytes.size() != sizeof h + HEAD_SIZE + out.count * sizeof(GameStat)) {
        std::memset(&out, 0, sizeof out);
        return false;
    }
    bytes.copy(reinterpret_cast<char*>(out.games), out.count * sizeof(GameStat), sizeof h + HEAD_SIZE);
    fix_bool(out.windows_ok);
    fix_bool(out.prev_week_ok);
    fix_bool(out.month_ok);
    fix_bool(out.hours_ok);
    fix_bool(out.approximate);
    fix_bool(out.truncated);
    for (uint32_t i = 0; i < out.count; i++) {
        out.games[i].name[PLAYSTATS_NAME_LEN - 1] = '\0';
        fix_bool(out.games[i].totals_ok);
    }
    return true;
}

void shift_days(PlayStats& stats, int days)
{
    if (days == 0 || !stats.windows_ok) return;
    // The week before the last 7, the month and the hours are not known any
    // more day by day.
    stats.prev_week_ok = false;
    stats.month_ok = false;
    stats.hours_ok = false;
    if (days < 0) {
        stats.windows_ok = false;
        return;
    }
    const int wday0 = stats.day_wday[0];
    for (int k = 0; k < 7; k++) stats.day_wday[k] = (uint8_t)(((wday0 + days - k) % 7 + 7) % 7);
    for (uint32_t i = 0; i < stats.count; i++) {
        GameStat& g = stats.games[i];
        uint32_t week = 0;
        for (int k = 6; k >= 0; k--) {
            g.day_s[k] = k >= days ? g.day_s[k - days] : 0;
            week += g.day_s[k];
        }
        g.today_s = g.day_s[0];
        g.week_s  = week;
    }
}

}   // namespace play_cache
