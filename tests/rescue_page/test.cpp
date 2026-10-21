// Host tests for util/rescue_page: the printable rescue sheet.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "check.h"
#include <string>

#include "util/rescue_page.hpp"

int main()
{
    using rescue_page::escape;
    CHECK(escape("a & <b> \"c\" 'd'") == "a &amp; &lt;b&gt; &quot;c&quot; &#39;d&#39;");
    CHECK(escape("été") == "été");

    // The QR code: an SVG of its modules, at the size asked.
    const std::string svg = rescue_page::qr_svg("https://github.com/JigSawFr/PlayGuard", 180);
    CHECK(svg.rfind("<svg", 0) == 0 && svg.find("width=\"180\"") != std::string::npos && svg.find("h1v1h-1z") != std::string::npos);
    CHECK(rescue_page::qr_svg(std::string(4000, 'x'), 180).empty());   // too long for a QR code

    rescue_page::Text t;
    t.lang = "fr";
    t.title = "Fiche de secours";
    t.subtitle = "Comment annuler le verrou <sans> la console";
    t.pc_title = "Depuis un ordinateur";
    t.pc_text = "Supprimez atmosphere/config/override_config.ini";
    t.mod_title = "Avec le module rescue";
    t.mod_text = "Un fichier vide nommé RESCUE";
    t.qr_caption = "L'aide, sur un téléphone";
    t.footer = "Écrite le 2026-10-20";
    const std::string page = rescue_page::html(t, "https://example.org/a?b=1&c=2");
    CHECK(page.rfind("<!doctype html>", 0) == 0 && page.find("<html lang=\"fr\">") != std::string::npos);
    CHECK(page.find("<h1>Fiche de secours</h1>") != std::string::npos);
    CHECK(page.find("&lt;sans&gt;") != std::string::npos && page.find("<sans>") == std::string::npos);
    CHECK(page.find("L&#39;aide") != std::string::npos);
    CHECK(page.find("href=\"https://example.org/a?b=1&amp;c=2\"") != std::string::npos);
    CHECK(page.find("<svg") != std::string::npos && page.find("http") != std::string::npos);
    CHECK(page.find("<script") == std::string::npos);   // nothing runs, nothing is fetched
    t.lang.clear();
    CHECK(rescue_page::html(t, "x").find("<html lang=\"en\">") != std::string::npos);
    return CHECK_DONE("rescue page escaping, QR code and layout assertions passed");
}
