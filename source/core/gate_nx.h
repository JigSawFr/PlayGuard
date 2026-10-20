// gate_nx — the gate itself (core/gate.h), with libnx: run by the gate's
// own .nro (gate/) and by PlayGuard, first thing in main(), for a forwarder
// that starts PlayGuard as its gate (made before the gate had its own .nro).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// When this program was started as a gate ("--pin-gate" in its argv): in
// the homebrew's free hours, or with no PIN on the console, hands over to
// it (envSetNextLoad, once this program exits); else the system's PIN
// screen first, B or an error ending the forwarder (back to the HOME menu).
// The try goes to the log (GATE_LOG_NAME). True then: the caller just exits.
// False, and nothing done, when it was not started as a gate.
bool gate_nx_run(void);

#ifdef __cplusplus
}
#endif
