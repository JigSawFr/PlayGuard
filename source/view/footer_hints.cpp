// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/footer_hints.hpp"

#include <algorithm>
#include <cmath>

// Each hint's room on either side and between its button and its text
// (borealis' hint XML), and the least they get here: 2 × 6 px still part
// one hint from the next.
static constexpr float HINT_PADDING     = 16;
static constexpr float HINT_PADDING_MIN = 6;
static constexpr float HINT_GAP         = 8;
static constexpr float HINT_GAP_MIN     = 4;
// Kept between the clock and the first hint.
static constexpr float CLOCK_GAP = 16;

FooterHints::FooterHints()
{
    // After brls::Hints' own subscription (its constructor's): the hints are
    // refilled when this runs. Their layout is computed at once (View::
    // invalidate), so fit() reads their width before the next frame.
    this->subscription = brls::Application::getGlobalHintsUpdateEvent()->subscribe([this]() { this->fit(); });
}

FooterHints::~FooterHints()
{
    brls::Application::getGlobalHintsUpdateEvent()->unsubscribe(this->subscription);
}

void FooterHints::fit()
{
    // Only in the footer (brls::BottomBar), next to its clock; the keyboard
    // dialog's hints have none.
    brls::Box* row = this->getParent();
    if (!row || this->getChildren().empty())
        return;
    brls::View* status = nullptr;   // battery, Wi-Fi and clock
    for (brls::View* v : row->getChildren())
        if (v != this) status = v;
    auto* clock = status ? dynamic_cast<brls::Label*>(status->getView("brls/hints/time")) : nullptr;
    if (!clock)
        return;
    // A new screen's clock is set when it is first drawn: until then it has
    // no width of its own. Fitted after that frame.
    if (clock->getFullText().empty()) {
        std::weak_ptr<bool> weak = this->alive;
        brls::sync([this, weak]() {
            if (!weak.expired()) this->fit();
        });
        return;
    }

    clock->setSingleLine(true);
    status->setShrink(0);
    this->setShrink(0);   // the hints at the width they ask for

    // The row has no padding on its sides (borealis' bottom_bar.cpp).
    const float room = row->getWidth() - status->getWidth() - CLOCK_GAP;
    const float over = this->getWidth() - room;
    if (over <= 0)
        return;

    // Closer together, by no more than they need: less room on their sides
    // first, then between button and text. Refilled hints are new views, so
    // each starts from borealis' spacing again.
    float need = std::ceil(over / this->getChildren().size());   // per hint
    const float padding_cut = std::min(std::ceil(need / 2), HINT_PADDING - HINT_PADDING_MIN);
    need = std::max(0.0f, need - 2 * padding_cut);
    const float gap_cut = std::min(need, HINT_GAP - HINT_GAP_MIN);
    for (brls::View* v : this->getChildren()) {
        auto* hint = dynamic_cast<brls::Box*>(v);
        if (!hint) continue;
        hint->setPaddingLeft(HINT_PADDING - padding_cut);
        hint->setPaddingRight(HINT_PADDING - padding_cut);
        if (brls::View* text = hint->getView("hint"))
            text->setMarginLeft(HINT_GAP - gap_cut);
    }
    // Still too wide: the texts are cut ("…"), the clock is not.
    if (this->getWidth() > room)
        this->setShrink(1);
}

void FooterHints::install()
{
    brls::Application::registerXMLView("brls:Hints", FooterHints::create);
}

brls::View* FooterHints::create()
{
    return new FooterHints();
}
