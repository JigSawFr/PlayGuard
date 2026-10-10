# Thin shim over CMake — the real build is in CMakeLists.txt.
#
#   make              -> ./playguard.nro            (needs devkitPro, DEVKITPRO set)
#   make dist         -> ./playguard.zip            (unzip onto the SD card root)
#   make desktop      -> ./build-desktop/playguard  (UI with a simulated backend; needs GLFW)
#   make web          -> ./build-web/simulator/          (the same in a browser; needs Emscripten's emcmake)
#   make web-serve    -> serves it at http://localhost:8000 (with the headers its threads need)
#   make test         -> host unit tests (plain gcc/g++, ASan + UBSan; parallel, all suites)
#   make fuzz         -> libFuzzer runs of the parsers (clang; FUZZ_SECONDS=30 each, parallel)
#   make nxlink       -> push to a Switch running hbmenu (press Y there first)
#
# GL=1 builds the Switch .nro with OpenGL (mesa) instead of deko3d.
# JOBS=n sets the parallel build (and host test) jobs (default: every core).
# SAN= turns the sanitizers of the host tests off (e.g. a compiler without
# them). COV=--coverage builds them for gcov (CI: with gcc and SAN=, summarised
# by gcovr).
# SOURCE_DATE_EPOCH (default: the last commit's time) dates every file of the
# release zips, so the same commit gives the same zip.
# CMAKE_C_COMPILER_LAUNCHER / CMAKE_CXX_COMPILER_LAUNCHER=ccache in the
# environment are picked up by CMake (CI uses them). CMAKE_ARGS=... is passed
# to the configure step of `make` (so `make dist`) and `make desktop` (CI
# builds both with CMAKE_ARGS=-DPLAYGUARD_WERROR=ON, warnings as errors).

TARGET  := playguard
BUILD   := build
DESKTOP := build-desktop
WEB     := build-web
CC      ?= gcc
CXX     ?= g++
TESTOUT := $(BUILD)/host-tests
JOBS    ?= $(shell nproc 2>/dev/null || echo 4)
SAN     ?= -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined
COV     ?=
# -Itests: tests/check.h, the CHECK() every suite uses.
CWARN   := -Wall -Wextra -Werror $(SAN) $(COV) -Itests
SOURCE_DATE_EPOCH ?= $(shell git log -1 --format=%ct 2>/dev/null || echo 0)
# A zip that depends only on the files: one date for all, entries in a fixed
# order, no extra file attributes (-X), and TZ=UTC as a zip keeps local times.
# Run inside the tree, the zip file last.
ZIP_TREE = find . -exec touch -h -d @$(SOURCE_DATE_EPOCH) {} + && find . -mindepth 1 | LC_ALL=C sort | TZ=UTC zip -X -@
CMAKE_ARGS ?=
JSON_HPP := extern/borealis/library/include/borealis/extern/nlohmann/json.hpp
# The remote link's console-free C (source/sync/), shared by its host tests.
SYNC_PURE := source/sync/sync_json.c source/sync/mqtt_packet.c source/sync/sync_conf.c source/sync/sync_apply.c \
             source/sync/sync_records.c source/sync/sync_entities.c source/sync/sync_discovery.c source/sync/sync_state.c

.PHONY: all clean dist nxlink desktop web web-serve test fuzz check rescue dist-rescue agent dist-agent FORCE

RENDERER := $(if $(GL),-DUSE_DEKO3D=OFF,-DUSE_DEKO3D=ON)

# The optional sysmodules first: the .nro carries them in its romfs, so
# Console › Optional modules can install them (CMakeLists.txt, cmake/bundle_sysmodules.cmake).
all: rescue agent
	@cmake -B $(BUILD) -S . -DPLATFORM_SWITCH=ON $(RENDERER) $(CMAKE_ARGS)
	@cmake --build $(BUILD) --target $(TARGET).nro -j $(JOBS)
	@cp $(BUILD)/$(TARGET).nro  $(TARGET).nro
	@cp $(BUILD)/$(TARGET).nacp $(TARGET).nacp

