// Host tests for Console › Forwarders' UI-free parts: util/nro.cpp (an .nro's
// icon and NACP, a JPEG's size, the .nro files of /switch), util/
// forwarders.cpp (what PlayGuard remembers making: saved, replaced,
// forgotten, a damaged file), util/forwarder_icon.cpp (any icon made the
// HOME menu's 256 x 256 JPEG) and action/forwarders_logic.cpp (the rows from
// the HOME menu and the file, a new forwarder's name, whether the console
// can start one), util/forwarder_gate.cpp (the argv of a forwarder that
// asks for the PIN).
#include "check.h"
#include "fs.hpp"
#include "temp_dir.hpp"
#include <cstring>
#include <string>
#include <vector>

#include "action/forwarders_logic.hpp"
#include "util/forwarder_gate.hpp"
#include "util/forwarder_icon.hpp"
#include "util/forwarders.hpp"
#include "util/nro.hpp"

#include <stb_image.h>
#include <stb_image_write.h>
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

// The start of a JPEG: SOI, an APP0, a frame header of `w` x `h`, the start
// of its image data.
std::vector<uint8_t> jpeg(int w, int h, uint8_t sof = 0xC0)
{
    std::vector<uint8_t> j = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0,
                               0xFF, sof, 0x00, 0x11, 0x08, (uint8_t)(h >> 8), (uint8_t)h, (uint8_t)(w >> 8), (uint8_t)w, 3 };
    j.resize(j.size() + 9, 0x11);
    j.insert(j.end(), { 0xFF, 0xDA, 0x00, 0x02, 0x00, 0xFF, 0xD9 });
    return j;
}

// `j` with an APP1 (EXIF) segment after its SOI.
std::vector<uint8_t> with_exif(std::vector<uint8_t> j)
{
    j.insert(j.begin() + 2, { 0xFF, 0xE1, 0x00, 0x08, 'E', 'x', 'i', 'f', 0, 0 });
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

    // Only what the HOME menu decodes: no EXIF, no progressive coding.
    CHECK(nro::jpeg_plain(jpeg(256, 256)) && nro::jpeg_plain(jpeg(256, 256, 0xC1)));
    CHECK(!nro::jpeg_plain(jpeg(256, 256, 0xC2)) && !nro::icon_usable(jpeg(256, 256, 0xC2)));
    CHECK(!nro::jpeg_plain(with_exif(jpeg(256, 256))) && !nro::icon_usable(with_exif(jpeg(256, 256))));
    std::vector<uint8_t> comment = jpeg(256, 256);
    comment.insert(comment.begin() + 2, { 0xFF, 0xFE, 0x00, 0x04, 'h', 'i' });
    CHECK(nro::jpeg_plain(comment));
    std::vector<uint8_t> no_data = jpeg(256, 256);
    no_data.resize(no_data.size() - 7);
    CHECK(!nro::jpeg_plain(no_data));                                            // no image data
    CHECK(!nro::jpeg_plain({ 0xFF, 0xD8, 0xFF, 0xDA, 0x00, 0x02 }));            // image data, no frame
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

    // Protected with the PIN: its gate kept; a flag without a gate is none.
    b.pin = true;
    b.gate = "sdmc:/switch/playguard/playguard.nro";
    CHECK(forwarders::save(b));
    CHECK(forwarders::find(b.id, &found) && found.pin && found.gate == b.gate);
    CHECK(forwarders::find(a.id, &found) && !found.pin && found.gate.empty());
    b.pin = false;
    CHECK(forwarders::save(b));
    CHECK(forwarders::find(b.id, &found) && !found.pin && found.gate.empty());
    // Told gone once: kept, and dropped once it is false again.
    b.gone = true;
    CHECK(forwarders::save(b));
    CHECK(forwarders::find(b.id, &found) && found.gone);
    b.gone = false;
    CHECK(forwarders::save(b));
    CHECK(forwarders::find(b.id, &found) && !found.gone);

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
                       {"id": "0512345678abc000", "nro": "sdmc:/dup.nro"},
                       {"id": "0512345678abd000", "nro": "sdmc:/c.nro", "pin": true}]})");
    all = forwarders::list();
    CHECK(all.size() == 2);
    if (all.size() == 2) CHECK(all[0].argv == "sdmc:/b.nro" && all[0].name == "B b" && !all[1].pin);
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
    CHECK(!rows[3].pin && !rows[2].pin);
    // Protected with the PIN: its gate gone, it would not start either.
    forwarders::Made locked = gone;
    locked.pin = true;
    locked.gate = "sdmc:/switch/moved/moved.nro";
    std::vector<Installed> one = { { 4, "zeta", "", "" } };
    auto pinned = merge({ locked }, one, exists, "");
    CHECK(pinned.size() == 1 && pinned[0].pin && pinned[0].state == State::GateMissing);
    locked.gate = "sdmc:/switch/playguard/playguard.nro";
    pinned = merge({ locked }, one, exists, "");
    CHECK(pinned.size() == 1 && pinned[0].pin && pinned[0].state == State::Ready);
    pinned = merge({ locked }, {}, exists, "");
    CHECK(pinned.size() == 1 && pinned[0].pin && pinned[0].state == State::NotInstalled && !pinned[0].gone_seen);
    locked.gone = true;
    pinned = merge({ locked }, {}, exists, "");
    CHECK(pinned.size() == 1 && pinned[0].gone_seen);
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

