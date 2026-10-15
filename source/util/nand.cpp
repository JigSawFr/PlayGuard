// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/nand.hpp"

namespace nand
{

std::string key(Storage storage)
{
    switch (storage) {
        case Storage::SysMMC: return "sysmmc";
        case Storage::EmuMMC: return "emummc";
        case Storage::Unknown: break;
    }
    return "unknown";
}

}   // namespace nand