# Layout on the SD card: one folder per app (hbmenu / hb-appstore / sphaira
# convention) + the sphaira GitHub-updater entry. The GPL text travels with
# the binary.
dist: all
	@echo making dist ...
	@rm -rf out/ $(TARGET).zip
	@mkdir -p out/switch/$(TARGET) out/config/sphaira/github
	@cp $(BUILD)/$(TARGET).nro out/switch/$(TARGET)/
	@cp LICENSE out/switch/$(TARGET)/LICENSE.txt
	@cp packaging/sphaira/$(TARGET).json out/config/sphaira/github/
	@cd out && $(ZIP_TREE) ../$(TARGET).zip

# The optional recovery sysmodule (sysmodule/rescue/), as its own asset so it
# is a deliberate install, never part of the default one. The zip drops into
# the SD-card root: Atmosphère runs 4200000000505247 at boot (the boot2.flag).
RESCUE_TID := 4200000000505247
# playguard-rescue.txt next to the module: the PlayGuard version it comes
# with and its revision (tools/rescue_rev.cmake), which the app compares with
# its own to tell a module out of date (action/rescue.hpp).
RESCUE_VERSION := $(shell sed -n 's/^set(VERSION_[A-Z]* "\([0-9]*\)").*/\1/p' CMakeLists.txt | paste -sd.)
rescue:
	@$(MAKE) --no-print-directory -C sysmodule/rescue

dist-rescue: rescue
	@echo making rescue dist ...
	@rm -rf out-rescue/ playguard-rescue.zip
	@mkdir -p out-rescue/atmosphere/contents/$(RESCUE_TID)/flags
	@cp sysmodule/rescue/out/playguard-rescue.nsp out-rescue/atmosphere/contents/$(RESCUE_TID)/exefs.nsp
	@touch out-rescue/atmosphere/contents/$(RESCUE_TID)/flags/boot2.flag
	@printf '{\n  "name": "PlayGuard rescue",\n  "tid": "%s",\n  "requires_reboot": true\n}\n' $(RESCUE_TID) \
		> out-rescue/atmosphere/contents/$(RESCUE_TID)/toolbox.json
	@cp LICENSE out-rescue/atmosphere/contents/$(RESCUE_TID)/LICENSE.txt
	@printf '%s %s\n' '$(RESCUE_VERSION)' "$$(cmake -DROOT=$(CURDIR) -P tools/rescue_rev.cmake)" > out-rescue/atmosphere/contents/$(RESCUE_TID)/playguard-rescue.txt
	@cd out-rescue && $(ZIP_TREE) ../playguard-rescue.zip

# The optional remote-link agent (sysmodule/agent/): its own asset too. It
# starts at boot (boot2.flag) and can be started or stopped without a reboot.
AGENT_TID := 4200000000504741
agent:
	@$(MAKE) --no-print-directory -C sysmodule/agent

dist-agent: agent
	@echo making agent dist ...
	@rm -rf out-agent/ playguard-agent.zip
	@mkdir -p out-agent/atmosphere/contents/$(AGENT_TID)/flags
	@cp sysmodule/agent/out/playguard-agent.nsp out-agent/atmosphere/contents/$(AGENT_TID)/exefs.nsp
	@touch out-agent/atmosphere/contents/$(AGENT_TID)/flags/boot2.flag
	@printf '{\n  "name": "PlayGuard agent",\n  "tid": "%s",\n  "requires_reboot": false\n}\n' $(AGENT_TID) \
		> out-agent/atmosphere/contents/$(AGENT_TID)/toolbox.json
	@cp LICENSE out-agent/atmosphere/contents/$(AGENT_TID)/LICENSE.txt
	@cd out-agent && $(ZIP_TREE) ../playguard-agent.zip

desktop:
	@cmake -B $(DESKTOP) -S . -DPLATFORM_DESKTOP=ON -DCMAKE_BUILD_TYPE=Release $(CMAKE_ARGS)
	@cmake --build $(DESKTOP) -j $(JOBS)

# The web simulator (web/README.md). cmake found by the shell: emsdk_env.sh
# puts Emscripten's folder on the PATH, whose cmake/ folder make would run.
web:
	@emcmake cmake -B $(WEB) -S . -DCMAKE_BUILD_TYPE=Release $(CMAKE_ARGS)
	@"$$(command -v cmake)" --build $(WEB) -j $(JOBS)

web-serve: web
	@python3 web/serve.py $(WEB)/simulator 8000

