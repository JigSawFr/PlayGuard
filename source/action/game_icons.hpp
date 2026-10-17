// game_icons — the games' icons for the Games screen and a game's own screen:
// read off the main thread from the control data, their bytes kept as an LRU
// (action/icon_logic.hpp), decoded small once for the run (NVG images, up to
// a fixed number; past it, a cell decodes and frees its own). None in applet
// mode (opened from the album: a few MB is too much). UI thread only.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <vector>

#include "util/pctl_ops_c.hpp"

namespace game_icons
{

// Icons are decoded at this size at most: a cover of the Games screen's grid
// is drawn at 144 px. The control data's are 256 x 256.
constexpr int ICON_PX = 144;

// False in applet mode: no icon is read at all.
bool wanted();

// The icon's bytes as the control data holds them (a game's screen shows them
// larger), or nullptr when not read (or there is none).
const std::vector<unsigned char>* bytes(u64 app_id);

// A texture of the icon: one kept for the run (`owned` false: never free it),
// else a new one the caller owns (`owned` true); 0 when the icon is not read
// yet, or cannot be decoded.
int texture(u64 app_id, bool& owned);

// The game has no icon to read, or it is read already.
bool known(u64 app_id);

// Reads the icons of `ids` not read yet, off the main thread (one read at a
// time: asked meanwhile, the next read takes them). `done` runs on the UI
// thread once icons arrive; only the last caller's is kept (it checks that
// its screen is still there).
void load(const std::vector<u64>& ids, std::function<void()> done);

}   // namespace game_icons
