// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "util/config.hpp"

#include <borealis/extern/nlohmann/json.hpp>
#include <initializer_list>
#include <iterator>

#include "util/paths.hpp"

namespace config
{

static Config s_config;
static Loaded s_loaded = Loaded::Missing;

Config& get() { return s_config; }
Loaded loaded() { return s_loaded; }

namespace
{
// One field at a time: a field of the wrong type keeps its default, the
// others are still read (a hand-edited or older file loses one setting, not
// the remembered firmware choice). Each reader returns false when the field is
// there but refused, so related fields can be dropped together.
bool read_string(const nlohmann::json& j, const char* key, std::string& out)
{
    auto it = j.find(key);
    if (it == j.end()) return true;
    if (!it->is_string()) return false;
    out = it->get<std::string>();
    return true;
}

// Exactly a JSON boolean / integer: no 1 for true, no 90.5 for 90.
bool read_bool(const nlohmann::json& j, const char* key, bool& out)
{
    auto it = j.find(key);
    if (it == j.end()) return true;
    if (!it->is_boolean()) return false;
    out = it->get<bool>();
    return true;
}

// A JSON integer read as 64-bit (an unsigned one above INT64_MAX would wrap
// to a negative) and kept only within +-1000000, so no value is narrowed.
bool small_int(const nlohmann::json& v, int& out)
{
    if (!v.is_number_integer()) return false;
    if (v.is_number_unsigned() && v.get<unsigned long long>() > 1000000ULL) return false;
    const long long n = v.get<long long>();
    if (n < -1000000 || n > 1000000) return false;
    out = (int)n;
    return true;
}

bool read_int(const nlohmann::json& j, const char* key, int& out)
{
    auto it = j.find(key);
    if (it == j.end()) return true;
    return small_int(*it, out);
}

// A list of integers; one value of another type (30.5, "30") or out of range
// refuses the whole list (left empty, which sanitize() never keeps), as for
// a single value: skipping it would turn [15, 30.5, 30, 60] into a valid set.
std::vector<int> read_int_list(const nlohmann::json& list)
{
    std::vector<int> v;
    for (const auto& a : list) {
        int n;
        if (!small_int(a, n)) return {};
        v.push_back(n);
    }
    return v;
}

void clear_extra(Config& c)
{
    c.extra_weekday = -1;
    c.extra_date.clear();
    c.extra_base = c.extra_value = 0;
}

void one_of(std::string& value, std::initializer_list<const char*> allowed, const char* fallback)
{
    for (const char* a : allowed)
        if (value == a) return;
    value = fallback;
}

// "YYYY-MM-DD", digits where they belong.
bool is_date(const std::string& s)
{
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (int i : { 0, 1, 2, 3, 5, 6, 8, 9 })
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}
}   // namespace

void sanitize(Config& c)
{
    bool language_ok = false;
    for (const char* l : LANGUAGES) language_ok |= c.language == l;
    if (!language_ok) c.language = "system";
    one_of(c.theme, { "system", "light", "dark" }, "system");
    one_of(c.update_via, { "", "inplace", "auto", "sphaira", "appstore", "manual" }, "");
    one_of(c.pin_lock, { "off", "changes", "open" }, "changes");
    one_of(c.dev_builds_order, { "newest", "number" }, "newest");
    one_of(c.fw_gate_choice, { "", "read_only", "probe", "risk" }, "");
    if (c.fw_gate_choice.empty()) c.fw_gate_fw.clear(), c.fw_gate_app.clear();
    if (c.ntp_server.size() > MAX_HOST) c.ntp_server.clear();
    bool tab_ok = false;
    for (const char* t : START_TABS) tab_ok |= c.start_tab == t;
    if (!tab_ok) c.start_tab = "dashboard";
    bool set_ok = false;
    for (const auto& set : EXTRA_SETS)
        set_ok |= c.extra_amounts == std::vector<int>(std::begin(set), std::end(set));
    if (!set_ok) c.extra_amounts = { 15, 30, 60 };
    if (c.activity_period < 0 || c.activity_period > 2) c.activity_period = 1;
    if (c.export_format < 0 || c.export_format > 3) c.export_format = 0;
    bool keep_ok = false;
    for (int k : BACKUP_KEEP) keep_ok |= c.backup_keep == k;
    if (!keep_ok) c.backup_keep = 0;
    keep_ok = false;
    for (int k : LOGS_KEEP) keep_ok |= c.logs_keep == k;
    if (!keep_ok) c.logs_keep = 20;
    keep_ok = false;
    for (int k : LOGS_KEEP_DAYS) keep_ok |= c.logs_keep_days == k;
    if (!keep_ok) c.logs_keep_days = 0;
    keep_ok = false;
    for (int k : LOGS_KEEP_MB) keep_ok |= c.logs_keep_mb == k;
    if (!keep_ok) c.logs_keep_mb = 0;
    if (!is_date(c.update_checked)) c.update_checked.clear();
    if (c.update_latest.size() > 32) c.update_latest.clear();
    if (c.sys_latest.size() > 96) c.sys_latest.clear();
    if (!is_date(c.sys_checked)) c.sys_checked.clear();
    if (c.update_snooze_version.size() > 32 || !is_date(c.update_snooze_until))
        c.update_snooze_version.clear(), c.update_snooze_until.clear();
    if (!is_date(c.support_reminded)) c.support_reminded.clear();
    if (c.seen_version.size() > 32) c.seen_version.clear();
    if (c.crash_seen.size() > 64) c.crash_seen.clear();
    if (!is_date(c.summary_seen)) c.summary_seen.clear();
    if (!is_date(c.console_lock_since)) c.console_lock_since.clear();
    {
        std::vector<std::string> read;
        for (const auto& s : c.notices_read)
            if (!s.empty() && s.size() <= 96) read.push_back(s);
        if (read.size() > 48) read.erase(read.begin(), read.end() - 48);
        c.notices_read = read;
    }

    std::vector<std::string> servers;
    for (const auto& s : c.custom_servers)
        if (!s.empty() && s.size() <= MAX_HOST && servers.size() < MAX_CUSTOM_SERVERS) servers.push_back(s);
    c.custom_servers = servers;

    // A pending extra-time record must describe a real weekday limit, or it
    // could later "put back" a nonsense value (or "no limit").
    const bool weekday_ok = c.extra_weekday >= 0 && c.extra_weekday <= 6;
    // extra_base may be "no limit" (0xFFFF): No more play today on a day
    // that had no limit puts "no limit" back.
    const bool base_ok    = (c.extra_base >= 0 && c.extra_base <= 1440) || c.extra_base == 0xFFFF;
    const bool values_ok  = base_ok && c.extra_value >= 0 && c.extra_value <= 1440 && c.extra_date.size() == 10;
    if (!weekday_ok || !values_ok) clear_extra(c);

    // The limits console lock saved to put back: exactly seven, each a real
    // limit (0..1440 min) or "no limit" (0xFFFF). Otherwise drop them (turning
    // the lock off then clears the limit rather than restoring nonsense).
    bool prev_ok = c.console_lock_prev.size() == 7;
    for (int v : c.console_lock_prev)
        prev_ok = prev_ok && ((v >= 0 && v <= 1440) || v == 0xFFFF);
    if (!prev_ok) c.console_lock_prev.clear();
}

// What belongs to the system PlayGuard runs on (state.json, util/nand.hpp):
// the console lock, the extra-time record, a pending relock and the
// firmware choice. Read from state.json, or from config.json when this
// system has no state.json yet (the versions before it kept them there).
static void clear_system(Config& c)
{
    c.console_lock = false;
    c.console_lock_prev.clear();
    c.console_lock_since.clear();
    c.relock_pending = false;
    c.fw_gate_fw.clear(), c.fw_gate_app.clear(), c.fw_gate_choice.clear();
    clear_extra(c);
}

static void read_system(const nlohmann::json& j, Config& c)
{
    read_bool(j, "console_lock", c.console_lock);
    read_string(j, "console_lock_since", c.console_lock_since);
    auto prev = j.find("console_lock_prev");
    if (prev != j.end() && prev->is_array())
        c.console_lock_prev = read_int_list(*prev);   // sanitize() keeps it only when it is seven valid limits
    read_bool(j, "relock_pending", c.relock_pending);

    // The firmware choice and the extra-time record are all-or-nothing. Every
    // field is read (&=, no short circuit; clang warns about a bool `a & b`).
    bool gate_ok = read_string(j, "fw_gate_fw", c.fw_gate_fw);
    gate_ok &= read_string(j, "fw_gate_app", c.fw_gate_app);
    gate_ok &= read_string(j, "fw_gate_choice", c.fw_gate_choice);
    if (!gate_ok) c.fw_gate_fw.clear(), c.fw_gate_app.clear(), c.fw_gate_choice.clear();
    bool extra_ok = read_int(j, "extra_weekday", c.extra_weekday);
    extra_ok &= read_string(j, "extra_date", c.extra_date);
    extra_ok &= read_int(j, "extra_base", c.extra_base);
    extra_ok &= read_int(j, "extra_value", c.extra_value);
    if (!extra_ok) clear_extra(c);
}

// The file as JSON, or null when it is not a JSON object.
static nlohmann::json parse_object(const std::string& text)
{
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(text);
    } catch (...) {
        return nullptr;   // not JSON at all
    }
    return j.is_object() ? j : nlohmann::json(nullptr);
}