# Host tests: one program per suite, built straight from its sources (no
# object files, so gcov's .gcno/.gcda sit next to the program in
# build/host-tests/). A suite is three variables: <name>_SRC (its sources,
# the test last), <name>_FLAGS (the standard, defines, include paths; a C++
# suite without one gets TEST_CXXFLAGS) and an optional <name>_LIBS; the test
# source's suffix picks the compiler. One more when needed: <name>_ARGS (what
# the program is run with).
# `make test` builds and runs every suite in parallel (JOBS) and keeps going
# when one fails, so a run reports every failing suite, not the first: each
# suite's output is in build/host-tests/<name>.log, and <name>.ok marks a
# passed one (CI reads both for the job summary).

# C: the service layer (source/core) against the fake <switch.h> of each
# suite (-DNX_HOST_TEST), and the C helpers of source/util.
pctl_SRC   := source/core/pctl_ops.c source/core/pctl_dump.c source/core/pure.c source/core/rescue.c source/core/write_guard.c tests/pctl_session/test.c
pctl_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/pctl_session -Isource/core
time_SRC   := source/core/time_ops.c source/core/calendar.c source/core/write_guard.c tests/time_ops/test.c
time_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/time_ops -Isource/core
sysinfo_SRC   := source/core/sysinfo.c source/core/pure.c tests/sysinfo/test.c
sysinfo_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/sysinfo -Isource/core
gamecheck_SRC   := source/core/gamecheck.c source/core/gamecheck_rules.c source/core/nacp.c source/core/nacp_read.c source/core/write_guard.c tests/gamecheck/test.c
gamecheck_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/gamecheck -Isource/core
gamecheck_LIBS  := -lz
calendar_SRC   := source/core/calendar.c tests/calendar/test.c
calendar_FLAGS := -std=gnu11 -Isource/core
rescue_SRC   := source/core/rescue.c tests/rescue/test.c
rescue_FLAGS := -std=c11 -Isource/core
forwarder_pack_SRC   := source/core/forwarder_pack.c tests/forwarder_pack/test.c
forwarder_pack_FLAGS := -std=c11 -Isource/core
ntp_SRC   := source/util/ntp_packet.c tests/ntp_packet/test.c
ntp_FLAGS := -std=c11 -Isource/util
playlog_SRC   := source/util/playlog.c tests/playlog/test.c
playlog_FLAGS := -std=c11 -Isource/util
startup_SRC   := source/util/startup.c tests/startup/test.c
startup_FLAGS := -std=gnu11 -Isource/util
playstats_SRC   := source/core/playstats_logic.c source/core/nacp.c tests/playstats/test.c
playstats_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/playstats -Isource/core
playstats_LIBS  := -lz

