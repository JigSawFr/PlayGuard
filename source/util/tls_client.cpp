// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
// The console build only (switch-mbedtls); PLAYGUARD_TLS_CLIENT builds it on
// a PC, to try it against a server.
#if defined(__SWITCH__) || defined(PLAYGUARD_TLS_CLIENT)

#include "util/tls_client.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

#include "util/extra_ca.hpp"
#include "util/http_logic.hpp"

namespace tls_client
{

namespace
{
using Clock = std::chrono::steady_clock;

// Room for the status line and headers on top of the body's limit.
constexpr size_t HEADER_ROOM = 32 * 1024;
// A read waits this long at most, so that `abort` is seen in time.
constexpr uint32_t READ_SLICE_MS = 1000;

struct Session
{
    mbedtls_net_context      net;
    mbedtls_ssl_context      ssl;
    mbedtls_ssl_config       conf;
    mbedtls_x509_crt         roots;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context drbg;

    Session()
    {
        mbedtls_net_init(&net);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&roots);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&drbg);
    }
    ~Session()
    {
        mbedtls_net_free(&net);
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
        mbedtls_x509_crt_free(&roots);
        mbedtls_ctr_drbg_free(&drbg);
        mbedtls_entropy_free(&entropy);
    }
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
};

bool fail(std::string* error, const std::string& why)
{
    if (error) *error = why;
    return false;
}

std::string describe(int rc)
{
    char text[160] = "";
    mbedtls_strerror(rc, text, sizeof(text));
    return text;
}

// Why the server's certificate was refused, from mbedTLS's flags.
std::string refused(uint32_t flags)
{
    if (flags & MBEDTLS_X509_BADCERT_CN_MISMATCH) return "the certificate is for another server";
    if (flags & (MBEDTLS_X509_BADCERT_EXPIRED | MBEDTLS_X509_BADCERT_FUTURE))
        return "the certificate is out of its dates (is the console's clock right?)";
    if (flags & MBEDTLS_X509_BADCERT_NOT_TRUSTED) return "the certificate is not from a known authority";
    char hex[16];
    std::snprintf(hex, sizeof(hex), "%X", (unsigned)flags);
    return std::string("the certificate was refused (flags 0x") + hex + ")";
}
}   // namespace

bool request(const std::string& url, const std::string* post, const std::vector<std::string>& headers,
             const std::string& agent, size_t max_bytes, long timeout_s, const std::atomic<bool>& abort,
             std::string* raw, std::string* error)
{
    raw->clear();
    std::string host, path;
    int port = 0;
    if (!http_logic::split_url(url, &host, &port, &path)) return fail(error, "not an https:// URL");
    const Clock::time_point deadline = Clock::now() + std::chrono::seconds(std::max(timeout_s, 1L));
    auto expired = [&]() { return abort || Clock::now() >= deadline; };
    auto stop_reason = [&]() { return std::string(abort ? "PlayGuard is quitting" : "timed out"); };

    Session s;
    for (const char* pem : extra_ca::roots()) {
        // PEM is parsed with its terminating NUL counted.
        const int rc = mbedtls_x509_crt_parse(&s.roots, reinterpret_cast<const unsigned char*>(pem), std::strlen(pem) + 1);
        if (rc != 0) return fail(error, "a built-in root does not parse (" + describe(rc) + ")");
    }
#ifndef __SWITCH__
    // To try it on a PC behind an intercepting proxy: its CA, in a PEM file.
    if (const char* test_ca = std::getenv("PLAYGUARD_TLS_TEST_CA")) mbedtls_x509_crt_parse_file(&s.roots, test_ca);
#endif
    static const char PERSONAL[] = "PlayGuard tls_client";
    int rc = mbedtls_ctr_drbg_seed(&s.drbg, mbedtls_entropy_func, &s.entropy,
                                   reinterpret_cast<const unsigned char*>(PERSONAL), sizeof(PERSONAL) - 1);
    if (rc != 0) return fail(error, "no random source (" + describe(rc) + ")");

    rc = mbedtls_net_connect(&s.net, host.c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP);
    if (rc != 0) return fail(error, "cannot connect to " + host + " (" + describe(rc) + ")");

    rc = mbedtls_ssl_config_defaults(&s.conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                     MBEDTLS_SSL_PRESET_DEFAULT);
    if (rc != 0) return fail(error, "TLS set-up failed (" + describe(rc) + ")");
    // The certificate and the name must check out: never optional.
    mbedtls_ssl_conf_authmode(&s.conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&s.conf, &s.roots, nullptr);
    mbedtls_ssl_conf_rng(&s.conf, mbedtls_ctr_drbg_random, &s.drbg);
    mbedtls_ssl_conf_min_version(&s.conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);   // TLS 1.2
    mbedtls_ssl_conf_read_timeout(&s.conf, READ_SLICE_MS);
    rc = mbedtls_ssl_setup(&s.ssl, &s.conf);
    if (rc == 0) rc = mbedtls_ssl_set_hostname(&s.ssl, host.c_str());
    if (rc != 0) return fail(error, "TLS set-up failed (" + describe(rc) + ")");
    mbedtls_ssl_set_bio(&s.ssl, &s.net, mbedtls_net_send, nullptr, mbedtls_net_recv_timeout);

    while ((rc = mbedtls_ssl_handshake(&s.ssl)) != 0) {
        if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE && rc != MBEDTLS_ERR_SSL_TIMEOUT) {
            if (rc == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
                return fail(error, refused(mbedtls_ssl_get_verify_result(&s.ssl)));
            return fail(error, "TLS handshake failed (" + describe(rc) + ")");
        }
        if (expired()) return fail(error, stop_reason());
    }

    const std::string text = http_logic::request_text(host, path, agent, headers, post);
    size_t sent = 0;
    while (sent < text.size()) {
        rc = mbedtls_ssl_write(&s.ssl, reinterpret_cast<const unsigned char*>(text.data()) + sent, text.size() - sent);
        if (rc > 0) {
            sent += (size_t)rc;
            continue;
        }
        if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE)
            return fail(error, "sending failed (" + describe(rc) + ")");
        if (expired()) return fail(error, stop_reason());
    }

    unsigned char buf[4096];
    for (;;) {
        rc = mbedtls_ssl_read(&s.ssl, buf, sizeof(buf));
        if (rc > 0) {
            if (!http_logic::fits(raw->size(), (size_t)rc, max_bytes + HEADER_ROOM))
                return fail(error, "response too large");
            raw->append(reinterpret_cast<const char*>(buf), (size_t)rc);
            continue;
        }
        // The end: the server's close_notify, or the connection closed
        // (http_logic::parse_response then tells a cut answer apart).
        if (rc == 0 || rc == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || rc == MBEDTLS_ERR_NET_CONN_RESET) break;
        if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE && rc != MBEDTLS_ERR_SSL_TIMEOUT)
            return fail(error, "receiving failed (" + describe(rc) + ")");
        if (expired()) return fail(error, stop_reason());
    }
    mbedtls_ssl_close_notify(&s.ssl);
    return true;
}

}   // namespace tls_client

#endif
