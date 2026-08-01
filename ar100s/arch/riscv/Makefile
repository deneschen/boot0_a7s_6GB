obj-y += cpu/

obj-y += plat-$(CFG_CHIP_PLATFORM)/
CFLAGS-y += -Iarch/riscv/plat-$(CFG_CHIP_PLATFORM)/inc
CFLAGS-y += -Iservice/standby/suspend_resume/mem-$(CFG_CHIP_PLATFORM)/
LINK_RAM-y := arch/riscv/plat-$(CFG_CHIP_PLATFORM)/ram.ld

