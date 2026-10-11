// CadranDigits — a line of text in the display face of the "Cadran" boards
// (Sora Bold, OFL, resources/font/sora_bold_latin.ttf, loaded as
// "cadran/digits"; the default font when it is missing): the big figures,
// the card values, the chips. borealis' Label draws with the one default font.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

class CadranDigits : public brls::View
{
  public:
    CadranDigits();

    void setText(const std::string& text);
    void setFontSize(float size);
    void setTextColor(NVGcolor color);
    // At most that wide: a longer text is drawn smaller to fit (0: never).
    void setFitWidth(float width);

    void draw(NVGcontext* vg, float x, float y, float width, float height,
              brls::Style style, brls::FrameContext* ctx) override;

    static brls::View* create();

  private:
    std::string text;
    float fontSize = 32.0f;
    float drawSize = 32.0f;   // fontSize, or less to fit fitWidth
    float fitWidth = 0.0f;
    NVGcolor color;
    int font;

    void measure();   // the view's size from the text
};
