// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/http_logic.hpp"

namespace http_logic
{

bool fits(size_t have, size_t n, size_t max)
{
    return have + n <= max;
}

std::vector<std::string> request_headers(const char* content_type, const std::vector<std::string>* extra)
{
    std::vector<std::string> out;
    if (content_type) {
        out.push_back(std::string("Content-Type: ") + content_type);
        out.push_back("Expect:");
    }
    if (extra) out.insert(out.end(), extra->begin(), extra->end());
    return out;
}

bool succeeded(const Outcome& o, std::string* error)
{
    std::string why;
    if (o.status >= 400) why = "HTTP " + std::to_string(o.status);
    else if (o.overflow) why = "response too large";
    else if (o.file_error) why = "cannot write the file";
    else if (!o.transfer_ok) why = o.transfer_error;
    else return true;
    if (error) *error = why;
    return false;
}

bool redirect_ok(long status, const std::string& target, std::string* error)
{
    if (status < 300 || status >= 400 || target.compare(0, 8, "https://") != 0) {
        if (error) *error = "no redirect (HTTP " + std::to_string(status) + ")";
        return false;
    }
    return true;
}

bool split_url(const std::string& url, std::string* host, int* port, std::string* path)
{
    if (url.compare(0, 8, "https://") != 0) return false;
    const size_t end = url.find_first_of("/?#", 8);
    std::string authority = url.substr(8, end == std::string::npos ? std::string::npos : end - 8);
    std::string rest = end == std::string::npos ? std::string() : url.substr(end);
    const size_t hash = rest.find('#');
    if (hash != std::string::npos) rest.erase(hash);
    if (rest.empty() || rest[0] != '/') rest = "/" + rest;
    int p = 443;
    const size_t colon = authority.find(':');
    if (colon != std::string::npos) {
        const std::string digits = authority.substr(colon + 1);
        if (digits.empty() || digits.size() > 5 ||
            digits.find_first_not_of("0123456789") != std::string::npos)
            return false;
        p = std::stoi(digits);
        authority.erase(colon);
        if (p < 1 || p > 65535) return false;
    }
    if (authority.empty() || authority.find_first_of("@ \t\r\n") != std::string::npos) return false;
    *host = authority;
    *port = p;
    *path = rest;
    return true;
}

std::string request_text(const std::string& host, const std::string& path, const std::string& agent,
                         const std::vector<std::string>& headers, const std::string* body)
{
    std::string out = std::string(body ? "POST " : "GET ") + path + " HTTP/1.1\r\nHost: " + host +
                      "\r\nUser-Agent: " + agent + "\r\nAccept: */*\r\nConnection: close\r\n";
    for (const std::string& h : headers) {
        const size_t colon = h.find(':');
        if (colon == std::string::npos || h.find_first_not_of(' ', colon + 1) == std::string::npos) continue;
        out += h + "\r\n";
    }
    if (body) out += "Content-Length: " + std::to_string(body->size()) + "\r\n";
    out += "\r\n";
    if (body) out += *body;
    return out;
}

namespace
{
std::string lower(std::string s)
{
    for (char& c : s)
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
    return s;
}

// The value of header `name` (lower case) in `head`, its lines after the status line; "" without one.
std::string header(const std::string& head, const std::string& name)
{
    size_t at = head.find("\r\n");
    while (at != std::string::npos && at + 2 < head.size()) {
        const size_t from = at + 2;
        const size_t end = head.find("\r\n", from);
        const std::string line = head.substr(from, end == std::string::npos ? std::string::npos : end - from);
        const size_t colon = line.find(':');
        if (colon != std::string::npos && lower(line.substr(0, colon)) == name) {
            const size_t b = line.find_first_not_of(" \t", colon + 1);
            const size_t e = line.find_last_not_of(" \t");
            return b == std::string::npos ? std::string() : line.substr(b, e - b + 1);
        }
        at = end;
    }
    return "";
}

bool fail(std::string* error, const char* why)
{
    if (error) *error = why;
    return false;
}
}   // namespace

bool parse_response(const std::string& raw, long* status, std::string* body, std::string* error)
{
    const size_t head_end = raw.find("\r\n\r\n");
    if (raw.compare(0, 7, "HTTP/1.") != 0 || raw.size() < 12 || raw[8] != ' ' || head_end == std::string::npos)
        return fail(error, "not an HTTP answer");
    const std::string code = raw.substr(9, 3);
    if (code.find_first_not_of("0123456789") != std::string::npos) return fail(error, "not an HTTP answer");
    const std::string head = raw.substr(0, head_end);
    const std::string rest = raw.substr(head_end + 4);
    *status = std::stol(code);
    body->clear();

    if (lower(header(head, "transfer-encoding")).find("chunked") != std::string::npos) {
        size_t at = 0;
        for (;;) {
            const size_t eol = rest.find("\r\n", at);
            if (eol == std::string::npos) return fail(error, "the answer was cut short");
            const std::string size_line = rest.substr(at, eol - at);
            const size_t digits = size_line.find_first_not_of("0123456789abcdefABCDEF");
            const std::string hex = size_line.substr(0, digits);
            if (hex.empty() || hex.size() > 8) return fail(error, "a bad chunk in the answer");
            const size_t n = std::stoul(hex, nullptr, 16);
            at = eol + 2;
            if (n == 0) return true;   // trailers, if any, are not needed
            if (rest.size() < at + n + 2 || rest.compare(at + n, 2, "\r\n") != 0)
                return fail(error, "the answer was cut short");
            body->append(rest, at, n);
            at += n + 2;
        }
    }
    const std::string length = header(head, "content-length");
    if (!length.empty()) {
        if (length.size() > 12 || length.find_first_not_of("0123456789") != std::string::npos)
            return fail(error, "a bad Content-Length in the answer");
        const size_t n = std::stoul(length);
        if (rest.size() < n) return fail(error, "the answer was cut short");
        *body = rest.substr(0, n);
        return true;
    }
    *body = rest;   // the body runs to the end of the connection
    return true;
}

}   // namespace http_logic
