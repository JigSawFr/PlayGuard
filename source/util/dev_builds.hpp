// dev_builds — install another build of PlayGuard in place, from the
// developer tools: one of the last releases, one of the last commits of main, or
// the newest build of an open pull request (from this repository: a fork's
// build is never offered, nobody with write access has reviewed its code).
//
// A release is the playguard.nro of a GitHub release (pre-releases included,
// marked), readable without an account. The others are the playguard-<version>-<ref>-<commit>-nro
// artifact the build workflow keeps for every run (.github/workflows/build.yml
// names each artifact of a run after its build and its component; the
// recovery sysmodule is the -rescue one): a zip holding playguard.nro, which
// GitHub only hands to a signed-in user, hence the token of
// util/github_auth.hpp. Artifacts expire (90 days by default).
//
// The download is checked (size and the SHA-256 GitHub records; for an
// artifact, the zip's, then the CRC-32 of playguard.nro inside it; the NRO
// header) before it replaces the running .nro, which hbloader loaded whole
// into memory, so it can be overwritten once romfs:/ lets go of it
// (platform_romfs_release, dev_build_flow.cpp); then hbloader starts it.
//
// Everything but fetch() and download() is plain C++ for the host tests
// (tests/dev_builds); those two are in dev_builds_net.cpp.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>

#include <cstdint>
#include <string>
#include <vector>

namespace dev_builds
{

enum class Kind
{
    Release,       // a release (its playguard.nro)
    Main,          // a commit of main
    PullRequest,   // the newest build of an open pull request
};

struct Build
{
    Kind        kind = Kind::Main;
    bool        artifact = false;   // url, size and sha256 are those of a zip holding playguard.nro
    std::string version;   // Release: "1.2.0"
    bool        prerelease = false;   // Release: marked a pre-release on GitHub
    int         pr = 0;    // PullRequest: its number
    std::string title;     // PullRequest: its title
    std::string commit;    // Main / PullRequest: 7 hex digits
    std::string date;      // "2026-10-09T12:03:00Z", when it was built
    std::string url;       // the download (an artifact's archive_download_url needs the token)
    uint64_t    size = 0;
    std::string sha256;    // 64 hex digits; empty when GitHub gave no digest
    std::string notes;     // Release: its notes (the release's Markdown body); not kept in the cache
};

// One run's artifact, as GET /repos/…/actions/artifacts lists it.
struct Artifact
{
    std::string sha256, url, date, branch, commit;   // commit: the run's head, 7 hex digits
    uint64_t    size = 0;
    int64_t     repo = 0, head_repo = 0;   // the run's repository and the head's (a fork's for its PRs)
};

// An open pull request (GET /repos/…/pulls).
struct Pull
{
    int         number = 0;
    std::string title, branch;
    int64_t     head_repo = 0;
};

// The latest release (GET /repos/…/releases/latest): its playguard.nro and
// its notes (at most MAX_NOTES bytes, cut on a line). False for anything
// else (a pre-release, no such asset, malformed).
constexpr size_t MAX_NOTES = 16 * 1024;
bool parse_release(const std::string& json, Build* out);
// The releases (GET /repos/…/releases), newest first as listed: each one with
// a playguard.nro, pre-releases included (Build::prerelease), drafts left
// out, at most `keep`, one per version, without their notes. False when the
// answer is not a list.
constexpr size_t MAX_RELEASES = 20;
bool parse_releases(const std::string& json, std::vector<Build>* out, size_t keep = MAX_RELEASES);
// The artifact holding playguard.nro: "playguard-<version>-<ref>-<commit>-nro",
// or "playguard_release", the name the runs before that scheme gave it.
bool is_build_artifact(const std::string& name);
// The build artifacts (is_build_artifact) that have not expired, newest first
// as listed. *listed (when given) is how many artifacts the page held, of
// any name: a short page is the last one.
bool parse_artifacts(const std::string& json, std::vector<Artifact>* out, size_t* listed = nullptr);
bool parse_pulls(const std::string& json, std::vector<Pull>* out);

// The builds: main's newest `keep_main` commits (pushed to this repository's
// main), then for each open pull request from a branch of this repository
// the newest artifact of that branch (pull requests from forks are left out). Newest first within each kind; one per commit.
std::vector<Build> combine(const std::vector<Artifact>& artifacts, const std::vector<Pull>& pulls,
                           size_t keep_main = 20);

// How Developer tools › Install another build orders main's commits and the
// pull requests (config.json "dev_builds_order").
enum class Order
{
    Newest,   // the newest build first
    Number,   // pull requests by number, the highest first (main: newest first)
};
// "newest" or "number"; anything else is Newest.
Order order_named(const std::string& name);
const char* order_name(Order order);

// The builds of `kind` in `all`, in `order`; builds that tie keep the order
// they are listed in.
std::vector<Build> of_kind(const std::vector<Build>& all, Kind kind, Order order);

// "sha256:<64 hex>" (GitHub's digest) -> the 64 hex digits, lower case;
// empty when malformed.
std::string digest_hex(const std::string& digest);

// An NRO: "NRO0" at offset 0x10.
bool is_nro(const std::string& head);

// Checks a downloaded file against `size` and `sha256` (when known).
bool verify_file(const std::string& path, uint64_t size, const std::string& sha256, std::string* error);
// Checks that `path` is an NRO.
bool verify_nro(const std::string& path, std::string* error);

// Replaces `target` with `fresh` (both on the same file system): target
// moves aside to "<target>.old", fresh takes its name, then the old one is
// removed. When fresh cannot take its place, the old one is put back. A
// missing target is simply created.
bool replace(const std::string& target, const std::string& fresh, std::string* error);

// The last list fetched, kept on the SD card (cache/dev_builds.json) so the
// next opening shows it at once while it is fresh.
struct Cache
{
    std::vector<Build> builds;
    bool    needs_login = false;   // as fetch() set it: signing in or out makes the list stale
    int64_t fetched_at = 0;        // POSIX seconds
};

// A list older than this is fetched again before it is shown.
constexpr int64_t CACHE_MAX_AGE_S = 10 * 60;

std::string encode_cache(const Cache& cache);
// False for anything else (damaged, another layout); a build that does not
// look like one (a URL not on https://github.com/ or https://api.github.com/,
// a malformed commit or digest) is left out.
bool decode_cache(const std::string& json, Cache* out);
// Fresh: fetched less than CACHE_MAX_AGE_S before `now` (not in the future:
// the clock was set back), in the same sign-in state.
bool cache_fresh(const Cache& cache, int64_t now, bool needs_login);

// The builds that can be installed: the last releases, then (with a token)
// main and the pull requests. Blocking (network): run it with brls::async.
// *needs_login is set when there is no token, so only the releases are listed.
// On the desktop build PLAYGUARD_SIM_DEV_BUILDS ("offline", or a folder
// holding releases.json, artifacts.json and pulls.json; latest.json for
// fetch_release) replaces the network.
bool fetch(std::vector<Build>* out, bool* needs_login, std::string* error);

// The latest release alone (no account needed): what Update in place
// installs. Blocking (network); simulated like fetch().
bool fetch_release(Build* out, std::string* error);

// Downloads `b` to `path` (playguard.nro itself, out of the zip for an
// artifact) and verifies it. Blocking. On the desktop build, with
// PLAYGUARD_SIM_DEV_BUILDS set, a URL "https://<path>" is a local file.
// `progress` (when set) gets the bytes received and the total, from the
// calling thread.
bool download(const Build& b, const std::string& path, std::string* error,
              std::function<void(uint64_t done, uint64_t total)> progress = nullptr);

}   // namespace dev_builds
