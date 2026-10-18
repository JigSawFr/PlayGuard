// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sync_entities.h"

#include <string.h>

// Discovery abbreviations (HA): unit_of_meas, dev_cla, stat_cla, ent_cat,
// pl_on / pl_off (binary_sensor, switch commands), stat_on / stat_off
// (switch state), ops (select options), cmd_tpl (command template), en
// (enabled by default), ic (icon).
#define E_MINUTES "\"unit_of_meas\":\"min\",\"dev_cla\":\"duration\""
#define E_MEASURE E_MINUTES ",\"stat_cla\":\"measurement\""
// A daily total, back to 0 at midnight (HA's "total_increasing", like a daily meter).
#define E_TOTAL E_MINUTES ",\"stat_cla\":\"total_increasing\""
#define E_BINARY "\"pl_on\":\"True\",\"pl_off\":\"False\""
#define E_SWITCH "\"stat_on\":\"True\",\"stat_off\":\"False\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\""
#define E_LIMIT "\"min\":0,\"max\":1440,\"step\":5,\"mode\":\"box\"," E_MINUTES ",\"ent_cat\":\"config\""
#define E_DIAG "\"ent_cat\":\"diagnostic\""
#define E_CONFIG "\"ent_cat\":\"config\""
#define E_SPACE "\"unit_of_meas\":\"MiB\",\"dev_cla\":\"data_size\",\"stat_cla\":\"measurement\""
// The document an entity's template reads (NULL: the state).
#define T_SYSTEM "system"
#define T_GAMES "games"

