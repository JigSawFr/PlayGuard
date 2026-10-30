// Host tests for source/util/sync_files.cpp: sync.conf saved and read back
// (unknown lines kept), the console id made once, config.json's records to
// the shared form and back (a change reported, an incomplete record left
// out), the agent's nro_state.txt (only once the link was set up, read back
// by the agent's own parser, no line break smuggled into a value), and the
// profiles and names files.
#include "check.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "util/config.hpp"
#include "util/paths.hpp"
#include "util/sync_files.hpp"

static void test_conf()
{
    CHECK(!sync_files::exists());
    SyncConf c = sync_files::load();   // no file: the defaults
    CHECK(!c.enabled && c.port == 1883 && c.policy == SyncPolicy_Ask && !c.remote_timer_writes && c.ha_discovery);

    const uint8_t random[4] = { 0xA1, 0x02, 0xC3, 0x0D };
    CHECK(sync_files::ensure_id(c, random));
    CHECK(!std::strcmp(c.console_id, "a102c30d"));
    const uint8_t other[4] = { 1, 2, 3, 4 };
    CHECK(!sync_files::ensure_id(c, other) && !std::strcmp(c.console_id, "a102c30d"));   // made once

    c.enabled = true;
    std::snprintf(c.host, sizeof(c.host), "%s", "mqtt.lan");
    std::snprintf(c.username, sizeof(c.username), "%s", "playguard");
    std::snprintf(c.password, sizeof(c.password), "%s", "p=ss word#1");
    c.policy = SyncPolicy_Auto;
    c.remote_timer_writes = true;
    std::string err;
    CHECK(sync_files::save(c, &err));
    CHECK(sync_files::exists());

    // A key a newer version wrote, added by hand: kept by the next save.
    std::string text;
    CHECK(paths::read_file(sync_files::conf_file(), text));
    CHECK(text.find("p=ss word#1") != std::string::npos);
    CHECK(paths::atomic_write(sync_files::conf_file(), text + "future_key=42\n"));

    SyncConf back = sync_files::load();
    CHECK(back.enabled && !std::strcmp(back.host, "mqtt.lan") && !std::strcmp(back.username, "playguard"));
    CHECK(!std::strcmp(back.password, "p=ss word#1") && back.policy == SyncPolicy_Auto && back.remote_timer_writes);
    CHECK(!std::strcmp(back.console_id, "a102c30d") && !sync_conf_problem(&back));
    CHECK(sync_files::save(back));
    CHECK(paths::read_file(sync_files::conf_file(), text));
    CHECK(text.find("future_key=42") != std::string::npos);

    // A newer version's file: read for what this one knows, never saved over.
    const std::string newer = "schema=2\nenabled=1\nhost=broker.lan\n";
    CHECK(paths::atomic_write(sync_files::conf_file(), newer));
    SyncConf later = sync_files::load();
    CHECK(later.enabled && !std::strcmp(later.host, "broker.lan"));
    later.enabled = false;
    err.clear();
    CHECK(!sync_files::save(later, &err) && !err.empty());
    CHECK(paths::read_file(sync_files::conf_file(), text) && text == newer);
    CHECK(sync_files::save(back));

    // One sync.conf for every system on the SD card; each system's files in
    // its own folder, and on an emuMMC a device of its own: the id and the
    // name differ, the file keeps the console's.
    CHECK(sync_files::dir() == paths::data_dir() + "/sync");
    CHECK(sync_files::system_dir() == paths::nand_dir() + "/sync" && !back.emummc);
    paths::set_nand("emummc");
    SyncConf emu = sync_files::load();
    CHECK(emu.emummc && !std::strcmp(emu.console_id, "a102c30d"));
    char id[SYNC_ID_LEN + 1];
    sync_conf_device_id(&emu, id);
    CHECK(!std::strcmp(id, "2102c30d"));
    CHECK(sync_files::save(emu));
    CHECK(paths::read_file(sync_files::conf_file(), text) && text.find("console_id=a102c30d\n") != std::string::npos);
    paths::set_nand("unknown");
    CHECK(!sync_files::load().emummc);
}

