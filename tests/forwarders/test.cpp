// Host tests for Console › Forwarders' UI-free parts: util/nro.cpp (an .nro's
// icon and NACP, a JPEG's size, the .nro files of /switch), util/
// forwarders.cpp (what PlayGuard remembers making: saved, replaced,
// forgotten, a damaged file) and action/forwarders_logic.cpp (the rows from
// the HOME menu and the file, a new forwarder's name, whether the console
// can start one).
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"
#include <cstring>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"
#include "util/paths.hpp"

namespace
{

void put32(std::vector<uint8_t>& b, size_t at, uint32_t v)
{
    for (int i = 0; i < 4; i++) b[at + i] = (uint8_t)(v >> (8 * i));
}

void put64(std::vector<uint8_t>& b, size_t at, uint64_t v)
{
    for (int i = 0; i < 8; i++) b[at + i] = (uint8_t)(v >> (8 * i));
}

// The start of a JPEG: SOI, an APP0, a frame header of `w` x `h`.
std::vector<uint8_t> jpeg(int w, int h, uint8_t sof = 0xC0)
{
    std::vector<uint8_t> j = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0,
                               0xFF, sof, 0x00, 0x11, 0x08, (uint8_t)(h >> 8), (uint8_t)h, (uint8_t)(w >> 8), (uint8_t)w, 3 };
    j.resize(j.size() + 9, 0x11);
    j.push_back(0xFF);
    j.push_back(0xD9);
    return j;
}

// An .nro as elf2nro writes one: 0x80 bytes of program (its header at 0x10),
// then the asset section with `icon` and a NACP naming `name` (none when
// empty), then a RomFS.
std::vector<uint8_t> make_nro(const std::vector<uint8_t>& icon, const std::string& name, const std::string& author = "",
                              const std::string& version = "")
{
    const size_t program = 0x100;
    std::vector<uint8_t> b(program, 0);
    std::memcpy(b.data() + 0x10, "NRO0", 4);
    put32(b, 0x18, (uint32_t)program);
    std::vector<uint8_t> nacp;
    if (!name.empty()) {
        nacp.assign(nro::NACP_SIZE, 0);
        std::memcpy(nacp.data() + 2 * 0x300, name.data(), name.size());   // a later language only
        std::memcpy(nacp.data() + 2 * 0x300 + 0x200, author.data(), author.size());
        std::memcpy(nacp.data() + 0x3060, version.data(), version.size());
    }
    std::vector<uint8_t> asset(0x38, 0);
    std::memcpy(asset.data(), "ASET", 4);
    put64(asset, 8, 0x38);
    put64(asset, 0x10, icon.size());
    put64(asset, 0x18, 0x38 + icon.size());
    put64(asset, 0x20, nacp.size());
    put64(asset, 0x28, 0x38 + icon.size() + nacp.size());
    put64(asset, 0x30, 0x1000);
    b.insert(b.end(), asset.begin(), asset.end());
    b.insert(b.end(), icon.begin(), icon.end());
    b.insert(b.end(), nacp.begin(), nacp.end());
    b.resize(b.size() + 0x1000, 0xEE);   // the RomFS
    return b;
}

void write_bytes(const std::string& path, const std::vector<uint8_t>& data)
{
    fs::write(path, std::string(data.begin(), data.end()));
}

}   // namespace

static void test_jpeg()
{
    int w = 0, h = 0;
    CHECK(nro::jpeg_size(jpeg(256, 256), w, h) && w == 256 && h == 256);
    CHECK(nro::jpeg_size(jpeg(320, 200, 0xC2), w, h) && w == 320 && h == 200);   // progressive
    CHECK(!nro::jpeg_size({ 0x89, 'P', 'N', 'G' }, w, h));
    CHECK(!nro::jpeg_size({ 0xFF, 0xD8, 0xFF, 0xDA, 0x00, 0x02 }, w, h));        // image data, no frame
    std::vector<uint8_t> cut = jpeg(256, 256);
    cut.resize(24);
    CHECK(!nro::jpeg_size(cut, w, h));
    CHECK(nro::icon_usable(jpeg(256, 256)));
    CHECK(!nro::icon_usable(jpeg(128, 128)));
    CHECK(!nro::icon_usable({}));
    std::vector<uint8_t> big = jpeg(256, 256);
    big.resize(nro::ICON_MAX + 1, 0);
    CHECK(!nro::icon_usable(big));
}

