// pt_flow_internal — what the pt_flow*.cpp files share and nothing else
// uses: the record of an unlock, which every unlock writes first (config
// relock_pending, pt_flow_unlock.cpp).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

namespace pt_flow
{

// False when the record could not be saved to the SD card. Setting it waits
// for the write: it must be on the card before the unlock it is for.
bool set_relock_pending(bool on);

// After an unlock attempt that failed: drop the record only when the console
// reads back as locked (the unlock may have gone through and its lock failed).
void settle_failed_unlock();

}   // namespace pt_flow
