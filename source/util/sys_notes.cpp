// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/sys_notes.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

#include <borealis/extern/nlohmann/json.hpp>

#include "util/date.hpp"
#include "util/json_util.hpp"
#include "util/safe_text.hpp"
#include "util/sys_updates.hpp"
#include "util/text.hpp"
#include "util/update.hpp"

namespace sys_notes
{

namespace
{
using nlohmann::json;

std::string join(const std::vector<std::string>& lines)
{
    std::string out;
    for (const std::string& l : lines) out += l + "\n";
    return out;
}

// "6.5" -> "6.5.0": update::compare_versions wants three numbers.
std::string three(const std::string& v)
{
    size_t dots = 0;
    for (char c : v) dots += c == '.' ? 1 : 0;
    return dots == 1 ? v + ".0" : v;
}

int compare(const std::string& a, const std::string& b)
{
    return update::compare_versions(three(a), three(b));
}

// A version of exactly three numbers ("23.0.1"), else "".
bool firmware_version(const std::string& v)
{
    int parts[3];
    return update::parse_version(v, parts) && v.find_first_not_of("0123456789.") == std::string::npos &&
           std::count(v.begin(), v.end(), '.') == 2;
}

// "2026-10-16T00:05:51Z" -> "2026-10-16"; "" when it is not a date.
std::string day_of(const std::string& iso)
{
    date::Ymd d;
    if (iso.size() < 10 || !date::parse_ranged(iso.substr(0, 10), &d) || d.year < 2000) return "";
    return iso.substr(0, 10);
}

// Notes: clean, at most MAX_NOTES bytes, cut after a whole line.
std::string cut_notes(const std::string& text)
{
    return safe_text::clean_notes(text, MAX_NOTES);
}

bool https_on(const std::string& url, const char* host)
{
    return text::starts_with(url, std::string("https://") + host + "/") && url.size() < 512;
}

// A Markdown heading's text ("## FAQ", or the line over a setext rule), else "".
std::string heading_text(const std::vector<std::string>& lines, size_t i, bool* setext)
{
    *setext = false;
    const std::string t = text::trim_space(lines[i]);
    if (!t.empty() && t[0] == '#') {
        size_t n = t.find_first_not_of('#');
        if (n == std::string::npos || n > 6 || t[n] != ' ') return "";
        return text::trim_space(t.substr(n));
    }
    if (i + 1 < lines.size() && !t.empty()) {
        const std::string next = text::trim_space(lines[i + 1]);
        if (next.size() >= 3 && (next.find_first_not_of('-') == std::string::npos ||
                                 next.find_first_not_of('=') == std::string::npos)) {
            *setext = true;
            return t;
        }
    }
    return "";
}

// A line that is only an image ("![banner](https://…)").
bool image_only(const std::string& line)
{
    const std::string t = text::trim_space(line);
    return text::starts_with(t, "![") && t.back() == ')' && t.find("](") != std::string::npos;
}
}   // namespace

const char* key(Which which)
{
    switch (which) {
        case Which::Atmosphere: return "atmosphere";
        case Which::Hekate: return "hekate";
        case Which::Firmware: return "firmware";
    }
    return "";
}

const char* product(Which which)
{
    switch (which) {
        case Which::Atmosphere: return "Atmosphère";
        case Which::Hekate: return "hekate";
        case Which::Firmware: return "";
    }
    return "";
}

List& Cache::of(Which which)
{
    return which == Which::Atmosphere ? atmosphere : which == Which::Hekate ? hekate : firmware;
}

const List& Cache::of(Which which) const
{
    return which == Which::Atmosphere ? atmosphere : which == Which::Hekate ? hekate : firmware;
}

const char* repo(Which which)
{
    return which == Which::Atmosphere ? "Atmosphere-NX/Atmosphere" : which == Which::Hekate ? "CTCaer/hekate" : "";
}

bool parse_github(const std::string& text, Which which, std::vector<Release>* out, size_t keep)
{
    return json_util::parse_with(text, [which, out, keep](const json& j) {
        if (!j.is_array()) return false;
        for (const auto& r : j) {
            if (out->size() >= keep) break;
            if (!r.is_object() || !r.contains("draft") || json_util::flag_at(r, "draft") ||
                json_util::flag_at(r, "prerelease"))
                continue;
            Release x;
            x.version = sys_updates::version_of_tag(json_util::string_at(r, "tag_name"));
            if (x.version.empty()) continue;
            if (std::any_of(out->begin(), out->end(), [&x](const Release& o) { return o.version == x.version; }))
                continue;
            x.date = day_of(json_util::string_at(r, "published_at"));
            const std::string page = json_util::string_at(r, "html_url");
            if (https_on(page, "github.com")) x.page = safe_text::clean(page, 512, false);
            const std::string body = cut_notes(json_util::string_at(r, "body"));
            x.supports = supported_firmware(body);
            x.notes = trim_notes(which, body);
            out->push_back(x);
        }
        return true;
    });
}

std::string trim_notes(Which which, const std::string& md)
{
    const std::vector<std::string> lines = text::lines(md);
    std::vector<std::string> out;
    bool skipping = false;
    for (size_t i = 0; i < lines.size(); i++) {
        const std::string& line = lines[i];
        if (which == Which::Atmosphere &&
            text::starts_with(text::lower_ascii(text::trim_space(line)), "and the following was changed in"))
            break;
        bool setext = false;
        const std::string h = text::lower_ascii(heading_text(lines, i, &setext));
        if (!h.empty()) {
            // hekate's FAQ (a folded block on GitHub, a wall of text here)
            // and its Download heading over nothing (the files are GitHub's).
            skipping = which == Which::Hekate && (h == "faq" || text::starts_with(h, "download"));
            if (skipping && setext) i++;
            if (skipping) continue;
        }
        if (skipping || image_only(line)) continue;
        out.push_back(line);
    }
    // Neither blank lines first nor last.
    while (!out.empty() && text::trim_space(out.back()).empty()) out.pop_back();
    size_t first = 0;
    while (first < out.size() && text::trim_space(out[first]).empty()) first++;
    std::string text = join(std::vector<std::string>(out.begin() + (long)first, out.end()));
    if (!text.empty()) text.pop_back();
    return text;
}

std::string supported_firmware(const std::string& notes)
{
    std::string best;
    for (const std::string& raw : text::lines(notes)) {
        const std::string line = text::lower_ascii(raw);
        if (line.find("support") == std::string::npos) continue;
        for (const char* lead : { "for ", "hos ", "firmware ", "supported: " }) {
            size_t at = 0;
            while ((at = line.find(lead, at)) != std::string::npos) {
                at += std::strlen(lead);
                // "for **23.0.0**", "for v23.0.0": past emphasis marks.
                size_t s = at;
                while (s < line.size() && (line[s] == '*' || line[s] == '_' || line[s] == '`')) s++;
                size_t e = s;
                while (e < line.size() && (std::isdigit((unsigned char)line[e]) || line[e] == '.')) e++;
                std::string v = line.substr(s, e - s);
                while (!v.empty() && v.back() == '.') v.pop_back();   // "for 23.0.0."
                if (firmware_version(v) && (best.empty() || compare(v, best) > 0)) best = v;
            }
        }
    }
    return best;
}

std::string wiki_date(const std::string& text)
{
    static const char* MONTHS[] = { "january", "february", "march",     "april",   "may",      "june",
                                    "july",    "august",   "september", "october", "november", "december" };
    const std::string t = text::lower_ascii(text::trim_space(text));
    for (int m = 0; m < 12; m++) {
        const size_t n = std::strlen(MONTHS[m]);
        if (t.compare(0, n, MONTHS[m]) != 0 || t.size() <= n || t[n] != ' ') continue;
        int d = 0, y = 0;
        char comma = 0;
        if (std::sscanf(t.c_str() + n, " %d%c %d", &d, &comma, &y) != 3 || comma != ',') return "";
        if (d < 1 || d > 31 || y < 2000 || y > 9999) return "";
        char out[16];
        std::snprintf(out, sizeof(out), "%04d-%02d-%02d", y, m + 1, d);
        return out;
    }
    return "";
}

std::vector<Version> parse_versions(const std::string& wikitext, size_t keep)
{
    // Rows: "|-", then a cell per line: "| [[23.0.1]]", "| October 1, 2026 (UTC)", ...
    std::vector<Version> all;
    const std::vector<std::string> lines = text::lines(wikitext);
    for (size_t i = 0; i < lines.size(); i++) {
        const std::string t = text::trim_space(lines[i]);
        if (!text::starts_with(t, "| [[")) continue;
        const size_t end = t.find("]]");
        if (end == std::string::npos) continue;
        std::string v = t.substr(4, end - 4);
        const size_t bar = v.find('|');
        if (bar != std::string::npos) v.erase(bar);
        if (!firmware_version(v)) continue;
        Version x;
        x.version = v;
        const std::string next = i + 1 < lines.size() ? text::trim_space(lines[i + 1]) : "";
        if (text::starts_with(next, "|") && !text::starts_with(next, "|-")) x.date = wiki_date(next.substr(1));
        all.push_back(x);
    }
    std::sort(all.begin(), all.end(), [](const Version& a, const Version& b) { return compare(a.version, b.version) > 0; });
    all.erase(std::unique(all.begin(), all.end(), [](const Version& a, const Version& b) { return a.version == b.version; }),
              all.end());
    if (all.size() > keep) all.resize(keep);
    return all;
}

std::string wiki_to_markdown(const std::string& wikitext)
{
    std::string out;
    for (const std::string& whole : text::lines(wikitext)) {
        // Bullets first ("*", "**", "#"…: nested items; in wikitext bold is
        // ''', so a leading * is always a bullet).
        size_t depth = 0;
        while (depth < whole.size() && (whole[depth] == '*' || whole[depth] == '#')) depth++;
        const std::string raw = whole.substr(depth);
        std::string line;
        // Inline: [[page|text]] / [[page]] -> text; [url text] -> text,
        // [url] -> nothing; '''bold''' -> **bold**, ''it'' -> *it*;
        // {{template}} and <ref>…</ref> / other tags dropped.
        for (size_t i = 0; i < raw.size();) {
            if (raw.compare(i, 2, "[[") == 0) {
                const size_t end = raw.find("]]", i + 2);
                if (end != std::string::npos) {
                    std::string inner = raw.substr(i + 2, end - i - 2);
                    const size_t bar = inner.rfind('|');
                    line += bar == std::string::npos ? inner : inner.substr(bar + 1);
                    i = end + 2;
                    continue;
                }
            }
            if (raw[i] == '[' && (raw.compare(i + 1, 8, "https://") == 0 || raw.compare(i + 1, 7, "http://") == 0)) {
                const size_t end = raw.find(']', i);
                if (end != std::string::npos) {
                    const std::string inner = raw.substr(i + 1, end - i - 1);
                    const size_t space = inner.find(' ');
                    if (space != std::string::npos) line += text::trim_space(inner.substr(space + 1));
                    i = end + 1;
                    continue;
                }
            }
            if (raw.compare(i, 2, "{{") == 0) {
                const size_t end = raw.find("}}", i + 2);
                if (end != std::string::npos) {
                    i = end + 2;
                    continue;
                }
            }
            if (raw.compare(i, 5, "<ref>") == 0 || raw.compare(i, 5, "<ref ") == 0) {
                const size_t end = raw.find("</ref>", i);
                if (end != std::string::npos) {
                    i = end + 6;
                    continue;
                }
            }
            if (raw[i] == '<') {
                const size_t end = raw.find('>', i);
                if (end != std::string::npos && end - i < 40) {
                    if (raw.compare(i, 4, "<br>") == 0 || raw.compare(i, 5, "<br/>") == 0 || raw.compare(i, 6, "<br />") == 0)
                        line += " ";
                    i = end + 1;
                    continue;
                }
            }
            if (raw.compare(i, 3, "'''") == 0) {
                line += "**";
                i += 3;
                continue;
            }
            if (raw.compare(i, 2, "''") == 0) {
                line += "*";
                i += 2;
                continue;
            }
            line += raw[i++];
        }
        line = text::trim_space(line);
        if (depth > 0 && !line.empty()) out += std::string((std::min<size_t>(depth, 4) - 1) * 2, ' ') + "- " + line + "\n";
        else out += line + "\n";
    }
    while (!out.empty() && (out.back() == '\n' || out.back() == ' ')) out.pop_back();
    return out;
}

std::string firmware_page(const std::string& version)
{
    return "https://switchbrew.org/wiki/" + version;
}

bool parse_firmware_page(const std::string& wikitext, Release* out)
{
    const std::vector<std::string> lines = text::lines(wikitext);
    std::vector<std::string> log;
    bool in_log = false, found = false;
    for (const std::string& raw : lines) {
        const std::string t = text::trim_space(raw);
        const std::string l = text::lower_ascii(t);
        // "The Switch 23.0.1 system update was released on October 1, 2026 (UTC)."
        const size_t on = l.find("was released on ");
        if (on != std::string::npos && out->date.empty()) out->date = wiki_date(t.substr(on + 16));
        if (text::starts_with(l, "security flaws fixed:")) {
            const std::string v = text::trim_space(l.substr(21));
            out->security = text::starts_with(v, "yes");
        }
        // pctl among the updated sysmodules.
        if (text::starts_with(l, "** sysmodules:")) {
            std::string list = l.substr(14);
            for (char& c : list)
                if (c == '.') c = ',';
            list = "," + list + ",";
            for (size_t i = 0; i < list.size(); i++)
                if (list[i] == ' ') list.erase(i--, 1);
            out->pctl = list.find(",pctl,") != std::string::npos;
        }
        if (text::starts_with(t, "==")) {
            const std::string title = text::lower_ascii(t);
            in_log = title.find("change-log") != std::string::npos || title.find("changelog") != std::string::npos;
            found = found || in_log;
            continue;
        }
        if (!in_log) continue;
        // "[https://… Official] ALL change-log:": where it comes from, not news.
        if (l.find("change-log:") != std::string::npos && t.find("[http") != std::string::npos) continue;
        log.push_back(raw);
    }
    if (!found) return false;
    out->notes = cut_notes(wiki_to_markdown(join(log)));
    return true;
}

std::vector<size_t> tabs(const List& list, const std::string& installed, size_t* hidden, int* installed_at,
                         size_t* newer_from)
{
    std::vector<size_t> newer, older;   // newest first, as listed
    *installed_at = -1;
    for (size_t i = 0; i < list.releases.size(); i++) {
        const int c = installed.empty() ? 1 : compare(list.releases[i].version, installed);
        if (c > 0) newer.push_back(i);
        else if (c == 0 && *installed_at < 0) *installed_at = (int)i;
        else if (c < 0) older.push_back(i);
    }
    *hidden = newer.size() > MAX_NEWER ? newer.size() - MAX_NEWER : 0;
    if (newer.size() > MAX_NEWER) newer.resize(MAX_NEWER);
    // The tabs left take the older ones, from the closest.
    const size_t room = 1 + MAX_NEWER - newer.size() - (*installed_at >= 0 ? 1 : 0);
    if (older.size() > room) older.resize(room);
    std::vector<size_t> out(older.rbegin(), older.rend());
    if (*installed_at >= 0) out.push_back((size_t)*installed_at);
    if (newer_from) *newer_from = out.size();
    out.insert(out.end(), newer.rbegin(), newer.rend());
    return out;
}

size_t newer_count(const List& list, const std::string& installed)
{
    if (installed.empty()) return 0;
    size_t n = 0;
    for (const Release& r : list.releases) n += compare(r.version, installed) > 0 ? 1 : 0;
    return n;
}

std::string supports_up_to(const List& list, const std::string& version)
{
    std::string best;
    for (const Release& r : list.releases) {
        if (r.supports.empty() || compare(r.version, version) > 0) continue;
        if (best.empty() || compare(r.supports, best) > 0) best = r.supports;
    }
    return best;
}

bool covers(const std::string& max, const std::string& fw)
{
    int m[3], f[3];
    if (!update::parse_version(max, m) || !update::parse_version(fw, f)) return false;
    return compare(max, fw) >= 0 || (m[0] == f[0] && m[1] == f[1]);
}

std::string first_supporting(const List& list, const std::string& fw)
{
    std::string first;
    for (const Release& r : list.releases)   // newest first: the last match is the oldest
        if (covers(supports_up_to(list, r.version), fw) && (first.empty() || compare(r.version, first) < 0))
            first = r.version;
    return first;
}

Need need_for(const List& list, const std::string& installed, const std::string& fw, std::string* version)
{
    version->clear();
    if (list.releases.empty() || fw.empty()) return Need::Nothing;
    const Release& newest = list.releases.front();
    const std::string newest_max = supports_up_to(list, newest.version);
    // Nothing named in the notes: nothing to say.
    if (newest_max.empty()) return Need::Nothing;
    if (!covers(newest_max, fw)) {
        *version = newest.version;
        return Need::Wait;
    }
    if (installed.empty() || covers(supports_up_to(list, installed), fw)) return Need::Nothing;
    // Support only grows from one release to the next: the first that
    // covers it is newer than the installed one (an installed one older than
    // the list covers nothing the list knows of).
    const std::string first = first_supporting(list, fw);
    if (first.empty() || compare(first, installed) <= 0) return Need::Nothing;
    *version = first;
    return Need::Update;
}

std::string encode(const Cache& cache)
{
    json j = json::object();
    for (Which w : { Which::Atmosphere, Which::Hekate, Which::Firmware }) {
        const List& l = cache.of(w);
        json releases = json::array();
        for (const Release& r : l.releases) {
            json x = { { "version", r.version }, { "date", r.date }, { "notes", r.notes }, { "page", r.page },
                       { "supports", r.supports } };
            if (r.security) x["security"] = true;
            if (r.pctl) x["pctl"] = true;
            releases.push_back(x);
        }
        j[key(w)] = { { "fetched", l.fetched }, { "releases", releases } };
    }
    return j.dump(1);
}

bool decode(const std::string& text, Cache* out)
{
    Cache c;
    const bool ok = json_util::parse_with(text, [&c](const json& j) {
        if (!j.is_object()) return false;
        for (Which w : { Which::Atmosphere, Which::Hekate, Which::Firmware }) {
            const auto it = j.find(key(w));
            if (it == j.end() || !it->is_object()) continue;
            List& l = c.of(w);
            l.fetched = day_of(json_util::string_at(*it, "fetched"));
            const auto rs = it->find("releases");
            if (rs == it->end() || !rs->is_array()) continue;
            for (const auto& x : *rs) {
                if (l.releases.size() >= MAX_RELEASES) break;
                if (!x.is_object()) continue;
                Release r;
                r.version = sys_updates::version_of_tag(json_util::string_at(x, "version"));
                if (r.version.empty()) continue;
                r.date = day_of(json_util::string_at(x, "date"));
                r.notes = cut_notes(json_util::string_at(x, "notes"));
                const std::string page = json_util::string_at(x, "page");
                if (https_on(page, "github.com") || https_on(page, "switchbrew.org"))
                    r.page = safe_text::clean(page, 512, false);
                const std::string sup = json_util::string_at(x, "supports");
                if (firmware_version(sup)) r.supports = sup;
                r.security = json_util::flag_at(x, "security");
                r.pctl = json_util::flag_at(x, "pctl");
                l.releases.push_back(r);
            }
        }
        return true;
    });
    if (ok) *out = c;
    return ok;
}

void merge(List* kept, const List& fresh)
{
    std::vector<Release> merged = fresh.releases;
    for (Release& r : merged) {
        if (!r.notes.empty()) continue;
        for (const Release& old : kept->releases)
            if (old.version == r.version && !old.notes.empty()) {
                r.notes = old.notes;
                r.security = old.security;
                r.pctl = old.pctl;
                if (r.date.empty()) r.date = old.date;
            }
    }
    kept->releases = merged;
    kept->fetched = fresh.fetched.empty() ? kept->fetched : fresh.fetched;
}

}   // namespace sys_notes
