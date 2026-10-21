// ui::pick_actions — a list of things to do as a sheet in the Cadran look:
// a header (what it is about, a line under it), then the actions in cards,
// a section's kicker above each group, each row its icon in a soft square,
// its label and, at its end, what is set now; removals in coral. It sits
// where borealis' Dropdown does (880 px wide, above the hints), over the
// screen it was opened from.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <algorithm>
#include <memory>

#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

namespace ui
{
namespace
{
namespace kit = cadran::kit;

constexpr float ROW_HEIGHT = 60;      // cadran::List::row()'s
constexpr float KICKER_ROOM = 38;     // a section's kicker and its margins
constexpr float CARD_GAP = 14;

// The sheet's panel: its list's foot fades into it while more rows follow.
class Panel : public brls::Box
{
  public:
    Panel() : brls::Box(brls::Axis::COLUMN) {}
    brls::ScrollingFrame* frame = nullptr;

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override
    {
        brls::Box::draw(vg, x, y, width, height, style, ctx);
        if (this->frame)
            kit::draw_more_below(vg, this->frame, this->frame->getX(), this->frame->getY(), this->frame->getWidth(),
                                 this->frame->getHeight(), "cadran/surf", cadran::PICKER_CORNER);
    }
};

class Sheet : public brls::Box
{
  public:
    Sheet(const std::string& title, const std::string& subtitle, const std::vector<Choice>& choices)
        : brls::Box(brls::Axis::COLUMN), choices(choices)
    {
        brls::Theme theme = brls::Application::getTheme();
        this->setBackgroundColor(theme["brls/backdrop"]);
        this->setJustifyContent(brls::JustifyContent::FLEX_END);
        this->setAlignItems(brls::AlignItems::STRETCH);

        auto* panel = new Panel();
        panel->setMarginLeft(200);
        panel->setMarginRight(200);
        panel->setMarginBottom(20);
        panel->setCornerRadius(cadran::PICKER_CORNER);
        panel->setBackgroundColor(theme["cadran/surf"]);
        panel->setBorderColor(theme["cadran/line"]);
        panel->setBorderThickness(1);
        panel->setAlignItems(brls::AlignItems::STRETCH);

        // The header: its title, and a line under it when there is one.
        auto* header = new brls::Box(brls::Axis::COLUMN);
        header->setPadding(subtitle.empty() ? 26 : 22, 36, subtitle.empty() ? 22 : 18, 36);
        header->setLineColor(theme["cadran/line"]);
        header->setLineBottom(1);
        header->setShrink(0);
        brls::Label* name = kit::label(title, 22, "cadran/text");
        name->setSingleLine(true);
        header->addView(name);
        if (!subtitle.empty()) {
            brls::Label* line = kit::label(subtitle, kit::TEXT_META, "cadran/text2");
            line->setMarginTop(6);
            header->addView(line);
        }
        panel->addView(header);

        auto* frame = new brls::ScrollingFrame();
        frame->setGrow(1);
        frame->setScrollingBehavior(brls::ScrollingBehavior::CENTERED);
        auto* column = new brls::Box(brls::Axis::COLUMN);
        column->setAlignItems(brls::AlignItems::STRETCH);
        column->setPadding(18, 28, 22, 28);
        float height = 18 + 22;
        cadran::Card* card = nullptr;
        for (size_t i = 0; i < this->choices.size(); i++) {
            const Choice& c = this->choices[i];
            // A new group: its kicker (when named) and its card.
            if (!card || c.group != this->choices[i - 1].group) {
                if (!c.group.empty()) {
                    auto* kicker = new brls::Label();
                    kit::set_kicker(kicker, c.group);
                    kicker->setSingleLine(true);
                    kicker->setMarginLeft(6);
                    kicker->setMarginTop(card ? CARD_GAP + 4 : 0);
                    kicker->setMarginBottom(10);
                    column->addView(kicker);
                    height += KICKER_ROOM;
                } else if (card) {
                    height += CARD_GAP;
                }
                card = new cadran::Card();
                if (c.group.empty() && column->getChildren().size() > 0) card->setMarginTop(CARD_GAP);
                column->addView(card);
            }
            card->addView(this->row(c, i));
            height += ROW_HEIGHT;
        }
        frame->setContentView(column);
        panel->frame = frame;
        panel->addView(frame);
        // As tall as its rows, up to most of the screen; the rest scrolls.
        const float head = subtitle.empty() ? 76 : 98;
        panel->setHeight(std::min(head + height, brls::Application::contentHeight * 0.8f));
        this->addView(panel);
        this->addView(new brls::BottomBar());

        this->registerAction("hints/back"_i18n, brls::BUTTON_B, [](brls::View*) {
            brls::Application::popActivity(brls::TransitionAnimation::FADE);
            return true;
        }, false, false, brls::SOUND_BACK);
    }

    bool isTranslucent() override { return true; }

    brls::View* getDefaultFocus() override
    {
        return this->first ? this->first : brls::Box::getDefaultFocus();
    }

  private:
    std::vector<Choice> choices;
    brls::View* first = nullptr;

    brls::DetailCell* row(const Choice& c, size_t index)
    {
        brls::Theme theme = brls::Application::getTheme();
        auto* cell = new brls::DetailCell();
        cell->setText(c.label);
        cell->setDetailText(c.detail);
        cadran::List::row(cell, c.icon);
        cell->detail->setMaxWidth(320);
        cell->detail->setSingleLine(true);
        cell->detail->setVisibility(c.detail.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
        const char* tint = nullptr;
        switch (c.tone) {
            case Choice::Tone::PLAIN: break;
            case Choice::Tone::ON:
                tint = "cadran/teal";
                break;
            case Choice::Tone::WARN:
                tint = "cadran/amber";
                break;
            case Choice::Tone::DANGER:
                tint = "cadran/coral";
                cell->title->setTextColor(theme["cadran/coral"]);
                break;
        }
        cell->detail->setTextColor(theme[tint ? tint : "cadran/text2"]);
        if (tint && c.icon)
            cadran::List::recolor_icon(cell, nullptr, tint,
                                       c.tone == Choice::Tone::DANGER ? "cadran/coral_soft"
                                       : c.tone == Choice::Tone::WARN ? "cadran/amber_surf"
                                                                      : "cadran/teal_soft");
        // Chosen: the sheet goes, then the action runs.
        const std::function<void()> run = c.run;
        cell->registerClickAction([run](brls::View*) {
            brls::Application::popActivity(brls::TransitionAnimation::FADE, [run]() {
                if (run) brls::sync(run);
            });
            return true;
        });
        if (index == 0) this->first = cell;
        return cell;
    }
};
}   // namespace

void pick_actions(const std::string& title, const std::vector<Choice>& choices, const std::string& subtitle)
{
    if (choices.empty()) return;
    brls::Application::pushActivity(new brls::Activity(new Sheet(title, subtitle, choices)),
                                    brls::TransitionAnimation::FADE);
}

}   // namespace ui
