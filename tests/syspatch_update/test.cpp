// Host tests for source/util/syspatch_update.cpp: the latest release as
// GitHub's API gives it, version comparison, and installing a release's zip
// on a fake SD card (only its own files, the ones replaced kept, nothing
// touched on a damaged zip). The network half (syspatch_update_net.cpp) is
// left to the console.
#include "check.h"
#include "temp_dir.hpp"
#include <ctime>
#include <string>

#include "util/paths.hpp"
#include "util/syspatch_update.hpp"
#include "util/zip_write.hpp"

using namespace syspatch_update;

static const std::string DIGEST = "sha256:" + std::string(64, 'a');

static std::string release_json(const std::string& tag, const std::string& asset, bool prerelease = false,
                                long long size = 123456, const std::string& digest = DIGEST)
{
    return "{\"tag_name\":\"" + tag + "\",\"draft\":false,\"prerelease\":" + (prerelease ? "true" : "false") +
           ",\"html_url\":\"https://github.com/impeeza/sys-patch/releases/tag/" + tag +
           "\",\"assets\":[{\"name\":\"notes.txt\",\"browser_download_url\":\"https://x/notes.txt\",\"size\":3},"
           "{\"name\":\"" + asset + "\",\"browser_download_url\":\"https://github.com/impeeza/sys-patch/releases/download/" +
           tag + "/" + asset + "\",\"size\":" + std::to_string(size) + ",\"digest\":\"" + digest + "\"}]}";
}

static std::string zip_of(const std::vector<std::pair<std::string, std::string>>& files)
{
    std::tm now{};
    now.tm_year = 126;
    now.tm_mday = 21;
    return zip_write::stored(now, files);
}

