// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/http.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>

#include "app.hpp"
#include "util/http_logic.hpp"

namespace http
{

static std::mutex s_lock;   // one request at a time; also guards the global init
static bool       s_ready = false;
static std::atomic<bool> s_abort{ false };   // abort_all(): the app is quitting

namespace
{
struct Sink
{
    std::string* body;             // the response in memory, or…
    size_t       max;
    bool         overflow = false;
    FILE*        file = nullptr;   // … written to this file
    size_t       written = 0;
    bool         file_error = false;
    std::function<void(uint64_t, uint64_t)> progress{};
    CURL*        curl = nullptr;
    uint64_t     base = 0;          // bytes already in the file (a download resumed)
    bool         status_seen = false;
    const std::atomic<bool>* cancel = nullptr;   // set: the transfer stops (download())
};

bool cancelled(const Sink* sink)
{
    return sink && sink->cancel && sink->cancel->load();
}

// Called by curl about once a second, even while waiting for the server.
int on_progress(void* user, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
{
    return s_abort || cancelled(static_cast<const Sink*>(user)) ? 1 : 0;   // non-zero aborts the transfer
}

size_t on_data(char* data, size_t size, size_t count, void* user)
{
    auto* sink = static_cast<Sink*>(user);
    if (s_abort || cancelled(sink)) return 0;
    const size_t n = size * count;
    // A resumed download the server answers in full (200, not 206): it
    // starts over, from an empty file.
    if (sink->file && sink->base && !sink->status_seen) {
        sink->status_seen = true;
        long status = 0;
        curl_easy_getinfo(sink->curl, CURLINFO_RESPONSE_CODE, &status);
        if (status != 206) {
            std::fflush(sink->file);
            if (::ftruncate(::fileno(sink->file), 0) != 0) {
                sink->file_error = true;
                return 0;
            }
            std::rewind(sink->file);
            sink->base = 0;
        }
    }
    const size_t have = sink->file ? sink->base + sink->written : sink->body->size();
    if (!http_logic::fits(have, n, sink->max)) {
        sink->overflow = true;
        return 0;   // aborts the transfer (CURLE_WRITE_ERROR)
    }
    if (!sink->file) {
        sink->body->append(data, n);
        return n;
    }
    if (std::fwrite(data, 1, n, sink->file) != n) {
        sink->file_error = true;
        return 0;
    }
    sink->written += n;
    if (sink->progress) {
        curl_off_t total = -1;
        curl_easy_getinfo(sink->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &total);
        sink->progress(sink->base + sink->written, total > 0 ? sink->base + (uint64_t)total : 0);
    }
    return n;
}
}   // namespace

namespace
{
// What a request adds to a plain GET.
struct Extra
{
    const std::string* post = nullptr;           // POST this body…
    const char*        content_type = nullptr;   // … as this type
    const Headers*     headers = nullptr;
    bool               follow = true;            // follow redirects (never for a POST)
    std::string*       location = nullptr;       // the redirect target, when not followed
    Sink*              to_file = nullptr;        // replaces the in-memory sink (download)
};

bool perform(const std::string& url, const Extra& x, std::string* body, std::string* error, size_t max_bytes,
             long timeout_s, long* status_out)
{
    const std::string* post = x.post;
    Sink* to_file = x.to_file;
    std::lock_guard<std::mutex> guard(s_lock);
    body->clear();
    if (status_out) *status_out = 0;
    if (s_abort) {
        if (error) *error = "PlayGuard is quitting";
        return false;
    }
    if (!s_ready) {
        // On the Switch this starts the ssl and csrng services (switch-curl's libnx backend).
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            if (error) *error = "libcurl initialisation failed";
            return false;
        }
        s_ready = true;
    }
    CURL* curl = curl_easy_init();
    if (!curl) {
        if (error) *error = "libcurl initialisation failed";
        return false;
    }