static void test_parse()
{
    nro::Info info;
    const std::vector<uint8_t> icon = jpeg(256, 256);
    CHECK(nro::parse(make_nro(icon, "ftpd pro", "mtheall", "3.2.0"), info));
    CHECK(info.icon == icon && info.nacp.size() == nro::NACP_SIZE);
    CHECK(info.name == "ftpd pro" && info.author == "mtheall" && info.version == "3.2.0");

    CHECK(nro::parse(make_nro(icon, ""), info));   // no NACP
    CHECK(info.icon == icon && info.nacp.empty() && info.name.empty());

    std::vector<uint8_t> no_assets = make_nro(icon, "x");
    no_assets.resize(0x100);   // the program only
    CHECK(nro::parse(no_assets, info) && info.icon.empty() && info.nacp.empty());

    std::vector<uint8_t> bad = make_nro(icon, "x");
    std::memcpy(bad.data() + 0x10, "NSO0", 4);
    CHECK(!nro::parse(bad, info));
    bad = make_nro(icon, "x");
    put32(bad, 0x18, 0x7FFFFFFF);   // a size beyond the file
    CHECK(!nro::parse(bad, info));
    CHECK(!nro::parse({}, info));

    // A name that is no UTF-8 is not shown.
    CHECK(nro::parse(make_nro(icon, "\xFF\xFE", "a"), info) && info.name.empty());
}

static void test_files()
{
    TempDir tmp("forwarders_nro");
    const std::string root = tmp.path();
    const std::vector<uint8_t> icon = jpeg(256, 256);
    write_bytes(root + "/switch/ftpd/ftpd.nro", make_nro(icon, "ftpd", "mtheall"));
    write_bytes(root + "/switch/Alpha.NRO", make_nro(icon, "Alpha"));
    write_bytes(root + "/switch/.overlays/ovl.nro", make_nro(icon, "o"));
    write_bytes(root + "/switch/deep/deeper/x.nro", make_nro(icon, "x"));
    write_bytes(root + "/switch/notes.txt", { 'a' });
    write_bytes(root + "/top.nro", make_nro(icon, "t"));
    const auto found = nro::find(root);
    CHECK(found.size() == 2);
    if (found.size() == 2) CHECK(found[0] == "/switch/Alpha.NRO" && found[1] == "/switch/ftpd/ftpd.nro");
    CHECK(nro::find(root + "/nothing").empty());

    nro::Info info;
    CHECK(nro::read(root + "/switch/ftpd/ftpd.nro", info) && info.name == "ftpd" && info.icon == icon);
    CHECK(!nro::read(root + "/switch/notes.txt", info));
    CHECK(!nro::read(root + "/missing.nro", info));

    CHECK(nro::sdmc_path("/switch/x.nro") == "sdmc:/switch/x.nro");
    CHECK(nro::sdmc_path("switch/x.nro").empty());
    CHECK(nro::sd_path("sdmc:/switch/x.nro") == "/switch/x.nro");
    CHECK(nro::sd_path("/switch/x.nro").empty());
}

static void test_registry()
{
    TempDir tmp("forwarders_reg", TempDir::Enter);
    CHECK(forwarders::list().empty());
    forwarders::Made a;
    a.id = 0x050537E2542E2000ull;
    a.nro = a.argv = "sdmc:/switch/playguard/playguard.nro";
    a.name = "PlayGuard";
    a.author = "JigSawFr";
    CHECK(forwarders::save(a));
    forwarders::Made b = a;
    b.id = 0x0512345678ABC000ull;
    b.nro = b.argv = "sdmc:/switch/ftpd.nro";
    b.name = "ftpd é";
    CHECK(forwarders::save(b));
    auto all = forwarders::list();
    CHECK(all.size() == 2 && all[0].name == "PlayGuard" && all[1].name == "ftpd é");

    a.name = "Parental";   // edited: replaced in place
    CHECK(forwarders::save(a));
    all = forwarders::list();
    CHECK(all.size() == 2 && all[0].name == "Parental");
    forwarders::Made found;
    CHECK(forwarders::find(b.id, &found) && found.nro == b.nro);
    CHECK(!forwarders::find(1, &found));

    CHECK(forwarders::forget(a.id));
    all = forwarders::list();
    CHECK(all.size() == 1 && all[0].id == b.id);
    CHECK(forwarders::forget(0x0599999999999000ull));   // none: nothing to do
    CHECK(forwarders::list().size() == 1);

    // A damaged file reads as none; a broken entry is skipped, a missing
    // argv is the .nro's path.
    const std::string file = paths::nand_dir() + "/forwarders.json";
    fs::write(file, "{ not json");
    CHECK(forwarders::list().empty());
    fs::write(file, R"({"forwarders": [{"id": "zz", "nro": "sdmc:/a.nro"}, {"id": "0512345678abc000"},
                       {"id": "0512345678abc000", "nro": "sdmc:/b.nro", "name": "  B\u0001  b  "}, 7,
                       {"id": "0512345678abc000", "nro": "sdmc:/dup.nro"}]})");
    all = forwarders::list();
    CHECK(all.size() == 1);
    if (all.size() == 1) CHECK(all[0].argv == "sdmc:/b.nro" && all[0].name == "B b");
}

