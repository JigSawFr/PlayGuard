// export_flow — "Export" of the Activity tab and the History screen: the
// format asked (the one chosen last selected, and remembered), then a table
// saved to exports/ (util/table_export.hpp), encoded and written off the UI
// thread; a toast says where, a dialog why not.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <functional>
#include <string>

#include "util/table_export.hpp"

namespace export_flow
{

// `make` builds the table once a format is picked (UI thread); it is saved
// as exports/<stem>_<date and time>.<ext>. `what` (a string literal) names
// the write in the log.
void choose(const char* what, const std::string& stem, std::function<table_export::Table()> make);

}   // namespace export_flow
