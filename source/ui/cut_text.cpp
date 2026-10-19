// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cut_text.hpp"

#include <borealis.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>

#include "action/frame_stats_flow.hpp"

namespace cut_text
{
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
namespace
{
std::set<std::string> s_seen;

// The labels shown under `view` that are cut; a hidden box hides its own.
void walk(brls::View* view, const std::string& screen, FILE* out)
{
    if (!view || view->getVisibility() != brls::Visibility::VISIBLE) return;
    if (auto* label = dynamic_cast<brls::Label*>(view)) {
        if (label->getWidth() > 0 && label->isTruncated()) {
            const std::string line = screen + "\t" + label->getFullText();
            if (s_seen.insert(line).second) {
                std::fprintf(out, "%s\n", line.c_str());
                std::fflush(out);
            }
        }
        return;
    }
    if (auto* box = dynamic_cast<brls::Box*>(view))
        for (brls::View* child : box->getChildren()) walk(child, screen, out);
}
}   // namespace
#endif

void sample()
{
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
    static const char* path = std::getenv("PLAYGUARD_SIM_CUT_TEXT");
    if (!path) return;
    static FILE* out = std::fopen(path, "a");
    if (!out) return;
    static auto last = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    if (now - last < std::chrono::seconds(1)) return;
    last = now;
    const auto& stack = brls::Application::getActivitiesStack();
    if (stack.empty()) return;
    walk(stack.back()->getContentView(), frame_stats_flow::screen(), out);
#endif
}
}   // namespace cut_text
