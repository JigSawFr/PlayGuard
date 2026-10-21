// rescue_page — the rescue sheet of Console › Forwarders › Lock the homebrew
// menu as a page to print (config/playguard/rescue.html): how to undo the
// lock without the console, in the UI's language, with the help's QR code
// drawn in SVG. One file, nothing fetched: it opens from the SD card on any
// computer. Plain C++ (no UI) for the host tests (tests/rescue_page).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace rescue_page
{

// Its words, already in the UI's language (UTF-8); `lang` its code ("fr").
struct Text
{
    std::string lang, title, subtitle;
    std::string pc_title, pc_text;     // from a computer
    std::string mod_title, mod_text;   // with the rescue module
    std::string qr_caption, footer;    // under the QR code; the last line (when it was written)
};

// The page: `url` as a QR code (and as a link). Every text escaped.
std::string html(const Text& t, const std::string& url);

// `text` safe in HTML (& < > " ').
std::string escape(const std::string& text);

// The SVG of `text`'s QR code, `size` px wide; "" when it does not fit one.
std::string qr_svg(const std::string& text, int size);

}   // namespace rescue_page