# C++: the UI-free helpers and flow logic (source/util, source/action), by
# default with TEST_CXXFLAGS (a <name>_FLAGS only where a suite differs).
# borealis' headers (nlohmann/json) as system headers: their warnings are not
# PlayGuard's (clang 22 deprecates json.hpp's literal operators, under -Werror).
# util/date stands on core/calendar.c: a suite that links it builds that C
# file too (its <name>_CSRC, without having to say so).
TEST_CXXFLAGS   := -std=c++17 -Isource -isystem extern/borealis/library/include
CALENDAR_C      := source/core/calendar.c
CALENDAR_CFLAGS := -std=gnu11 -Isource/core
date_SRC    := source/util/date.cpp tests/date/test.cpp
# The header-only helpers: utf8, emoji, text, json_util.
text_util_SRC   := tests/text_util/test.cpp
patches_SRC   := source/util/paths.cpp source/util/patches.cpp tests/patches/test.cpp
changelog_SRC   := source/util/changelog.cpp tests/changelog/test.cpp
support_SRC   := source/util/date.cpp source/util/support.cpp tests/support/test.cpp
pt_logic_SRC   := source/action/pt_logic.cpp source/util/date.cpp tests/pt_logic/test.cpp
pin_lock_logic_SRC   := source/action/pin_lock_logic.cpp tests/pin_lock_logic/test.cpp
console_lock_logic_SRC   := source/action/console_lock_logic.cpp tests/console_lock_logic/test.cpp
history_logic_SRC   := source/util/date.cpp source/action/history_logic.cpp tests/history_logic/test.cpp
timer_health_logic_SRC   := source/action/timer_health_logic.cpp tests/timer_health_logic/test.cpp
notice_logic_SRC   := source/util/date.cpp source/action/notice_logic.cpp tests/notice_logic/test.cpp
sys_updates_SRC   := source/util/sys_updates.cpp tests/sys_updates/test.cpp
# The network half twice from one test: -D__SWITCH__ for the console's path
# (a fake http::redirect in the test), without it for the desktop's simulation.
sys_updates_net_SRC   := source/util/sys_updates.cpp source/util/sys_updates_net.cpp tests/sys_updates_net/test.cpp
sys_updates_net_FLAGS := -std=c++17 -Isource -D__SWITCH__
sys_updates_sim_SRC   := source/util/sys_updates.cpp source/util/sys_updates_net.cpp source/sim/sim_net.cpp source/sim/sim_env.cpp source/util/paths.cpp tests/sys_updates_net/test.cpp
ntp_servers_SRC   := source/util/ntp_servers.cpp tests/ntp_servers/test.cpp
dns_hosts_SRC   := source/util/dns_hosts.cpp tests/dns_hosts/test.cpp
sys_notes_SRC   := source/util/date.cpp source/util/safe_text.cpp source/util/markdown.cpp source/util/update.cpp source/util/sys_updates.cpp source/util/sys_notes.cpp tests/sys_notes/test.cpp
offline_clock_logic_SRC   := source/action/offline_clock_logic.cpp tests/offline_clock_logic/test.cpp
icon_logic_SRC   := source/action/icon_logic.cpp tests/icon_logic/test.cpp
games_logic_SRC   := source/action/games_logic.cpp tests/games_logic/test.cpp
write_queue_logic_SRC   := source/action/write_queue_logic.cpp tests/write_queue_logic/test.cpp
write_queue_logic_LIBS  := -pthread
worker_threads_SRC   := source/util/worker_threads.cpp tests/worker_threads/test.cpp
worker_threads_LIBS  := -pthread
data_cleanup_SRC   := source/util/data_cleanup.cpp tests/data_cleanup/test.cpp
error_hint_logic_SRC   := source/action/error_hint_logic.cpp tests/error_hint_logic/test.cpp
outside_change_logic_SRC   := source/util/date.cpp source/action/outside_change_logic.cpp tests/outside_change_logic/test.cpp
backup_SRC   := source/util/paths.cpp source/util/backup.cpp tests/backup/test.cpp
backup_logic_SRC   := source/util/paths.cpp source/util/backup.cpp source/util/pt_block.cpp source/action/backup_logic.cpp tests/backup_logic/test.cpp
http_logic_SRC   := source/util/http_logic.cpp tests/http_logic/test.cpp
table_export_SRC   := source/util/paths.cpp source/util/zip_write.cpp source/util/table_export.cpp source/util/table_export_csv.cpp source/util/table_export_json.cpp source/util/table_export_xlsx.cpp source/util/table_export_pdf.cpp tests/table_export/test.cpp
table_export_LIBS  := -lz
update_SRC   := source/util/date.cpp source/util/update.cpp tests/update/test.cpp
config_SRC   := source/util/paths.cpp source/util/date.cpp source/util/config.cpp tests/config/test.cpp
profiles_SRC   := source/util/paths.cpp source/util/profiles.cpp tests/profiles/test.cpp
forwarders_SRC    := source/util/paths.cpp source/util/nro.cpp source/util/forwarders.cpp source/util/forwarder_icon.cpp source/util/forwarder_gate.cpp source/util/patches.cpp source/action/forwarders_logic.cpp tests/forwarders/test.cpp
forwarders_FLAGS  := $(TEST_CXXFLAGS) -isystem extern/borealis/library/include/borealis/extern/nanovg -isystem extern/stb
forwarders_CSRC   := tests/forwarders/stb.c extern/stb/stb_image_write.c
forwarders_CFLAGS := -std=c11 -w -isystem extern/borealis/library/include/borealis/extern/nanovg -isystem extern/stb
forwarders_LIBS   := -lm
pt_block_SRC   := source/util/paths.cpp source/util/pt_block.cpp tests/pt_block/test.cpp
launcher_SRC   := source/util/paths.cpp source/util/launcher.cpp tests/launcher/test.cpp
markdown_SRC   := source/util/safe_text.cpp source/util/markdown.cpp tests/markdown/test.cpp
rollback_SRC   := source/util/paths.cpp source/util/rollback.cpp tests/rollback/test.cpp
history_SRC   := source/util/paths.cpp source/util/history.cpp tests/history/test.cpp
log_upload_SRC   := source/util/paths.cpp source/util/pt_log.cpp source/util/pt_block.cpp source/util/log_files.cpp source/util/log_upload.cpp tests/log_upload/test.cpp
dev_builds_SRC   := source/util/paths.cpp source/util/sha256.cpp source/util/date.cpp source/util/update.cpp source/util/safe_text.cpp source/util/dev_builds.cpp source/util/zip_read.cpp source/util/github_auth.cpp tests/dev_builds/test.cpp
dev_builds_LIBS  := -lz
# The download and its resume: without PLAYGUARD_SIM_DEV_BUILDS the desktop
# build takes the console's path, here against a fake http::download and
# http::redirect in the test (-D__SWITCH__ would reach date.cpp's switch.h).
dev_builds_net_SRC   := $(filter-out tests/%,$(dev_builds_SRC)) source/util/http_logic.cpp source/util/zip_write.cpp source/sim/sim_net.cpp source/sim/sim_env.cpp source/util/dev_builds_net.cpp tests/dev_builds_net/test.cpp
dev_builds_net_LIBS  := -lz
play_cache_SRC   := source/util/play_cache.cpp tests/play_cache/test.cpp
game_cache_SRC   := source/util/game_cache.cpp tests/game_cache/test.cpp
perf_SRC   := source/util/perf.cpp tests/perf/test.cpp
activity_summary_SRC   := source/util/activity_summary.cpp tests/activity_summary/test.cpp
# The text helpers that only translate, over ui/tr.hpp's stand-in for
# borealis' getStr and fmt header-only (the catalogs read by the test).
format_text_SRC   := source/ui/format_text.cpp tests/format_text/test.cpp
format_text_FLAGS := $(TEST_CXXFLAGS) -isystem extern/borealis/library/lib/extern/fmt/include -DFMT_HEADER_ONLY -DPLAYGUARD_HOST_I18N
cadran_text_SRC   := source/ui/cadran_text.cpp tests/cadran_text/test.cpp
cadran_text_FLAGS := $(format_text_FLAGS)
game_check_text_SRC   := source/action/game_check_text.cpp tests/game_check_text/test.cpp
game_check_text_CSRC   := source/core/gamecheck_rules.c
game_check_text_CFLAGS := -std=gnu11 -Isource/core
rating_logic_SRC   := source/action/rating_logic.cpp tests/rating_logic/test.cpp
schedule_logic_SRC   := source/action/schedule_logic.cpp source/util/date.cpp tests/schedule_logic/test.cpp
pt_log_SRC   := source/util/paths.cpp source/util/pt_log.cpp tests/pt_log/test.cpp
session_log_SRC   := source/util/paths.cpp source/util/log_files.cpp source/util/session_log.cpp source/util/crash_report.cpp tests/session_log/test.cpp
# A C source a C++ suite links (clang++ refuses to build a .c as C++).
session_log_CSRC  := source/util/crash_dump.c
session_log_CFLAGS := -std=c11 -Isource/util
crash_dump_SRC   := source/util/crash_dump.c tests/crash_dump/test.c
crash_dump_FLAGS := -std=c11 -Isource/util
# The desktop and web simulators' console (source/sim/*.c, sim_state.h) and
# their PLAYGUARD_SIM_* variables (core/sim_env.h), with the console-free
# parts of the service layer they share with the console.
sim_backend_SRC    := source/sim/sim_env.cpp source/sim/sim_platform.cpp source/sim/sim_forwarder.cpp source/util/paths.cpp source/util/sha256.cpp source/util/nro.cpp tests/sim_backend/test.cpp
sim_backend_CSRC   := $(addprefix source/sim/,sim_state.c sim_games.c sim_pctl.c sim_time.c sim_playstats.c sim_gamecheck.c) \
                      $(addprefix source/core/,pure.c calendar.c write_guard.c forwarder_pack.c) source/util/playlog.c