static void test_text()
{
    using forwarders::clean_text;
    CHECK(clean_text("  My   game  ", 64) == "My game");
    CHECK(clean_text("a\tb\nc", 64) == "abc");
    CHECK(clean_text("\xFF" "ok", 64) == "ok");
    CHECK(clean_text("Pokémon", 4) == "Poké");
    CHECK(clean_text("a/b", 64) == "a/b");
    CHECK(forwarders::hex_id(0x050537E2542E2000ull) == "050537e2542e2000");
    uint64_t id = 0;
    CHECK(forwarders::parse_id("050537E2542E2000", &id) && id == 0x050537E2542E2000ull);
    CHECK(!forwarders::parse_id("050537e2542e200", &id));
    CHECK(!forwarders::parse_id("050537e2542e200g", &id));
    CHECK(!forwarders::parse_id("0000000000000000", &id));
}

static void test_merge()
{
    using namespace forwarders_logic;
    forwarders::Made own, gone, moved;
    own.id = 3;
    own.nro = own.argv = "sdmc:/switch/playguard/playguard.nro";
    own.name = "PlayGuard (made)";
    gone.id = 4;
    gone.nro = gone.argv = "sdmc:/switch/gone.nro";
    gone.name = "zeta";
    moved.id = 5;
    moved.nro = moved.argv = "sdmc:/switch/moved/moved.nro";
    std::vector<Installed> on;
    on.push_back({ 3, "PlayGuard", "JigSawFr", "1.4.0" });
    on.push_back({ 9, "", "", "" });              // another tool's, no name read
    on.push_back({ 5, "", "", "" });
    on.push_back({ 7, "checkpoint", "BG", "3.10" });
    on.push_back({ 7, "dup", "", "" });
    const auto exists = [](const std::string& p) { return p != "sdmc:/switch/moved/moved.nro"; };
    const auto rows = merge({ own, gone, moved }, on, exists, "sdmc:/switch/playguard/playguard.nro");
    if (!CHECK(rows.size() == 5)) return;
    // By name, ignoring case: 0000000000000009 < checkpoint < moved < PlayGuard < zeta.
    CHECK(rows[0].id == 9 && rows[0].state == State::Elsewhere && rows[0].name == "0000000000000009" && rows[0].nro.empty());
    CHECK(rows[1].id == 7 && rows[1].name == "checkpoint" && rows[1].state == State::Elsewhere);
    CHECK(rows[2].id == 5 && rows[2].name == "moved" && rows[2].state == State::NroMissing);
    CHECK(rows[3].id == 3 && rows[3].name == "PlayGuard" && rows[3].state == State::Ready && rows[3].self);
    CHECK(rows[3].version == "1.4.0" && rows[3].author == "JigSawFr");
    CHECK(rows[4].id == 4 && rows[4].state == State::NotInstalled && rows[4].name == "zeta" && !rows[4].self);
    const Row* self = self_row(rows);
    CHECK(self && self->id == 3);
    CHECK(!self_row(merge({}, on, exists, "")));

    CHECK(file_title("sdmc:/switch/ftpd/ftpd.nro") == "ftpd");
    CHECK(file_title("x.NRO") == "x");
    CHECK(file_title("sdmc:/a/b.txt") == "b.txt");
    CHECK(default_name("  Goldleaf ", "sdmc:/switch/Goldleaf.nro") == "Goldleaf");
    CHECK(default_name("", "sdmc:/switch/hb/hb-menu.nro") == "hb-menu");
    CHECK(default_name(std::string(100, 'n'), "x").size() == forwarders::MAX_NAME);
}

static void test_patches()
{
    using forwarders_logic::Patches;
    using forwarders_logic::patches_state;
    patches::Report r;
    CHECK(patches_state(r) == Patches::Missing);
    r.files = { "es" };
    CHECK(patches_state(r) == Patches::Missing);
    r.files = { "es", "fs", "loader" };
    CHECK(patches_state(r) == Patches::Ready);
    r.files = { "hekate" };
    CHECK(patches_state(r) == Patches::Ready);

    patches::Report s;
    s.syspatch_installed = true;
    CHECK(patches_state(s) == Patches::Missing);   // not at boot
    s.syspatch_at_boot = true;
    CHECK(patches_state(s) == Patches::Unknown);   // no log
    s.log_present = s.log_current = true;
    s.fs = s.ldr = s.es = patches::Module::SysPatch;
    CHECK(patches_state(s) == Patches::Ready);
    s.es = patches::Module::Failed;   // es does not matter to a forwarder
    CHECK(patches_state(s) == Patches::Ready);
    s.ldr = patches::Module::Unpatched;
    CHECK(patches_state(s) == Patches::Missing);
    s.fs = s.ldr = s.es = patches::Module::Skipped;
    CHECK(patches_state(s) == Patches::Missing);
}

int main()
{
    test_jpeg();
    test_parse();
    test_files();
    test_registry();
    test_text();
    test_merge();
    test_patches();
    return CHECK_DONE("forwarders tests passed");
}