static void test_lock_check()
{
    using namespace forwarders_logic;
    CHECK(is_menu("sdmc:/hbmenu.nro") && is_menu("sdmc:/switch/sphaira/sphaira.nro") && is_menu("sdmc:/x/HBMENU.NRO"));
    CHECK(!is_menu("sdmc:/switch/cyberfoil.nro") && !is_menu("") && !is_menu("sdmc:/switch/sphaira/old_sphaira.nro"));

    Row self;
    self.id = 1;
    self.name = "PlayGuard";
    self.nro = "sdmc:/switch/playguard/playguard.nro";
    self.self = true;
    Row menu;
    menu.id = 2;
    menu.name = "sphaira";
    menu.nro = "sdmc:/switch/sphaira/sphaira.nro";
    menu.pin = true;
    Row game;
    game.id = 3;
    game.name = "Cyberfoil";
    game.nro = "sdmc:/switch/cyberfoil.nro";
    LockCheck c = lock_check({ self, menu, game }, true, Patches::Ready);
    CHECK(c.ready() && c.routes() && c.menus == std::vector<std::string>{ "sphaira" } && c.open_doors.empty());
    // Each missing piece refuses it.
    CHECK(!lock_check({ self, menu }, false, Patches::Ready).ready());
    CHECK(!lock_check({ self, menu }, true, Patches::Missing).ready());
    CHECK(lock_check({ self, menu }, true, Patches::Unknown).ready());   // sys-patch there, its log silent
    c = lock_check({ menu, game }, true, Patches::Ready);
    CHECK(!c.ready() && !c.self && !c.routes());
    c = lock_check({ self, game }, true, Patches::Ready);
    CHECK(!c.ready() && c.menus.empty());
    // A menu not protected, or not there: no way back, and an open door.
    Row open_menu = menu;
    open_menu.pin = false;
    c = lock_check({ self, open_menu }, true, Patches::Ready);
    CHECK(c.menus.empty() && c.open_doors == std::vector<std::string>{ "sphaira" });
    Row gone_menu = menu;
    gone_menu.state = State::NroMissing;
    CHECK(lock_check({ self, gone_menu }, true, Patches::Ready).menus.empty());
    gone_menu.state = State::NotInstalled;
    c = lock_check({ self, gone_menu }, true, Patches::Ready);
    CHECK(c.menus.empty() && c.open_doors.empty());
    Row self_broken = self;
    self_broken.state = State::GateMissing;
    CHECK(!lock_check({ self_broken, menu }, true, Patches::Ready).self);
    // Another app's forwarder: what it starts is unknown, it is told.
    Row other;
    other.id = 4;
    other.name = "Save Keeper";
    other.state = State::Elsewhere;
    c = lock_check({ self, menu, other }, true, Patches::Ready);
    CHECK(c.ready() && c.open_doors == std::vector<std::string>{ "Save Keeper" });
}

