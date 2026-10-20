// ui — read-only greying, showing / hiding views with the focus, tabs, the unlock banner.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#include "action/console_actions.hpp"
#include "activity/main_activity.hpp"
#include "app.hpp"

#include <algorithm>
#include <yoga/Yoga.h>

using namespace brls::literals;

namespace ui
{

bool refuse_read_only()
{
    if (!app::read_only()) return false;
    refuse(NXM_RC_READ_ONLY);
    return true;
}

void show_writable(brls::DetailCell* cell, bool writable, NVGcolor title, NVGcolor detail)
{
    const NVGcolor grey = nvgTransRGBA(color_text(), 110);
    cell->title->setTextColor(writable ? title : grey);
    if (auto* sw = dynamic_cast<brls::BooleanCell*>(cell)) {
        if (writable) sw->setOn(sw->isOn(), false);   // its own value colours back
        else sw->detail->setTextColor(grey);
        return;
    }
    cell->detail->setTextColor(writable ? detail : grey);
}

void show_writable(brls::DetailCell* cell, bool writable)
{
    show_writable(cell, writable, color_text(), color_neutral());
}

void guard_switch(brls::BooleanCell* cell)
{
    cell->registerClickAction([cell](brls::View*) {
        if (refuse_read_only()) return true;
        cell->setOn(!cell->isOn());
        cell->getEvent()->fire(cell->isOn());
        return true;
    });
}

void set_visible(brls::View* view, bool visible)
{
    if (!view) return;
    const bool had_focus = !visible && view->isFocused();
    view->setVisibility(visible ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    if (!had_focus) return;

    // Never leave the focus on a view that just disappeared: prefer the
    // nearest focusable sibling, then the nearest one of each enclosing box in
    // turn (the line may sit in a card of its own), then the container.
    brls::Box* parent = view->getParent();
    if (!parent) return;
    for (brls::View* node = view; node->getParent(); node = node->getParent()) {
        const auto& children = node->getParent()->getChildren();
        const int count = (int)children.size();
        int index = -1;
        for (int i = 0; i < count; i++)
            if (children[i] == node) index = i;
        for (int distance = 1; index >= 0 && distance < count; distance++) {
            for (int i : { index + distance, index - distance }) {
                if (i < 0 || i >= count) continue;
                if (brls::View* target = children[i]->getDefaultFocus()) {
                    brls::Application::giveFocus(target);
                    return;
                }
            }
        }
        // Not past the scrolling frame: the rest of the screen is another list.
        if (dynamic_cast<brls::ScrollingFrame*>(node->getParent())) break;
    }
    brls::Application::giveFocus(parent);
}

void set_visible_all(std::initializer_list<std::pair<brls::View*, bool>> changes)
{
    for (const auto& c : changes)
        if (c.second) set_visible(c.first, true);
    for (const auto& c : changes)
        if (!c.second) set_visible(c.first, false);
}

void set_text(brls::Label* label, const std::string& text)
{
    if (label && label->getFullText() != text) label->setText(text);
}

void set_detail(brls::DetailCell* cell, const std::string& text)
{
    if (cell) set_text(cell->detail, text);
}

void batch_layout(brls::Box* box, const std::function<void()>& fill)
{
    // Out of its parent's Yoga node only: borealis' views, their parents and
    // the focus stay as they are, and a change inside stops at the box.
    YGNodeRef node = box ? box->getYGNode() : nullptr;
    YGNodeRef owner = node ? YGNodeGetOwner(node) : nullptr;
    uint32_t index = 0;
    while (owner && index < YGNodeGetChildCount(owner) && YGNodeGetChild(owner, index) != node) index++;
    if (!owner || index == YGNodeGetChildCount(owner)) {
        fill();
        return;
    }
    YGNodeRemoveChild(owner, node);
    fill();
    YGNodeInsertChild(owner, node, index);
    box->invalidate();
}

void keep_focus(brls::Activity* activity, brls::View* fallback)
{
    // The view to get the focus back: the end of the boxes' last focused
    // views. Box::removeView leaves a removed child there: the walk stops
    // at one no longer among its box's children (never read).
    brls::View* last = activity->getContentView();
    while (auto* box = dynamic_cast<brls::Box*>(last)) {
        brls::View* next = box->getLastFocusedView();
        const auto& children = box->getChildren();
        if (!next || std::find(children.begin(), children.end(), next) == children.end()) break;
        last = next;
    }
    bool gone = !last || !last->isFocusable();
    for (brls::View* v = last; v && !gone; v = v->getParent()) gone = v->getVisibility() != brls::Visibility::VISIBLE;
    if (gone) brls::Application::giveFocus(fallback ? fallback : activity->getContentView());
}

bool has_focus(brls::Activity* activity)
{
    brls::View* focus = brls::Application::getCurrentFocus();
    return focus && focus->getParentActivity() == activity;
}

bool on_top(brls::Activity* activity)
{
    const auto& stack = brls::Application::getActivitiesStack();
    return !stack.empty() && stack.back() == activity;
}

MainActivity* main_activity()
{
    for (brls::Activity* a : brls::Application::getActivitiesStack())
        if (auto* main = dynamic_cast<MainActivity*>(a)) return main;
    return nullptr;
}

void go_to_tab(brls::View* from, int position)
{
    brls::TabFrame* frame = nullptr;
    for (brls::View* v = from; v && !frame; v = v->getParent())
        frame = dynamic_cast<brls::TabFrame*>(v);
    if (!frame) return;
    brls::sync([frame, position]() {
        frame->focusTab(position);   // replaces the tab content
        const auto& children = frame->getChildren();
        if (children.size() >= 2) brls::Application::giveFocus(children.back());
    });
}

void open_screen(brls::View* content, const std::string& title)
{
    auto* frame = new brls::AppletFrame(content);
    frame->setTitle(title);
    frame->setIcon(brls::View::getFilePathXMLAttributeValue("@res/img/app_icon.jpg"));
    brls::Application::pushActivity(new brls::Activity(frame));
}

void init_unlock_banner(brls::DetailCell* cell, std::function<void()> after)
{
    cell->setText("playguard/security/banner_title"_i18n);
    cell->title->setTextColor(color_warn());
    cell->setBackgroundColor(nvgTransRGBA(color_warn(), 28));
    cell->setCornerRadius(6);
    cell->setDetailText("playguard/security/banner_action"_i18n);
    cell->registerClickAction([after](brls::View*) {
        console_actions::relock("", after);
        return true;
    });
}

void show_unlock_banner(brls::DetailCell* cell, bool unlocked)
{
    // Read-only: no action, so the title gets the whole width (an empty detail
    // label still reserves its space).
    cell->detail->setVisibility(app::read_only() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    set_visible(cell, unlocked);
}

}   // namespace ui
