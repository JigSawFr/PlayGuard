// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sync_system.h"

#include <stdio.h>

#include "sync_json.h"
#include "sync_state.h"   // SYNC_SCHEMA

static void kversion(SyncJson *j, const char *key, bool ok, unsigned a, unsigned b, unsigned c)
{
    sync_json_key(j, key);
    if (!ok) {
        sync_json_null(j);
        return;
    }
    char v[24];
    snprintf(v, sizeof(v), "%u.%u.%u", a, b, c);
    sync_json_str(j, v);
}

static void space(SyncJson *j, const char *key, const ConsoleSpace *s)
{
    sync_json_key(j, key);
    sync_json_obj(j);
    sync_json_kint_or_null(j, "free_mb", s && s->free_mb >= 0, s ? s->free_mb : 0);
    sync_json_kint_or_null(j, "total_mb", s && s->total_mb > 0, s ? s->total_mb : 0);
    sync_json_obj_end(j);
}

size_t sync_system_build(const SyncSystem *s, char *out, size_t cap)
{
    const SysInfo *si = s->sys;
    const ConsoleLive *l = s->live;
    SyncJson j;
    sync_json_init(&j, out, cap);
    sync_json_obj(&j);
    sync_json_kint(&j, "schema", SYNC_SCHEMA);
    sync_json_kint_or_null(&j, "ts", s->ts != 0, (int64_t)s->ts);

    sync_json_key(&j, "versions");
    sync_json_obj(&j);
    const uint32_t hos = si ? si->hos_version : 0;
    kversion(&j, "firmware", hos != 0, (hos >> 16) & 0xFF, (hos >> 8) & 0xFF, hos & 0xFF);
    kversion(&j, "atmosphere", si && si->ams_valid, si ? si->ams_major : 0, si ? si->ams_minor : 0,
             si ? si->ams_micro : 0);
    sync_json_kstr_or_null(&j, "hekate", s->hekate);
    sync_json_kstr_or_null(&j, "app", s->app_version);
    sync_json_kstr_or_null(&j, "agent", s->agent_version);
    sync_json_obj_end(&j);

    sync_json_key(&j, "hardware");
    sync_json_obj(&j);
    sync_json_kstr_or_null(&j, "model", l ? console_model_name(l->model) : NULL);
    sync_json_kstr_or_null(&j, "nickname", l ? l->nickname : NULL);
    sync_json_kstr_or_null(&j, "region", l ? l->region : NULL);
    const SysStorage st = si ? sysinfo_storage(si) : SysStorage_Unknown;
    sync_json_kstr_or_null(&j, "system", st == SysStorage_EmuMMC ? "emummc" : st == SysStorage_SysMMC ? "sysmmc" : NULL);
    sync_json_kbool_or_null(&j, "prodinfo_blanked", si && si->blank_valid, si && si->blank);
    sync_json_kstr_or_null(&j, "serial", s->serial);
    sync_json_obj_end(&j);

    sync_json_key(&j, "storage");
    sync_json_obj(&j);
    space(&j, "sd", s->sd);
    space(&j, "nand", s->nand);
    sync_json_obj_end(&j);

    sync_json_key(&j, "power");
    sync_json_obj(&j);
    sync_json_kint_or_null(&j, "battery_pct", l && l->battery_pct >= 0, l ? l->battery_pct : 0);
    // Charging: a charger that gives enough power or some (the official one, the dock, USB-PD).
    sync_json_kbool_or_null(&j, "charging", l && l->charger >= 0, l && (l->charger == 1 || l->charger == 2));
    sync_json_kstr_or_null(&j, "charger", l ? console_charger_name(l->charger) : NULL);
    sync_json_kbool_or_null(&j, "docked", l && l->docked >= 0, l && l->docked == 1);
    sync_json_kint_or_null(&j, "temperature_c", l && l->temperature_c > -1000, l ? l->temperature_c : 0);
    sync_json_obj_end(&j);

    sync_json_key(&j, "network");
    sync_json_obj(&j);
    sync_json_kstr_or_null(&j, "type", l ? console_network_name(l->network) : NULL);
    sync_json_kstr_or_null(&j, "ip", l ? l->ip : NULL);
    sync_json_kint_or_null(&j, "wifi_bars", l && l->wifi_bars >= 0, l ? l->wifi_bars : 0);
    sync_json_kstr_or_null(&j, "ssid", l ? l->ssid : NULL);
    sync_json_obj_end(&j);

    // When it started, to the minute: the same from one document to the next.
    const bool boot = l && l->uptime_s && s->ts > l->uptime_s;
    sync_json_kint_or_null(&j, "boot_ts", boot, boot ? (int64_t)((s->ts - l->uptime_s) / 60 * 60) : 0);

    sync_json_obj_end(&j);
    return sync_json_end(&j);
}
