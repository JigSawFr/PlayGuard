// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sync_icons.hpp"

#include <borealis.hpp>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "sync/sync_net.h"
#include "sync/sync_tls.h"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/sync_files.hpp"

namespace sync_icons
{

namespace
{
using Clock = std::chrono::steady_clock;
using namespace sync_icons_logic;

constexpr int CONNECT_TIMEOUT_MS = 8000;
constexpr auto RETRY_AFTER = std::chrono::minutes(5);   // a batch that could not connect
constexpr auto EXIT_WAIT = std::chrono::seconds(4);

// One batch: what the thread needs, and what it did.
struct Job
{
    SyncConf conf{};
    std::string base, target;
    char client_id[24] = "";
    Plan plan;

    std::mutex m;
    std::condition_variable cv;
    bool abort = false, finished = false;
    // Done: (app id, bytes; 0 none to publish), and the ones cleared.
    std::vector<std::pair<uint64_t, uint32_t>> sent;
    std::vector<uint64_t> cleared;
    size_t too_large = 0;
    std::string error;   // could not connect, or the connection was lost ("" neither)
    bool resend = false;
};

struct
{
    std::vector<Played> played;
    bool played_known = false;   // a read of the play log came: an empty set is one
    Sent sent;
    bool sent_loaded = false;
    bool resend = false;         // Sync now: every icon of the set again, until a batch got through
    std::shared_ptr<Job> job;
    std::thread thread;
    Clock::time_point retry_at{};
} s;

void noop(void*, const MqttPublish*)
{
}

bool aborted(Job& j)
{
    std::lock_guard<std::mutex> lk(j.m);
    return j.abort || playstats_cancelled();
}

int connect(MqttClient& c, Job& j)
{
    MqttConnect o;
    std::memset(&o, 0, sizeof(o));
    o.client_id = j.client_id;
    o.username = j.conf.username[0] ? j.conf.username : nullptr;
    o.password = j.conf.username[0] && j.conf.password[0] ? j.conf.password : nullptr;
    o.keepalive_s = 30;
    o.clean_session = true;
    // As the session does ("auto": 5.0, else 3.1.1 when the broker refuses it).
    o.version = j.conf.mqtt_version == SyncMqtt_V311 ? MQTT_V311 : MQTT_V5;
    int rc = mqtt_client_connect(&c, j.conf.host, j.conf.port, &o, CONNECT_TIMEOUT_MS);
    if (rc < 0 && j.conf.mqtt_version == SyncMqtt_Auto && c.version_refused) {
        o.version = MQTT_V311;
        rc = mqtt_client_connect(&c, j.conf.host, j.conf.port, &o, CONNECT_TIMEOUT_MS);
    }
    return rc;
}

void publish(MqttClient& c, Job& j, const std::string& sub, const void* bytes, size_t len, bool& ok)
{
    const std::string topic = j.base + "/" + sub;
    ok = mqtt_client_publish(&c, topic.c_str(), bytes, len, true) == 0;
}

// The thread: connects, sends the new icons one read at a time, clears the
// others, disconnects.
void run(std::shared_ptr<Job> job)
{
    Job& j = *job;
    SyncTcp tcp{ -1 };
    SyncTls tls;
    std::memset(&tls, 0, sizeof(tls));
    tls.tcp.fd = -1;
    const SyncIo io = j.conf.tls ? sync_tls_io(&tls, j.conf.ca_file) : sync_tcp_io(&tcp);
    uint8_t rx[512];   // a CONNACK: nothing is subscribed
    MqttClient c;
    mqtt_client_init(&c, io, rx, sizeof(rx), sync_now_ms, noop, nullptr);
    std::vector<std::pair<uint64_t, uint32_t>> sent;
    std::vector<uint64_t> cleared;
    size_t too_large = 0;
    std::string error;
    if (connect(c, j) < 0) {
        error = c.error[0] ? c.error : "cannot reach the broker";
        mqtt_client_close(&c, false);
    } else {
        for (const uint64_t id : j.plan.send) {
            if (aborted(j) || !c.connected) break;
            PlayIcon icon{ id, nullptr, 0 };
            playstats_icons(&icon, 1);
            bool ok = true;
            const uint32_t before = c.too_large;
            const uint32_t bytes = icon.jpeg ? (uint32_t)icon.size : 0;   // 0: the game has none
            if (bytes) publish(c, j, topic(id), icon.jpeg, bytes, ok);
            std::free(icon.jpeg);
            if (ok) sent.push_back({ id, bytes });
            else if (c.connected && c.too_large != before) {
                too_large++;   // larger than the broker takes (5.0): not tried again
                sent.push_back({ id, 0 });
            }
        }
        for (const uint64_t id : j.plan.clear) {
            if (aborted(j) || !c.connected) break;
            bool ok = false;
            publish(c, j, topic(id), "", 0, ok);
            if (ok) cleared.push_back(id);
        }
        if (!c.connected) error = c.error;
        mqtt_client_close(&c, true);
    }
    std::lock_guard<std::mutex> lk(j.m);
    j.sent = std::move(sent);
    j.cleared = std::move(cleared);
    j.too_large = too_large;
    j.error = error;
    j.finished = true;
    j.cv.notify_all();
}

void load_sent()
{
    if (s.sent_loaded) return;
    s.sent_loaded = true;
    std::string text;
    if (paths::read_file(sync_files::icons_file(), text)) s.sent = parse(text);
}

// A finished batch: what it did, into icons.txt.
void collect()
{
    if (!s.job) return;
    {
        std::lock_guard<std::mutex> lk(s.job->m);
        if (!s.job->finished) return;
    }
    if (s.thread.joinable()) s.thread.join();
    Job& j = *s.job;
    for (const auto& e : j.sent) record(s.sent, j.target, e.first, e.second, false);
    for (const uint64_t id : j.cleared) record(s.sent, j.target, id, 0, true);
    if (!j.sent.empty() || !j.cleared.empty()) sync_files::write_icons(text(s.sent));
    if (!j.error.empty()) {
        s.retry_at = Clock::now() + RETRY_AFTER;
        // The session file goes into reports, which are public: not the broker's name.
        std::string error = j.error;
        const size_t n = std::strlen(j.conf.host);
        for (size_t at; n && (at = error.find(j.conf.host)) != std::string::npos;) error.replace(at, n, "<broker>");
        brls::Logger::warning("sync: game icons: {} (again in 5 min)", error);
    } else if (j.resend) {
        s.resend = false;
    }
    if (!j.sent.empty() || !j.cleared.empty() || j.error.empty())
        brls::Logger::info("sync: game icons: {} published, {} cleared{}", j.sent.size(), j.cleared.size(),
                       j.too_large ? fmt::format(", {} larger than the broker takes", j.too_large) : "");
    s.job.reset();
}
}   // namespace

void set_played(const std::vector<Played>& games)
{
    s.played = games;
    s.played_known = true;
}

void step(const SyncConf& conf, bool online)
{
    collect();
    if (s.job || !online || Clock::now() < s.retry_at) return;
    // Not before the play log was read once: its set unknown is not an empty one.
    if (conf.publish_icons && !s.played_known) return;
    load_sent();
    char id[SYNC_ID_LEN + 1];
    sync_conf_device_id(&conf, id);
    if (!id[0]) return;
    auto job = std::make_shared<Job>();
    job->conf = conf;
    job->base = std::string(conf.topic_prefix) + "/" + id;
    job->target = target_of(conf.host, conf.port, job->base);
    std::snprintf(job->client_id, sizeof(job->client_id), "pg-%s-icons", id);
    const std::vector<uint64_t> want = conf.publish_icons ? wanted(s.played) : std::vector<uint64_t>{};
    job->plan = plan(want, s.sent, job->target);
    if (s.resend) {
        job->plan.send = want;
        job->resend = true;
    }
    if (job->plan.empty()) {
        s.resend = false;
        return;
    }
    try {
        s.thread = std::thread(run, job);
    } catch (const std::system_error& e) {
        brls::Logger::error("sync: no thread for the game icons ({})", e.what());
        s.retry_at = Clock::now() + RETRY_AFTER;
        return;
    }
    s.job = job;
}

void again()
{
    s.resend = true;
    s.retry_at = {};
}

void stop()
{
    if (!s.job) return;
    std::shared_ptr<Job> job = s.job;
    std::unique_lock<std::mutex> lk(job->m);
    job->abort = true;
    const bool ended = job->cv.wait_for(lk, EXIT_WAIT, [&]() { return job->finished; });
    lk.unlock();
    if (s.thread.joinable()) {
        if (ended) {
            s.thread.join();
        } else {
            brls::Logger::warning("sync: the game icons' thread did not end in time; left running");
            (void)new std::thread(std::move(s.thread));
        }
    }
    s.job.reset();
}

}   // namespace sync_icons