# Atomic coverage counters: the suite runs the same code from several threads
# at once, and gcov's plain counters then come out wrong (negative ones, that
# gcovr refuses).
sim_backend_FLAGS  := $(TEST_CXXFLAGS) -fprofile-update=atomic
sim_backend_CFLAGS := -std=gnu11 -Isource/core -fprofile-update=atomic
sim_backend_LIBS   := -pthread
frame_stats_SRC   := source/util/paths.cpp source/util/frame_stats.cpp tests/frame_stats/test.cpp
log_files_SRC   := source/util/paths.cpp source/util/nand.cpp source/util/log_files.cpp source/util/data_move.cpp tests/log_files/test.cpp
migrations_SRC   := source/util/paths.cpp source/util/date.cpp source/util/update.cpp source/util/migrations.cpp tests/migrations/test.cpp
modules_SRC   := source/util/paths.cpp source/util/sha256.cpp source/util/modules.cpp tests/modules/test.cpp
# The remote link's files on the SD card, with its C parts (built by $(CC):
# g++ would build a .c as C++).
sync_files_SRC   := source/util/date.cpp source/util/paths.cpp source/util/config.cpp source/util/profiles.cpp source/util/sync_files.cpp tests/sync_files/test.cpp
sync_files_CSRC  := $(CALENDAR_C) source/sync/sync_conf.c source/sync/sync_records.c
sync_files_CFLAGS := -std=gnu11 -Isource/sync -Isource/core

