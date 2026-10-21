// icon_sheet — a forwarder's HOME menu icon chosen among the album's
// screenshots (Console › Forwarders › Change the icon): a grid of them as
// the icons they would make (its .nro's own first), the one in use checked;
// beside it, what the HOME menu shows now and what it will show, the game
// the screenshot was taken in and when. L / R move the icon's square along
// the screenshot, Y takes the .nro's own back, A the one in focus. The
// screenshots are read and decoded off the UI thread (their Exif thumbnail,
// then the one in focus in full), so it opens at once.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"

namespace icon_sheet
{

// Opens it for `row` over `shots` (forwarder_flow::album_shots(), not
// empty). `done` runs once it has closed, with the screenshot chosen (""
// for the .nro's own) and where its square sits; not when closed with B, or
// with what is in use already.
void open(const forwarders_logic::Row& row, std::vector<std::string> shots,
          std::function<void(const std::string& shot, int x)> done);

}   // namespace icon_sheet
