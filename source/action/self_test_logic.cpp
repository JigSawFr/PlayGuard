// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/self_test_logic.hpp"

namespace self_test_logic
{

namespace
{
const char* mark(Result r)
{
    switch (r) {
        case Result::Pass: return "- [x] ";
        case Result::Warn: return "- [ ] \xE2\x9A\xA0 ";   // ⚠
        case Result::Fail: return "- [ ] \xE2\x9C\x97 ";   // ✗
        default: return "- [ ] \xE2\x80\x93 ";             // –
    }
}

// One line: no line ends inside it (a .nro's name, a file's text).
std::string one_line(const std::string& s)
{
    std::string out;
    for (const char c : s) out += c == '\n' || c == '\r' ? ' ' : c;
    return out;
}
}   // namespace

Tally tally(const std::vector<Item>& items)
{
    Tally t;
    for (const Item& i : items) {
        if (i.result == Result::Pass) t.pass++;
        else if (i.result == Result::Warn) t.warn++;
        else if (i.result == Result::Fail) t.fail++;
        else t.skip++;
    }
    return t;
}

std::string markdown(const Header& h, const std::vector<Item>& items)
{
    std::string out = "## PlayGuard console self-test\n\n";
    out += "PlayGuard " + one_line(h.version) + " · " + one_line(h.firmware) + " · " + one_line(h.storage) + " · " +
           one_line(h.when) + "\n";
    std::vector<std::string> sections;
    for (const Item& i : items) {
        bool seen = false;
        for (const std::string& s : sections) seen = seen || s == i.section;
        if (!seen) sections.push_back(i.section);
    }
    for (const std::string& s : sections) {
        out += "\n### " + one_line(s) + "\n\n";
        for (const Item& i : items)
            if (i.section == s) out += mark(i.result) + one_line(i.text) + "\n";
    }
    const Tally t = tally(items);
    out += "\n" + std::to_string(t.pass) + " passed, " + std::to_string(t.warn) + " to know, " + std::to_string(t.fail) +
           " failed, " + std::to_string(t.skip) + " not checked. The rest of docs/hardware-test.md is by hand.\n";
    return out;
}

}   // namespace self_test_logic
