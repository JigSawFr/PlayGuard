// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_text.hpp"

#include "ui/tr.hpp"

using namespace brls::literals;

namespace cadran_text
{

std::pair<std::string, std::string> split_state(const std::string& text)
{
    for (const char* sep : { " · ", "・" }) {
        const size_t at = text.rfind(sep);
        if (at != std::string::npos) return { text.substr(0, at), text.substr(at + std::string(sep).size()) };
    }
    return { text, "" };
}

std::vector<std::pair<std::string, std::string>> group_rows(const std::vector<std::pair<std::string, std::string>>& rows)
{
    if (rows.size() < 5) return rows;
    std::vector<std::pair<std::string, std::string>> runs;
    for (size_t i = 0; i < rows.size();) {
        size_t j = i;
        while (j + 1 < rows.size() && rows[j + 1].second == rows[i].second) j++;
        runs.push_back({ j > i ? rows[i].first + " – " + rows[j].first : rows[i].first, rows[i].second });
        i = j + 1;
    }
    return runs;
}

Failure failure(const std::string& text)
{
    Failure f;
    std::string rest = text;
    const std::string next = "\n\n" + "playguard/error/next_step"_i18n;
    const size_t step = rest.find(next);
    f.report = step != std::string::npos;
    if (f.report) rest.erase(step, next.size());
    const std::string dash = " — ";
    const size_t at = rest.find(dash);
    if (at != std::string::npos && at <= 120) {
        f.title = rest.substr(0, at);
        rest = rest.substr(at + dash.size());
    }
    const size_t hex = rest.find("0x");
    if (hex != std::string::npos && hex + 10 <= rest.size()) {
        const std::string number = rest.substr(hex, 10);
        const std::string with_hint = brls::getStr("playguard/error/code_hint", number, "");
        const std::string alone = brls::getStr("playguard/error/code", number);
        if (rest.compare(0, with_hint.size(), with_hint) == 0 && rest.size() > with_hint.size()) {
            f.code = alone;
            rest = rest.substr(with_hint.size());
            if (rest[0] >= 'a' && rest[0] <= 'z') rest[0] = (char)(rest[0] - 'a' + 'A');
        } else if (rest == alone) {
            f.code = alone;
            rest.clear();
        }
    }
    f.reason = rest;
    return f;
}

}   // namespace cadran_text
