// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sync_conf.h"

#include <stdio.h>
#include <string.h>

static void copy(char *dst, size_t cap, const char *src)
{
    snprintf(dst, cap, "%s", src);
}

void sync_conf_defaults(SyncConf *c)
{
    memset(c, 0, sizeof(*c));
    c->schema = SYNC_CONF_SCHEMA;
    c->port = 1883;
    c->policy = SyncPolicy_Ask;
    c->publish_activity = c->publish_games = c->publish_icons = c->publish_console = c->publish_network = c->publish_hardware = true;
    c->ha_discovery = true;
    c->poll_s = 30;
    copy(c->topic_prefix, sizeof(c->topic_prefix), "boreale");
    copy(c->discovery_prefix, sizeof(c->discovery_prefix), "homeassistant");
    c->log_level = 1;
}

bool sync_conf_id_valid(const char *id)
{
    if (!id || strlen(id) != SYNC_ID_LEN) return false;
    for (const char *p = id; *p; p++)
        if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f'))) return false;
    return true;
}

void sync_conf_device_id(const SyncConf *c, char out[SYNC_ID_LEN + 1])
{
    out[0] = '\0';
    if (!sync_conf_id_valid(c->console_id)) return;
    memcpy(out, c->console_id, SYNC_ID_LEN + 1);
    if (!c->emummc) return;
    const char d = out[0];
    const int v = (d >= '0' && d <= '9' ? d - '0' : d - 'a' + 10) ^ 8;
    out[0] = (char)(v < 10 ? '0' + v : 'a' + v - 10);
}

void sync_conf_device_name(const SyncConf *c, const char *nickname, char *out, size_t cap)
{
    if (!cap) return;
    const char *name = c->console_name[0]                                 ? c->console_name
                       : c->publish_hardware && nickname && nickname[0] ? nickname
                                                                          : "Nintendo Switch";
    snprintf(out, cap, "%s%s", name, c->emummc ? " (emuMMC)" : "");
}

// A topic level we put our own names under: letters, digits, '_' and '-'.
static bool prefix_valid(const char *s)
{
    if (!*s) return false;
    for (const char *p = s; *p; p++) {
        const char ch = *p;
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-'))
            return false;
    }
    return true;
}

const char *sync_mqtt_version_name(SyncMqttVersion v)
{
    switch (v) {
        case SyncMqtt_V5: return "5";
        case SyncMqtt_V311: return "3.1.1";
        default: return "auto";
    }
}

const char *sync_policy_name(SyncPolicy p)
{
    switch (p) {
        case SyncPolicy_Auto: return "auto";
        case SyncPolicy_Off: return "off";
        default: return "ask";
    }
}

typedef struct {
    SyncConf *c;
    int read;
    bool prefix_read;   // topic_prefix was in the file
} ParseCtx;

static void keep_extra(SyncConf *c, const char *key, const char *value)
{
    const size_t used = strlen(c->extra);
    const size_t need = strlen(key) + 1 + strlen(value) + 1;
    if (used + need + 1 > sizeof(c->extra)) {
        c->extra_lost = true;   // too many: the file is not written again
        return;
    }
    snprintf(c->extra + used, sizeof(c->extra) - used, "%s=%s\n", key, value);
}

static void str_field(char *dst, size_t cap, const char *value)
{
    if (strlen(value) < cap) copy(dst, cap, value);
}

