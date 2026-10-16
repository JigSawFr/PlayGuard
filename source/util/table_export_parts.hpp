// table_export_parts — what the files of util/table_export share: one
// renderer per format (table_export_<format>.cpp) and the cell helpers.
// Internal to table_export: include table_export.hpp instead.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "util/table_export.hpp"

namespace table_export
{
namespace detail
{

// row[i], or "" past its end.
std::string cell(const std::vector<std::string>& row, size_t i);
// A decimal integer that fits in 64 bits ("-12", not "1.5", not "").
bool        is_integer(const std::string& s);
std::tm     local_now();

std::string render_csv(const Table& t);
std::string render_json(const Table& t);
std::string render_xlsx(const Table& t);   // binary
std::string render_pdf(const Table& t);    // binary

}   // namespace detail
}   // namespace table_export
