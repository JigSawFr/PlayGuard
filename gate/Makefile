# PlayGuard's gate (see source/main.c): out/gate.nro, which the .nro carries
# as romfs:/gate/gate.nro (CMakeLists.txt). Its code is the app's own,
# source/core/gate.c and gate_nx.c.
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET      := gate
BUILD       := build
OUTDIR      := out
SOURCES     := source ../source/core ../extern/stb
INCLUDES    := ../source/core ../extern/stb ../extern/borealis/library/include
# Its screen's brand font, from PlayGuard's resources, in its romfs.
ROMFS       := build/romfs
# A NACP too: elf2nro writes the assets (the romfs with them) only when it
# has an icon, a NACP or a romfs image, not for --romfsdir alone; without
# them the gate had no romfs, and nothing after its code.
APP_TITLE   := PlayGuard gate
APP_AUTHOR  := JigSawFr
# The gate's own version, not PlayGuard's: a forwarder protected with the
# PIN pins the gate by its SHA-256 (gate-<hash>.nro), so moving it at each
# release would make a new gate file each release though nothing changed.
APP_VERSION := 1.0.0

# Only these: ../source/core also holds the app's service layer.
CFILES      := main.c gate.c gate_nx.c gate_screen.c gate_text.c pg_stb.c

ARCH    := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS  := -g -Wall -Wextra -Werror -O3 -ffunction-sections $(ARCH) $(INCLUDE) -D__SWITCH__ -std=gnu11
LDFLAGS  = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS    := -lnx -lm
LIBDIRS := $(PORTLIBS) $(LIBNX)

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT  := $(CURDIR)/$(OUTDIR)/$(TARGET)
export TOPDIR  := $(CURDIR)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

export LD := $(CC)
export OFILES   := $(CFILES:.c=.o)
export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export NROFLAGS := --romfsdir=$(CURDIR)/$(ROMFS) --nacp=$(OUTPUT).nacp

.PHONY: all clean $(BUILD)

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR)
	@mkdir -p $(ROMFS)/font
	@cp $(CURDIR)/../resources/font/sora_bold_latin.ttf $(ROMFS)/font/
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTDIR)

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
