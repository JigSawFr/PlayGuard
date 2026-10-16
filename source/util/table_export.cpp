// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/table_export.hpp"

#include <ctime>
#include <sys/stat.h>

#include "util/paths.hpp"
#include "util/table_export_parts.hpp"

namespace table_export
{

namespace detail
{
std::string cell(const std::vector<std::string>& row, size_t i)
{
    return i < row.size() ? row[i] : std::string();
}

bool is_integer(const std::string& s)
{
    size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
    if (i >= s.size() || s.size() - i > 18) return false;
    for (; i < s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

std::tm local_now()
{
    std::time_t now = std::time(nullptr);
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    return tmv;
}
}   // namespace detail

const char* extension(Format f)
{
    switch (f) {
        case Format::Csv:  return "csv";
        case Format::Json: return "json";
        case Format::Xlsx: return "xlsx";
        case Format::Pdf:  return "pdf";
    }
    return "txt";
}

std::string render(const Table& t, Format f)
{
    switch (f) {
        case Format::Csv:  return detail::render_csv(t);
        case Format::Json: return detail::render_json(t);
        case Format::Xlsx: return detail::render_xlsx(t);
        case Format::Pdf:  return detail::render_pdf(t);
    }
    return "";
}

std::string save(const Table& t, Format f, const std::string& dir, const std::string& base, std::string* error)
{
    const std::tm now = detail::local_now();
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &now);
    const std::string content = render(t, f);
    for (unsigned i = 0; i < 100; ++i) {
        std::string path = dir + "/" + base + "_" + stamp;
        if (i > 0) path += "_" + std::to_string(i);
        path += std::string(".") + extension(f);
        struct stat st;
        if (stat(path.c_str(), &st) == 0) continue;   // taken (not read: an export can be large)
        return paths::atomic_write(path, content, error) ? path : "";
    }
    if (error) *error = "No free file name";
    return "";
}

}   // namespace table_export
