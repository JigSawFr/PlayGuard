// PlayGuard's gate: the .nro a forwarder protected with the PIN starts first
// (core/gate.h, core/gate_nx.h). A few hundred kilobytes, so the PIN screen
// shows at once; PlayGuard writes it to config/playguard/gate.nro from its
// romfs. Started any other way, it does nothing.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <switch.h>

#include "gate_nx.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    // Its romfs carries PlayGuard's font for the screen (romfs:/font/).
    const bool romfs = R_SUCCEEDED(romfsInit());
    gate_nx_run();
    if (romfs) romfsExit();
    return 0;
}