static void test_gate()
{
    using forwarder_gate::argv_for;
    using forwarder_gate::parse;
    const std::string self = "sdmc:/switch/playguard/playguard.nro";
    CHECK(argv_for(self, "sdmc:/switch/cyberfoil.nro") ==
          "sdmc:/switch/playguard/playguard.nro --pin-gate sdmc:/switch/cyberfoil.nro");
    CHECK(argv_for("sdmc:/switch/play guard.nro", "\"sdmc:/switch/a b.nro\"") ==
          "\"sdmc:/switch/play guard.nro\" --pin-gate \"sdmc:/switch/a b.nro\"");

    std::string nro, argv;
    CHECK(parse(argv_for(self, "sdmc:/switch/cyberfoil.nro"), nro, argv));
    CHECK(nro == "sdmc:/switch/cyberfoil.nro" && argv == "sdmc:/switch/cyberfoil.nro");
    CHECK(parse(argv_for("sdmc:/switch/play guard.nro", "\"sdmc:/switch/a b.nro\" -x"), nro, argv));
    CHECK(nro == "sdmc:/switch/a b.nro" && argv == "\"sdmc:/switch/a b.nro\" -x");
    CHECK(parse(argv_for(self, "sdmc:/x.nro --flag 2"), nro, argv) && nro == "sdmc:/x.nro" && argv == "sdmc:/x.nro --flag 2");

    // Not a gate: started from a menu, or anything else.
    CHECK(!parse("sdmc:/switch/playguard/playguard.nro", nro, argv) && nro.empty() && argv.empty());
    CHECK(!parse("", nro, argv));
    CHECK(!parse(self + " --other sdmc:/x.nro", nro, argv));
    CHECK(!parse(self + " --pin-gate", nro, argv));
    CHECK(!parse(self + " --pin-gate ", nro, argv));
    CHECK(!parse(self + " --pin-gate /x.nro", nro, argv) && argv.empty());   // not on the SD card
    CHECK(!parse(self + " --pin-gate \"sdmc:/x.nro", nro, argv));           // unclosed quote
    CHECK(!parse("\"sdmc:/play guard.nro --pin-gate sdmc:/x.nro", nro, argv));
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

static void append(void* ctx, void* data, int size)
{
    auto* out = static_cast<std::vector<uint8_t>*>(ctx);
    out->insert(out->end(), (uint8_t*)data, (uint8_t*)data + size);
}

// The pixel (x, y) of a decoded image, RGB.
static bool colour_near(const std::vector<uint8_t>& jpeg, int x, int y, int r, int g, int b)
{
    int w = 0, h = 0, comp = 0;
    uint8_t* px = stbi_load_from_memory(jpeg.data(), (int)jpeg.size(), &w, &h, &comp, 3);
    if (!px) return false;
    const uint8_t* p = px + ((size_t)y * w + x) * 3;
    const bool near = std::abs(p[0] - r) < 24 && std::abs(p[1] - g) < 24 && std::abs(p[2] - b) < 24;
    stbi_image_free(px);
    return near;
}

static void test_icon()
{
    // A 64 x 64 PNG, its left half red, its right half transparent: a
    // 256 x 256 JPEG, red on the left, white on the right.
    std::vector<uint8_t> rgba(64 * 64 * 4, 0);
    for (int y = 0; y < 64; y++)
        for (int x = 0; x < 32; x++) {
            uint8_t* p = &rgba[((size_t)y * 64 + x) * 4];
            p[0] = 220, p[1] = 30, p[2] = 30, p[3] = 255;
        }
    std::vector<uint8_t> png;
    REQUIRE(stbi_write_png_to_func(append, &png, 64, 64, 4, rgba.data(), 64 * 4));
    const std::vector<uint8_t> made = forwarder_icon::normalize(png);
    CHECK(nro::icon_usable(made));
    CHECK(colour_near(made, 40, 128, 220, 30, 30) && colour_near(made, 216, 128, 255, 255, 255));

    // A JPEG of another size, shrunk: 512 x 300 of blue.
    std::vector<uint8_t> blue(512 * 300 * 3);
    for (size_t i = 0; i < blue.size(); i += 3) blue[i] = 20, blue[i + 1] = 60, blue[i + 2] = 200;
    std::vector<uint8_t> big;
    REQUIRE(stbi_write_jpg_to_func(append, &big, 512, 300, 3, blue.data(), 90));
    CHECK(!nro::icon_usable(big));
    const std::vector<uint8_t> shrunk = forwarder_icon::normalize(big);
    CHECK(nro::icon_usable(shrunk) && colour_near(shrunk, 128, 128, 20, 60, 200));

    // A 256 x 256 JPEG already: kept byte for byte.
    CHECK(forwarder_icon::normalize(shrunk) == shrunk);
    // One with EXIF data: made again, without it.
    const std::vector<uint8_t> exif = with_exif(shrunk);
    const std::vector<uint8_t> again = forwarder_icon::normalize(exif);
    CHECK(!nro::icon_usable(exif) && nro::icon_usable(again) && again != exif);
    CHECK(colour_near(again, 128, 128, 20, 60, 200));
    // No image: nothing.
    CHECK(forwarder_icon::normalize({}).empty());
    CHECK(forwarder_icon::normalize({ 1, 2, 3, 4, 5 }).empty());
    CHECK(forwarder_icon::jpeg_from_rgba(nullptr, 1, 1).empty());

    // The stand-in: the PNG in the middle of the pale square.
    const std::vector<uint8_t> stand_in = forwarder_icon::placeholder(png);
    CHECK(nro::icon_usable(stand_in));
    CHECK(colour_near(stand_in, 4, 4, 0xE8, 0xEC, 0xF1) && colour_near(stand_in, 60, 128, 220, 30, 30));
    CHECK(forwarder_icon::placeholder({ 9, 9 }).empty());
}

int main()
{
    test_icon();
    test_jpeg();
    test_parse();
    test_files();
    test_registry();
    test_text();
    test_merge();
    test_gate();
    test_lock_check();
    test_patches();
    return CHECK_DONE("forwarders tests passed");
}