static void test_records()
{
    config::Config c;
    c.extra_weekday = 3;
    c.extra_date = "2026-10-07";
    c.extra_base = 60;
    c.extra_value = 90;
    c.console_lock = true;
    c.console_lock_prev = { 60, 60, 60, 60, 60, 120, 65535 };
    c.relock_pending = true;

    SyncRecords r = sync_files::records_from(c);
    CHECK(r.extra_weekday == 3 && !std::strcmp(r.extra_date, "2026-10-07") && r.extra_base == 60 && r.extra_value == 90);
    CHECK(r.console_lock && r.console_lock_prev_ok && r.console_lock_prev[5] == 120 && r.console_lock_prev[6] == 65535);
    CHECK(r.relock_pending);

    config::Config same = c;
    CHECK(!sync_files::records_into(r, same));   // nothing changed

    r.extra_weekday = -1;
    r.console_lock = false;
    r.console_lock_prev_ok = false;
    r.relock_pending = false;
    CHECK(sync_files::records_into(r, c));
    CHECK(c.extra_weekday == -1 && c.extra_date.empty() && !c.console_lock && c.console_lock_prev.empty() &&
           !c.relock_pending);

    // An incomplete record is not one.
    config::Config broken;
    broken.extra_weekday = 2;
    broken.extra_date = "2026-10";
    broken.console_lock_prev = { 1, 2, 3 };
    r = sync_files::records_from(broken);
    CHECK(r.extra_weekday == -1 && !r.console_lock_prev_ok);
}

static void kv(void* ctx, const char* key, const char* value)
{
    auto* out = (std::string*)ctx;
    *out += std::string(key) + "=" + value + ";";
}

static void test_nro_state()
{
    config::Config c;
    c.extra_weekday = 1;
    c.extra_date = "2026-10-05";
    c.extra_base = 30;
    c.extra_value = 0;
    c.extra_auto_restore = true;
    c.fw_gate_fw = "24.0.0";
    c.fw_gate_app = "1.0.0";
    c.fw_gate_choice = "read_only\nrelock_pending=1";   // a line break would add a key
    c.pin_lock = "changes";
    // Read-only until PlayGuard says otherwise, and with the firmware it runs on.
    CHECK(sync_files::nro_state_text(c).find("read_only=1\n") != std::string::npos);
    sync_files::set_console("23.0.1", false);
    const std::string text = sync_files::nro_state_text(c);
    CHECK(text.find("read_only=0\n") != std::string::npos && text.find("firmware=23.0.1\n") != std::string::npos);
    sync_files::set_console("", false);   // no firmware known: never writable
    CHECK(sync_files::nro_state_text(c).find("read_only=1\n") != std::string::npos);
    sync_files::set_console("23.0.1", false);
    // The serial number for the agent only while PlayGuard hands it one.
    CHECK(text.find("serial=") == std::string::npos);
    sync_files::set_serial("XAW10012345678\nread_only=0");   // a line break would add a key
    CHECK(sync_files::nro_state_text(c).find("serial=XAW10012345678 read_only=0\n") != std::string::npos);
    sync_files::set_serial("XAW10012345678");
    CHECK(sync_files::nro_state_text(c).find("serial=XAW10012345678\n") != std::string::npos);
    sync_files::set_serial("");
    CHECK(sync_files::nro_state_text(c).find("serial=") == std::string::npos);

    SyncRecords r;
    sync_records_clear(&r);
    sync_records_parse(&r, text.data(), text.size());
    CHECK(r.extra_weekday == 1 && !std::strcmp(r.extra_date, "2026-10-05") && r.extra_base == 30 && r.extra_value == 0);
    CHECK(!r.relock_pending);   // the smuggled line did not become a key
    std::string keys;
    bl_kv_each(text.data(), text.size(), kv, &keys);
    CHECK(keys.find("extra_auto_restore=1;") != std::string::npos);
    CHECK(keys.find("fw_gate_fw=24.0.0;") != std::string::npos);
    CHECK(keys.find("fw_gate_choice=read_only relock_pending=1;") != std::string::npos);
    CHECK(keys.find("pin_lock=changes;") != std::string::npos);

    // Written only while sync.conf exists.
    const std::string file = sync_files::system_dir() + "/nro_state.txt";
    std::remove(file.c_str());
    std::remove(sync_files::conf_file().c_str());
    sync_files::export_nro_state();
    std::string read;
    CHECK(!paths::read_file(file, read));
    SyncConf conf = sync_files::load();
    CHECK(sync_files::save(conf));
    config::get() = c;
    sync_files::export_nro_state();
    CHECK(paths::read_file(file, read) && read == text);
}

