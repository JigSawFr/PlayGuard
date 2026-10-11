// Host tests for source/util/http_logic.cpp: the size limit of a response, the
// headers a request sends, the error a failed transfer reports and what
// counts as a redirect.
#include "check.h"
#include <string>
#include <vector>

#include "util/http_logic.hpp"

static void test_fits()
{
    CHECK(http_logic::fits(0, 0, 0));
    CHECK(http_logic::fits(0, 100, 100));     // exactly the limit is fine
    CHECK(!http_logic::fits(0, 101, 100));
    CHECK(http_logic::fits(60, 40, 100));
    CHECK(!http_logic::fits(60, 41, 100));    // the chunk that crosses it stops the transfer
    CHECK(!http_logic::fits(100, 1, 100));
}

static void test_headers()
{
    CHECK(http_logic::request_headers(nullptr, nullptr).empty());   // a plain GET
    const std::vector<std::string> extra = { "Authorization: Bearer abc", "Accept: application/json" };
    CHECK(http_logic::request_headers(nullptr, &extra) == extra);
    const auto post = http_logic::request_headers("application/x-www-form-urlencoded", &extra);
    REQUIRE(post.size() == 4);
    CHECK(post[0] == "Content-Type: application/x-www-form-urlencoded");
    CHECK(post[1] == "Expect:");               // no 100-continue round trip
    CHECK(post[2] == extra[0] && post[3] == extra[1]);
    const std::vector<std::string> none;
    CHECK(http_logic::request_headers("text/plain", &none).size() == 2);
}

static void test_outcome()
{
    http_logic::Outcome o;
    o.status = 200;
    std::string error = "untouched";
    CHECK(http_logic::succeeded(o, &error) && error == "untouched");
    CHECK(http_logic::succeeded(o, nullptr));
    o.status = 0;                              // no status read, transfer complete: only the transfer decides
    CHECK(http_logic::succeeded(o, &error));
    o.status = 302;                            // a redirect not followed (redirect()): not an error here
    CHECK(http_logic::succeeded(o, &error));

    // The status wins over everything else: a refusal's large HTML page
    // overflows, but the user should read "HTTP 403".
    o.status = 403;
    o.overflow = true;
    o.transfer_ok = false;
    o.transfer_error = "Failure writing output to destination";
    CHECK(!http_logic::succeeded(o, &error) && error == "HTTP 403");
    CHECK(!http_logic::succeeded(o, nullptr));

    o.status = 200;
    CHECK(!http_logic::succeeded(o, &error) && error == "response too large");
    o.overflow = false;
    o.file_error = true;
    CHECK(!http_logic::succeeded(o, &error) && error == "cannot write the file");
    o.file_error = false;
    CHECK(!http_logic::succeeded(o, &error) && error == "Failure writing output to destination");

    http_logic::Outcome timeout;
    timeout.transfer_ok = false;
    timeout.transfer_error = "Operation timed out after 10001 milliseconds";
    CHECK(!http_logic::succeeded(timeout, &error) && error == timeout.transfer_error);
    http_logic::Outcome server;
    server.status = 503;
    CHECK(!http_logic::succeeded(server, &error) && error == "HTTP 503");
}

static void test_redirect()
{
    std::string error;
    CHECK(http_logic::redirect_ok(302, "https://objects.githubusercontent.com/x.zip", &error));
    CHECK(http_logic::redirect_ok(301, "https://a", nullptr));
    CHECK(http_logic::redirect_ok(399, "https://a", nullptr));
    CHECK(!http_logic::redirect_ok(200, "", &error) && error == "no redirect (HTTP 200)");
    CHECK(!http_logic::redirect_ok(400, "https://a", &error) && error == "no redirect (HTTP 400)");
    CHECK(!http_logic::redirect_ok(299, "https://a", nullptr));
    // Never anywhere but https: no downgrade, no other scheme.
    CHECK(!http_logic::redirect_ok(302, "http://objects.githubusercontent.com/x.zip", &error) &&
          error == "no redirect (HTTP 302)");
    CHECK(!http_logic::redirect_ok(302, "HTTPS://upper.example", nullptr));
    CHECK(!http_logic::redirect_ok(302, "/relative/path", nullptr));
    CHECK(!http_logic::redirect_ok(302, "https:/", nullptr));
    CHECK(!http_logic::redirect_ok(302, "", nullptr));
}

