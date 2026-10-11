// http_logic — the decisions behind http, without libcurl: the size limit of a
// response, the request headers, how a transfer's outcome becomes the error
// the caller shows, and which answers count as a redirect. Plain C++ so the
// host tests can run it (tests/http_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace http_logic
{

// `n` more bytes after `have` stay within `max`; past it the transfer stops.
bool fits(size_t have, size_t n, size_t max);

// The headers a request sends, in order: for a POST (`content_type` set) its
// type and an empty "Expect:" (no 100-continue round trip), then `extra`.
std::vector<std::string> request_headers(const char* content_type, const std::vector<std::string>* extra);

// What the transfer ended with -> success, or false and *error (English).
// The status comes first: a refusal often comes as a large HTML page, so it
// says the status, not its size. `transfer_error` is libcurl's message when
// `transfer_ok` is false.
struct Outcome
{
    long        status = 0;           // HTTP status, 0 when the server did not answer
    bool        overflow = false;     // the body went past the size limit
    bool        file_error = false;   // the download could not be written
    bool        transfer_ok = true;   // CURLE_OK
    std::string transfer_error;
};
bool succeeded(const Outcome& o, std::string* error);

// A redirect() answer: a 3xx status pointing to an https:// URL. False and
// *error otherwise.
bool redirect_ok(long status, const std::string& target, std::string* error);

}   // namespace http_logic