static void test_lists()
{
    profiles::Profile school;
    school.name = "Semaine d'école";
    school.days = { 120, 60, 60, 60, 60, 60, 65535 };
    profiles::Profile empty;   // no name: left out
    const std::string p = sync_files::profiles_text({ school, empty });
    CHECK(p.find("120,60,60,60,60,60,65535=Semaine d'école\n") != std::string::npos);
    CHECK(p.find("\n=") == std::string::npos);

    const std::string n = sync_files::names_text({ { 0x0100000000010000ULL, "Super Mario Odyssey" },
                                                   { 0x01000A10041EA000ULL, "Line\nbreak" },
                                                   { 0x0100000000000001ULL, "" } });
    CHECK(n.find("0100000000010000=Super Mario Odyssey\n") != std::string::npos);
    CHECK(n.find("01000A10041EA000=Line break\n") != std::string::npos);
    CHECK(n.find("0100000000000001") == std::string::npos);

    sync_files::write_profiles({ school });
    sync_files::write_names({ { 0x0100000000010000ULL, "Super Mario Odyssey" } });
    std::string read;
    CHECK(paths::read_file(sync_files::dir() + "/profiles.txt", read) && read.find("=Semaine d'école") != std::string::npos);
    CHECK(paths::read_file(sync_files::system_dir() + "/names.txt", read) && read.find("Super Mario Odyssey") != std::string::npos);
}

// The Home Assistant integration's document: its two versions, kept as it
// came, removed when cleared.
static void test_integration()
{
    auto i = sync_files::parse_integration(R"({"schema":1,"version":"0.2.0","homeassistant":"2026.10.1"})");
    CHECK(i.version == "0.2.0" && i.homeassistant == "2026.10.1");
    i = sync_files::parse_integration(R"({"version":"1.0.0-beta.1","extra":[1,2]})");
    CHECK(i.version == "1.0.0-beta.1" && i.homeassistant.empty());   // no schema: 1; a field more skipped
    i = sync_files::parse_integration(R"({"schema":2,"version":"1.0.0","homeassistant":"2026.10.1"})");
    CHECK(i.version.empty() && i.homeassistant.empty());   // a newer schema: not read
    for (const char* bad : { "", "not json", "[1]", R"({"version":5})", R"({"version":"v1.0"})",
                             R"({"version":"1.0 beta"})", R"({"version":"1.0.0;rm"})",
                             R"({"version":"1234567890123456789012345"})" }) {
        i = sync_files::parse_integration(bad);
        CHECK(i.version.empty() && i.homeassistant.empty());
    }

    const std::string doc = R"({"schema":1,"version":"0.2.0"})";
    sync_files::write_integration(doc);
    std::string read;
    CHECK(sync_files::integration_file() == sync_files::system_dir() + "/integration.json");
    CHECK(paths::read_file(sync_files::integration_file(), read) && read == doc);
    sync_files::write_integration("");
    CHECK(!paths::read_file(sync_files::integration_file(), read));
    sync_files::write_integration("");   // nothing to remove: fine
}

