// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/table_export_parts.hpp"

#include <cstdint>

#include <aurore/cadran/util/utf8.hpp>

#include "util/zip_write.hpp"

namespace table_export
{
namespace detail
{

namespace
{
// Allowed in XML 1.0: no control character but tab and line ends, no U+FFFE / U+FFFF.
bool xml_char(uint32_t cp)
{
    if (cp < 0x20) return cp == '\t' || cp == '\n' || cp == '\r';
    return cp != 0xFFFE && cp != 0xFFFF;
}

// Badly encoded bytes become U+FFFD, characters XML does not allow are left out.
std::string xml_escape(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size();) {
        uint32_t cp;
        const size_t len = utf8::decode_replacing(s, i, &cp);
        const bool bad = cp == 0xFFFD && len == 1;
        i += len;
        if (bad) { out += "\xEF\xBF\xBD"; continue; }
        if (!xml_char(cp)) continue;
        switch (cp) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += s.substr(i - len, len);
        }
    }
    return out;
}

std::string column_letters(size_t i)
{
    std::string s;
    for (++i; i > 0; i = (i - 1) / 26) s.insert(s.begin(), (char)('A' + (i - 1) % 26));
    return s;
}

// Excel refuses []:*?/\ in a sheet name, an apostrophe first or last, and
// more than 31 characters (UTF-16 units). Cut on whole characters; badly
// encoded bytes and characters XML does not allow are left out.
std::string sheet_name(const std::string& in)
{
    std::string out;
    size_t units = 0;
    for (size_t i = 0; i < in.size();) {
        uint32_t cp;
        const size_t len = utf8::decode_replacing(in, i, &cp);
        i += len;
        if ((cp == 0xFFFD && len == 1) || !xml_char(cp) || cp < 0x20) continue;
        if (cp < 0x80 && std::string("[]:*?/\\").find((char)cp) != std::string::npos) continue;
        if (out.empty() && cp == '\'') continue;
        const size_t w = cp >= 0x10000 ? 2 : 1;
        if (units + w > 31) break;
        units += w;
        out += in.substr(i - len, len);
    }
    while (!out.empty() && out.back() == '\'') out.pop_back();
    return out.empty() ? "Sheet1" : out;
}

std::string sheet_xml(const Table& t)
{
    std::string x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                    "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><cols>";
    for (size_t i = 0; i < t.columns.size(); i++)
        x += "<col min=\"" + std::to_string(i + 1) + "\" max=\"" + std::to_string(i + 1) + "\" width=\"" +
             (i == 0 ? "40" : "18") + "\" customWidth=\"1\"/>";
    x += "</cols><sheetData>";

    auto text = [](size_t col, size_t row, const std::string& v, bool bold) {
        return "<c r=\"" + column_letters(col) + std::to_string(row) + "\" t=\"inlineStr\"" + (bold ? " s=\"1\"" : "") +
               "><is><t xml:space=\"preserve\">" + xml_escape(v) + "</t></is></c>";
    };
    // Title, subtitle, a blank row, then the header and the data.
    x += "<row r=\"1\">" + text(0, 1, t.title, true) + "</row>";
    x += "<row r=\"2\">" + text(0, 2, t.subtitle, false) + "</row>";
    x += "<row r=\"4\">";
    for (size_t i = 0; i < t.columns.size(); i++) x += text(i, 4, t.columns[i].label, true);
    x += "</row>";
    size_t r = 5;
    for (const auto& row : t.rows) {
        x += "<row r=\"" + std::to_string(r) + "\">";
        for (size_t i = 0; i < t.columns.size(); i++) {
            const std::string v = cell(row, i);
            if (v.empty()) continue;
            if (t.columns[i].numeric && is_integer(v))
                x += "<c r=\"" + column_letters(i) + std::to_string(r) + "\"><v>" + v + "</v></c>";
            else
                x += text(i, r, v, false);
        }
        x += "</row>";
        r++;
    }
    return x + "</sheetData></worksheet>";
}
}   // namespace

std::string render_xlsx(const Table& t)
{
    const std::string head = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    const std::string rel  = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";
    const std::string doc  = "application/vnd.openxmlformats-officedocument.spreadsheetml.";
    return zip_write::stored(local_now(), {
        { "[Content_Types].xml",
          head + "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                 "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                 "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                 "<Override PartName=\"/xl/workbook.xml\" ContentType=\"" + doc + "sheet.main+xml\"/>"
                 "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"" + doc + "worksheet+xml\"/>"
                 "<Override PartName=\"/xl/styles.xml\" ContentType=\"" + doc + "styles+xml\"/></Types>" },
        { "_rels/.rels",
          head + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                 "<Relationship Id=\"rId1\" Type=\"" + rel + "officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>" },
        { "xl/workbook.xml",
          head + "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
                 "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets>"
                 "<sheet name=\"" + xml_escape(sheet_name(t.sheet)) + "\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>" },
        { "xl/_rels/workbook.xml.rels",
          head + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                 "<Relationship Id=\"rId1\" Type=\"" + rel + "worksheet\" Target=\"worksheets/sheet1.xml\"/>"
                 "<Relationship Id=\"rId2\" Type=\"" + rel + "styles\" Target=\"styles.xml\"/></Relationships>" },
        { "xl/styles.xml",
          head + "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                 "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                 "<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                 "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill>"
                 "<fill><patternFill patternType=\"gray125\"/></fill></fills>"
                 "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                 "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                 "<cellXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
                 "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/></cellXfs>"
                 "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
                 "</styleSheet>" },
        { "xl/worksheets/sheet1.xml", sheet_xml(t) },
    });
}

}   // namespace detail
}   // namespace table_export
