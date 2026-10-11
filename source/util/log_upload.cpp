// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/log_upload.hpp"

#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <utility>

#include <borealis/extern/nlohmann/json.hpp>

#include "util/log_files.hpp"
#include "util/paths.hpp"
#include "util/pt_block.hpp"
#include "util/pt_log.hpp"

namespace log_upload
{

std::vector<Part> debug_files()
{
    std::vector<Part> out;
    std::string content;
    if (paths::read_file(pt_block::reference_path(), content))
        out.push_back({ "logs/play_timer/block.json", content });
    // The recorder's file can reach megabytes: its last lines only.
    if (paths::read_file(pt_log::path(), content))
        out.push_back({ "logs/play_timer/log.csv", pt_log::tail(content, PT_LOG_BYTES) });
    if (paths::read_file(paths::history_file(), content)) out.push_back({ "history.json", content });
    if (paths::read_file(paths::config_file(), content)) out.push_back({ "config.json", content });
    return out;
}

std::vector<Part> crash_files(size_t limit)
{
    std::vector<Part> out;
    for (const std::string& name : log_files::list(paths::crashes_dir(), log_files::CRASH)) {
        if (out.size() >= limit) break;
        std::string content;
        if (paths::read_file(paths::crashes_dir() + "/" + name, content))
            out.push_back({ "logs/crashes/" + name, content });
    }
    return out;
}

std::vector<std::string> saved_reports(size_t limit)
{
    std::vector<std::string> names = log_files::list(paths::reports_dir(), log_files::REPORT);
    if (names.size() > limit) names.resize(limit);
    return names;
}

namespace
{
const std::string CUT_NOTICE = "\n\n(cut: too large to send whole)\n";

// The first `room` bytes of `text`, back to the start of a character (never
// half of a UTF-8 sequence), and the notice, cut too when `room` is smaller.
std::string cut_to(const std::string& text, size_t room)
{
    size_t keep = room > CUT_NOTICE.size() ? room - CUT_NOTICE.size() : 0;
    while (keep > 0 && ((unsigned char)text[keep] & 0xC0) == 0x80) keep--;
    return text.substr(0, keep) + CUT_NOTICE.substr(0, room - keep);
}

std::string trimmed(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) b++;
    while (e > b && std::isspace((unsigned char)s[e - 1])) e--;
    return s.substr(b, e - b);
}
}   // namespace

std::string bundle(const std::vector<Part>& parts, size_t max_bytes, bool* truncated)
{
    std::string out;
    for (const Part& p : parts) {
        if (!out.empty()) out += "\n";
        out += "===== " + p.name + " =====\n" + p.content;
        if (!p.content.empty() && p.content.back() != '\n') out += "\n";
    }
    if (truncated) *truncated = false;
    if (out.size() <= max_bytes) return out;
    if (truncated) *truncated = true;
    return cut_to(out, max_bytes);
}

namespace
{
// The process of an Atmosphère crash report: its "Process Name:" line
// ("    Process Name:                    hbloader"), lower case.
std::string process_name(const std::string& text)
{
    const size_t at = text.find("Process Name:");
    if (at == std::string::npos) return "";
    const size_t end = text.find('\n', at);
    std::string name = trimmed(text.substr(at + 13, end == std::string::npos ? std::string::npos : end - at - 13));
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return name;
}
}   // namespace

std::vector<Part> atmosphere_crashes(std::time_t now, int days, size_t limit)
{
    const std::string dir = paths::sd_root() + (paths::sd_root().back() == '/' ? "" : "/") + "atmosphere/crash_reports";
    // "01791569454_010000000000100d.log": the time, a program id.
    std::vector<std::pair<long long, std::string>> found;
    if (DIR* d = opendir(dir.c_str())) {
        while (const dirent* e = readdir(d)) {
            const std::string name = e->d_name;
            const size_t sep = name.find('_');
            if (sep == std::string::npos || sep == 0 || sep > 12 || name.size() < 4 ||
                name.compare(name.size() - 4, 4, ".log") != 0 ||
                !std::all_of(name.begin(), name.begin() + sep, [](char c) { return c >= '0' && c <= '9'; }))
                continue;
            const long long t = std::stoll(name.substr(0, sep));
            if (t <= (long long)now && (long long)now - t <= (long long)days * 86400) found.push_back({ t, name });
        }
        closedir(d);
    }
    std::sort(found.begin(), found.end(),
              [](const auto& a, const auto& b) { return a.first != b.first ? a.first > b.first : a.second > b.second; });
    std::vector<Part> out;
    for (const auto& f : found) {
        if (out.size() >= limit) break;
        std::string content;
        if (!paths::read_file(dir + "/" + f.second, content) || process_name(content) != "hbloader") continue;
        if (content.size() > ATMOSPHERE_CRASH_BYTES) content = cut_to(content, ATMOSPHERE_CRASH_BYTES);
        out.push_back({ "atmosphere/crash_reports/" + f.second, content });
    }
    return out;
}