int main()
{
    TempDir tmp("syspatch_update");
    const std::string base = tmp.path();

    // The latest release.
    {
        Release r;
        CHECK(parse_release(release_json("v1.6.3.0", "sys-patch-v1.6.3.0.zip"), &r));
        CHECK(r.version == "1.6.3.0" && r.size == 123456 && r.sha256 == std::string(64, 'a'));
        CHECK(r.url == "https://github.com/impeeza/sys-patch/releases/download/v1.6.3.0/sys-patch-v1.6.3.0.zip");
        CHECK(r.page == "https://github.com/impeeza/sys-patch/releases/tag/v1.6.3.0");
        // No digest: parsed, with none (download refuses it).
        CHECK(parse_release(release_json("1.6.3.0", "sys-patch.zip", false, 10, ""), &r) && r.sha256.empty());
        CHECK(!parse_release(release_json("v1.6.3.0", "sys-patch-v1.6.3.0.zip", true), &r));      // pre-release
        CHECK(!parse_release(release_json("v1.6.3.0", "other.zip"), &r));                         // no zip of it
        CHECK(!parse_release(release_json("latest", "sys-patch.zip"), &r));                       // not a version
        CHECK(!parse_release(release_json("v1.6.3.0", "sys-patch.zip", false, 0), &r));           // empty
        CHECK(!parse_release(release_json("v1.6.3.0", "sys-patch.zip", false, 64ll << 20), &r));  // too large
        CHECK(!parse_release("[]", &r) && !parse_release("not json", &r));
    }

    // Versions.
    {
        CHECK(compare("1.6.3.0", "1.6.2.3") > 0 && compare("v1.6.2.3", "1.6.3.0") < 0);
        CHECK(compare("1.6.3.0-b962f75", "1.6.3.0") == 0 && compare("1.6.3", "1.6.3.0") == 0);
        CHECK(compare("1.10.0", "1.9.9") > 0 && compare("", "0.1") < 0);
        Release r;
        r.version = "1.6.3.0";
        CHECK(newer(r, "1.6.2.3-abcdef0") && !newer(r, "1.6.3.0-b962f75") && !newer(r, "1.7.0"));
        CHECK(newer(r, "") && newer(r, "?"));   // unknown: offered
    }

    // Installing on a fake SD card.
    {
        const std::string sd = base + "/sd";
        const std::string zip = base + "/sys-patch.zip";
        const std::string mod = sd + "/atmosphere/contents/420000000000000B";
        CHECK(paths::atomic_write(mod + "/exefs.nsp", "old nsp"));
        CHECK(paths::atomic_write(mod + "/toolbox.json", "old toolbox"));
        CHECK(paths::atomic_write(sd + "/config/sys-patch/config.ini", "[options]\n"));
        CHECK(paths::atomic_write(zip, zip_of({ { "atmosphere/contents/420000000000000B/exefs.nsp", "new nsp" },
                                                { "atmosphere/contents/420000000000000B/toolbox.json", "new toolbox" },
                                                { "atmosphere/contents/420000000000000B/flags/boot2.flag", "" },
                                                { "switch/.overlays/sys-patch-overlay.ovl", "ovl" },
                                                { "atmosphere/contents/0100000000001000/exefs.nsp", "not ours" } })));
        std::string err, got;
        CHECK(install(zip, sd, base + "/backup1", &err));
        CHECK(paths::read_file(mod + "/exefs.nsp", got) && got == "new nsp");
        CHECK(paths::read_file(mod + "/toolbox.json", got) && got == "new toolbox");
        CHECK(paths::file_exists(mod + "/flags/boot2.flag"));
        CHECK(paths::read_file(sd + "/switch/.overlays/sys-patch-overlay.ovl", got) && got == "ovl");
        CHECK(!paths::file_exists(sd + "/atmosphere/contents/0100000000001000/exefs.nsp"));   // only its own files
        CHECK(paths::read_file(sd + "/config/sys-patch/config.ini", got) && got == "[options]\n");   // its settings stay
        CHECK(paths::read_file(base + "/backup1/atmosphere/contents/420000000000000B/exefs.nsp", got) && got == "old nsp");
        CHECK(paths::read_file(base + "/backup1/atmosphere/contents/420000000000000B/toolbox.json", got) &&
              got == "old toolbox");
        CHECK(!paths::file_exists(base + "/backup1/switch/.overlays/sys-patch-overlay.ovl"));   // none before
        CHECK(!paths::file_exists(mod + "/exefs.nsp.new"));

        // A zip without the overlay: the sysmodule alone, the overlay left as it is.
        CHECK(paths::atomic_write(zip, zip_of({ { "atmosphere/contents/420000000000000B/exefs.nsp", "newer nsp" },
                                                { "atmosphere/contents/420000000000000B/toolbox.json", "t" },
                                                { "atmosphere/contents/420000000000000B/flags/boot2.flag", "" } })));
        CHECK(install(zip, sd, base + "/backup2", &err));
        CHECK(paths::read_file(mod + "/exefs.nsp", got) && got == "newer nsp");
        CHECK(paths::read_file(sd + "/switch/.overlays/sys-patch-overlay.ovl", got) && got == "ovl");

        // A required file missing: nothing replaced, nothing left behind.
        CHECK(paths::atomic_write(zip, zip_of({ { "atmosphere/contents/420000000000000B/exefs.nsp", "bad" } })));
        err.clear();
        CHECK(!install(zip, sd, base + "/backup3", &err) && !err.empty());
        CHECK(paths::read_file(mod + "/exefs.nsp", got) && got == "newer nsp");
        CHECK(!paths::file_exists(mod + "/exefs.nsp.new") && !paths::file_exists(base + "/backup3"));

        // Not a zip at all.
        CHECK(paths::atomic_write(zip, "garbage"));
        CHECK(!install(zip, sd, base + "/backup4", &err));
        CHECK(paths::read_file(mod + "/exefs.nsp", got) && got == "newer nsp");
    }

    CHECK(tmp.remove());
    return CHECK_DONE("sys-patch update assertions passed");
}
