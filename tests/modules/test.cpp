// Host tests for source/util/modules.cpp, PlayGuard's layer over the kit's
// util/sysmodule (which has the install, update, rollback and removal tests):
// the modules PlayGuard carries and what it calls them, a first install through
// the layer, and the recovery module's playguard-rescue.txt.
#include "check.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "util/modules.hpp"
#include "util/paths.hpp"
#include "util/sha256.hpp"
#include "util/title_id.hpp"

using namespace modules;

static std::string g_base;
static int g_n = 0;

static std::string fresh(const char* what)
{
    std::string root = g_base + "/" + what + std::to_string(g_n++);
    CHECK(paths::ensure_dir(root));
    return root;
}

static std::string read(const std::string& path)
{
    std::string text;
    CHECK(paths::read_file(path, text));
    return text;
}

static bool is_dir(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static void test_table()
{
    CHECK(all().size() == 2);
    CHECK(get(Id::Rescue).id == Id::Rescue && get(Id::Agent).id == Id::Agent);
    CHECK(get(Id::Rescue).tid == 0x4200000000505247ULL && !get(Id::Rescue).resident);
    CHECK(get(Id::Agent).tid == sysmodule::AGENT_TID && get(Id::Agent).resident);
    CHECK(std::string(get(Id::Rescue).name) == "rescue" && std::string(get(Id::Agent).name) == "agent");
    CHECK(title_id::text(0x4200000000505247ULL) == "4200000000505247");
    CHECK(dir("/", get(Id::Rescue)) == "/atmosphere/contents/4200000000505247");
    CHECK(dir("sd", get(Id::Agent)) == "sd/atmosphere/contents/4200000000504741");
    const std::string tb = toolbox_json(get(Id::Rescue));
    CHECK(tb.find("\"name\": \"PlayGuard rescue\"") != std::string::npos);
    CHECK(tb.find("\"tid\": \"4200000000505247\"") != std::string::npos);
    CHECK(tb.find("\"requires_reboot\": true") != std::string::npos);
    CHECK(toolbox_json(get(Id::Agent)).find("\"requires_reboot\": false") != std::string::npos);
}

// A module installed through the layer, as the screens do (the kit's tests cover the rest).
static void test_install_through_the_layer()
{
    const Module& m = get(Id::Agent);
    const std::string content = "NSP0 agent";
    Sha256 h;
    h.update(content);
    const std::string romfs = fresh("romfs");
    CHECK(paths::atomic_write(romfs + "/agent/exefs.nsp", content));
    CHECK(paths::atomic_write(romfs + "/agent/version.txt", "version=1.1.0\ncommit=abc1234\nsha256=" + h.hex() + "\n"));
    const Bundle b = bundled(romfs, m);
    CHECK(b.present && b.version == "1.1.0");
    const std::string sd = fresh("sd");
    CHECK(offer(state(sd, m), b) == Offer::Install);
    CHECK(install(sd, m, b, false));
    const State s = state(sd, m);
    CHECK(s.installed && s.at_boot && s.version == "1.1.0" && offer(s, b) == Offer::None);
    CHECK(read(dir(sd, m) + "/toolbox.json").find("PlayGuard agent") != std::string::npos);
    CHECK(uninstall(sd, m) && !is_dir(dir(sd, m)));
}

static void test_rescue_note()
{
    const Module& m = get(Id::Rescue);
    const std::string sd = fresh("sd");
    // "<version> <revision>", what action/rescue reads; none without a revision, and none replaced by an empty one.
    CHECK(write_rescue_note(sd, "1.2.0", "84d0eaa86878"));
    CHECK(read(dir(sd, m) + "/playguard-rescue.txt") == "1.2.0 84d0eaa86878\n");
    CHECK(write_rescue_note(sd, "1.2.0", ""));
    CHECK(read(dir(sd, m) + "/playguard-rescue.txt") == "1.2.0 84d0eaa86878\n");
    const std::string none = fresh("sd");
    CHECK(write_rescue_note(none, "1.2.0", "") && !is_dir(dir(none, m)));
}

int main()
{
    char base[] = "/tmp/playguard_modules_XXXXXX";
    REQUIRE(mkdtemp(base) != nullptr);
    g_base = base;

    test_table();
    test_install_through_the_layer();
    test_rescue_note();

    const std::string cleanup = std::string("rm -rf '") + base + "'";
    CHECK(std::system(cleanup.c_str()) == 0);
    return CHECK_DONE("modules table, install through the layer and rescue note assertions passed");
}