static void on_kv(void *vctx, const char *key, const char *value)
{
    ParseCtx *p = (ParseCtx *)vctx;
    SyncConf *c = p->c;
    uint32_t u = 0;
    bool b = false;
    p->read++;
    if (!strcmp(key, "schema")) {
        if (bl_parse_uint(value, 1, 0x7FFFFFFF, &u)) c->schema = (int)u;
    } else if (!strcmp(key, "enabled")) {
        if (bl_parse_bool(value, &b)) c->enabled = b;
    } else if (!strcmp(key, "host")) {
        str_field(c->host, sizeof(c->host), value);
    } else if (!strcmp(key, "port")) {
        if (bl_parse_uint(value, 1, 65535, &u)) c->port = (uint16_t)u;
    } else if (!strcmp(key, "tls")) {
        if (bl_parse_bool(value, &b)) c->tls = b;
    } else if (!strcmp(key, "ca_file")) {
        str_field(c->ca_file, sizeof(c->ca_file), value);
    } else if (!strcmp(key, "mqtt_version")) {
        if (!strcmp(value, "auto")) c->mqtt_version = SyncMqtt_Auto;
        else if (!strcmp(value, "5") || !strcmp(value, "5.0")) c->mqtt_version = SyncMqtt_V5;
        else if (!strcmp(value, "3.1.1") || !strcmp(value, "4")) c->mqtt_version = SyncMqtt_V311;
    } else if (!strcmp(key, "username")) {
        str_field(c->username, sizeof(c->username), value);
    } else if (!strcmp(key, "password")) {
        str_field(c->password, sizeof(c->password), value);
    } else if (!strcmp(key, "allow_anonymous")) {
        if (bl_parse_bool(value, &b)) c->allow_anonymous = b;
    } else if (!strcmp(key, "console_id")) {
        if (sync_conf_id_valid(value)) copy(c->console_id, sizeof(c->console_id), value);
    } else if (!strcmp(key, "console_name")) {
        str_field(c->console_name, sizeof(c->console_name), value);
    } else if (!strcmp(key, "policy")) {
        if (!strcmp(value, "ask")) c->policy = SyncPolicy_Ask;
        else if (!strcmp(value, "auto")) c->policy = SyncPolicy_Auto;
        else if (!strcmp(value, "off")) c->policy = SyncPolicy_Off;
    } else if (!strcmp(key, "remote_timer_writes")) {
        if (bl_parse_bool(value, &b)) c->remote_timer_writes = b;
    } else if (!strcmp(key, "publish_report")) {
        if (bl_parse_bool(value, &b)) c->publish_report = b;
    } else if (!strcmp(key, "publish_activity")) {
        if (bl_parse_bool(value, &b)) c->publish_activity = b;
    } else if (!strcmp(key, "publish_games")) {
        if (bl_parse_bool(value, &b)) c->publish_games = b;
    } else if (!strcmp(key, "publish_icons")) {
        if (bl_parse_bool(value, &b)) c->publish_icons = b;
    } else if (!strcmp(key, "publish_console")) {
        if (bl_parse_bool(value, &b)) c->publish_console = b;
    } else if (!strcmp(key, "publish_network")) {
        if (bl_parse_bool(value, &b)) c->publish_network = b;
    } else if (!strcmp(key, "publish_hardware")) {
        if (bl_parse_bool(value, &b)) c->publish_hardware = b;
    } else if (!strcmp(key, "publish_serial")) {
        if (bl_parse_bool(value, &b)) c->publish_serial = b;
    } else if (!strcmp(key, "ha_discovery")) {
        if (bl_parse_bool(value, &b)) c->ha_discovery = b;
    } else if (!strcmp(key, "poll_s")) {
        if (bl_parse_uint(value, 10, 300, &u)) c->poll_s = (uint16_t)u;
    } else if (!strcmp(key, "topic_prefix")) {
        if (strlen(value) < sizeof(c->topic_prefix) && prefix_valid(value)) {
            copy(c->topic_prefix, sizeof(c->topic_prefix), value);
            p->prefix_read = true;
        }
    } else if (!strcmp(key, "legacy_prefix")) {
        if (strlen(value) < sizeof(c->legacy_prefix) && prefix_valid(value))
            copy(c->legacy_prefix, sizeof(c->legacy_prefix), value);
    } else if (!strcmp(key, "discovery_prefix")) {
        if (strlen(value) < sizeof(c->discovery_prefix) && prefix_valid(value))
            copy(c->discovery_prefix, sizeof(c->discovery_prefix), value);
    } else if (!strcmp(key, "log_level")) {
        if (bl_parse_uint(value, 0, 2, &u)) c->log_level = (int)u;
    } else {
        keep_extra(c, key, value);
    }
}

bool sync_conf_parse(SyncConf *c, const char *text, size_t len)
{
    ParseCtx p = { c, 0, false };
    c->extra[0] = '\0';
    c->extra_lost = false;
    bl_kv_each(text, len, on_kv, &p);
    // Schema 1 published under <prefix>/<id>/ with "playguard" by default:
    // the default moves to "boreale", and the old tree is cleared once.
    if (p.read > 0 && c->schema == 1) {
        const char *old = p.prefix_read ? c->topic_prefix : "playguard";
        if (!c->legacy_prefix[0]) copy(c->legacy_prefix, sizeof(c->legacy_prefix), old);
        if (!strcmp(old, "playguard")) copy(c->topic_prefix, sizeof(c->topic_prefix), "boreale");
    }
    return p.read > 0 || len == 0;
}

