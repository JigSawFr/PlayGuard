// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/dev_builds.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <set>

#include <borealis/extern/nlohmann/json.hpp>

#include "util/sha256.hpp"
#include "util/update.hpp"

namespace dev_builds
{

namespace
{
using nlohmann::json;

bool is_hex(const std::string& s)
{
    return std::all_of(s.begin(), s.end(), [](char c) { return std::isxdigit((unsigned char)c) != 0; });
}

std::string lower(std::string s)
{
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

std::string str(const json& j, const char* key)
{
    const auto it = j.find(key);
    return it != j.end() && it->is_string() ? it->get<std::string>() : std::string();
}

int64_t integer(const json& j, const char* key)
{
    const auto it = j.find(key);
    return it != j.end() && it->is_number_integer() ? it->get<int64_t>() : 0;
}

// A full commit hash -> its first 7 digits; empty when it is not one.
std::string short_commit(const std::string& sha)
{
    return sha.size() >= 7 && sha.size() <= 64 && is_hex(sha) ? lower(sha.substr(0, 7)) : "";
}

bool https(const std::string& url) { return url.compare(0, 8, "https://") == 0; }

// At most MAX_NOTES bytes of `text`, cut after a whole line.
std::string cut_notes(std::string text)
{
    if (text.size() > MAX_NOTES) {
        const size_t cut = text.rfind('\n', MAX_NOTES);
        text.resize(cut == std::string::npos ? 0 : cut);
    }
    return text;
}

// Seconds since 1970 of "YYYY-MM-DDTHH:MM:SS…" (UTC); false when malformed.
bool iso_seconds(const std::string& iso, int64_t* out)
{
    int y, mo, d, h, mi, se;
    if (iso.size() < 19 || std::sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &y, &mo, &d, &h, &mi, &se) != 6) return false;
    if (mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || se > 60) return false;
    // Days from civil (Howard Hinnant's algorithm).
    y -= mo <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const int64_t yoe = y - era * 400;
    const int64_t doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const int64_t days = era * 146097 + doe - 719468;
    *out = days * 86400 + h * 3600 + mi * 60 + se;
    return true;
}

// Where fetch() finds builds: a release asset on github.com, an artifact on
// the API. A list kept on the SD card can be edited: nothing else is taken.
bool github_url(const std::string& url)
{
    return url.compare(0, 19, "https://github.com/") == 0 || url.compare(0, 23, "https://api.github.com/") == 0;
}

template <typename F>
bool parse(const std::string& text, F f)
{
    try {
        return f(json::parse(text));
    } catch (const std::exception&) {
        return false;
    }
}
}   // namespace

namespace
{
// A release's playguard.nro, with its notes. False when it has none (or a
// malformed one), or is a draft; a pre-release only when `prerelease_ok`.
bool release_build(const json& r, bool prerelease_ok, Build* out)
{
    if (!r.is_object() || r.value("draft", true)) return false;
    const bool prerelease = r.value("prerelease", true);
    if (prerelease && !prerelease_ok) return false;
    if (!r.contains("assets") || !r["assets"].is_array()) return false;
    const std::string tag = str(r, "tag_name");
    for (const auto& a : r["assets"]) {
        if (!a.is_object() || str(a, "name") != "playguard.nro") continue;
        Build b;
        b.kind = Kind::Release;
        b.prerelease = prerelease;
        b.version = tag.size() > 1 && tag[0] == 'v' ? tag.substr(1) : tag;
        b.url = str(a, "browser_download_url");
        b.date = str(a, "updated_at");
        b.sha256 = digest_hex(str(a, "digest"));
        b.notes = cut_notes(str(r, "body"));
        const int64_t size = integer(a, "size");
        if (b.version.empty() || !https(b.url) || size <= 0) return false;
        b.size = (uint64_t)size;
        *out = b;
        return true;
    }
    return false;
}
}   // namespace

bool parse_release(const std::string& text, Build* out)
{
    return parse(text, [out](const json& r) { return release_build(r, false, out); });
}

bool parse_releases(const std::string& text, std::vector<Build>* out, size_t keep)
{
    return parse(text, [out, keep](const json& j) {
        if (!j.is_array()) return false;
        std::set<std::string> seen;
        for (const auto& r : j) {
            if (out->size() >= keep) break;
            Build b;
            if (!release_build(r, true, &b) || !seen.insert(b.version).second) continue;
            out->push_back(b);
        }
        return true;
    });
}

bool is_build_artifact(const std::string& name)
{
    if (name == "playguard_release") return true;
    const std::string head = "playguard-", tail = "-nro";
    return name.size() > head.size() + tail.size() && name.compare(0, head.size(), head) == 0 &&
           name.compare(name.size() - tail.size(), tail.size(), tail) == 0;
}

bool parse_artifacts(const std::string& text, std::vector<Artifact>* out, size_t* listed)
{
    return parse(text, [out, listed](const json& j) {
        if (!j.is_object() || !j.contains("artifacts") || !j["artifacts"].is_array()) return false;
        if (listed) *listed = j["artifacts"].size();
        for (const auto& a : j["artifacts"]) {
            if (!a.is_object() || !is_build_artifact(str(a, "name")) || a.value("expired", true)) continue;
            if (!a.contains("workflow_run") || !a["workflow_run"].is_object()) continue;
            const auto& run = a["workflow_run"];
            Artifact x;
            x.url = str(a, "archive_download_url");
            x.date = str(a, "created_at");
            x.sha256 = digest_hex(str(a, "digest"));
            x.expires = str(a, "expires_at");
            x.run = integer(run, "id");
            x.branch = str(run, "head_branch");
            x.commit = short_commit(str(run, "head_sha"));
            x.repo = integer(run, "repository_id");
            x.head_repo = integer(run, "head_repository_id");
            const int64_t size = integer(a, "size_in_bytes");
            if (!https(x.url) || size <= 0 || x.commit.empty() || x.branch.empty() || !x.repo || !x.head_repo) continue;
            x.size = (uint64_t)size;
            out->push_back(x);
        }
        return true;
    });
}

namespace
{
bool pull_of(const json& p, Pull* out)
{
    if (!p.is_object() || !p.contains("head") || !p["head"].is_object()) return false;
    const auto& head = p["head"];
    auto repo_id = [](const json& side) {
        return side.contains("repo") && side["repo"].is_object() ? integer(side["repo"], "id") : 0;
    };
    Pull x;
    x.number = (int)integer(p, "number");
    x.title = str(p, "title");
    x.body = cut_notes(str(p, "body"));
    x.branch = str(head, "ref");
    x.head_sha = str(head, "sha");
    // A fork that was deleted has no repository: its builds cannot be matched.
    x.head_repo = repo_id(head);
    x.repo = p.contains("base") && p["base"].is_object() ? repo_id(p["base"]) : 0;
    if (x.number <= 0 || x.branch.empty() || !x.head_repo) return false;
    *out = x;
    return true;
}

Ci ci_of(const json& r)
{
    if (str(r, "status") != "completed") return Ci::Running;
    return str(r, "conclusion") == "success" ? Ci::Passed : Ci::Failed;
}
}   // namespace

bool parse_pulls(const std::string& text, std::vector<Pull>* out)
{
    return parse(text, [out](const json& j) {
        if (!j.is_array()) return false;
        for (const auto& p : j) {
            Pull x;
            if (pull_of(p, &x)) out->push_back(x);
        }
        return true;
    });
}

bool parse_pull(const std::string& text, Pull* out)
{
    return parse(text, [out](const json& j) { return pull_of(j, out); });
}

bool parse_runs(const std::string& text, std::vector<Run>* out)
{
    return parse(text, [out](const json& j) {
        if (!j.is_object() || !j.contains("workflow_runs") || !j["workflow_runs"].is_array()) return false;
        for (const auto& r : j["workflow_runs"]) {
            if (!r.is_object()) continue;
            Run x;
            x.id = integer(r, "id");
            x.sha = str(r, "head_sha");
            x.ci = ci_of(r);
            x.repo = r.contains("repository") && r["repository"].is_object() ? integer(r["repository"], "id") : 0;
            x.head_repo = r.contains("head_repository") && r["head_repository"].is_object()
                              ? integer(r["head_repository"], "id") : 0;
            x.branch = str(r, "head_branch");
            if (x.id > 0 && short_commit(x.sha) != "") out->push_back(x);
        }
        return true;
    });
}

void apply_runs(std::vector<Build>* builds, const std::vector<Run>& runs)
{
    for (Build& b : *builds) {
        if (!b.artifact || !b.run) continue;
        for (const Run& r : runs)
            if (r.id == b.run) b.ci = r.ci;
    }
}

bool parse_commit(const std::string& text, std::string* sha)
{
    return parse(text, [sha](const json& j) {
        if (!j.is_object()) return false;
        const std::string s = str(j, "sha");
        if (s.size() != 40 || !is_hex(s)) return false;
        *sha = lower(s);
        return true;
    });
}

bool parse_query(const std::string& text, Query* out)
{
    const size_t a = text.find_first_not_of(" \t");
    const size_t b = text.find_last_not_of(" \t");
    if (a == std::string::npos) return false;
    std::string t = text.substr(a, b - a + 1);
    Query q;
    const bool hash = t[0] == '#';
    if (hash) t = t.substr(1);
    const bool digits = !t.empty() && std::all_of(t.begin(), t.end(), [](char c) { return c >= '0' && c <= '9'; });
    // A number is a pull request (up to 6 digits); 7 hex digits or more a commit.
    if (digits && t.size() <= 6) {
        q.pr = std::atoi(t.c_str());
        if (q.pr <= 0) return false;
    } else if (!hash && t.size() >= 7 && t.size() <= 40 && is_hex(t)) {
        q.commit = lower(t);
    } else {
        return false;
    }
    *out = q;
    return true;
}

bool newest_build(const std::vector<Artifact>& artifacts, Kind kind, Build* out)
{
    const Artifact* best = nullptr;
    for (const Artifact& a : artifacts)
        if (!best || a.date > best->date) best = &a;
    if (!best) return false;
    Build b;
    b.kind = kind;
    b.artifact = true;
    b.commit = best->commit;
    b.branch = best->branch;
    b.date = best->date;
    b.url = best->url;
    b.size = best->size;
    b.sha256 = best->sha256;
    b.expires = best->expires;
    b.run = best->run;
    *out = b;
    return true;
}

std::vector<Build> combine(const std::vector<Artifact>& artifacts, const std::vector<Pull>& pulls, size_t keep_main)
{
    std::vector<Artifact> sorted = artifacts;
    std::stable_sort(sorted.begin(), sorted.end(), [](const Artifact& a, const Artifact& b) { return a.date > b.date; });
    auto build = [](const Artifact& a, Kind kind) {
        Build b;
        b.kind = kind;
        b.artifact = true;
        b.commit = a.commit;
        b.date = a.date;
        b.url = a.url;
        b.size = a.size;
        b.sha256 = a.sha256;
        b.expires = a.expires;
        b.run = a.run;
        return b;
    };
    std::vector<Build> out;
    std::set<std::string> seen;
    for (const Artifact& a : sorted) {
        if (out.size() >= keep_main) break;
        // Pushed to this repository's main (a fork's "main" is a pull request's branch).
        if (a.branch == "main" && a.head_repo == a.repo && seen.insert(a.commit).second) out.push_back(build(a, Kind::Main));
    }
    std::vector<Pull> by_number = pulls;
    std::sort(by_number.begin(), by_number.end(), [](const Pull& a, const Pull& b) { return a.number > b.number; });
    for (const Pull& p : by_number) {
        // Only branches of this repository: a fork's build runs code nobody
        // with write access has reviewed yet, and installing it replaces
        // PlayGuard itself.
        for (const Artifact& a : sorted) {
            if (a.head_repo != a.repo) continue;
            if (a.branch != p.branch || a.head_repo != p.head_repo) continue;
            Build b = build(a, Kind::PullRequest);
            b.pr = p.number;
            b.title = p.title;
            b.notes = p.body;
            out.push_back(b);
            break;   // the newest only
        }
    }
    return out;
}

Order order_named(const std::string& name)
{
    return name == "number" ? Order::Number : Order::Newest;
}

const char* order_name(Order order)
{
    return order == Order::Number ? "number" : "newest";
}

std::vector<Build> of_kind(const std::vector<Build>& all, Kind kind, Order order)
{
    std::vector<Build> out;
    for (const Build& b : all)
        if (b.kind == kind) out.push_back(b);
    if (order == Order::Number && kind == Kind::PullRequest)
        std::stable_sort(out.begin(), out.end(), [](const Build& a, const Build& b) { return a.pr > b.pr; });
    else   // ISO 8601 dates in UTC sort as text
        std::stable_sort(out.begin(), out.end(), [](const Build& a, const Build& b) { return a.date > b.date; });
    return out;
}

bool parse_iso(const std::string& iso, int64_t* seconds)
{
    return iso_seconds(iso, seconds);
}

int days_between(const std::string& now_iso, const std::string& iso)
{
    int64_t a, b;
    if (!iso_seconds(now_iso, &a) || !iso_seconds(iso, &b)) return 0;
    const int64_t d = b - a;
    return (int)(d >= 0 ? d / 86400 : -((-d + 86399) / 86400));
}

bool newer(const Build& b, const std::string& running_version, const Build* running)
{
    if (b.kind == Kind::Release) return !running_version.empty() && update::compare_versions(b.version, running_version) > 0;
    return running && b.artifact && b.commit != running->commit && b.date > running->date;
}

namespace
{
const char* ci_name(Ci c)
{
    switch (c) {
        case Ci::Running: return "running";
        case Ci::Passed: return "passed";
        case Ci::Failed: return "failed";
        case Ci::Unknown: break;
    }
    return "";
}

Ci ci_named(const std::string& n)
{
    return n == "running" ? Ci::Running : n == "passed" ? Ci::Passed : n == "failed" ? Ci::Failed : Ci::Unknown;
}

const char* kind_name(Kind k)
{
    switch (k) {
        case Kind::Release: return "release";
        case Kind::Main: return "main";
        case Kind::PullRequest: return "pr";
        case Kind::Commit: return "commit";
    }
    return "";
}
}   // namespace

std::string encode_cache(const Cache& cache)
{
    json list = json::array();
    for (const Build& b : cache.builds)
        list.push_back({ { "kind", kind_name(b.kind) }, { "artifact", b.artifact }, { "version", b.version },
                         { "prerelease", b.prerelease },
                         { "pr", b.pr }, { "title", b.title }, { "commit", b.commit }, { "date", b.date },
                         { "url", b.url }, { "size", b.size }, { "sha256", b.sha256 }, { "notes", b.notes },
                         { "branch", b.branch }, { "expires", b.expires }, { "run", b.run }, { "ci", ci_name(b.ci) } });
    const json j = { { "version", 1 }, { "fetched_at", cache.fetched_at }, { "needs_login", cache.needs_login },
                     { "builds", list } };
    return j.dump(1) + "\n";
}

bool decode_cache(const std::string& text, Cache* out)
{
    return parse(text, [out](const json& j) {
        if (!j.is_object() || integer(j, "version") != 1 || !j.contains("builds") || !j["builds"].is_array()) return false;
        Cache c;
        c.fetched_at = integer(j, "fetched_at");
        c.needs_login = j.value("needs_login", true);
        for (const auto& x : j["builds"]) {
            if (!x.is_object()) continue;
            Build b;
            const std::string kind = str(x, "kind");
            if (kind == "release") b.kind = Kind::Release;
            else if (kind == "main") b.kind = Kind::Main;
            else if (kind == "pr") b.kind = Kind::PullRequest;
            else if (kind == "commit") b.kind = Kind::Commit;
            else continue;
            b.artifact = x.value("artifact", false);
            b.version = str(x, "version");
            b.prerelease = b.kind == Kind::Release && x.value("prerelease", false);
            b.pr = (int)integer(x, "pr");
            b.title = str(x, "title");
            b.commit = str(x, "commit");
            b.date = str(x, "date");
            b.url = str(x, "url");
            b.sha256 = str(x, "sha256");
            b.notes = cut_notes(str(x, "notes"));
            b.branch = str(x, "branch");
            b.expires = str(x, "expires");
            b.run = integer(x, "run");
            b.ci = ci_named(str(x, "ci"));
            const int64_t size = integer(x, "size");
            const bool commit_ok = b.kind == Kind::Release ? b.commit.empty() : short_commit(b.commit) == b.commit;
            const bool sha_ok = b.sha256.empty() || (b.sha256.size() == 64 && is_hex(b.sha256) && lower(b.sha256) == b.sha256);
            if (!github_url(b.url) || size <= 0 || !commit_ok || !sha_ok) continue;
            if (b.kind == Kind::Release ? b.version.empty() : !b.artifact) continue;
            if (b.kind == Kind::PullRequest && b.pr <= 0) continue;
            b.size = (uint64_t)size;
            c.builds.push_back(b);
        }
        *out = c;
        return true;
    });
}

bool cache_fresh(const Cache& cache, int64_t now, bool needs_login)
{
    const int64_t age = now - cache.fetched_at;
    return cache.fetched_at > 0 && age >= 0 && age < CACHE_MAX_AGE_S && cache.needs_login == needs_login &&
           !cache.builds.empty();
}

std::string digest_hex(const std::string& digest)
{
    static const std::string PREFIX = "sha256:";
    if (digest.size() != PREFIX.size() + 64 || digest.compare(0, PREFIX.size(), PREFIX) != 0) return "";
    const std::string hex = digest.substr(PREFIX.size());
    return is_hex(hex) ? lower(hex) : "";
}

bool is_nro(const std::string& head)
{
    return head.size() >= 0x14 && head.compare(0x10, 4, "NRO0") == 0;
}

bool verify_file(const std::string& path, uint64_t size, const std::string& sha256, std::string* error)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        if (error) *error = "cannot read the download";
        return false;
    }
    Sha256 hash;
    uint64_t got = 0;
    char buf[16 * 1024];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) {
        hash.update(buf, n);
        got += n;
    }
    const bool read_error = std::ferror(f) != 0;
    std::fclose(f);
    if (read_error) {
        if (error) *error = "cannot read the download";
        return false;
    }
    if (got != size) {
        if (error) *error = "size " + std::to_string(got) + " instead of " + std::to_string(size);
        return false;
    }
    if (!sha256.empty() && hash.hex() != sha256) {
        if (error) *error = "SHA-256 does not match";
        return false;
    }
    return true;
}

bool verify_nro(const std::string& path, std::string* error)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    char head[0x20] = {};
    const size_t n = f ? std::fread(head, 1, sizeof(head), f) : 0;
    if (f) std::fclose(f);
    if (!is_nro(std::string(head, n))) {
        if (error) *error = "not an NRO file";
        return false;
    }
    return true;
}

bool replace(const std::string& target, const std::string& fresh, std::string* error, const std::string& keep_as)
{
    const std::string old = target + ".old";
    std::remove(old.c_str());   // left by an earlier attempt
    // FAT has no atomic replace: move the old one aside first.
    const bool had_target = std::rename(target.c_str(), old.c_str()) == 0;
    if (!had_target && errno != ENOENT) {
        if (error) *error = "cannot move " + target + " aside";
        return false;
    }
    if (std::rename(fresh.c_str(), target.c_str()) != 0) {
        if (had_target) std::rename(old.c_str(), target.c_str());
        if (error) *error = "cannot put the new build in place";
        return false;
    }
    if (had_target && !keep_as.empty()) {
        std::remove(keep_as.c_str());
        if (std::rename(old.c_str(), keep_as.c_str()) == 0) return true;
    }
    std::remove(old.c_str());
    return true;
}

}   // namespace dev_builds
