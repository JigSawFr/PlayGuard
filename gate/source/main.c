// PlayGuard's gate: the .nro a forwarder protected with the PIN starts first
// (core/gate.h, core/gate_nx.h). A few hundred kilobytes, so the PIN screen
// shows at once; PlayGuard writes it to config/playguard/gate.nro from its
// romfs. Started any other way, it goes back to the HOME menu.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <switch.h>

#include "gate_nx.h"

// libnx's own exit: 1 ends the application (back to the HOME menu) instead
// of going back to the forwarder's loader.
extern u32 __nx_applet_exit_mode;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    // Its romfs carries PlayGuard's font for the screen (romfs:/font/).
    const bool romfs = R_SUCCEEDED(romfsInit());
    // Nothing decided (no gate argv, or one it cannot read): back to the HOME
    // menu, never on to the loader, which would start the homebrew it guards.
    if (!gate_nx_run()) __nx_applet_exit_mode = 1;
    if (romfs) romfsExit();
    return 0;
}