size_t sync_conf_write(const SyncConf *c, char *out, size_t cap)
{
    const int n = snprintf(out, cap,
        "# PlayGuard remote link (MQTT / Home Assistant). Written by PlayGuard's\n"
        "# Sync screen; see docs/sync-protocol.md. Keep this file private: it\n"
        "# holds the broker password.\n"
        "schema=%d\n"
        "enabled=%d\n"
        "host=%s\n"
        "port=%u\n"
        "tls=%d\n"
        "ca_file=%s\n"
        "mqtt_version=%s\n"
        "username=%s\n"
        "password=%s\n"
        "allow_anonymous=%d\n"
        "console_id=%s\n"
        "console_name=%s\n"
        "policy=%s\n"
        "remote_timer_writes=%d\n"
        "publish_report=%d\n"
        "publish_activity=%d\n"
        "publish_games=%d\n"
        "publish_icons=%d\n"
        "publish_console=%d\n"
        "publish_network=%d\n"
        "publish_hardware=%d\n"
        "publish_serial=%d\n"
        "ha_discovery=%d\n"
        "poll_s=%u\n"
        "topic_prefix=%s\n"
        "discovery_prefix=%s\n"
        "log_level=%d\n"
        "%s%s%s"
        "%s",
        SYNC_CONF_SCHEMA, c->enabled, c->host, (unsigned)c->port, c->tls, c->ca_file,
        sync_mqtt_version_name(c->mqtt_version), c->username, c->password,
        c->allow_anonymous, c->console_id, c->console_name, sync_policy_name(c->policy), c->remote_timer_writes,
        c->publish_report, c->publish_activity, c->publish_games, c->publish_icons, c->publish_console, c->publish_network,
        c->publish_hardware, c->publish_serial, c->ha_discovery, (unsigned)c->poll_s, c->topic_prefix,
        c->discovery_prefix, c->log_level, c->legacy_prefix[0] ? "legacy_prefix=" : "", c->legacy_prefix,
        c->legacy_prefix[0] ? "\n" : "", c->extra);
    if (n < 0 || (size_t)n >= cap) return 0;
    return (size_t)n;
}

const char *sync_conf_unwritable(const SyncConf *c)
{
    if (c->schema > SYNC_CONF_SCHEMA) return "written by a newer version";
    if (c->extra_lost) return "too many unknown settings to keep";
    return NULL;
}

bool sync_conf_publishes(const SyncConf *c, SyncPublish p)
{
    switch (p) {
    case SyncPublish_Activity: return c->publish_activity;
    case SyncPublish_Games:    return c->publish_games;
    case SyncPublish_Icons:    return c->publish_icons;
    case SyncPublish_Console:  return c->publish_console;
    case SyncPublish_Network:  return c->publish_network;
    case SyncPublish_Hardware: return c->publish_hardware;
    case SyncPublish_Serial:   return c->publish_hardware && c->publish_serial;
    case SyncPublish_Report:   return c->publish_report;
    default:                   return true;
    }
}

const char *sync_publish_name(SyncPublish p)
{
    static const char *const NAMES[SyncPublish_Count] = { "controls", "activity", "games",  "icons",  "console",
                                                          "network",  "hardware", "serial", "report" };
    return p < SyncPublish_Count ? NAMES[p] : "controls";
}

const char *sync_conf_problem(const SyncConf *c)
{
    if (!c->host[0]) return "no broker host";
    if (!sync_conf_id_valid(c->console_id)) return "no console id";
    if (!c->username[0] && !c->allow_anonymous) return "no user name (anonymous brokers must be allowed explicitly)";
    // Line breaks would end up in the file: the next read would split them.
    const char *fields[] = { c->host, c->username, c->password, c->console_name, c->ca_file };
    for (size_t i = 0; i < sizeof(fields) / sizeof(fields[0]); i++)
        if (strchr(fields[i], '\n') || strchr(fields[i], '\r')) return "a setting holds a line break";
    return NULL;
}
