// pctl_ops_internal — what pctl_ops.c shares with pctl_dump.c: the typed
// readers, the PIN read and its wiping. Internal to the service layer (the UI
// includes pctl_ops.h); `static inline`, so nothing more is exported. The
// caller holds the session it passes.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once
#include "nx_types.h"
#include <string.h>

// Small typed readers. Reading a bool through a u8 keeps the transfer size at
// one byte whatever the platform's sizeof(bool).
static inline Result rd_bool(Service *s, u32 cmd, bool *out)
{
    u8 v = 0;
    Result rc = serviceDispatchOut(s, cmd, v);
    if (R_SUCCEEDED(rc)) *out = v != 0;
    return rc;
}
static inline Result rd_u8(Service *s, u32 cmd, u8 *out)
{
    u8 v = 0;
    Result rc = serviceDispatchOut(s, cmd, v);
    if (R_SUCCEEDED(rc)) *out = v;
    return rc;
}
static inline Result rd_u32(Service *s, u32 cmd, u32 *out)
{
    u32 v = 0;
    Result rc = serviceDispatchOut(s, cmd, v);
    if (R_SUCCEEDED(rc)) *out = v;
    return rc;
}
static inline Result rd_u64(Service *s, u32 cmd, u64 *out)
{
    u64 v = 0;
    Result rc = serviceDispatchOut(s, cmd, v);
    if (R_SUCCEEDED(rc)) *out = v;
    return rc;
}

static inline void secure_zero(void *p, size_t n)
{
    volatile u8 *b = (volatile u8 *)p;
    while (n--) *b++ = 0;
}

#define PIN_BUF 32

// GetPinCode (1208) into `pin` (PIN_BUF bytes, always NUL-terminated). The
// buffer is a HIPC *pointer* buffer, like 1201's (pctl_ops.c). The caller
// wipes `pin` and `*len`.
static inline Result read_pin(Service *srv, char *pin, u32 *len)
{
    memset(pin, 0, PIN_BUF);
    *len = 0;
    Result rc = serviceDispatchOut(srv, 1208, *len,
        .buffer_attrs = { SfBufferAttr_HipcPointer | SfBufferAttr_Out },
        .buffers      = { { pin, PIN_BUF } });
    pin[PIN_BUF - 1] = '\0';
    return rc;
}

// PlayTimerSettings: layout and codec in pure.h, known from 21.0.0 on.
static inline bool pt_fw_supported(void) { return hosversionAtLeast(21, 0, 0); }
