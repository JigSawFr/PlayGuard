// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/prodinfo.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <set>

#include "core/sysinfo.h"
#include "util/paths.hpp"

namespace prodinfo
{

namespace
{
constexpr size_t kSerialOffset  = 0x250;
constexpr size_t kSerialSize    = 0x18;
constexpr size_t kEccCertOffset = 0x480;
constexpr size_t kRsaCertOffset = 0x3FC0;
constexpr size_t kDeviceIdInCert = 0xC6;   // 16 hex digits in the certificate
constexpr uint64_t kDeviceIdMask = 0x00FFFFFFFFFFFFFFull;

bool is_cal0(const std::string& data)
{
    return data.size() >= kSerialOffset + kSerialSize && data.starts_with("CAL0");
}

bool device_id_at(const std::string& data, size_t cert, uint64_t* out)
{
    const size_t at = cert + kDeviceIdInCert;
    if (data.size() < at + 16) return false;
    char hex[17] = {};
    std::memcpy(hex, data.data() + at, 16);
    for (int i = 0; i < 16; i++)
        if (!std::isxdigit((unsigned char)hex[i])) return false;
    *out = std::strtoull(hex, nullptr, 16) & kDeviceIdMask;
    return true;
}
}   // namespace

std::string serial_of(const std::string& cal0)
{
    if (!is_cal0(cal0)) return "";
    std::string s(cal0.data() + kSerialOffset, kSerialSize);
    s = s.substr(0, s.find('\0'));
    if (s.empty() || s == SYSINFO_BLANK_SERIAL) return "";
    for (char c : s)
        if (!std::isalnum((unsigned char)c)) return "";
    return s;
}

bool device_id_of(const std::string& cal0, uint64_t* out)
{
    if (!is_cal0(cal0)) return false;
    // Newer consoles may leave the ECC certificate empty (Atmosphère's
    // ContainsCorrectDeviceId): then the RSA one.
    return device_id_at(cal0, kEccCertOffset, out) || device_id_at(cal0, kRsaCertOffset, out);
}

Found find(const std::string& root, bool device_id_valid, uint64_t device_id)
{
    const std::string rel = "atmosphere/automatic_backups";
    Found found;
    std::set<std::string> serials;
    std::string single_file;
    bool any = false;
    for (const std::string& name : paths::list_files(root + "/" + rel, ".bin")) {
        if (!name.contains("PRODINFO")) continue;
        std::string data;
        if (!paths::read_file(root + "/" + rel + "/" + name, data)) continue;
        const std::string serial = serial_of(data);
        if (serial.empty()) continue;   // BLANK_ / INVALID_ backups
        any = true;
        uint64_t id = 0;
        if (device_id_valid) {
            if (device_id_of(data, &id) && id == (device_id & kDeviceIdMask)) {
                found.source = Source::Matched;
                found.serial = serial;
                found.file   = rel + "/" + name;
                return found;
            }
            continue;
        }
        if (serials.insert(serial).second) single_file = rel + "/" + name;
    }
    if (!any) return found;
    if (device_id_valid) {
        found.source = Source::Others;
    } else if (serials.size() == 1) {
        found.source = Source::Single;
        found.serial = *serials.begin();
        found.file   = single_file;
    } else {
        found.source = Source::Ambiguous;
    }
    return found;
}

}   // namespace prodinfo
