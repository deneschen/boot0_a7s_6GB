SHELL=bash
# SUB_VER=1.0.0
PATH :=$(CURDIR)/tools/bin:$(PATH)
MAKEFLAGS += --no-print-directory
include scripts/Build.include

-include .config
include arch/$(subst ",,$(CFG_ARCH_PLATFORM))/Makefile

ifeq ($(V),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q=@
endif

ifneq ($(CFG_ARCH_PLATFORM),)
ifneq ($(CFG_RISCV),)
# Prefer a system RISC-V bare-metal toolchain (Ubuntu: gcc-riscv64-unknown-elf).
# Optional local vendor tree under tools/ is still searched first if present.
PATH :=$(CURDIR)/tools/riscv64-elf-x86_64-20201104/bin/:$(PATH)
CC=riscv64-unknown-elf-gcc
LD=riscv64-unknown-elf-ld
AR=riscv64-unknown-elf-ar
OBJC=riscv64-unknown-elf-objcopy
OBJDUMP=riscv64-unknown-elf-objdump
NM=riscv64-unknown-elf-nm
SIZE=riscv64-unknown-elf-size
STRIP=riscv64-unknown-elf-strip

# Mainline gcc notes (no functional change vs T-Head vendor gcc):
#  * -mtune=e906 dropped: private T-Head schedule hint
#  * _zicsr_zifencei: mainline binutils needs CSR/fence.i named explicitly
#  * T-Head CSR mnemonics replaced by numeric addresses in e907 sources
#  * picolibc headers if present (Ubuntu: picolibc-riscv64-unknown-elf)
PICOLIBC_INC := $(wildcard /usr/lib/picolibc/riscv64-unknown-elf/include)
MARCH_FLAGS ?= -mcmodel=medany -mabi=ilp32e -march=rv32emc_zicsr_zifencei \
	-fsingle-precision-constant \
	$(if $(PICOLIBC_INC),-I$(PICOLIBC_INC),)
MARCH_LD_FLAGS ?=-melf32lriscv

CFLAGS += -Iarch/riscv/includes/
else
$(error arch $(CFG_ARCH_PLATFORM) not support yet.)
endif
endif

CFLAGS += -Iinclude
CFLAGS += -Iinclude/driver
CFLAGS += -Iinclude/service
CFLAGS += -Iinclude/system
CFLAGS += -Ilibrary/libfdt/include
CFLAGS += $(CFLAGS-y)
CFLAGS += -Iinclude/generated
CFLAGS += -include include/generated/autoconf.h
CFLAGS += -include conf.h
# CFLAGS += -D"SUB_VER=BUILD_STR($(SUB_VER))"
# CFLAGS += -D"SUB_VER=$(SUB_VER)"


# must change to platform
# CFLAGS += -Iout/openrisc-plat-sun8iw12p1/include/generated

# must add from config
# CFLAGS += -DSUB_VER=3

# DRAM and ddr type
export CC LD AR OBJCOPY OBJDUMP STRIP LD_LIBRARY_PATH MARCH_LD_FLAGS
export CFLAGS
export HOSTCC := gcc
export CONFIG_ = CFG_




.PHONY : FORCE menuconfig $(FORCE)


define filechk_version
	(echo "/*";\
	echo " * THIS IS CREATE WITH VERSION CHK";\
	echo " * DO NOT CHANGE IT MANUAL";\
	echo " */";\
	echo ;\
	echo "#ifndef _SUB_VER_";\
	echo "#define _SUB_VER_";\
	echo ;\
	echo "#define SUB_VER \"`scripts/setlocalversion`\"";\
	echo ;\
	echo "#endif /* _SUB_VER_ */";)
endef

target_name = $(OUTPUT)scp
all: $(target_name).bin

sub_dir := arch driver library service system

prepare := include/generated/conf.h scripts/basic/fixdep
prepare += include/generated/autoconf.h

$(addprefix $(OUTPUT),$(addsuffix /obj-in.o,$(sub_dir))): $(prepare) FORCE
	$(Q)$(MAKE) -f scripts/Makefile.build dir=$(notdir $(patsubst %/,%,$(dir $@)))

clean:
	$(Q)echo CLEAN  objects
	$(Q)find -name '*.o' -delete -o  -name '.*.cmd' -delete -o -name '.*.d' -delete
	$(Q)find -name '*.tmp' -delete
	$(Q)find -wholename './scripts/basic/fixdep' -delete
	$(Q)find -path 'include/generated' -delete
#	$(Q)if test -d include;then rm include -r;fi
	$(Q)find -wholename './scp.*' -delete


include/generated/conf.h: Makefile FORCE
	$(call filechk,version)

# because old project use CFG_ so we change it
menuconfig:FORCE
	@scripts/mconf Kconfig

PHONY += savedefconfig

savedefconfig:FORCE
	$(Q)echo save defconfig to defconfig
	$(Q)scripts/conf --savedefconfig defconfig Kconfig

include/generated/autoconf.h:.config
	$(Q)mkdir -p include/generated
	$(Q)mkdir -p include/config
	$(Q)scripts/conf --silentoldconfig Kconfig

.config:
	$(Q)echo use make menuconfig to generate config file
	$(Q)echo
	$(Q)false


config_save:FORCE
	$(Q)scripts/conf --savedefconfig defconfig Kconfig
	$(Q)(read -e -i def -p '(Config file name?)' config_file;\
	if [ $${config_file} ]; \
	then \
	mv defconfig coco/arch/configs/$${config_file}_defconfig;\
       	fi;)

test:
	@echo CFLAGS_ARCH=$(CFLAGS-y)

$(notdir $(wildcard arch/configs/*_defconfig)): FORCE
	$(Q)scripts/conf --defconfig=arch/configs/$@ Kconfig

scripts/basic/fixdep: scripts/basic/fixdep.c
	$(Q)$(if $(OUTPUT),mkdir -p $(OUTPUT),)
	$(Q)gcc $^ -o $(if $(OUTPUT),$(OUTPUT:%/=%)/fixdep,$@)

#compiler config options
CFLAGS  += -g -c -nostdlib -Wall -Werror -Os $(MARCH_FLAGS)

#linker config options
LD_FLAGS= --stats -T$(LINK_RAM-y) $(MARCH_LD_FLAGS)

# all: ar100.elf  ar100.raw ar100.dis  system.map ar100.bin ar100.hex

$(target_name).dis: $(target_name).elf
	$(Q)$(OBJDUMP) -S -d $< > $@

$(target_name).raw: $(target_name).elf
	$(Q)$(OBJC) -O binary $< $@

$(target_name).elf:$(addsuffix /obj-in.o,$(addprefix $(OUTPUT),$(sub_dir))) FORCE
	@printf "\r\n\t--- Linking ---\r\n"
	$(Q)$(LD) -Map $(basename $@).map -Bstatic $(addprefix $(OUTPUT),$(addsuffix /obj-in.o,$(sub_dir))) $(LD_FLAGS) $(shell cat .lib) -o $@
	$(Q)$(NM) $@ | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort >> $(basename $@).map
		$(Q)$(OBJDUMP) -x -d -S $@ >> $(basename $@).map
	$(Q)$(SIZE) $(addprefix $(OUTPUT),$(addsuffix /obj-in.o,$(sub_dir))) $@

$(target_name).bin: $(target_name).raw
	cp $^ $@
	@if [ $${LICHEE_BRANDY_OUT_DIR} ];then cp $@ $${LICHEE_BRANDY_OUT_DIR}; \
		echo copy $@ to $${LICHEE_BRANDY_OUT_DIR};\
		fi

$(target_name).hex: $(target_name).bin
	bin2hex -i $^ -o $@ -s 0x4000

objs_size: FORCE
	$(Q)find $(OUTPUT) -name '*.o' -exec $(SIZE) {} \+
