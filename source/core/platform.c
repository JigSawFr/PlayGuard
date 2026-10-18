// PlayGuard — console services for the UI, with libnx (see platform.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "platform.h"

#include <stdio.h>

PlatformInput platform_numpad(const char *header, const char *guide, const char *initial, int max_len,
                              char *out, size_t out_size)
{
    if (!out || out_size == 0) return PLATFORM_INPUT_CANCELLED;
    out[0] = '\0';
    // The number pad with a ':' key, so "1:30" can be typed. borealis'
    // openForNumber reads the result with stoll and would stop at the colon.
    SwkbdConfig kbd;
    if (R_FAILED(swkbdCreate(&kbd, 0))) return PLATFORM_INPUT_NONE;
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetType(&kbd, SwkbdType_NumPad);
    swkbdConfigSetLeftOptionalSymbolKey(&kbd, ":");
    swkbdConfigSetHeaderText(&kbd, header);
    swkbdConfigSetSubText(&kbd, guide);
    swkbdConfigSetStringLenMax(&kbd, (u32)max_len);
    swkbdConfigSetInitialText(&kbd, initial);
    swkbdConfigSetBlurBackground(&kbd, true);
    const Result rc = swkbdShow(&kbd, out, out_size);
    swkbdClose(&kbd);
    return R_SUCCEEDED(rc) && out[0] ? PLATFORM_INPUT_OK : PLATFORM_INPUT_CANCELLED;
}

bool platform_in_focus(void)
{
    return appletGetFocusState() == AppletFocusState_InFocus;
}

bool platform_can_launch(void)
{
    return envHasNextLoad();
}

bool platform_set_next_load(const char *path)
{
    // hbloader passes the argument string as is; like its default
    // ("sdmc:/hbmenu.nro"), argv[0] is the full path of the homebrew.
    return path && envHasNextLoad() && R_SUCCEEDED(envSetNextLoad(path, path));
}

// hbmenu's own exit (+): libnx then ends the application itself (appletExit:
// IApplicationFunctions' exit) instead of going back to hbloader.
extern u32 __nx_applet_exit_mode;

void platform_exit_to_home(void)
{
    if (appletGetAppletType() == AppletType_Application) __nx_applet_exit_mode = 1;
}

bool platform_region(int *region)
{
    SetRegion r;
    if (!region || R_FAILED(setGetRegionCode(&r))) return false;
    *region = (int)r;
    return true;
}

bool platform_held_lr(void)
{
    // Read a few times over 30 ms: the first state read may predate the
    // controllers being polled. A connected controller's state is this
    // moment's, so it settles it at once (the 40 ms of waiting were most of
    // the start-up's "previous build check" on a console).
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    const u64 both = HidNpadButton_L | HidNpadButton_R;
    for (int i = 0; i < 4; i++) {
        padUpdate(&pad);
        if ((padGetButtons(&pad) & both) == both) return true;
        if (padIsConnected(&pad)) return false;
        if (i < 3) svcSleepThread(10000000ULL);
    }
    return false;
}

bool platform_boot_choice(const char *text)
{
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    printf("%s", text ? text : "");
    consoleUpdate(NULL);
    bool a = false;
    // Only a new press counts: L + R, held to get here, answer nothing.
    while (appletMainLoop()) {
        padUpdate(&pad);
        const u64 down = padGetButtonsDown(&pad);
        if (down & HidNpadButton_A) {
            a = true;
            break;
        }
        if (down & HidNpadButton_B) break;
        consoleUpdate(NULL);
    }
    // The screen goes back to borealis, which sets up its own.
    consoleExit(NULL);
    return a;
}

void platform_romfs_release(void)
{
    romfsExit();
}

void platform_romfs_mount(void)
{
    const Result rc = romfsInit();
    if (R_FAILED(rc)) fprintf(stderr, "platform: romfs not mounted again (0x%X)\n", (unsigned)rc);
}

bool platform_random(void *buf, size_t size)
{
    if (!buf) return false;
    randomGet(buf, size);
    return true;
}

bool platform_code_range(uint64_t *base, uint64_t *size)
{
    // The region this function is in: the .nro's text, which hbloader maps
    // from its first byte (NRO header included), so an offset from it is an
    // address of the linked .elf.
    MemoryInfo mi;
    u32 pi;
    if (!base || !size || R_FAILED(svcQueryMemory(&mi, &pi, (u64)(uintptr_t)&platform_code_range))) return false;
    *base = mi.addr;
    *size = mi.size;
    return true;
}

bool platform_read64(uint64_t addr, uint64_t *value, void *ctx)
{
    (void)ctx;
    MemoryInfo mi;
    u32 pi;
    if (!value || (addr & 7) || R_FAILED(svcQueryMemory(&mi, &pi, addr))) return false;
    if (mi.type == MemType_Unmapped || !(mi.perm & Perm_R) || addr + 8 > mi.addr + mi.size) return false;
    *value = *(const volatile u64 *)(uintptr_t)addr;
    return true;
}

bool platform_memory(uint64_t *used, uint64_t *total)
{
    if (!used || !total) return false;
    return R_SUCCEEDED(svcGetInfo(used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0)) &&
           R_SUCCEEDED(svcGetInfo(total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0));
}

void platform_crash(void)
{
    diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
}
