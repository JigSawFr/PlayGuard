// pctl_ops_c — the C service layer's headers (source/core/), in one include
// for the C++ UI. Each of them declares its functions extern "C" itself
// (#ifdef __cplusplus), so it can also be included on its own.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include "core/gamecheck.h"
#include "core/modules_nx.h"
#include "core/pctl_ops.h"
#include "core/playstats.h"
#include "core/sysinfo.h"
#include "core/time_ops.h"
#include "core/write_guard.h"