static const SyncEntity ENTITIES[] = {
    // binary sensors
    { "parental_controls_enabled", "binary_sensor", SyncWrite_None, "Parental controls",
      "{{ value_json.controls.enabled }}", E_BINARY ",\"ic\":\"mdi:shield-account\"", NULL },
    { "temp_unlocked", "binary_sensor", SyncWrite_None, "Temporarily unlocked",
      "{{ value_json.controls.temp_unlocked }}", E_BINARY ",\"dev_cla\":\"lock\"", NULL },
    { "pin_set", "binary_sensor", SyncWrite_None, "PIN set", "{{ value_json.controls.pin_set }}", E_BINARY "," E_DIAG, NULL },
    { "companion_linked", "binary_sensor", SyncWrite_None, "Companion app linked",
      "{{ value_json.controls.companion_linked }}", E_BINARY ",\"ic\":\"mdi:cellphone-link\"", NULL },
    { "timer_enabled", "binary_sensor", SyncWrite_None, "Play timer", "{{ value_json.timer.enabled }}",
      E_BINARY ",\"ic\":\"mdi:timer-outline\"", NULL },
    { "limit_reached", "binary_sensor", SyncWrite_None, "Time is up", "{{ value_json.timer.limit_reached }}",
      E_BINARY ",\"ic\":\"mdi:timer-alert-outline\"", NULL },
    { "clock_accurate", "binary_sensor", SyncWrite_None, "Network clock accurate",
      "{{ value_json.clock_accurate }}", E_BINARY "," E_DIAG, NULL },

    // sensors
    { "used_screen_time", "sensor", SyncWrite_None, "Used screen time", "{{ value_json.timer.used_min }}", E_TOTAL, NULL },
    { "used_screen_time_log", "sensor", SyncWrite_None, "Used screen time (play log)",
      "{{ value_json.activity_today.used_min }}", E_TOTAL, NULL },
    { "screen_time_remaining", "sensor", SyncWrite_None, "Screen time remaining",
      "{{ value_json.timer.remaining_min }}", E_MEASURE, NULL },
    { "extended_screen_time", "sensor", SyncWrite_None, "Extended screen time",
      "{{ value_json.timer.extended_today_min }}", E_MINUTES, NULL },
    { "now_playing", "sensor", SyncWrite_None, "Now playing",
      "{{ value_json.activity_today.now_playing.name or value_json.activity_today.now_playing.app_id }}",
      "\"ic\":\"mdi:gamepad-variant\"", NULL },
    { "rating_age", "sensor", SyncWrite_None, "Age rating", "{{ value_json.controls.rating_age }}", E_DIAG, NULL },
    { "firmware", "sensor", SyncWrite_None, "Firmware", "{{ value_json.console.firmware }}", E_DIAG, NULL },
    { "atmosphere", "sensor", SyncWrite_None, "Atmosphère", "{{ value_json.console.atmosphere }}", E_DIAG, NULL },
    { "spent_raw", "sensor", SyncWrite_None, "Time spent (1952)", "{{ value_json.timer.spent_raw_min }}",
      E_MINUTES "," E_DIAG ",\"en\":false", NULL },
    { "last_order", "sensor", SyncWrite_None, "Last order", "{{ value_json.link.last_result }}", E_DIAG, NULL },

    // limits
    { "limit_sun", "number", SyncWrite_Timer, "Limit Sunday", "{{ value_json.timer.limits.sun }}", E_LIMIT, NULL },
    { "limit_mon", "number", SyncWrite_Timer, "Limit Monday", "{{ value_json.timer.limits.mon }}", E_LIMIT, NULL },
    { "limit_tue", "number", SyncWrite_Timer, "Limit Tuesday", "{{ value_json.timer.limits.tue }}", E_LIMIT, NULL },
    { "limit_wed", "number", SyncWrite_Timer, "Limit Wednesday", "{{ value_json.timer.limits.wed }}", E_LIMIT, NULL },
    { "limit_thu", "number", SyncWrite_Timer, "Limit Thursday", "{{ value_json.timer.limits.thu }}", E_LIMIT, NULL },
    { "limit_fri", "number", SyncWrite_Timer, "Limit Friday", "{{ value_json.timer.limits.fri }}", E_LIMIT, NULL },
    { "limit_sat", "number", SyncWrite_Timer, "Limit Saturday", "{{ value_json.timer.limits.sat }}", E_LIMIT, NULL },
    { "limit_uniform", "number", SyncWrite_Timer, "Limit every day", "{{ value_json.timer.uniform_min }}", E_LIMIT, NULL },
    { "max_screentime_today", "number", SyncWrite_Timer, "Max screentime today",
      "{{ value_json.timer.limit_today_min }}", E_LIMIT, NULL },

    // bedtime
    { "bedtime_alarm", "time", SyncWrite_Timer, "Bedtime alarm", "{{ value_json.timer.bedtime.start }}", E_CONFIG, NULL },
    { "bedtime_end_time", "time", SyncWrite_Timer, "Bedtime end time", "{{ value_json.timer.bedtime.end }}", E_CONFIG, NULL },
    { "bedtime_enabled", "switch", SyncWrite_Timer, "Bedtime alarm enabled",
      "{{ value_json.timer.bedtime.enabled }}", E_SWITCH "," E_CONFIG, NULL },

    // switches
    { "console_lock", "switch", SyncWrite_Timer, "Block all games", "{{ value_json.timer.console_locked }}",
      E_SWITCH ",\"ic\":\"mdi:lock\"", NULL },
    { "play_timer_alarm", "switch", SyncWrite_Timer, "Time's up alarm", "{{ value_json.timer.alarm_on }}",
      E_SWITCH "," E_CONFIG, NULL },
    { "unlocked", "switch", SyncWrite_Timer, "Unlocked", "{{ value_json.controls.temp_unlocked }}",
      E_SWITCH ",\"ic\":\"mdi:lock-open-variant\"", NULL },
    { "vr_restricted", "switch", SyncWrite_Free, "VR mode restricted", "{{ value_json.controls.vr_restricted }}",
      E_SWITCH "," E_CONFIG, NULL },
    { "sns_post_restricted", "switch", SyncWrite_Free, "Posting to social media restricted",
      "{{ value_json.controls.sns_post_restricted }}", E_SWITCH "," E_CONFIG, NULL },
    { "free_communication_restricted", "switch", SyncWrite_Free, "Communication restricted",
      "{{ value_json.controls.free_communication_restricted }}", E_SWITCH "," E_CONFIG, NULL },

    // selects (the profile's options are added when the discovery is built).
    // The level "none" is shown as "unrestricted": HA reads a select's "None"
    // as unknown. The order still carries "none" (cmd_tpl).
    { "restriction_level", "select", SyncWrite_Free, "Restriction level",
      "{{ 'unrestricted' if value_json.controls.level == 'none' else value_json.controls.level }}",
      "\"ops\":[\"unrestricted\",\"young_child\",\"child\",\"teen\",\"custom\"],"
      "\"cmd_tpl\":\"{{ 'none' if value == 'unrestricted' else value }}\"," E_CONFIG, NULL },
    { "profile", "select", SyncWrite_Timer, "Profile", "{{ None }}", E_CONFIG, NULL },

    // buttons
    { "sync_now", "button", SyncWrite_Free, "Sync now", NULL, "\"ic\":\"mdi:sync\"," E_DIAG, NULL },
    { "lock_now", "button", SyncWrite_Free, "Turn parental controls back on", NULL, "\"ic\":\"mdi:lock\"", NULL },
    { "export_report", "button", SyncWrite_Free, "Export a diagnostic report", NULL, E_DIAG, NULL },
    { "add_bonus_time_15", "button", SyncWrite_Timer, "Extra time +15 min", NULL, "\"ic\":\"mdi:timer-plus-outline\"", NULL },
    { "add_bonus_time_30", "button", SyncWrite_Timer, "Extra time +30 min", NULL, "\"ic\":\"mdi:timer-plus-outline\"", NULL },
    { "add_bonus_time_60", "button", SyncWrite_Timer, "Extra time +1 h", NULL, "\"ic\":\"mdi:timer-plus-outline\"", NULL },
    { "stop_today", "button", SyncWrite_Timer, "No more play today", NULL, "\"ic\":\"mdi:timer-off-outline\"", NULL },
    { "remove_limit", "button", SyncWrite_Timer, "Remove the limit", NULL, "\"ic\":\"mdi:timer-remove-outline\"", NULL },

    // events (state topic and event types set when the discovery is built)
    { "events", "event", SyncWrite_None, "Events", NULL, "", NULL },

    // The console itself (the system document): versions, hardware, storage,
    // power, network. Diagnostic but for what a parent may want on a card.
    { "model", "sensor", SyncWrite_None, "Model", "{{ value_json.hardware.model }}",
      "\"dev_cla\":\"enum\",\"ops\":[\"erista\",\"mariko\",\"lite\",\"oled\",\"other\"]," E_DIAG, T_SYSTEM },
    { "nickname", "sensor", SyncWrite_None, "Console nickname", "{{ value_json.hardware.nickname }}",
      "\"ic\":\"mdi:tag-text-outline\"," E_DIAG, T_SYSTEM },
    { "region", "sensor", SyncWrite_None, "Region", "{{ value_json.hardware.region }}",
      "\"dev_cla\":\"enum\",\"ops\":[\"jpn\",\"usa\",\"eur\",\"aus\",\"htk\",\"chn\"]," E_DIAG, T_SYSTEM },
    { "hekate", "sensor", SyncWrite_None, "hekate", "{{ value_json.versions.hekate }}", E_DIAG, T_SYSTEM },
    { "system_storage", "sensor", SyncWrite_None, "System", "{{ value_json.hardware.system }}",
      "\"dev_cla\":\"enum\",\"ops\":[\"sysmmc\",\"emummc\"]," E_DIAG, T_SYSTEM },
    { "prodinfo_blanked", "binary_sensor", SyncWrite_None, "PRODINFO blanked",
      "{{ value_json.hardware.prodinfo_blanked }}", E_BINARY "," E_DIAG, T_SYSTEM },
    // Only with "Publish the serial number" on the console; off in Home Assistant too.
    { "serial", "sensor", SyncWrite_None, "Serial number", "{{ value_json.hardware.serial }}",
      E_DIAG ",\"en\":false", T_SYSTEM },
    { "sd_free", "sensor", SyncWrite_None, "SD card free space", "{{ value_json.storage.sd.free_mb }}",
      E_SPACE ",\"ic\":\"mdi:micro-sd\"", T_SYSTEM },
    { "sd_total", "sensor", SyncWrite_None, "SD card size", "{{ value_json.storage.sd.total_mb }}",
      E_SPACE ",\"ic\":\"mdi:micro-sd\"," E_DIAG, T_SYSTEM },
    { "nand_free", "sensor", SyncWrite_None, "Internal storage free space", "{{ value_json.storage.nand.free_mb }}",
      E_SPACE ",\"ic\":\"mdi:harddisk\"", T_SYSTEM },
    { "nand_total", "sensor", SyncWrite_None, "Internal storage size", "{{ value_json.storage.nand.total_mb }}",
      E_SPACE ",\"ic\":\"mdi:harddisk\"," E_DIAG, T_SYSTEM },
    { "battery", "sensor", SyncWrite_None, "Battery", "{{ value_json.power.battery_pct }}",
      "\"unit_of_meas\":\"%\",\"dev_cla\":\"battery\",\"stat_cla\":\"measurement\"", T_SYSTEM },
    { "charging", "binary_sensor", SyncWrite_None, "Charging", "{{ value_json.power.charging }}",
      E_BINARY ",\"dev_cla\":\"battery_charging\"", T_SYSTEM },
    { "docked", "binary_sensor", SyncWrite_None, "Docked", "{{ value_json.power.docked }}",
      E_BINARY ",\"ic\":\"mdi:television\"", T_SYSTEM },
    { "temperature", "sensor", SyncWrite_None, "Temperature", "{{ value_json.power.temperature_c }}",
      "\"unit_of_meas\":\"\xc2\xb0" "C\",\"dev_cla\":\"temperature\",\"stat_cla\":\"measurement\"," E_DIAG, T_SYSTEM },
    { "network", "sensor", SyncWrite_None, "Connection", "{{ value_json.network.type }}",
      "\"dev_cla\":\"enum\",\"ops\":[\"none\",\"wifi\",\"ethernet\"]," E_DIAG, T_SYSTEM },
    { "ip_address", "sensor", SyncWrite_None, "IP address", "{{ value_json.network.ip }}",
      "\"ic\":\"mdi:ip-network\"," E_DIAG, T_SYSTEM },
    { "wifi_signal", "sensor", SyncWrite_None, "Wi-Fi signal", "{{ value_json.network.wifi_bars }}",
      "\"stat_cla\":\"measurement\",\"ic\":\"mdi:wifi\"," E_DIAG, T_SYSTEM },
    // The network's name: off in Home Assistant until wanted.
    { "ssid", "sensor", SyncWrite_None, "Wi-Fi network", "{{ value_json.network.ssid }}",
      "\"ic\":\"mdi:wifi\"," E_DIAG ",\"en\":false", T_SYSTEM },
    { "last_boot", "sensor", SyncWrite_None, "Last started",
      "{{ as_datetime(value_json.boot_ts) if value_json.boot_ts else None }}", "\"dev_cla\":\"timestamp\"," E_DIAG,
      T_SYSTEM },

    // The games (the games document, PlayGuard's): the per-game sensors are
    // the integration's (one device payload could not hold dozens).
    { "games_installed", "sensor", SyncWrite_None, "Games installed", "{{ value_json.installed }}",
      "\"stat_cla\":\"measurement\",\"ic\":\"mdi:gamepad-square\"", T_GAMES },
    { "play_time_total", "sensor", SyncWrite_None, "Play time (all time)", "{{ value_json.total_min }}",
      E_MINUTES ",\"stat_cla\":\"total_increasing\",\"ic\":\"mdi:timer-sand-complete\"", T_GAMES },
    { "accounts", "sensor", SyncWrite_None, "User accounts", "{{ value_json.accounts }}",
      "\"ic\":\"mdi:account-multiple\"," E_DIAG, T_GAMES },
};

const SyncEntity *sync_entities(size_t *count)
{
    *count = sizeof(ENTITIES) / sizeof(ENTITIES[0]);
    return ENTITIES;
}

const SyncEntity *sync_entity_find(const char *id)
{
    for (size_t i = 0; i < sizeof(ENTITIES) / sizeof(ENTITIES[0]); i++)
        if (!strcmp(ENTITIES[i].id, id)) return &ENTITIES[i];
    return NULL;
}
