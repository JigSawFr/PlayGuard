// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/system_titles.hpp"

namespace system_titles
{

namespace
{
// Those a parent may meet, from switchbrew's title list; the others have no
// name a parent would know (a keyboard, an error screen).
const struct
{
    uint64_t    id;
    const char* key;
} TITLES[] = {
    { 0x0100000000001000ULL, "home" },          // qlaunch: the HOME menu (and System Settings, News)
    { 0x0100000000001002ULL, "amiibo" },        // cabinet
    { 0x0100000000001003ULL, "controllers" },   // controller
    { 0x0100000000001007ULL, "user_select" },   // playerSelect
    { 0x0100000000001009ULL, "mii" },           // miiEdit
    { 0x010000000000100AULL, "web" },           // LibAppletWeb
    { 0x010000000000100BULL, "eshop" },         // LibAppletShop
    { 0x010000000000100DULL, "album" },         // photoViewer
    { 0x010000000000100FULL, "web" },           // LibAppletOff (offline web)
    { 0x0100000000001013ULL, "user_page" },     // myPage
    { 0x0100000000001015ULL, "maintenance" },   // maintenance (Maintenance Mode)
    { 0x0100000000001042ULL, "web" },           // systemWeb, 22.0.0+
    { 0x0100000000001043ULL, "web" },           // openWeb, 22.0.0+
};
}   // namespace

const char* key(uint64_t app_id)
{
    for (const auto& t : TITLES)
        if (t.id == app_id) return t.key;
    return nullptr;
}

}   // namespace system_titles
