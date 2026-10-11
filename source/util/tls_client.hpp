// tls_client — an HTTPS request on a TLS connection of PlayGuard's own
// (mbedTLS), for the servers the console's ssl service refuses although their
// certificate is sound. Its verifier (NSS) wants "Key Encipherment" in an RSA
// server certificate, which Let's Encrypt has left out since its "generation
// Y" (bpa.st's, October 2026, "Digital Signature" only): every such site
// fails with "SSL peer certificate or SSH remote key was not OK", whatever
// roots are added (util/extra_ca.hpp). http.cpp retries those here.
//
// The server is checked as strictly, against extra_ca::roots() only (Let's
// Encrypt's), its name included: a server under another root still fails.
// No redirects, HTTP/1.1, one request per connection. On the Switch only
// (switch-mbedtls); the parsing is http_logic's, host-tested.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <atomic>
#include <cstddef>
#include <string>
#include <vector>

namespace tls_client
{

// Sends the request (a POST when `post` is set, `headers` as
// http_logic::request_headers() gives them) and reads the whole answer into
// *raw, at most `max_bytes` (more: false, "response too large"). False and
// *error (English) when the connection, the server's certificate or the
// exchange fails, after `timeout_s`, or once `abort` is set.
bool request(const std::string& url, const std::string* post, const std::vector<std::string>& headers,
             const std::string& agent, size_t max_bytes, long timeout_s, const std::atomic<bool>& abort,
             std::string* raw, std::string* error);

}   // namespace tls_client
