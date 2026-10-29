// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cover_strip.hpp"

#include <algorithm>
#include <cmath>

#include "action/game_icons.hpp"

namespace
{
constexpr float EASE_US = 280000.0f;   // a slide of one cover, in µs
constexpr int   SIDE    = 5;           // covers drawn on each side of the middle
constexpr int   AHEAD   = 8;           // icons read ahead of the middle
constexpr float GONE_US = 350000.0f;   // a removed cover floating away, in µs (a removal's step)

// A coral disc with a "!" (in an ink that reads on it) on the top right
// corner of a cover at (left, top, w): a game that stays.
void failed_badge(NVGcontext* vg, float left, float top, float w, float alpha)
{
    NVGcolor coral = aurore::Application::getTheme()["cadran/coral"];
    coral.a *= alpha;
    const float r = std::max(9.0f, w * 0.13f);
    const float cx = left + w - r * 0.55f, cy = top + r * 0.55f;
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgFillColor(vg, coral);
    nvgFill(vg);
    NVGcolor ink = aurore::Application::getTheme()["playguard/badge/ink"];
    ink.a *= alpha;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, cx - r * 0.11f, cy - r * 0.55f, r * 0.22f, r * 0.68f, r * 0.11f);
    nvgCircle(vg, cx, cy + r * 0.42f, r * 0.13f);
    nvgFillColor(vg, ink);
    nvgFill(vg);
}

float eased(float t)   // ease-out cubic, 0..1
{
    const float r = 1.0f - t;
    return 1.0f - r * r * r;
}
}   // namespace

CoverStrip::CoverStrip()
{
    this->setFocusable(true);
    this->setHideHighlight(true);
}

CoverStrip::~CoverStrip()
{
    NVGcontext* vg = aurore::Application::getNVGContext();
    for (const auto& [game, tex] : this->owned) nvgDeleteImage(vg, tex);
}

void CoverStrip::set_games(const std::vector<u64>& games)
{
    this->ids = games;
    this->marks.assign(games.size(), Mark::NONE);
    this->marked_at.assign(games.size(), 0);
    this->from = this->to = 0;
    this->since = 0;
}

void CoverStrip::mark(size_t index, Mark what)
{
    if (index >= this->marks.size()) return;
    this->marks[index] = what;
    this->marked_at[index] = aurore::getCPUTimeUsec();
}

void CoverStrip::set_done(u32 done)
{
    const float target = (float)std::min<size_t>(done, this->ids.empty() ? 0 : this->ids.size() - 1);
    if (target == this->to) return;
    const aurore::Time now = aurore::getCPUTimeUsec();
    this->from = this->at(now);
    this->to = target;
    this->since = now;
}

std::vector<u64> CoverStrip::wanted() const
{
    std::vector<u64> out;
    const int middle = (int)std::lround(this->to);
    for (int i = std::max(0, middle - SIDE); i < (int)this->ids.size() && i <= middle + AHEAD; i++)
        if (!game_icons::known(this->ids[i])) out.push_back(this->ids[i]);
    return out;
}

float CoverStrip::at(aurore::Time now) const
{
    if (!this->since) return this->to;
    const float t = std::min(1.0f, (float)(now - this->since) / EASE_US);
    return this->from + (this->to - this->from) * eased(t);
}

int CoverStrip::texture(u64 game)
{
    if (const auto it = this->owned.find(game); it != this->owned.end()) return it->second;
    bool own = false;
    if (const int tex = game_icons::texture(game, own)) {
        if (own) this->owned[game] = tex;
        return tex;
    }
    return game_icons::placeholder();
}

