// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/nand.hpp"

#include "core/nand_key.h"

namespace nand
{

std::string key(Storage storage)
{
    // The agent sysmodule names the folder the same way (core/nand_key.h).
    return nand_key_name(storage != Storage::Unknown, storage == Storage::EmuMMC);
}

}   // namespace nand
