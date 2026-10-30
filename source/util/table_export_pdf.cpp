// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/table_export_parts.hpp"

#include <algorithm>
#include <cstdint>
#include <format>

#include <aurore/cadran/util/utf8.hpp>

namespace table_export
{
namespace detail
{

namespace
{
// Windows-1252 (the standard fonts' WinAnsiEncoding) for one code point.
char winansi(uint32_t cp)
{
    if (cp < 0x20) return ' ';
    if (cp < 0x7F || (cp >= 0xA0 && cp <= 0xFF)) return (char)cp;
    static const struct { uint16_t cp; uint8_t byte; } extra[] = {
        { 0x20AC, 0x80 }, { 0x201A, 0x82 }, { 0x0192, 0x83 }, { 0x201E, 0x84 }, { 0x2026, 0x85 }, { 0x2020, 0x86 },
        { 0x2021, 0x87 }, { 0x02C6, 0x88 }, { 0x2030, 0x89 }, { 0x0160, 0x8A }, { 0x2039, 0x8B }, { 0x0152, 0x8C },
        { 0x017D, 0x8E }, { 0x2018, 0x91 }, { 0x2019, 0x92 }, { 0x201C, 0x93 }, { 0x201D, 0x94 }, { 0x2022, 0x95 },
        { 0x2013, 0x96 }, { 0x2014, 0x97 }, { 0x02DC, 0x98 }, { 0x2122, 0x99 }, { 0x0161, 0x9A }, { 0x203A, 0x9B },
        { 0x0153, 0x9C }, { 0x017E, 0x9E }, { 0x0178, 0x9F }, { 0x202F, 0xA0 }, { 0x2009, 0x20 },
    };
    for (const auto& e : extra)
        if (e.cp == cp) return (char)e.byte;
    return '?';
}

std::string to_winansi(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size();) {
        uint32_t cp;
        const size_t len = utf8::decode(s, i, &cp);
        if (!len) { out += '?'; i++; continue; }   // badly encoded: one '?' a byte
        out += winansi(cp);
        i += len;
    }
    return out;
}

// Advance widths (1/1000 em) of Helvetica for 0x20..0x7E; other bytes use 556.
const uint16_t HELVETICA[95] = {
    278, 278, 355, 556, 556, 889, 667, 191, 333, 333, 389, 584, 278, 333, 278, 278, 556, 556, 556, 556,
    556, 556, 556, 556, 556, 556, 278, 278, 584, 584, 584, 556, 1015, 667, 667, 722, 722, 667, 611, 778,
    722, 278, 500, 667, 556, 833, 722, 778, 667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278,
    278, 278, 469, 556, 333, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833, 556, 556,
    556, 556, 333, 500, 278, 556, 500, 722, 500, 500, 500, 334, 260, 334, 584,
};

// Width in points of WinAnsi text; bold is about 8 % wider.
double text_width(const std::string& s, double size, bool bold)
{
    double w = 0;
    for (unsigned char c : s) w += (c >= 0x20 && c <= 0x7E) ? HELVETICA[c - 0x20] : (c == 0x85 ? 1000 : 556);
    return w * size / 1000.0 * (bold ? 1.08 : 1.0);
}

// Widths are compared with this much slack: a column is sized from the very
// same measurement, give or take rounding.
const double SLACK = 0.01;

// Cuts WinAnsi text to `max` points, ending with "…".
std::string fit(std::string s, double max, double size, bool bold)
{
    max += SLACK;
    if (text_width(s, size, bold) <= max) return s;
    while (!s.empty() && text_width(s + "\x85", size, bold) > max) s.pop_back();
    return s + "\x85";
}

std::string pdf_string(const std::string& winansi_text)
{
    std::string out = "(";
    for (unsigned char c : winansi_text) {
        if (c == '(' || c == ')' || c == '\\') {
            out += '\\';
            out += (char)c;
        } else if (c < 0x20 || c >= 0x7F) {
            out += std::format("\\{:03o}", c);
        } else {
            out += (char)c;
        }
    }
    return out + ")";
}

std::string num(double v)
{
    return std::format("{:.2f}", v);
}

std::string text_at(double x, double y, const std::string& winansi_text, double size, bool bold)
{
    return "BT /" + std::string(bold ? "F2 " : "F1 ") + num(size) + " Tf " + num(x) + " " + num(y) + " Td " +
           pdf_string(winansi_text) + " Tj ET\n";
}

// The narrowest width that shows a header label on at most two lines.
double header_width(const std::string& s, double size)
{
    double best = text_width(s, size, true);
    for (size_t i = s.find(' '); i != std::string::npos; i = s.find(' ', i + 1))
        best = std::min(best, std::max(text_width(s.substr(0, i), size, true), text_width(s.substr(i + 1), size, true)));
    return best;
}

// A header label on one line, or two split at a space (the longest first
// line that leaves a second one that fits too).
std::vector<std::string> header_lines(const std::string& s, double max, double size)
{
    max += SLACK;
    if (text_width(s, size, true) <= max) return { s };
    size_t best = std::string::npos;
    for (size_t i = s.find(' '); i != std::string::npos; i = s.find(' ', i + 1))
        if (text_width(s.substr(0, i), size, true) <= max && text_width(s.substr(i + 1), size, true) <= max) best = i;
    if (best == std::string::npos) return { fit(s, max, size, true) };
    return { s.substr(0, best), s.substr(best + 1) };
}

// UTF-16BE hex string for the document information (PDFDocEncoding has no em
// dash). Badly encoded bytes become U+FFFD.
std::string pdf_text_string(const std::string& text)
{
    std::string out = "<FEFF";
    for (size_t i = 0; i < text.size();) {
        uint32_t cp;
        i += utf8::decode_replacing(text, i, &cp);
        if (cp >= 0x10000) {
            cp -= 0x10000;
            out += std::format("{:04X}", 0xD800 + (cp >> 10));
            cp = 0xDC00 + (cp & 0x3FF);
        }
        out += std::format("{:04X}", cp & 0xFFFF);
    }
    return out + ">";
}
}   // namespace

std::string render_pdf(const Table& t)
{
    // A4 landscape, in points.
    const double W = 842, H = 595, M = 36, PAD = 4;
    const double FONT = 8.5, HEAD_FONT = 8, LINE = 13;
    const size_t ncol = t.columns.size();

    // Everything in WinAnsi first, so widths are measured on what is drawn.
    std::vector<std::string> labels;
    for (const auto& c : t.columns) labels.push_back(to_winansi(c.label));
    std::vector<std::vector<std::string>> rows;
    for (const auto& r : t.rows) {
        std::vector<std::string> out;
        for (size_t i = 0; i < ncol; i++) out.push_back(to_winansi(cell(r, i)));
        rows.push_back(out);
    }

    // Column widths: what the content needs (a header may take two lines),
    // then the first column (the name) absorbs what is left or missing.
    std::vector<double> width(ncol, 0);
    for (size_t i = 0; i < ncol; i++) {
        width[i] = header_width(labels[i], HEAD_FONT);
        for (const auto& r : rows) width[i] = std::max(width[i], text_width(r[i], FONT, false));
        width[i] += 2 * PAD;
    }
    double used = 0;
    for (size_t i = 1; i < ncol; i++) used += width[i];
    if (ncol > 0) width[0] = std::max(80.0, W - 2 * M - used);
    std::vector<double> x(ncol + 1, M);
    for (size_t i = 0; i < ncol; i++) x[i + 1] = x[i] + width[i];

    const double table_top = H - M - 52;   // below the title and subtitle
    const double header_h  = 2 * (HEAD_FONT + 2) + 6;
    const size_t per_page  = std::max<size_t>(1, (size_t)((table_top - header_h - M - 10) / LINE));
    const size_t pages     = std::max<size_t>(1, (rows.size() + per_page - 1) / per_page);
    const std::string title = to_winansi(t.title), subtitle = to_winansi(t.subtitle);

    std::vector<std::string> contents;
    for (size_t p = 0; p < pages; p++) {
        std::string c;
        c += text_at(M, H - M - 16, fit(title, W - 2 * M, 16, true), 16, true);
        c += "0.4 g\n" + text_at(M, H - M - 32, fit(subtitle, W - 2 * M, 9, false), 9, false) + "0 g\n";
        // Header (one or two lines), then a rule under it.
        for (size_t i = 0; i < ncol; i++) {
            const auto lines = header_lines(labels[i], width[i] - 2 * PAD, HEAD_FONT);
            for (size_t k = 0; k < lines.size(); k++) {
                const double y = table_top - (HEAD_FONT + 2) * (k + 1) - (lines.size() == 1 ? HEAD_FONT + 2 : 0);
                const double tx = t.columns[i].numeric ? x[i + 1] - PAD - text_width(lines[k], HEAD_FONT, true) : x[i] + PAD;
                c += text_at(tx, y, lines[k], HEAD_FONT, true);
            }
        }
        const double rule = table_top - header_h;
        c += "0.6 w " + num(M) + " " + num(rule) + " m " + num(x[ncol]) + " " + num(rule) + " l S\n";
        // Rows, with a light band on every other one.
        const size_t first = p * per_page, last = std::min(rows.size(), first + per_page);
        for (size_t r = first; r < last; r++) {
            const double base = rule - LINE * (double)(r - first + 1);
            if ((r - first) % 2 == 1)
                c += "0.94 g " + num(M) + " " + num(base - 3.5) + " " + num(x[ncol] - M) + " " + num(LINE) + " re f 0 g\n";
            for (size_t i = 0; i < ncol; i++) {
                const std::string v = fit(rows[r][i], width[i] - 2 * PAD, FONT, false);
                if (v.empty()) continue;
                const double tx = t.columns[i].numeric ? x[i + 1] - PAD - text_width(v, FONT, false) : x[i] + PAD;
                c += text_at(tx, base, v, FONT, false);
            }
        }
        const std::string page = std::to_string(p + 1) + " / " + std::to_string(pages);
        c += "0.4 g\n" + text_at(W - M - text_width(page, 8, false), M / 2, page, 8, false) + "0 g\n";
        contents.push_back(c);
    }

    // Objects: 1 catalog, 2 page tree, 3-4 fonts, 5 info, then content + page per page.
    std::vector<std::string> objs(5);
    std::string kids;
    for (size_t p = 0; p < pages; p++) {
        const size_t content = objs.size() + 1, page = content + 1;
        objs.push_back("<< /Length " + std::to_string(contents[p].size()) + " >>\nstream\n" + contents[p] + "\nendstream");
        objs.push_back("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 842 595] /Resources << /Font << /F1 3 0 R "
                       "/F2 4 0 R >> >> /Contents " + std::to_string(content) + " 0 R >>");
        kids += std::to_string(page) + " 0 R ";
    }
    objs[0] = "<< /Type /Catalog /Pages 2 0 R >>";
    objs[1] = "<< /Type /Pages /Kids [ " + kids + "] /Count " + std::to_string(pages) + " >>";
    objs[2] = "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>";
    objs[3] = "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold /Encoding /WinAnsiEncoding >>";
    objs[4] = "<< /Title " + pdf_text_string(t.title) + " /Producer (PlayGuard) >>";

    std::string out = "%PDF-1.4\n%\xE2\xE3\xCF\xD3\n";
    std::vector<size_t> offsets;
    for (size_t i = 0; i < objs.size(); i++) {
        offsets.push_back(out.size());
        out += std::to_string(i + 1) + " 0 obj\n" + objs[i] + "\nendobj\n";
    }
    const size_t xref = out.size();
    out += "xref\n0 " + std::to_string(objs.size() + 1) + "\n0000000000 65535 f \n";
    for (size_t off : offsets) {
        out += std::format("{:010} 00000 n \n", off);
    }
    out += "trailer\n<< /Size " + std::to_string(objs.size() + 1) + " /Root 1 0 R /Info 5 0 R >>\nstartxref\n" +
           std::to_string(xref) + "\n%%EOF\n";
    return out;
}

}   // namespace detail
}   // namespace table_export
