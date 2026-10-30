# playguard-agent — the remote link's background sysmodule (see source/main.c).
# Produces out/playguard-agent.nsp, which the release zip places at
# atmosphere/contents/4200000000504741/exefs.nsp and PlayGuard bundles in its
# romfs (Console › Optional modules). It links the app's C service layer
# (source/core/) and the remote link (source/link/, source/sync/) unchanged.

# TOPDIR: this folder, in both passes (the second runs from build/).
TOPDIR      ?= $(CURDIR)
TARGET      := playguard-agent
SOURCES     := source ../../source/core ../../source/sync ../../source/link ../../source/util
INCLUDES    := source ../../source/core ../../source/sync ../../source/link ../../source/util
APP_JSON    := $(TOPDIR)/playguard-agent.json

# The version PlayGuard shows for it (the app's, from CMakeLists.txt).
AGENT_VERSION ?= $(shell sed -n 's/^set(VERSION_\(MAJOR\|MINOR\|ALTER\) "\([0-9]*\)").*/\2/p' $(TOPDIR)/../../CMakeLists.txt | paste -sd.)
EXTRA_CFLAGS := -DAGENT_VERSION=\"$(AGENT_VERSION)\"

CFILES      := main.c agent_core.c agent_ipc.c \
               pctl_ops.c pctl_dump.c pure.c rescue.c write_guard.c time_ops.c calendar.c sysinfo.c \
               playstats.c playstats_logic.c playlog.c nacp.c nacp_read.c \
               console_info.c console_info_pure.c \
               bl_json.c mqtt_packet.c mqtt_client.c sync_conf.c sync_apply.c sync_records.c sync_exec.c \
               sync_entities.c sync_discovery.c sync_state.c sync_system.c sync_watch.c sync_engine.c \
               bl_net.c bl_tls.c sync_icons.c sync_titles.c
# nacp.c reads compressed game titles (zlib).
EXTRA_LIBS  := -lz

include $(TOPDIR)/../common.mk