    Sink  own{ body, max_bytes };
    Sink& sink = to_file ? *to_file : own;
    sink.curl = curl;
    const std::string agent = "PlayGuard/" + app::version();
    char message[CURL_ERROR_SIZE] = "";
    struct curl_slist* headers = nullptr;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, x.follow && !post ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    // HTTPS only, redirects included (no downgrade to http:// or other schemes).
#if LIBCURL_VERSION_NUM >= 0x075500
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");
#else
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, (long)CURLPROTO_HTTPS);
#endif
    // The server's certificate and name are checked (libcurl's defaults,
    // written out so that no build or backend change can turn them off).
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, std::min(timeout_s, 30L));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_s);
    if (to_file) {   // a long transfer: give up on a stall rather than at the end
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
        if (to_file->base) curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)to_file->base);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, on_progress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &sink);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sink);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, message);
    if (post) {
        // Never followed (above): a redirect must not turn the POST into a GET elsewhere.
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post->data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)post->size());
    }
    for (const std::string& h : http_logic::request_headers(post ? x.content_type : nullptr, x.headers))
        headers = curl_slist_append(headers, h.c_str());
    if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    const CURLcode rc = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (x.location) {
        char* target = nullptr;
        curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &target);
        *x.location = target ? target : "";
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    if (status_out) *status_out = status;

    http_logic::Outcome outcome;
    outcome.status      = status;
    outcome.overflow    = sink.overflow;
    outcome.file_error  = sink.file_error;
    outcome.transfer_ok = rc == CURLE_OK;
    if (rc != CURLE_OK) outcome.transfer_error = message[0] ? message : curl_easy_strerror(rc);
    return http_logic::succeeded(outcome, error);
}
}   // namespace

bool get(const std::string& url, std::string* body, std::string* error, size_t max_bytes, long timeout_s,
         const Headers& headers, long* status)
{
    Extra x;
    x.headers = &headers;
    return perform(url, x, body, error, max_bytes, timeout_s, status);
}

bool post(const std::string& url, const std::string& data, const char* content_type, std::string* body,
          std::string* error, long* status, size_t max_bytes, long timeout_s, const Headers& headers)
{
    Extra x;
    x.post = &data;
    x.content_type = content_type;
    x.headers = &headers;
    return perform(url, x, body, error, max_bytes, timeout_s, status);
}

bool redirect(const std::string& url, std::string* target, std::string* error, const Headers& headers)
{
    Extra x;
    x.headers = &headers;
    x.follow = false;
    x.location = target;
    std::string body;
    long status = 0;
    if (!perform(url, x, &body, error, 64 * 1024, 30, &status)) return false;
    return http_logic::redirect_ok(status, *target, error);
}

bool download(const std::string& url, const std::string& path, std::string* error, size_t max_bytes,
              long timeout_s, std::function<void(uint64_t, uint64_t)> progress, uint64_t resume_from,
              const std::atomic<bool>* cancel)
{
    // Owner-writable only (fopen would ask for 0666): an executable lands here.
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | (resume_from ? O_APPEND : O_TRUNC), 0644);
    FILE* file = fd >= 0 ? ::fdopen(fd, resume_from ? "ab" : "wb") : nullptr;
    if (!file) {
        if (fd >= 0) ::close(fd);
        if (error) *error = "cannot create " + path;
        return false;
    }
    std::string unused;
    Sink sink{ &unused, max_bytes };
    sink.file = file;
    sink.base = resume_from;
    sink.progress = std::move(progress);
    sink.cancel = cancel;
    Extra x;
    x.to_file = &sink;
    bool ok = perform(url, x, &unused, error, max_bytes, timeout_s, nullptr);
    if (cancel && cancel->load()) {
        if (error) *error = "cancelled";
        ok = false;
    }
    if (std::fclose(file) != 0 && ok) {
        if (error) *error = "cannot write the file";
        ok = false;
    }
    return ok;
}

void abort_all()
{
    s_abort = true;
}

void cleanup()
{
    // A request still running (the app quit during a check) would hold the
    // exit for up to its timeout: skip the cleanup, the process ends anyway.
    std::unique_lock<std::mutex> guard(s_lock, std::try_to_lock);
    if (!guard.owns_lock() || !s_ready) return;
    curl_global_cleanup();
    s_ready = false;
}

}   // namespace http
