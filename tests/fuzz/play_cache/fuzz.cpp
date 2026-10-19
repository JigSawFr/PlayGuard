// libFuzzer target: the file of source/util/play_cache.cpp, read back from
// the SD card (a damaged or hand-edited one included). Beyond the
// sanitizers: what decode() accepts encodes to the same bytes, its games are
// bounded and their names end inside their arrays, and the day windows shift
// by any count without reading past them.
#include <cstring>
#include <memory>
#include <string>

#include "fuzz.h"
#include "util/play_cache.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string bytes(reinterpret_cast<const char*>(data), size);
    // About 60 KB: on the heap.
    auto stats = std::make_unique<PlayStats>();
    if (!play_cache::decode(bytes, *stats)) return 0;
    FUZZ_ASSERT(stats->count <= PLAYSTATS_MAX);
    for (uint32_t i = 0; i < stats->count; i++)
        FUZZ_ASSERT(std::memchr(stats->games[i].name, 0, sizeof stats->games[i].name) != nullptr);

    const std::string again = play_cache::encode(*stats);
    auto back = std::make_unique<PlayStats>();
    FUZZ_ASSERT(play_cache::decode(again, *back));
    FUZZ_ASSERT(play_cache::encode(*back) == again);

    // What the Activity tab and Today read of it.
    uint64_t sum = stats->windows_ok + stats->approximate + stats->truncated;
    for (uint32_t i = 0; i < stats->count; i++) sum += stats->games[i].totals_ok;
    (void)sum;

    const int days = size ? (int)(data[size - 1] % 12) - 2 : 0;   // -2 to 9
    play_cache::shift_days(*stats, days);
    play_cache::encode(*stats);
    return 0;
}