# C: the remote link (source/sync/) and the agent sysmodule's core. sync_core
# writes the documents it builds (_ARGS: where) for tools/check_sync_json.py.
sync_core_SRC   := $(SYNC_PURE) tests/sync_core/test.c
sync_core_FLAGS := -std=gnu11 -Isource/sync -Isource/core
sync_core_ARGS  := $(TESTOUT)/sync
sync_exec_SRC   := source/core/pctl_ops.c source/core/pure.c source/core/rescue.c source/core/write_guard.c source/sync/sync_exec.c source/sync/sync_records.c source/sync/sync_apply.c source/sync/sync_conf.c tests/sync_exec/test.c
sync_exec_FLAGS := -std=gnu11 -DNX_HOST_TEST -Itests/pctl_session -Isource/core -Isource/sync
sync_engine_SRC   := $(SYNC_PURE) source/sync/mqtt_client.c source/sync/sync_engine.c tests/sync_engine/test.c
sync_engine_FLAGS := -std=gnu11 -Isource/sync -Isource/core
agent_SRC   := sysmodule/agent/source/agent_core.c source/sync/sync_conf.c source/sync/sync_records.c tests/agent/test.c
agent_FLAGS := -std=gnu11 -Isource/sync -Isource/core -Isysmodule/agent/source