void CoverStrip::draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
                      aurore::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    if (this->ids.empty()) return;
    aurore::Theme theme = aurore::Application::getTheme();
    const aurore::Time now = aurore::getCPUTimeUsec();
    const float pos = this->at(now);
    const float size = height;   // the cover in the middle, square
    const float mid = x + width / 2;

    // Farthest first, the middle one last, on top.
    std::vector<int> order;
    for (int i = (int)std::floor(pos) - SIDE; i <= (int)std::ceil(pos) + SIDE; i++)
        if (i >= 0 && i < (int)this->ids.size()) order.push_back(i);
    std::sort(order.begin(), order.end(),
              [pos](int a, int b) { return std::fabs(a - pos) > std::fabs(b - pos); });

    nvgSave(vg);
    nvgIntersectScissor(vg, x, y, width, height);
    for (const int i : order) {
        const float d = (float)i - pos;
        const float near = std::min(std::fabs(d), 1.0f);       // 0 in the middle, 1 from the next one on
        const float far = std::max(std::fabs(d) - 1.0f, 0.0f);  // beyond the next one
        // Turned pages: narrower, and a little lower, the further they are.
        // A removed cover floats up, shrinks and fades; gone, it is not drawn.
        const Mark what = this->marks.empty() ? Mark::NONE : this->marks[i];
        float gone = 0;
        if (what == Mark::GONE) {
            gone = eased(std::min(1.0f, (float)(now - this->marked_at[i]) / GONE_US));
            if (gone >= 1.0f) continue;
        }
        const float shrink = 1.0f - 0.45f * gone;
        const float w = size * (1.0f - 0.62f * near) * (1.0f - 0.06f * std::min(far, 4.0f)) * shrink;
        const float h = size * (1.0f - 0.18f * near - 0.03f * std::min(far, 4.0f)) * shrink;
        const float offset = (d < 0 ? -1.0f : 1.0f) * (size * 0.52f * near + size * 0.24f * far + w / 2 * near / shrink);
        const float cx = mid + offset;
        const float left = cx - w / 2, top = y + (height - h) / 2 - size * 0.22f * gone;
        // Fading out toward the ends; the games checked dimmed, but not one
        // that stays after a removal.
        float fade = 1.0f - std::min(far / (SIDE - 1.0f), 1.0f) * 0.85f;
        if (d < -0.5f && what != Mark::FAILED) fade *= 0.55f;
        fade *= 1.0f - gone;
        const float radius = 10 * w / size + 2;

        const int tex = this->texture(this->ids[i]);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, left, top, w, h, radius);
        if (tex) {
            nvgFillPaint(vg, nvgImagePattern(vg, left, top, w, h, 0, tex, this->getAlpha() * fade));
        } else {
            NVGcolor well = theme["cadran/well"];
            well.a *= this->getAlpha() * fade;
            nvgFillColor(vg, well);
        }
        nvgFill(vg);
        // A turned page darker toward its outer edge, as if it curved away.
        if (near > 0) {
            const float shade = 0.45f * near * this->getAlpha() * fade;
            const float inner = d < 0 ? left + w : left, outer = d < 0 ? left : left + w;
            nvgFillPaint(vg, nvgLinearGradient(vg, inner, top, outer, top, nvgRGBAf(0, 0, 0, 0), nvgRGBAf(0, 0, 0, shade)));
            nvgFill(vg);
        }
        // A line round each cover, as the covers of the list have.
        NVGcolor line = theme["cadran/line"];
        line.a *= this->getAlpha() * fade;
        nvgStrokeColor(vg, line);
        nvgStrokeWidth(vg, 1);
        nvgStroke(vg);
        if (what == Mark::FAILED) failed_badge(vg, left, top, w, this->getAlpha() * fade);
    }
    nvgRestore(vg);

    // The textures of the covers gone by freed (a whole check can go through
    // a few hundred games).
    for (auto it = this->owned.begin(); it != this->owned.end();) {
        const bool shown = std::any_of(order.begin(), order.end(), [&](int i) { return this->ids[i] == it->first; });
        if (shown) {
            ++it;
        } else {
            nvgDeleteImage(vg, it->second);
            it = this->owned.erase(it);
        }
    }
}

CoverStrip* cover_strip_above(cadran::Progress& body, const std::vector<u64>& ids)
{
    auto* strip = new CoverStrip();
    strip->setFocusable(false);
    strip->setHeight(96);
    strip->setMarginBottom(18);
    strip->set_games(ids);
    body.box->addView(strip, 0);
    return strip;
}

void cover_strip_read(CoverStrip* strip)
{
    if (!game_icons::wanted()) return;
    const std::vector<u64> wanted = strip->wanted();
    if (!wanted.empty()) game_icons::load(wanted, []() {});
}
