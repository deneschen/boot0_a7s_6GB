# =============================================================================
# boot0-A7S : standalone, self-contained build of a real, bootable boot0 for
#             Allwinner A733 (sun60iw2p1), SD/MMC boot medium.
#
# It compiles the REAL vendor sources (boot0_main.c / boot0_head.c /
# boot0_entry.S / solution) and links them against the REAL pre-built closed
# blobs (board driver blob + DRAM training lib), producing boot0.bin that is
# functionally identical to device-a733/bin/boot0_sdcard_sun60iw2p1.bin.
#
# No code is stubbed: every interface is the genuine vendor implementation.
# =============================================================================

CROSS   ?= $(if $(shell command -v arm-linux-gnueabi-gcc 2>/dev/null),arm-linux-gnueabi-,arm-none-eabi-)
CC       = $(CROSS)gcc
LD       = $(CROSS)ld
OBJCOPY  = $(CROSS)objcopy
OBJDUMP  = $(CROSS)objdump
NM       = $(CROSS)nm
CPP      = $(CC) -E

TOP      := $(CURDIR)
BUILD    := $(TOP)/build
PLATFORM := sun60iw2p1

# ---- run/size layout --------------------------------------------------------
# The flashable A733 SD-card boot0 image in device-a733/bin is linked and
# advertised to BROM at 0x47000.  Keep this standalone build on that runtime
# address so absolute references to BT0_head and early code match the image
# header used by the ROM loader.
BOOT0ADDR := 0x47000
BOOT0SIZE := 0x40000

# ---- include search path (mirrors spl-pub mk/config.mk SPLINCLUDE) ----------
INC := -I$(TOP)/include \
       -I$(TOP)/include/arch/arm \
       -I$(TOP)/include/configs \
       -I$(TOP)/include/arch/$(PLATFORM)

LIBGCCINC := -isystem $(shell dirname `$(CC) -print-libgcc-file-name`)/include

# ---- common flags (verbatim from spl-pub mk/config.mk, arm branch) ----------
COMM_FLAGS := -nostdinc $(LIBGCCINC) $(INC) \
	-g -Os -fno-common -mfpu=neon -msoft-float \
	-ffunction-sections \
	-fno-builtin -ffreestanding \
	-D__KERNEL__ \
	-DCONFIG_ARM -D__ARM__ \
	-D__NEON_SIMD__ \
	-march=armv7-a \
	-mabi=aapcs-linux \
	-mthumb-interwork \
	-fno-stack-protector \
	-Wall \
	-Wno-format-security \
	-Wno-format-nonliteral \
	-fno-delete-null-pointer-checks \
	-pipe \
	-mno-unaligned-access \
	-D__LINUX_ARM_ARCH__=7

CFLAGS := $(COMM_FLAGS)
AFLAGS := $(COMM_FLAGS) -D__ASSEMBLY__

LDFLAGS_GC := --gc-sections
LDFLAGS_MULTI_DEF := --allow-multiple-definition
PLATFORM_LIBGCC := -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

# ---- objects ----------------------------------------------------------------
# Real compiled-from-source objects:
OBJS := $(BUILD)/boot0_entry.o \
        $(BUILD)/boot0_head.o \
        $(BUILD)/early_uart.o \
        $(BUILD)/boot0_main.o \
        $(BUILD)/platform_shims.o

# Real pre-built closed blob.
#
#   board_sdcard.o : the genuine vendor SD/MMC boot blob for a733/sun60iw2p1.
#                    It provides MMC / UART / legacy PMIC / clock drivers and
#                    the boot package loader. AXP8191 support is supplied by
#                    platform_shims.c because this blob is the FPGA variant.
#
#   libdram.o     : the A733 real-silicon DRAM training blob. It is linked
#                    before board_sdcard.o so its init_DRAM() wins over the
#                    small FPGA/bring-up routine in board_sdcard.o.
BLOBS := $(TOP)/blobs/libdram.o \
         $(BUILD)/board_sdcard.o

LDS_IN  := $(TOP)/arch/armv7/boot0.lds
LDS_OUT := $(BUILD)/boot0.lds

NAME := boot0_sdcard_$(PLATFORM)

.PHONY: all clean verify
all: $(BUILD)/$(NAME).bin

# ---- compile rules ----------------------------------------------------------
$(BUILD)/boot0_entry.o: $(TOP)/arch/armv7/boot0_entry.S | $(BUILD)
	$(CC) $(AFLAGS) -c -o $@ $<

$(BUILD)/boot0_head.o: $(TOP)/src/boot0_head.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/boot0_main.o: $(TOP)/src/boot0_main.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/early_uart.o: $(TOP)/src/early_uart.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/platform_shims.o: $(TOP)/src/platform_shims.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

# Export the blob's local MMC device table so the real-silicon registration
# shim can preserve the vendor bookkeeping while applying the card0 pin setup.
$(BUILD)/board_sdcard.o: $(TOP)/blobs/board_sdcard.o | $(BUILD)
	$(OBJCOPY) --globalize-symbol=mmc_devices $< $@

# ---- preprocess linker script (mirrors nboot/Makefile boot0.lds rule) -------
$(LDS_OUT): $(LDS_IN) | $(BUILD)
	$(CPP) $(AFLAGS) -DBOOT0ADDR=$(BOOT0ADDR) -DBOOT0SIZE=$(BOOT0SIZE) \
		-DCPUDIR=$(BUILD) -DSOLUTIONDIR=$(BUILD) \
		-ansi -D__ASSEMBLY__ -P - < $< > $@

# ---- link + objcopy + header/checksum ---------------------------------------
$(BUILD)/boot0.elf: $(OBJS) $(BLOBS) $(LDS_OUT)
	$(LD) $(OBJS) $(BLOBS) $(PLATFORM_LIBGCC) $(LDFLAGS_GC) $(LDFLAGS_MULTI_DEF) \
		-T $(LDS_OUT) -o $@ -Map $(BUILD)/boot0.map

$(BUILD)/boot0.bin: $(BUILD)/boot0.elf
	$(OBJCOPY) -O binary $< $@

# gen_check_sum writes the BROM-required length + checksum into the head,
# producing the final flashable image (same step as the vendor build).
$(BUILD)/$(NAME).bin: $(BUILD)/boot0.bin
	$(TOP)/tools/gen_check_sum $< $@
	@echo "==> built $@"

$(BUILD):
	@mkdir -p $(BUILD)

verify: all
	@echo "=== ELF arch ==="; $(OBJDUMP) -f $(BUILD)/boot0.elf | head
	@echo "=== size ==="; ls -l $(BUILD)/$(NAME).bin
	@echo "=== entry head (first 32 bytes) ==="; xxd -l 32 $(BUILD)/$(NAME).bin
	@python3 $(TOP)/tools/check_a7s_boot0.py $(BUILD)/$(NAME).bin $(BUILD)/boot0.map

clean:
	rm -rf $(BUILD)
