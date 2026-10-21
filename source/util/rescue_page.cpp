// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/rescue_page.hpp"

#include <qrcodegen.hpp>

namespace rescue_page
{

std::string escape(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c;
        }
    }
    return out;
}

std::string qr_svg(const std::string& text, int size)
{
    try {
        const auto qr = qrcodegen::QrCode::encodeText(text.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
        const int n = qr.getSize(), border = 2, side = n + 2 * border;
        // One path of the dark modules, in module units.
        std::string path;
        for (int y = 0; y < n; y++)
            for (int x = 0; x < n; x++)
                if (qr.getModule(x, y))
                    path += "M" + std::to_string(x + border) + "," + std::to_string(y + border) + "h1v1h-1z";
        return "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " + std::to_string(side) + " " +
               std::to_string(side) + "\" width=\"" + std::to_string(size) + "\" height=\"" + std::to_string(size) +
               "\" shape-rendering=\"crispEdges\"><rect width=\"100%\" height=\"100%\" fill=\"#fff\"/><path d=\"" + path +
               "\" fill=\"#0B1424\"/></svg>";
    } catch (...) {
        return "";
    }
}

std::string html(const Text& t, const std::string& url)
{
    const auto step = [](const char* n, const std::string& title, const std::string& text) {
        return "<li><span class=\"n\">" + std::string(n) + "</span><div><h2>" + escape(title) + "</h2><p>" + escape(text) +
               "</p></div></li>\n";
    };
    return "<!doctype html>\n<html lang=\"" + escape(t.lang.empty() ? "en" : t.lang) +
           "\">\n<head>\n<meta charset=\"utf-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           "<title>PlayGuard · " + escape(t.title) + "</title>\n<style>\n"
           "body{margin:0;background:#fff;color:#0B1424;font:16px/1.5 system-ui,-apple-system,'Segoe UI',sans-serif}\n"
           "main{max-width:760px;margin:40px auto;padding:0 24px}\n"
           "header{display:flex;align-items:center;gap:14px;border-bottom:2px solid #2EC4A6;padding-bottom:16px}\n"
           ".mark{width:40px;height:40px;border-radius:50%;background:#0B1424;color:#2EC4A6;display:grid;place-items:center;"
           "font-weight:700}\n"
           "h1{font-size:26px;margin:0}\n.sub{margin:4px 0 0;color:#4A5A72}\n"
           ".body{display:flex;gap:28px;align-items:flex-start;margin-top:24px}\n"
           "ol{list-style:none;margin:0;padding:0;flex:1}\nli{display:flex;gap:14px;margin-bottom:18px}\n"
           ".n{flex:none;width:30px;height:30px;border-radius:50%;background:#E6F7F3;color:#0B1424;display:grid;"
           "place-items:center;font-weight:700}\n"
           "h2{font-size:18px;margin:2px 0 4px}\np{margin:0}\ncode{background:#EEF2F8;padding:1px 5px;border-radius:4px}\n"
           "figure{margin:0;text-align:center;flex:none}\nfigcaption{font-size:13px;color:#4A5A72;margin-top:6px;"
           "max-width:180px}\n"
           "footer{margin-top:24px;font-size:13px;color:#4A5A72;border-top:1px solid #D5DCE6;padding-top:12px}\n"
           "a{color:#0B1424}\n@media print{main{margin:0 auto}}\n</style>\n</head>\n<body>\n<main>\n"
           "<header><div class=\"mark\">P</div><div><h1>" + escape(t.title) + "</h1><p class=\"sub\">" + escape(t.subtitle) +
           "</p></div></header>\n<div class=\"body\">\n<ol>\n" + step("1", t.pc_title, t.pc_text) +
           step("2", t.mod_title, t.mod_text) + "</ol>\n<figure>" + qr_svg(url, 180) + "<figcaption>" +
           escape(t.qr_caption) + "</figcaption></figure>\n</div>\n<footer>" + escape(t.footer) + " · <a href=\"" +
           escape(url) + "\">" + escape(url) + "</a></footer>\n</main>\n</body>\n</html>\n";
}

}   // namespace rescue_page
