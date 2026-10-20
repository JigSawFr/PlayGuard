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
    gate_nx_run();
    return 0;
}
