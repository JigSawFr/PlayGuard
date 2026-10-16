// Host tests for source/core/console_info_pure.c: the model, region, charger
// and network names, the IP address, hekate's version, the serial number in
// a PRODINFO image and in Atmosphère's backup of it.
#define _POSIX_C_SOURCE 200809L
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "console_info.h"

static void test_names(void)
{
    CHECK(console_model_of(1) == ConsoleModel_Erista && console_model_of(3) == ConsoleModel_Mariko);
    CHECK(console_model_of(4) == ConsoleModel_Lite && console_model_of(6) == ConsoleModel_Oled);
    CHECK(console_model_of(2) == ConsoleModel_Other && console_model_of(5) == ConsoleModel_Other);
    CHECK(console_model_of(0) == ConsoleModel_Unknown && console_model_of(9) == ConsoleModel_Unknown);
    CHECK(!strcmp(console_model_name(ConsoleModel_Oled), "oled") && !console_model_name(ConsoleModel_Unknown));
    CHECK(!strcmp(console_model_name(ConsoleModel_Other), "other") && !console_model_name((ConsoleModel)42));
    CHECK(!strcmp(console_region_name(2), "eur") && !strcmp(console_region_name(4), "htk"));
    CHECK(!console_region_name(-1) && !console_region_name(6));
    CHECK(!strcmp(console_charger_name(1), "official") && !console_charger_name(-1) && !console_charger_name(4));
    CHECK(!strcmp(console_network_name(1), "wifi") && !console_network_name(-1));

    ConsoleLive l;
    console_live_init(&l);
    CHECK(l.battery_pct == -1 && l.charger == -1 && l.docked == -1 && l.temperature_c == -1000);
    CHECK(l.network == -1 && l.wifi_bars == -1 && !l.uptime_s && !l.nickname[0] && l.model == ConsoleModel_Unknown);

    const uint8_t ip[4] = { 192, 168, 1, 20 };
    uint32_t addr;
    memcpy(&addr, ip, 4);
    char text[16];
    console_ip_text(addr, text);
    CHECK(!strcmp(text, "192.168.1.20"));
}

static void test_hekate(void)
{
    uint8_t bin[1100];
    memset(bin, 0xAA, sizeof(bin));
    memcpy(bin + 0x94, "ICTC\x06\x05\x04", 7);
    for (int i = 0; i < 3; i++) bin[0x98 + i] = (uint8_t)('0' + (6 - i));
    char v[16];
    CHECK(console_hekate_version(bin, sizeof(bin), v) && !strcmp(v, "6.5.4"));
    CHECK(!console_hekate_version(bin, 0x90, v));   // cut before the magic
    memset(bin, 0, sizeof(bin));
    memcpy(bin + 1020, "ICTC678", 7);               // past the first kilobyte
    CHECK(!console_hekate_version(bin, sizeof(bin), v));
    memcpy(bin + 10, "ICTC\xff\x01\x02", 7);        // not digits
    CHECK(!console_hekate_version(bin, sizeof(bin), v));
}

static void write_file(const char *path, const void *data, size_t len)
{
    FILE *f = fopen(path, "wb");
    REQUIRE(f != NULL);
    REQUIRE(fwrite(data, 1, len, f) == len);
    fclose(f);
}

static void cal0(uint8_t *buf, size_t len, const char *serial)
{
    memset(buf, 0, len);
    memcpy(buf, "CAL0", 4);
    memcpy(buf + 0x250, serial, strlen(serial));
}

