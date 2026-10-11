// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/http.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "app.hpp"
#include "util/extra_ca.hpp"
#include "util/http_logic.hpp"
#include "util/tls_client.hpp"

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
};

// Called by curl about once a second, even while waiting for the server.
int on_progress(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
{
    return s_abort ? 1 : 0;   // non-zero aborts the transfer
}

size_t on_data(char* data, size_t size, size_t count, void* user)
{
    auto* sink = static_cast<Sink*>(user);
    if (s_abort) return 0;
    const size_t n = size * count;
    const size_t have = sink->file ? sink->written : sink->body->size();
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
        sink->progress(sink->written, total > 0 ? (uint64_t)total : 0);
    }
    return n;
}

#ifdef __SWITCH__
// switch-curl hands its ssl-service context here before the handshake: the
// roots the firmware's list lacks (util/extra_ca.hpp) are added to it. The
// built-in list stays; a root the service refuses only leaves the servers
// under it unverifiable, as before.
CURLcode on_ssl_context(CURL*, void* context, void*)
{
    for (const char* pem : extra_ca::roots()) {
        const Result rc = sslContextImportServerPki(static_cast<SslContext*>(context), pem,
                                                    (u32)std::strlen(pem), SslCertificateFormat_Pem, nullptr);
        if (R_FAILED(rc)) std::fprintf(stderr, "http: extra root refused (0x%X)\n", (unsigned)rc);
    }
    return CURLE_OK;
}
#endif
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
    bool*              refused = nullptr;        // set when the server's certificate was refused
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
#ifdef __SWITCH__
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, on_ssl_context);
#endif
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, std::min(timeout_s, 30L));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_s);
    if (to_file) {   // a long transfer: give up on a stall rather than at the end
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, on_progress);
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
    if (x.refused) *x.refused = rc == CURLE_PEER_FAILED_VERIFICATION;

    http_logic::Outcome outcome;
    outcome.status      = status;
    outcome.overflow    = sink.overflow;
    outcome.file_error  = sink.file_error;
    outcome.transfer_ok = rc == CURLE_OK;
    if (rc != CURLE_OK) outcome.transfer_error = message[0] ? message : curl_easy_strerror(rc);
    return http_logic::succeeded(outcome, error);
}

#ifdef __SWITCH__
// The request again on a TLS connection of PlayGuard's own (util/tls_client.hpp),
// when the console's ssl service refused a certificate it cannot check
// (Let's Encrypt's without Key Encipherment). The refusal comes before
// anything is sent, so a POST is never sent twice. No redirects followed.
bool own_connection(const std::string& url, const Extra& x, std::string* body, std::string* error, size_t max_bytes,
                    long timeout_s, long* status_out)
{
    std::lock_guard<std::mutex> guard(s_lock);
    const std::vector<std::string> headers = http_logic::request_headers(x.post ? x.content_type : nullptr, x.headers);
    std::string raw, got;
    if (!tls_client::request(url, x.post, headers, "PlayGuard/" + app::version(), max_bytes, timeout_s, s_abort, &raw,
                             error))
        return false;
    long status = 0;
    if (!http_logic::parse_response(raw, &status, &got, error)) return false;
    if (status_out) *status_out = status;
    if (!x.post && status >= 300 && status < 400) {
        if (error) *error = "redirected (HTTP " + std::to_string(status) + "), not followed";
        return false;
    }
    http_logic::Outcome outcome;
    outcome.status   = status;
    outcome.overflow = got.size() > max_bytes;
    if (!http_logic::succeeded(outcome, error)) return false;
    *body = std::move(got);
    return true;
}
#endif

// perform(), then own_connection() when the console refused the certificate.
bool perform_or_own(const std::string& url, Extra x, std::string* body, std::string* error, size_t max_bytes,
                    long timeout_s, long* status_out)
{
    bool refused = false;
    x.refused = &refused;
    if (perform(url, x, body, error, max_bytes, timeout_s, status_out)) return true;
#ifdef __SWITCH__
    if (refused) {
        std::string why;
        if (own_connection(url, x, body, &why, max_bytes, timeout_s, status_out)) return true;
        if (error) *error = why;   // the console's refusal is known; this is the news
    }
#endif
    return false;
}
}   // namespace

bool get(const std::string& url, std::string* body, std::string* error, size_t max_bytes, long timeout_s,
         const Headers& headers, long* status)
{
    Extra x;
    x.headers = &headers;
    return perform_or_own(url, x, body, error, max_bytes, timeout_s, status);
}

bool post(const std::string& url, const std::string& data, const char* content_type, std::string* body,
          std::string* error, long* status, size_t max_bytes, long timeout_s, const Headers& headers)
{
    Extra x;
    x.post = &data;
    x.content_type = content_type;
    x.headers = &headers;
    return perform_or_own(url, x, body, error, max_bytes, timeout_s, status);
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
              long timeout_s, std::function<void(uint64_t, uint64_t)> progress)
{
    // Owner-writable only (fopen would ask for 0666): an executable lands here.
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE* file = fd >= 0 ? ::fdopen(fd, "wb") : nullptr;
    if (!file) {
        if (fd >= 0) ::close(fd);
        if (error) *error = "cannot create " + path;
        return false;
    }
    std::string unused;
    Sink sink{ &unused, max_bytes };
    sink.file = file;
    sink.progress = std::move(progress);
    Extra x;
    x.to_file = &sink;
    bool ok = perform(url, x, &unused, error, max_bytes, timeout_s, nullptr);
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
