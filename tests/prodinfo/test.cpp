// Host tests for source/util/prodinfo.cpp: the serial number and the
// DeviceId in a CAL0 image, and which backup of atmosphere/automatic_backups/
// is this console's, on fake SD card trees.
#include "check.h"
#include "temp_dir.hpp"
#include <cstdio>
#include <cstring>
#include <string>

#include "util/paths.hpp"
#include "util/prodinfo.hpp"

using namespace prodinfo;

static int g_n = 0;
static std::string g_base;

static std::string fresh_root()
{
    std::string root = g_base + "/sd" + std::to_string(g_n++);
    CHECK(paths::ensure_dir(root));
    return root;
}

// A CAL0 image of 0x8000 bytes with `serial` at 0x250 and `device_id` as 16
// hex digits in the ECC certificate (0x480 + 0xC6), or the RSA one.
static std::string cal0(const char* serial, uint64_t device_id, bool rsa = false)
{
    std::string d(0x8000, '\0');
    std::memcpy(&d[0], "CAL0", 4);
    std::memcpy(&d[0x250], serial, std::strlen(serial));
    char hex[17];
    std::snprintf(hex, sizeof(hex), "%016llX", (unsigned long long)device_id);
    std::memcpy(&d[(rsa ? 0x3FC0 : 0x480) + 0xC6], hex, 16);
    return d;
}

static void put(const std::string& root, const std::string& name, const std::string& data)
{
    CHECK(paths::atomic_write(root + "/atmosphere/automatic_backups/" + name, data));
}

int main()
{
    TempDir tmp("prodinfo");
    g_base = tmp.path();

    // One image.
    CHECK(serial_of(cal0("XAW10012345678", 1)) == "XAW10012345678");
    CHECK(serial_of(cal0("XAW00000000000", 1)).empty());       // blanked
    CHECK(serial_of(cal0("", 1)).empty());
    CHECK(serial_of(cal0("XAW1-0012", 1)).empty());            // not alphanumeric
    CHECK(serial_of(std::string("CAL0", 4)).empty());          // too short
    std::string bad = cal0("XAW10012345678", 1);
    bad[0] = 'X';
    CHECK(serial_of(bad).empty());
    uint64_t id = 0;
    CHECK(device_id_of(cal0("XAW1", 0x0123456789ABCDEFull), &id) && id == 0x0023456789ABCDEFull);   // low 56 bits
    CHECK(device_id_of(cal0("XAW1", 0x42, true), &id) && id == 0x42);                                 // RSA certificate
    CHECK(!device_id_of(std::string(0x8000, '\0').replace(0, 4, "CAL0"), &id));

    // No folder, no backup.
    std::string root = fresh_root();
    CHECK(find(root, true, 7).source == Source::None);
    put(root, "BLANK_PRODINFO_01020304.bin", cal0("XAW00000000000", 7));
    CHECK(find(root, true, 7).source == Source::None);

    // This console's backup among another's (the card moved between two).
    root = fresh_root();
    put(root, "XAW10000000001_PRODINFO.bin", cal0("XAW10000000001", 1));
    put(root, "XAW70000000007_PRODINFO.bin", cal0("XAW70000000007", 7));
    put(root, "notes.txt", "x");
    Found f = find(root, true, 0xFF00000000000007ull);   // the high byte is not compared
    CHECK(f.source == Source::Matched && f.serial == "XAW70000000007");
    CHECK(f.file == "atmosphere/automatic_backups/XAW70000000007_PRODINFO.bin");
    // Only other consoles'.
    f = find(root, true, 9);
    CHECK(f.source == Source::Others && f.serial.empty());
    // DeviceId unknown: two consoles, no guess.
    CHECK(find(root, false, 0).source == Source::Ambiguous);

    // DeviceId unknown, one console (a backup and its hashed copy).
    root = fresh_root();
    put(root, "XAW10000000001_PRODINFO.bin", cal0("XAW10000000001", 1));
    put(root, "XAW10000000001_PRODINFO_A1B2C3D4.bin", cal0("XAW10000000001", 1));
    f = find(root, false, 0);
    CHECK(f.source == Source::Single && f.serial == "XAW10000000001");

    return CHECK_DONE("prodinfo tests passed");
}
