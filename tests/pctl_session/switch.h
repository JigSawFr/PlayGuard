// Minimal fake <switch.h> for the pctl_ops host tests. It models service
// ownership, firmware version and command payloads — not Horizon IPC encoding.
#pragma once
#include "fake_switch_base.h"

typedef struct { unsigned unused; } Service;

bool hosversionAtLeast(u8 major, u8 minor, u8 micro);
Result pctlInitialize(void);
void pctlExit(void);
Service *pctlGetServiceSession_Service(void);
Result pctlauthRegisterPasscode(void);
Result pctlauthShowForConfiguration(void);
/* libnx's crypto/hmac.h; the mock records the key it is given. */
void hmacSha256CalculateMac(void *dst, const void *key, size_t key_size, const void *src, size_t size);
/* set:sys, for the recovery proof's secret (GetMiiAuthorId). */
typedef struct { u8 uuid[0x10]; } Uuid;
Result setsysInitialize(void);
void setsysExit(void);
Result setsysGetMiiAuthorId(Uuid *out);

/* Buffer descriptors (the "..." arguments) keep only their attributes, address
 * and size, so the mock can fill GetPinCode's buffer and check what 1201
 * receives. The values mirror libnx's SfBufferAttr bits. */
enum {
    SfBufferAttr_In          = 1u << 0,
    SfBufferAttr_Out         = 1u << 1,
    SfBufferAttr_HipcMapAlias = 1u << 2,
    SfBufferAttr_HipcPointer = 1u << 3,
};
typedef struct { const void *ptr; size_t size; } SfBuffer;
typedef struct { u32 buffer_attrs[8]; SfBuffer buffers[8]; } SfDispatchParams;

Result mock_dispatch(Service *srv, u32 command, void *out, size_t out_size,
                     const void *in, size_t in_size, SfDispatchParams params);

#define serviceDispatch(srv, command, ...) \
    mock_dispatch((srv), (command), NULL, 0, NULL, 0, (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchOut(srv, command, value, ...) \
    mock_dispatch((srv), (command), &(value), sizeof(value), NULL, 0, (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchIn(srv, command, value, ...) \
    mock_dispatch((srv), (command), NULL, 0, &(value), sizeof(value), (SfDispatchParams){ __VA_ARGS__ })
#define serviceDispatchInOut(srv, command, in_value, out_value, ...) \
    mock_dispatch((srv), (command), &(out_value), sizeof(out_value), &(in_value), sizeof(in_value), \
                  (SfDispatchParams){ __VA_ARGS__ })