// There but not readable as settings: defaults, and the file is put aside
// before anything (support_flow, fw_gate save by themselves at start-up)
// writes the defaults over it.
static void damaged()
{
    s_config = Config{};
    s_loaded = paths::put_aside(paths::config_file()) ? Loaded::PutAside : Loaded::Stuck;
}

static void load_shared()
{
    std::string text;
    if (!paths::read_file(paths::config_file(), text)) {
        if (paths::exists_or_pending(paths::config_file())) damaged();   // missing: a first run, silently
        return;
    }
    const nlohmann::json j = parse_object(text);
    if (j.is_null()) {
        damaged();
        return;
    }
    s_loaded = Loaded::Read;

    Config& c = s_config;
    read_string(j, "language", c.language);
    read_string(j, "theme", c.theme);
    read_string(j, "ntp_server", c.ntp_server);
    read_bool(j, "advanced", c.advanced);
    read_bool(j, "auto_relock", c.auto_relock);
    read_bool(j, "extra_auto_restore", c.extra_auto_restore);
    read_bool(j, "dev_mode", c.dev_mode);
    read_bool(j, "pt_log", c.pt_log);
    read_string(j, "update_via", c.update_via);
    read_bool(j, "update_daily", c.update_daily);
    read_string(j, "update_checked", c.update_checked);
    read_string(j, "update_latest", c.update_latest);
    read_string(j, "sys_latest", c.sys_latest);
    read_string(j, "sys_checked", c.sys_checked);
    read_string(j, "update_snooze_version", c.update_snooze_version);
    read_string(j, "update_snooze_until", c.update_snooze_until);
    read_string(j, "start_tab", c.start_tab);
    read_int(j, "activity_period", c.activity_period);
    read_int(j, "export_format", c.export_format);
    read_int(j, "backup_keep", c.backup_keep);
    read_int(j, "logs_keep", c.logs_keep);
    read_int(j, "logs_keep_days", c.logs_keep_days);
    read_int(j, "logs_keep_mb", c.logs_keep_mb);
    read_bool(j, "clock_check_at_start", c.clock_check_at_start);
    read_bool(j, "intro", c.intro);
    read_string(j, "dev_builds_order", c.dev_builds_order);
    read_string(j, "pin_lock", c.pin_lock);
    read_bool(j, "onboarding_at_start", c.onboarding_at_start);
    read_bool(j, "support_reminder", c.support_reminder);
    read_string(j, "support_reminded", c.support_reminded);
    read_string(j, "seen_version", c.seen_version);
    read_string(j, "crash_seen", c.crash_seen);
    read_bool(j, "notices_system", c.notices_system);
    read_bool(j, "notices_summary", c.notices_summary);
    read_bool(j, "notices_reminders", c.notices_reminders);
    read_string(j, "summary_seen", c.summary_seen);
    auto read = j.find("notices_read");
    if (read != j.end() && read->is_array())
        for (const auto& v : *read)
            if (v.is_string()) c.notices_read.push_back(v.get<std::string>());
    auto amounts = j.find("extra_amounts");
    if (amounts != j.end() && amounts->is_array())
        c.extra_amounts = read_int_list(*amounts);   // sanitize() keeps it only when it is one of EXTRA_SETS
    auto servers = j.find("custom_servers");
    if (servers != j.end() && servers->is_array())
        for (const auto& v : *servers)
            if (v.is_string()) c.custom_servers.push_back(v.get<std::string>());
    read_system(j, c);   // the versions before state.json; replaced by it below
}