void test_agent_events()
{
    const std::string text =
        "1760000000\tlimit_uniform\t90\t1\tnone\t1\t7\tremote\t60,60,60,60,60,60,60\t90,90,90,90,90,90,90\t-1\n"
        "1760000100\tconsole_lock\tON\t1\tnone\t1\t7\tremote_console_lock\t90,90,90,90,90,90,90\t90,90,90,0,90,90,90\t1\r\n"
        "1760000200\tunlock\tPRESS\t1\tnone\t7\t0\t\t\t\n"            // older line: no console-lock field
        "1760000300\tlimit_uniform\t90\t1\tnone\t1\t7\tremote\t60,60\t90,90,90,90,90,90,90\t-1\n"   // too few
        "garbage\n"
        "1760000400\t\t90\t1\tnone\t1\t0\tremote\t\t\t-1\n"   // no entity
        "\n";
    const auto ev = sync_files::parse_agent_events(text);
    CHECK(ev.size() == 3);
    CHECK(ev[0].ts == 1760000000ULL && ev[0].entity == "limit_uniform" && ev[0].payload == "90" && ev[0].applied);
    CHECK(ev[0].change == 1 && ev[0].before.size() == 7 && ev[0].after[6] == 90 && ev[0].console_lock_after == -1);
    CHECK(ev[1].source == "remote_console_lock" && ev[1].after[3] == 0 && ev[1].console_lock_after == 1);
    CHECK(ev[2].entity == "unlock" && ev[2].before.empty() && ev[2].source == "remote" && ev[2].console_lock_after == -1);

    // A newer agent's line, with a field this version does not know: the
    // known ones are read. The lines that do not parse are handed back.
    std::string rejected;
    const auto newer = sync_files::parse_agent_events(
        "1760000500\tlimit_uniform\t75\t1\tnone\t1\t7\tremote\t90,90,90,90,90,90,90\t75,75,75,75,75,75,75\t-1\tfuture\n"
        "1760000600\tlimit_uniform\t75\n",
        &rejected);
    CHECK(newer.size() == 1 && newer[0].ts == 1760000500ULL && newer[0].after[0] == 75 && newer[0].console_lock_after == -1);
    CHECK(rejected == "1760000600\tlimit_uniform\t75\n");
    rejected.clear();
    CHECK(sync_files::parse_agent_events(text, &rejected).size() == 3);
    CHECK(std::count(rejected.begin(), rejected.end(), '\n') == 3);   // too few values, garbage, no entity

    // Whose records are newer: the agent's only once it wrote some, and not
    // before PlayGuard's last save.
    const std::string agent_state = sync_files::system_dir() + "/agent_state.txt", nro = sync_files::system_dir() + "/nro_state.txt";
    std::remove(agent_state.c_str());
    CHECK(!sync_files::agent_records_newer());
    CHECK(paths::atomic_write(agent_state, "schema=1\n"));
    std::remove(nro.c_str());
    CHECK(sync_files::agent_records_newer());
    CHECK(paths::atomic_write(nro, "schema=1\n"));
    struct timeval old_times[2] = { { 1000000000, 0 }, { 1000000000, 0 } };
    CHECK(utimes(agent_state.c_str(), old_times) == 0);
    CHECK(!sync_files::agent_records_newer());
    CHECK(utimes(nro.c_str(), old_times) == 0);
    CHECK(sync_files::agent_records_newer());

    // Taken once: renamed, read, removed; the lines it could not read kept
    // aside in agent_events.log.rejected (added to, never replaced).
    CHECK(paths::ensure_dir(sync_files::system_dir()));
    const std::string aside = sync_files::system_dir() + "/agent_events.log.rejected";
    std::remove(aside.c_str());
    CHECK(paths::atomic_write(sync_files::system_dir() + "/agent_events.log", text));
    CHECK(sync_files::take_agent_events().size() == 3);
    CHECK(sync_files::take_agent_events().empty());
    struct stat st;
    CHECK(stat((sync_files::system_dir() + "/agent_events.log").c_str(), &st) != 0);
    CHECK(stat((sync_files::system_dir() + "/agent_events.log.read").c_str(), &st) != 0);
    std::string kept;
    CHECK(paths::read_file(aside, kept) && kept.find("garbage\n") != std::string::npos);
    CHECK(paths::atomic_write(sync_files::system_dir() + "/agent_events.log", "more garbage\n"));
    CHECK(sync_files::take_agent_events().empty());
    CHECK(paths::read_file(aside, kept) && kept.find("garbage\n") != std::string::npos && kept.ends_with("more garbage\n"));
    std::remove(aside.c_str());

    // The icons the agent published: taken the same way; icons.txt next to it.
    CHECK(sync_files::take_agent_icons().empty());
    CHECK(paths::atomic_write(sync_files::system_dir() + "/agent_icons.txt", "target=0a1b2c3d\n0100000000010000=5\n"));
    CHECK(sync_files::take_agent_icons() == "target=0a1b2c3d\n0100000000010000=5\n");
    CHECK(sync_files::take_agent_icons().empty());
    CHECK(stat((sync_files::system_dir() + "/agent_icons.txt.read").c_str(), &st) != 0);
    sync_files::write_icons("target=0a1b2c3d\n");
    CHECK(sync_files::icons_file() == sync_files::system_dir() + "/icons.txt");
    std::string written;
    CHECK(paths::read_file(sync_files::icons_file(), written) && written == "target=0a1b2c3d\n");
}

int main()
{
    char dir[] = "/tmp/playguard_sync_files_XXXXXX";
    REQUIRE(mkdtemp(dir) != nullptr);
    REQUIRE(chdir(dir) == 0);   // paths::data_dir() is ./playguard_data on the host
    REQUIRE(paths::ensure_dir(paths::data_dir()));

    test_conf();
    test_records();
    test_nro_state();
    test_lists();
    test_integration();
    test_agent_events();

    const std::string cleanup = std::string("rm -rf '") + dir + "'";
    CHECK(std::system(cleanup.c_str()) == 0);
    return CHECK_DONE("sync_files: sync.conf, records, nro_state.txt, profiles, names, the integration, agent records and events assertions passed");
}
