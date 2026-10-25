// icon_logic — the games' icons without borealis (action/game_icons): the bytes
// kept as an LRU of a fixed number of games (the control data's JPEG, up to
// 128 KiB each, read again from ns when evicted), and the decoded image
// shrunk to the size it is drawn at (a 256 x 256 icon shown at 44 px).
// Plain C++ so the host tests can run it (tests/icon_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <utility>
#include <vector>

namespace icon_logic
{

// The icons' bytes for at most `capacity` games: past it, the one used
// least recently goes.
class Lru
{
  public:
    explicit Lru(size_t most) : capacity(most) { }

    // The bytes for `id`, now the most recently used; nullptr when not kept.
    // Valid until the next put().
    const std::vector<unsigned char>* get(uint64_t id);
    bool contains(uint64_t id) const { return this->index.count(id) != 0; }
    // Kept as the most recently used (replacing what `id` had).
    void put(uint64_t id, std::vector<unsigned char> bytes);
    // No longer kept (nothing when it was not).
    void erase(uint64_t id);
    size_t size() const { return this->index.size(); }

  private:
    using Entry = std::pair<uint64_t, std::vector<unsigned char>>;
    size_t capacity;
    std::list<Entry> order;   // most recently used first
    std::map<uint64_t, std::list<Entry>::iterator> index;
};

// The size a `w` x `h` image is shrunk to so its longer side is at most
// `max_side`, keeping its shape (at least 1 x 1); a smaller one keeps its own.
std::pair<int, int> fit(int w, int h, int max_side);

// `src`, RGBA `w` x `h`, shrunk to `dw` x `dh` (no larger than it): each
// pixel is the mean of the source pixels it covers, so a 256 px icon keeps
// its detail at 88. Empty for a bad size.
std::vector<unsigned char> shrink(const unsigned char* src, int w, int h, int dw, int dh);

}   // namespace icon_logic
