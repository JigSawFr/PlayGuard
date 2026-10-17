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

typedef struct { char number[0x18]; } SetSysSerialNumber;
Result setsysInitialize(void);
void setsysExit(void);
Result setsysGetSerialNumber(SetSysSerialNumber *out);
