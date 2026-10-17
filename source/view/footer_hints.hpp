// FooterHints — borealis' footer hints (brls:Hints), fitted to the footer.
// borealis gives the hints the width they ask for and the clock what is
// left: with five hints in French and the Switch's battery and Wi-Fi icons,
// the clock wrapped onto two lines and every hint ended in "…". Here the
// clock keeps its one line; the hints first come closer together (less room
// around each), and only when even that is not enough are their texts cut.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class FooterHints : public brls::Hints
{
  public:
    FooterHints();
    ~FooterHints() override;

    // Takes brls:Hints' place in borealis' footer. Called once, at start-up,
    // before the first screen opens.
    static void install();
    static brls::View* create();

  private:
    void fit();

    brls::VoidEvent::Subscription subscription;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);   // guards the fit after the first frame
};