// state.json: missing keeps what config.json had (nothing, or what a version
// before state.json kept there); damaged is put aside like config.json, with
// the same notice, and this system's fields start from their defaults.
static void load_state()
{
    std::string text;
    if (!paths::read_file(paths::state_file(), text)) {
        if (!paths::exists_or_pending(paths::state_file())) return;
    } else {
        const nlohmann::json j = parse_object(text);
        if (!j.is_null()) {
            clear_system(s_config);
            read_system(j, s_config);
            return;
        }
    }
    clear_system(s_config);
    const bool aside = paths::put_aside(paths::state_file());
    if (!aside) s_loaded = Loaded::Stuck;
    else if (s_loaded != Loaded::Stuck) s_loaded = Loaded::PutAside;
}

void load()
{
    s_config = Config{};
    s_loaded = Loaded::Missing;
    load_shared();
    load_state();
    sanitize(s_config);
}

bool save()
{
    Snapshot snap;
    return snapshot(snap) && write(snap);
}

bool write(const Snapshot& snap)
{
    // Both, even when the first fails.
    const bool shared = paths::atomic_write(paths::config_file(), snap.shared);
    const bool system = paths::atomic_write(paths::state_file(), snap.system);
    return shared && system;
}

bool snapshot(Snapshot& snap)
{
    if (s_loaded == Loaded::Stuck) return false;   // it would overwrite the unreadable file
    const auto dump = [](const nlohmann::json& j) {
        return j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace) + "\n";
    };
    nlohmann::json j;
    j["schema"]          = SCHEMA;
    j["language"]        = s_config.language;
    j["theme"]           = s_config.theme;
    j["ntp_server"]      = s_config.ntp_server;
    j["custom_servers"]  = s_config.custom_servers;
    j["advanced"]        = s_config.advanced;
    j["auto_relock"]     = s_config.auto_relock;
    j["extra_auto_restore"] = s_config.extra_auto_restore;
    j["dev_mode"]        = s_config.dev_mode;
    j["pt_log"]          = s_config.pt_log;
    j["update_via"]      = s_config.update_via;
    j["update_daily"]    = s_config.update_daily;
    j["update_checked"]  = s_config.update_checked;
    j["update_latest"]   = s_config.update_latest;
    j["sys_latest"]      = s_config.sys_latest;
    j["sys_checked"]     = s_config.sys_checked;
    j["update_snooze_version"] = s_config.update_snooze_version;
    j["update_snooze_until"]   = s_config.update_snooze_until;
    j["start_tab"]       = s_config.start_tab;
    j["extra_amounts"]   = s_config.extra_amounts;
    j["activity_period"] = s_config.activity_period;
    j["export_format"]   = s_config.export_format;
    j["backup_keep"]     = s_config.backup_keep;
    j["logs_keep"]       = s_config.logs_keep;
    j["logs_keep_days"]  = s_config.logs_keep_days;
    j["logs_keep_mb"]    = s_config.logs_keep_mb;
    j["clock_check_at_start"] = s_config.clock_check_at_start;
    j["intro"]           = s_config.intro;
    j["dev_builds_order"] = s_config.dev_builds_order;
    j["pin_lock"]        = s_config.pin_lock;
    j["onboarding_at_start"] = s_config.onboarding_at_start;
    j["support_reminder"] = s_config.support_reminder;
    j["support_reminded"] = s_config.support_reminded;
    j["seen_version"]    = s_config.seen_version;
    j["crash_seen"]      = s_config.crash_seen;
    j["notices_system"]    = s_config.notices_system;
    j["notices_summary"]   = s_config.notices_summary;
    j["notices_reminders"] = s_config.notices_reminders;
    j["notices_read"]      = s_config.notices_read;
    j["summary_seen"]      = s_config.summary_seen;
    snap.shared = dump(j);

    nlohmann::json st;
    st["schema"]            = SCHEMA;
    st["console_lock"]      = s_config.console_lock;
    st["console_lock_prev"] = s_config.console_lock_prev;
    st["console_lock_since"] = s_config.console_lock_since;
    st["fw_gate_fw"]        = s_config.fw_gate_fw;
    st["fw_gate_app"]       = s_config.fw_gate_app;
    st["fw_gate_choice"]    = s_config.fw_gate_choice;
    st["extra_weekday"]     = s_config.extra_weekday;
    st["extra_date"]        = s_config.extra_date;
    st["extra_base"]        = s_config.extra_base;
    st["extra_value"]       = s_config.extra_value;
    st["relock_pending"]    = s_config.relock_pending;
    snap.system = dump(st);
    return true;
}

}   // namespace config