static void test_serial(const char *root)
{
    uint8_t buf[0x300];
    char s[CONSOLE_SERIAL_LEN + 1];
    cal0(buf, sizeof(buf), "XAW10012345678");
    CHECK(console_cal0_serial(buf, sizeof(buf), s) && !strcmp(s, "XAW10012345678"));
    CHECK(!console_cal0_serial(buf, 0x260, s));   // too short for the field
    cal0(buf, sizeof(buf), SYSINFO_BLANK_SERIAL);
    CHECK(!console_cal0_serial(buf, sizeof(buf), s));
    cal0(buf, sizeof(buf), "XAW1001234");
    buf[0x252] = 'w';
    CHECK(!console_cal0_serial(buf, sizeof(buf), s));   // not a serial
    cal0(buf, sizeof(buf), "XAW100");
    CHECK(!console_cal0_serial(buf, sizeof(buf), s));   // too short
    cal0(buf, sizeof(buf), "XAW10012345678");
    buf[0] = 'X';
    CHECK(!console_cal0_serial(buf, sizeof(buf), s));   // not PRODINFO

    // The system's own serial, when not blanked.
    SysInfo si;
    memset(&si, 0, sizeof(si));
    si.serial_valid = true;
    snprintf(si.serial, sizeof(si.serial), "XAJ70000000001");
    CHECK(console_real_serial(&si, root, s) && !strcmp(s, "XAJ70000000001"));
    // Blanked: Atmosphère's backup, else nothing.
    snprintf(si.serial, sizeof(si.serial), SYSINFO_BLANK_SERIAL);
    si.blank_valid = si.blank = true;
    CHECK(!console_real_serial(&si, root, s));
    char dir[512], path[600];
    snprintf(dir, sizeof(dir), "%s/atmosphere", root);
    REQUIRE(mkdir(dir, 0755) == 0);
    snprintf(dir, sizeof(dir), "%s/atmosphere/automatic_backups", root);
    REQUIRE(mkdir(dir, 0755) == 0);
    CHECK(!console_real_serial(&si, root, s));
    snprintf(path, sizeof(path), "%s/notes.txt", dir);
    write_file(path, "PRODINFO", 8);
    snprintf(path, sizeof(path), "%s/XAW00000000000_PRODINFO.bin", dir);
    cal0(buf, sizeof(buf), SYSINFO_BLANK_SERIAL);
    write_file(path, buf, sizeof(buf));   // a backup of a blanked one: no use
    CHECK(!console_real_serial(&si, root, s));
    snprintf(path, sizeof(path), "%s/XAW10012345678_PRODINFO.bin", dir);
    cal0(buf, sizeof(buf), "XAW10012345678");
    write_file(path, buf, sizeof(buf));
    char with_slash[520];
    snprintf(with_slash, sizeof(with_slash), "%s/", root);
    CHECK(console_real_serial(&si, with_slash, s) && !strcmp(s, "XAW10012345678"));
    // A system serial that only looks blank (no Atmosphère flag): the backup too.
    si.blank_valid = si.blank = false;
    CHECK(console_real_serial(&si, root, s) && !strcmp(s, "XAW10012345678"));
}

static void test_hekate_file(const char *root)
{
    char v[16], path[512];
    CHECK(!console_hekate(root, v));
    snprintf(path, sizeof(path), "%s/bootloader", root);
    REQUIRE(mkdir(path, 0755) == 0);
    snprintf(path, sizeof(path), "%s/bootloader/update.bin", root);
    uint8_t bin[256];
    memset(bin, 0, sizeof(bin));
    memcpy(bin + 0x94, "ICTC654", 7);
    write_file(path, bin, sizeof(bin));
    CHECK(console_hekate(root, v) && !strcmp(v, "6.5.4"));
}

int main(void)
{
    char root[] = "/tmp/playguard_console_info_XXXXXX";
    REQUIRE(mkdtemp(root) != NULL);
    test_names();
    test_hekate();
    test_serial(root);
    test_hekate_file(root);
    char cleanup[600];
    snprintf(cleanup, sizeof(cleanup), "rm -rf '%s'", root);
    CHECK(system(cleanup) == 0);
    return CHECK_DONE("console_info: names, IP, hekate, PRODINFO serial and its backup assertions passed");
}