std::vector<Part> gist_files(const std::vector<Part>& parts, size_t max_bytes, bool* truncated)
{
    std::vector<Part> out;
    size_t total = 0;
    if (truncated) *truncated = false;
    for (size_t i = 0; i < parts.size(); i++) {
        // Its path under logs/, its folders joined with "_" (a gist file
        // name has no "/"): "logs/play_timer/block.json" is play_timer_block.json.
        std::string name = parts[i].name;
        if (name.compare(0, 5, "logs/") == 0) name.erase(0, 5);
        std::replace(name.begin(), name.end(), '/', '_');
        const std::string file =
            std::to_string(i + 1) + "-" + (name.find('.') == std::string::npos ? std::string("report.txt") : name);
        const std::string content = parts[i].content.empty() ? std::string("(empty)\n") : parts[i].content;
        if (total + content.size() <= max_bytes) {
            out.push_back({ file, content });
            total += content.size();
            continue;
        }
        if (truncated) *truncated = true;
        if (max_bytes - total > CUT_NOTICE.size()) out.push_back({ file, cut_to(content, max_bytes - total) });
        break;
    }
    return out;
}

size_t total_bytes(const std::vector<Part>& files)
{
    size_t n = 0;
    for (const Part& f : files) n += f.content.size();
    return n;
}

namespace
{
// "https://<host>/<id>" with the id made only of the given characters.
bool link_with_id(const std::string& text, const std::string& prefix, bool (*ok)(unsigned char))
{
    if (text.compare(0, prefix.size(), prefix) != 0) return false;
    const std::string id = text.substr(prefix.size());
    return !id.empty() && id.size() <= 64 &&
           std::all_of(id.begin(), id.end(), [ok](char c) { return ok((unsigned char)c); });
}

bool alnum(unsigned char c)
{
    return std::isalnum(c) != 0;
}

// The value after "<label>:" on its own line of `body`, trimmed.
std::string field(const std::string& body, const std::string& label)
{
    size_t at = 0;
    while (at < body.size()) {
        size_t end = body.find('\n', at);
        if (end == std::string::npos) end = body.size();
        const std::string line = body.substr(at, end - at);
        if (line.compare(0, label.size() + 1, label + ":") == 0) return trimmed(line.substr(label.size() + 1));
        at = end + 1;
    }
    return "";
}
}   // namespace

bool parse_bpaste(const std::string& body, std::string* url, std::string* removal)
{
    const std::string link = field(body, "Paste URL");
    const std::string remove = field(body, "Removal URL");
    if (!link_with_id(link, "https://bpa.st/", alnum) || !link_with_id(remove, "https://bpa.st/remove/", alnum))
        return false;
    *url = link;
    *removal = remove;
    return true;
}

bool parse_gist(const std::string& body, std::string* url)
{
    try {
        const auto j = nlohmann::json::parse(body);
        const auto it = j.is_object() ? j.find("html_url") : j.end();
        if (it == j.end() || !it->is_string()) return false;
        // "https://gist.github.com/<id>", or ".../<user>/<id>".
        const std::string link = it->get<std::string>();
        if (!link_with_id(link, "https://gist.github.com/", [](unsigned char c) {
                return std::isalnum(c) || c == '-' || c == '/';
            }) || link.find("//", 8) != std::string::npos || link.back() == '/')
            return false;
        *url = link;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string url_encode(const std::string& text)
{
    static const char HEX[] = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : text) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            out += '%';
            out += HEX[c >> 4];
            out += HEX[c & 0xF];
        }
    }
    return out;
}

std::string issue_url(const std::string& repo_url, const std::string& paste_url, const IssueFields& fields)
{
    // Issue forms take each field's id as a query parameter.
    std::string url = repo_url + "/issues/new?template=1-bug.yml&report=" + url_encode(paste_url);
    auto add = [&url](const char* id, const std::string& value) {
        if (!value.empty()) url += std::string("&") + id + "=" + url_encode(value);
    };
    add("version", fields.version);
    add("firmware", fields.firmware);
    add("atmosphere", fields.atmosphere);
    add("launcher", fields.launcher);
    add("storage", fields.storage);
    return url;
}

std::string form(const std::string& text, std::string* content_type)
{
    std::string boundary = "PlayGuardBoundary";
    for (int n = 0; text.find(boundary) != std::string::npos; n++) boundary = "PlayGuardBoundary" + std::to_string(n);
    *content_type = "multipart/form-data; boundary=" + boundary;
    std::string out;
    auto add = [&](const char* name, const std::string& value) {
        out += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"" + name + "\"\r\n\r\n" + value + "\r\n";
    };
    add("raw", text);
    add("lexer", "text");   // shown as plain text, never highlighted as code
    add("expiry", BPASTE_EXPIRY);
    return out + "--" + boundary + "--\r\n";
}

std::string gist_body(const std::vector<Part>& files, const std::string& version)
{
    nlohmann::json j;
    j["description"] = "PlayGuard " + version + " diagnostic report";
    j["public"] = false;
    j["files"] = nlohmann::json::object();
    for (const Part& f : files) j["files"][f.name]["content"] = f.content;
    // A cut can never split a character (gist_files()); anything else invalid is replaced.
    return j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
}

std::string short_url(const std::string& url)
{
    const size_t scheme = url.find("://");
    return scheme == std::string::npos ? url : url.substr(scheme + 3);
}

}   // namespace log_upload
