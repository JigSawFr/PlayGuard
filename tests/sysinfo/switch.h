// Minimal fake <switch.h> for the sysinfo host tests.
#pragma once
#include "fake_switch_base.h"

typedef int SplConfigItem;
typedef enum { AppletType_Application = 0, AppletType_LibraryApplet = 2 } AppletType;

u32 hosversionGet(void);
bool hosversionIsAtmosphere(void);
Result splInitialize(void);
void splExit(void);
Result splGetConfig(SplConfigItem item, u64 *out);
AppletType appletGetAppletType(void);
#define CUR_PROCESS_HANDLE 0xFFFF8001
typedef enum { InfoType_ProgramId = 18 } InfoType;
Result svcGetInfo(u64 *out, u32 id0, u32 handle, u64 id1);

typedef struct { char number[0x18]; } SetSysSerialNumber;
Result setsysInitialize(void);
void setsysExit(void);
Result setsysGetSerialNumber(SetSysSerialNumber *out);