static void test_own_connection()
{
    std::string host, path;
    int port = 0;
    CHECK(http_logic::split_url("https://bpa.st/curl", &host, &port, &path) && host == "bpa.st" && port == 443 &&
          path == "/curl");
    CHECK(http_logic::split_url("https://example.org:8443", &host, &port, &path) && host == "example.org" &&
          port == 8443 && path == "/");
    CHECK(http_logic::split_url("https://a.b/x?y=1#frag", &host, &port, &path) && path == "/x?y=1");
    CHECK(http_logic::split_url("https://a.b?q", &host, &port, &path) && host == "a.b" && path == "/?q");
    CHECK(!http_logic::split_url("http://bpa.st/curl", &host, &port, &path));
    CHECK(!http_logic::split_url("https:///x", &host, &port, &path));
    CHECK(!http_logic::split_url("https://a.b:0/", &host, &port, &path));
    CHECK(!http_logic::split_url("https://a.b:99999/", &host, &port, &path));
    CHECK(!http_logic::split_url("https://a.b:x/", &host, &port, &path));
    CHECK(!http_logic::split_url("https://user@a.b/", &host, &port, &path));

    const std::string body = "raw=hello";
    const std::vector<std::string> headers = http_logic::request_headers("text/plain", nullptr);
    CHECK(http_logic::request_text("bpa.st", "/curl", "PlayGuard/1.0.0", headers, &body) ==
          "POST /curl HTTP/1.1\r\nHost: bpa.st\r\nUser-Agent: PlayGuard/1.0.0\r\nAccept: */*\r\n"
          "Connection: close\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nraw=hello");
    CHECK(http_logic::request_text("a.b", "/", "UA", {}, nullptr) ==
          "GET / HTTP/1.1\r\nHost: a.b\r\nUser-Agent: UA\r\nAccept: */*\r\nConnection: close\r\n\r\n");

    long status = 0;
    std::string out, error;
    CHECK(http_logic::parse_response("HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello+extra", &status, &out, &error) &&
          status == 200 && out == "hello");
    CHECK(http_logic::parse_response("HTTP/1.1 201 Created\r\ntransfer-encoding: Chunked\r\n\r\n"
                                     "4\r\nPast\r\n6;ext=1\r\ne URL:\r\n0\r\n\r\n",
                                     &status, &out, &error) &&
          status == 201 && out == "Paste URL:");
    CHECK(http_logic::parse_response("HTTP/1.0 404 Not Found\r\nServer: x\r\n\r\nno such", &status, &out, &error) &&
          status == 404 && out == "no such");
    CHECK(!http_logic::parse_response("HTTP/1.1 200 OK\r\nContent-Length: 50\r\n\r\nshort", &status, &out, &error) &&
          error == "the answer was cut short");
    CHECK(!http_logic::parse_response("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n9\r\nabc", &status, &out,
                                      &error));
    CHECK(!http_logic::parse_response("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nzz\r\n", &status, &out,
                                      &error));
    CHECK(!http_logic::parse_response("SSH-2.0-OpenSSH\r\n\r\n", &status, &out, &error) && error == "not an HTTP answer");
    CHECK(!http_logic::parse_response("HTTP/1.1 200 OK\r\nno end of headers", &status, &out, &error));
    CHECK(!http_logic::parse_response("HTTP/1.1 2x0 OK\r\n\r\n", &status, &out, &error));
}

int main()
{
    test_fits();
    test_headers();
    test_outcome();
    test_redirect();
    test_own_connection();
    return CHECK_DONE("http_logic size limit, headers, outcome, redirect and own-connection tests passed");
}
