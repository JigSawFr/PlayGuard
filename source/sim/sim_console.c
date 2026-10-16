// The desktop build's console_info readers: a Switch OLED on Wi-Fi, in
// handheld mode, its values overridable for the smoke tests:
// PLAYGUARD_SIM_MODEL (a SetSysProductModel number, 6 by default),
// PLAYGUARD_SIM_BATTERY (percent; "none" unknown), PLAYGUARD_SIM_DOCKED,
// PLAYGUARD_SIM_NETWORK ("wifi", "ethernet", "none"), PLAYGUARD_SIM_SD_FREE_MB.
// The parsing (console_info_pure.c) is the real one.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../core/console_info.h"

static long env_long(const char *name, long fallback)
{
    const char *v = getenv(name);
    if (!v || !*v) return fallback;
    if (!strcmp(v, "none")) return -1;
    return strtol(v, NULL, 10);
}

void console_live_read(ConsoleLive *out)
{
    console_live_init(out);
    out->model = console_model_of((int)env_long("PLAYGUARD_SIM_MODEL", 6));
    snprintf(out->nickname, sizeof(out->nickname), "%s", "PlayGuard's Switch");
    snprintf(out->region, sizeof(out->region), "%s", "eur");
    out->battery_pct = (int)env_long("PLAYGUARD_SIM_BATTERY", 87);
    out->charger = out->battery_pct < 0 ? -1 : 1;
    out->docked = getenv("PLAYGUARD_SIM_DOCKED") ? 1 : 0;
    out->temperature_c = 41;
    const char *net = getenv("PLAYGUARD_SIM_NETWORK");
    out->network = !net || !strcmp(net, "wifi") ? 1 : !strcmp(net, "ethernet") ? 2 : 0;
    if (out->network > 0) snprintf(out->ip, sizeof(out->ip), "%s", "192.168.1.20");
    if (out->network == 1) {
        out->wifi_bars = 3;
        snprintf(out->ssid, sizeof(out->ssid), "%s", "Maison");
    }
    // Started five hours ago, to the minute (stable from one document to the next).
    out->uptime_s = 5 * 3600 + (uint64_t)(time(NULL) % 60);
}

void console_space_read(ConsoleSpace *sd, ConsoleSpace *nand)
{
    sd->total_mb = 59640;
    sd->free_mb = env_long("PLAYGUARD_SIM_SD_FREE_MB", 21344);
    nand->total_mb = 26592;
    nand->free_mb = 18470;
}
