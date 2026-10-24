// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "sync_discovery.h"

#include <stdio.h>
#include <string.h>

#include "sync_entities.h"
#include "sync_json.h"

#define REPO_URL "https://github.com/JigSawFr/PlayGuard"

size_t sync_topic_base(const SyncConf *conf, char *out, size_t cap)
{
    char id[SYNC_ID_LEN + 1];
    sync_conf_device_id(conf, id);
    const int n = snprintf(out, cap, "%s/%s", conf->topic_prefix, id);
    return n < 0 || (size_t)n >= cap ? 0 : (size_t)n;
}

size_t sync_discovery_topic(const SyncConf *conf, char *out, size_t cap)
{
    char id[SYNC_ID_LEN + 1];
    sync_conf_device_id(conf, id);
    const int n = snprintf(out, cap, "%s/device/playguard_%s/config", conf->discovery_prefix, id);
    return n < 0 || (size_t)n >= cap ? 0 : (size_t)n;
}

static bool announced(const SyncEntity *e, const SyncDiscovery *d)
{
    if (e->write == SyncWrite_None) return true;
    if (d->read_only) return false;
    if (e->write == SyncWrite_Timer && !d->conf->remote_timer_writes) return false;
    if (!strcmp(e->id, "profile") && d->n_profiles == 0) return false;   // nothing to choose from
    return true;
}

size_t sync_discovery_build(const SyncDiscovery *d, char *out, size_t cap)
{
    const SyncConf *c = d->conf;
    char base[96], topic[160], uid[96], id[SYNC_ID_LEN + 1], name[SYNC_DEVICE_NAME_MAX];
    if (!sync_topic_base(c, base, sizeof(base))) return 0;
    sync_conf_device_id(c, id);
    sync_conf_device_name(c, name, sizeof(name));
    const char *version = d->sw_version && *d->sw_version ? d->sw_version : "unknown";

    SyncJson j;
    sync_json_init(&j, out, cap);
    sync_json_obj(&j);

    sync_json_key(&j, "dev");
    sync_json_obj(&j);
    snprintf(uid, sizeof(uid), "playguard_%s", id);
    sync_json_key(&j, "ids");
    sync_json_arr(&j);
    sync_json_str(&j, uid);
    sync_json_arr_end(&j);
    sync_json_kstr(&j, "name", name);
    sync_json_kstr(&j, "mf", "PlayGuard");
    sync_json_kstr(&j, "mdl", "Nintendo Switch");
    sync_json_kstr(&j, "sw", version);
    sync_json_obj_end(&j);

    sync_json_key(&j, "o");
    sync_json_obj(&j);
    sync_json_kstr(&j, "name", "PlayGuard");
    sync_json_kstr(&j, "sw", version);
    sync_json_kstr(&j, "url", REPO_URL);
    sync_json_obj_end(&j);

    snprintf(topic, sizeof(topic), "%s/availability", base);
    sync_json_kstr(&j, "avty_t", topic);

    sync_json_key(&j, "cmps");
    sync_json_obj(&j);
    size_t n = 0;
    const SyncEntity *list = sync_entities(&n);
    for (size_t i = 0; i < n; i++) {
        const SyncEntity *e = &list[i];
        sync_json_key(&j, e->id);
        sync_json_obj(&j);
        sync_json_kstr(&j, "p", e->platform);
        // Not offered now (remote_timer_writes off, read-only, no profile):
        // its platform alone, which removes it from Home Assistant if an
        // earlier discovery had it, rather than leaving it there to refuse
        // every order.
        if (!announced(e, d)) {
            sync_json_obj_end(&j);
            continue;
        }
        sync_json_kstr(&j, "name", e->name);
        snprintf(uid, sizeof(uid), "playguard_%s_%s", id, e->id);
        sync_json_kstr(&j, "uniq_id", uid);
        if (!strcmp(e->platform, "event")) {
            snprintf(topic, sizeof(topic), "%s/event", base);
            sync_json_kstr(&j, "stat_t", topic);
            sync_json_key(&j, "evt_typ");
            sync_json_arr(&j);
            // The events the engine sends (sync_engine.c's event()).
            static const char *const types[] = { "command_applied", "command_rejected", "command_waiting" };
            for (size_t t = 0; t < sizeof(types) / sizeof(types[0]); t++) sync_json_str(&j, types[t]);
            sync_json_arr_end(&j);
        } else if (e->value_template) {
            snprintf(topic, sizeof(topic), "%s/%s", base, e->topic ? e->topic : "state");
            sync_json_kstr(&j, "stat_t", topic);
            sync_json_kstr(&j, "val_tpl", e->value_template);
        }
        if (e->write != SyncWrite_None) {
            snprintf(topic, sizeof(topic), "%s/%s/set", base, e->id);
            sync_json_kstr(&j, "cmd_t", topic);
            // Retained: an order sent while the console sleeps waits on the
            // broker. Optimistic: the entity keeps the value asked meanwhile.
            sync_json_kbool(&j, "ret", true);
            if (e->value_template) sync_json_kbool(&j, "opt", true);
        }
        if (!strcmp(e->id, "profile")) {
            sync_json_key(&j, "ops");
            sync_json_arr(&j);
            for (size_t p = 0; p < d->n_profiles; p++) sync_json_str(&j, d->profiles[p]);
            sync_json_arr_end(&j);
        }
        if (e->extra && *e->extra) sync_json_members(&j, e->extra);
        sync_json_obj_end(&j);
    }
    sync_json_obj_end(&j);
    sync_json_obj_end(&j);
    return sync_json_end(&j);
}
