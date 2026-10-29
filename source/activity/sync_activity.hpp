// SyncActivity — Console › Remote access: the remote link's settings
// (sync.conf: the MQTT broker, the console's name, what orders from Home
// Assistant may do), its live status, "Sync now" and its log. Every change
// of a setting goes through the PIN check of a console change (Security ›
// Protect PlayGuard with the PIN): the link can change the console.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <memory>
#include <string>
#include <vector>

#include <aurore/cadran/ui/guarded.hpp>

#include "sync/sync_conf.h"

class SyncActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/sync.xml");

    void onContentAvailable() override;
    void onResume() override;
    ~SyncActivity() override;

  private:
    SyncConf conf{};
    // False once the screen is gone: a save finishing after it touches nothing
    // (ui::guarded).
    ui::Alive alive = ui::make_alive();
    aurore::RepeatingTimer status_timer;
    std::string status_line;   // what the status cell says

    void refresh();
    void refresh_status();
    void refresh_ha_install();   // the integration's cell: installed, newer release
    // After the PIN check: shows `next`, saves sync.conf off the UI thread
    // (ui::write_later), then applies it. False when refused (the settings on
    // screen go back); a save that fails says so and shows the file again.
    bool commit(SyncConf next);
    // What is published: each switch, its title and its setting.
    struct Published {
        aurore::BooleanCell* cell;
        const char* title;
        bool SyncConf::*flag;
    };
    std::vector<Published> published();

    AURORE_BIND(aurore::Label,       intro,     "sy_intro");
    AURORE_BIND(aurore::BooleanCell, enabled,   "sy_enabled");
    AURORE_BIND(aurore::DetailCell,  host,      "sy_host");
    AURORE_BIND(aurore::DetailCell,  port,      "sy_port");
    AURORE_BIND(aurore::BooleanCell, tls,       "sy_tls");
    AURORE_BIND(aurore::DetailCell,  mqtt_version, "sy_mqtt_version");
    AURORE_BIND(aurore::DetailCell,  user,      "sy_user");
    AURORE_BIND(aurore::DetailCell,  password,  "sy_password");
    AURORE_BIND(aurore::DetailCell,  name,      "sy_name");
    AURORE_BIND(aurore::DetailCell,  policy,    "sy_policy");
    AURORE_BIND(aurore::BooleanCell, timer,     "sy_timer");
    AURORE_BIND(aurore::DetailCell,  ha_install, "sy_ha_install");
    AURORE_BIND(aurore::BooleanCell, discovery, "sy_discovery");
    AURORE_BIND(aurore::Label,       published_note, "sy_published_note");
    AURORE_BIND(aurore::BooleanCell, pub_activity, "sy_pub_activity");
    AURORE_BIND(aurore::BooleanCell, pub_games, "sy_pub_games");
    AURORE_BIND(aurore::BooleanCell, pub_icons, "sy_pub_icons");
    AURORE_BIND(aurore::BooleanCell, pub_console, "sy_pub_console");
    AURORE_BIND(aurore::BooleanCell, pub_network, "sy_pub_network");
    AURORE_BIND(aurore::BooleanCell, pub_hardware, "sy_pub_hardware");
    AURORE_BIND(aurore::BooleanCell, serial,    "sy_serial");
    AURORE_BIND(aurore::BooleanCell, report,    "sy_report");
    AURORE_BIND(aurore::DetailCell,  status,    "sy_status");
    AURORE_BIND(aurore::DetailCell,  sync_now,  "sy_sync_now");
    AURORE_BIND(aurore::DetailCell,  log,       "sy_log");
    AURORE_BIND(aurore::DetailCell,  id,        "sy_id");
    AURORE_BIND(aurore::Label,       note,      "sy_note");
};