TESTS := $(sort $(patsubst %_SRC,%,$(filter %_SRC,$(.VARIABLES))))
TEST_PROGRAMS := $(addprefix $(TESTOUT)/,$(TESTS))
TEST_OKS := $(addsuffix .ok,$(TEST_PROGRAMS))
# Every header: a suite is rebuilt when one changes (coarse, but a suite
# compiles in seconds and CI always starts cold).
TEST_HEADERS := $(wildcard tests/*.h tests/*.hpp tests/*/*.h source/*.hpp source/*/*.h source/*/*.hpp sysmodule/*/source/*.h)

$(TESTOUT):
	@mkdir -p $@

# make only compares dates: this file is rewritten when the compiler or the
# flags change (CC=clang, SAN=, COV=--coverage), so the suites are rebuilt.
TEST_FLAGS_FILE := $(TESTOUT)/flags
$(TEST_FLAGS_FILE): FORCE | $(TESTOUT)
	@echo "$(CC) $(CXX) $(CWARN)" | cmp -s - $@ || echo "$(CC) $(CXX) $(CWARN)" > $@
FORCE:

# The suite's program. Second expansion: the pattern's stem names the
# variables above.
.SECONDEXPANSION:
# An optional <name>_CSRC (C files, with <name>_CFLAGS) is built by $(CC)
# into <name>.<file>.o each and linked into a C++ suite; one that links
# util/date.cpp gets CALENDAR_C (with CALENDAR_CFLAGS) when it names none.
test_flags  = $(or $($1_FLAGS),$(if $(filter %.cpp,$($1_SRC)),$(TEST_CXXFLAGS)))
test_csrc   = $(or $($1_CSRC),$(if $(filter source/util/date.cpp,$($1_SRC)),$(CALENDAR_C)))
test_cflags = $(if $($1_CSRC),$($1_CFLAGS),$(CALENDAR_CFLAGS))
test_cobj   = $(TESTOUT)/$1.$(basename $(notdir $2)).o
test_cobjs  = $(foreach c,$(call test_csrc,$1),$(call test_cobj,$1,$c))
$(TEST_PROGRAMS): $(TESTOUT)/%: $$($$*_SRC) $$(call test_csrc,$$*) $(TEST_HEADERS) $(TEST_FLAGS_FILE) | $(TESTOUT)
	$(foreach c,$(call test_csrc,$*),$(CC) $(call test_cflags,$*) $(CWARN) -c $c -o $(call test_cobj,$*,$c) &&) true
	$(if $(filter %.cpp,$($*_SRC)),$(CXX),$(CC)) $(call test_flags,$*) $(CWARN) $($*_SRC) $(call test_cobjs,$*) $($*_LIBS) -o $@

$(TESTOUT)/sync:
	@mkdir -p $@
$(TESTOUT)/sync_core.ok: | $(TESTOUT)/sync

# Running it: the output goes to <name>.log and the terminal; <name>.ok only
# when it passed.
$(TEST_OKS): $(TESTOUT)/%.ok: $(TESTOUT)/%
	@rm -f $@
	@$< $($*_ARGS) > $<.log 2>&1; r=$$?; cat $<.log; if [ $$r -eq 0 ]; then touch $@; else echo "$*: exit status $$r" >&2; exit $$r; fi

test:
	@test -f $(JSON_HPP) || { echo "borealis is missing (nlohmann/json for the tests): run git submodule update --init"; exit 1; }
	@rm -f $(TEST_OKS)
	@$(MAKE) --no-print-directory -k $(if $(findstring -j,$(MAKEFLAGS)),,-j$(JOBS)) -Otarget $(TEST_OKS)

# Fuzzing: one libFuzzer program per parser of outside data (tests/fuzz/,
# see tests/README.md), built by clang with ASan and UBSan, each run for
# FUZZ_SECONDS from the seed corpus in tests/fuzz/<name>/corpus (left as it
# is: what a run finds goes to build/fuzz/<name>.corpus), with the libFuzzer
# dictionary tests/fuzz/<name>/dict when there is one. In parallel like
# `make test`, all of them even when one fails; a crash leaves its input as
# build/fuzz/<name>-crash-* (replay it: build/fuzz/<name> <that file>). A
# target is <name>_FUZZ (its sources, the fuzz target last; a .c one makes
# it a C program), an optional <name>_FUZZ_CSRC (one C source a C++ target
# links), <name>_FUZZ_FLAGS and <name>_FUZZ_LIBS. A parser a suite tests
# takes that suite's sources: $(filter-out tests/%,$(<suite>_SRC)).
FUZZOUT      := $(BUILD)/fuzz
FUZZ_SECONDS ?= 30
FUZZ_CC      ?= clang
FUZZ_CXX     ?= clang++
FUZZ_SAN     := -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g -O1
FUZZ_CFLAGS  := -Wall -Wextra -Werror $(FUZZ_SAN) -Itests/fuzz -Isource -isystem extern/borealis/library/include
zip_read_FUZZ      := source/util/zip_read.cpp tests/fuzz/zip_read/fuzz.cpp
zip_read_FUZZ_LIBS := -lz
markdown_FUZZ      := $(filter-out tests/%,$(markdown_SRC)) tests/fuzz/markdown/fuzz.cpp
changelog_FUZZ     := $(filter-out tests/%,$(changelog_SRC)) tests/fuzz/changelog/fuzz.cpp
rescue_FUZZ        := $(filter-out tests/%,$(rescue_SRC)) tests/fuzz/rescue/fuzz.c
config_FUZZ        := $(filter-out tests/%,$(config_SRC)) tests/fuzz/config/fuzz.cpp
config_FUZZ_CSRC   := $(CALENDAR_C)
game_cache_FUZZ    := $(filter-out tests/%,$(game_cache_SRC)) tests/fuzz/game_cache/fuzz.cpp
game_cache_FUZZ_CSRC := source/core/gamecheck_rules.c
play_cache_FUZZ    := $(filter-out tests/%,$(play_cache_SRC)) tests/fuzz/play_cache/fuzz.cpp
FUZZERS := $(sort $(patsubst %_FUZZ,%,$(filter %_FUZZ,$(.VARIABLES))))
FUZZ_PROGRAMS := $(addprefix $(FUZZOUT)/,$(FUZZERS))
FUZZ_RUNS := $(addsuffix .run,$(FUZZ_PROGRAMS))

$(FUZZOUT):
	@mkdir -p $@

# Rebuilt when the Makefile changes too (the flags above).
$(FUZZ_PROGRAMS): $(FUZZOUT)/%: $$($$*_FUZZ) $$($$*_FUZZ_CSRC) tests/fuzz/fuzz.h $(TEST_HEADERS) Makefile | $(FUZZOUT)
	$(if $($*_FUZZ_CSRC),$(FUZZ_CC) -std=gnu11 $(FUZZ_CFLAGS) $($*_FUZZ_FLAGS) -c $($*_FUZZ_CSRC) -o $@.c.o)
	$(if $(filter %.c,$(lastword $($*_FUZZ))),$(FUZZ_CC) -std=gnu11 -Isource/core,$(FUZZ_CXX) -std=c++17) \
		$(FUZZ_CFLAGS) $($*_FUZZ_FLAGS) $($*_FUZZ) $(if $($*_FUZZ_CSRC),$@.c.o) $($*_FUZZ_LIBS) -o $@

# A run: its output in <name>.log, its final figures on the terminal (tagged
# with its name), and on a failure what it found, with the input libFuzzer
# saved. The paths are absolute: the config target works in a folder of its
# own (paths::data_dir() is relative).
$(FUZZ_RUNS): $(FUZZOUT)/%.run: $(FUZZOUT)/%
	@mkdir -p $<.corpus
	@rm -f $<-crash-* $<-leak-* $<-timeout-* $<-oom-*
	@$(abspath $<) -max_total_time=$(FUZZ_SECONDS) -timeout=10 -rss_limit_mb=2048 -print_final_stats=1 \
		$(if $(wildcard tests/fuzz/$*/dict),-dict=$(abspath tests/fuzz/$*/dict)) -artifact_prefix=$(abspath $<)- $(abspath $<.corpus) $(abspath tests/fuzz/$*/corpus) > $<.log 2>&1; r=$$?; \
		if [ $$r -eq 0 ]; then grep -E '^(Done|stat::(number_of_executed_units|new_units_added|peak_rss_mb))' $<.log; \
		else grep -v 'Uses:' $<.log | tail -n 40; fi | sed 's/^/[$*] /'; \
		if [ $$r -ne 0 ]; then echo "$*: the fuzzer stopped with status $$r: $$(ls $<-* 2>/dev/null | head -n 1)" >&2; exit $$r; fi

fuzz:
	@test -f $(JSON_HPP) || { echo "borealis is missing (nlohmann/json for the config target): run git submodule update --init"; exit 1; }
	@$(MAKE) --no-print-directory -k $(if $(findstring -j,$(MAKEFLAGS)),,-j$(JOBS)) -Otarget $(FUZZ_RUNS)

check: test
	python3 tools/check_resources.py .
	python3 tools/check_sync_json.py $(TESTOUT)/sync
	python3 tools/gen_compat.py . $(BUILD)/compat.json

clean:
	@echo clean ...
	@rm -rf $(BUILD) $(DESKTOP) $(WEB) out out-rescue out-agent $(TARGET).zip playguard-rescue.zip playguard-agent.zip $(TARGET).nro $(TARGET).nacp $(TARGET).elf
	@$(MAKE) --no-print-directory -C sysmodule/rescue clean 2>/dev/null || true
	@$(MAKE) --no-print-directory -C sysmodule/agent clean 2>/dev/null || true
	@$(MAKE) --no-print-directory -C forwarder clean 2>/dev/null || true

nxlink: all
	nxlink $(BUILD)/$(TARGET).nro
