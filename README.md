# 全志 A733 / A7S 初始化启动流程分析

> 分析对象：`/home/denes/Allwinner/A7S/u-boot-aw2501`、当前
> `/home/denes/Allwinner/A7S/boot0-A7S` 实现及 A7S 实机日志。
>
> 结论基于仓库源码、Makefile、ELF/镜像头、预编译对象符号与反汇编。A733 Boot ROM、BL31、DRAM 训练库和部分 boot0 板级库没有完整源码，因此本文明确区分“源码确认”“二进制确认”“实机确认”和“推断”，不把相邻的包装源码当成底层实现。

> 状态说明（2026-08-08）：本文保留了原厂 `u-boot-aw2501` 的 TOC1
> `boot_package` 分析，作为历史路径和证据记录。当前已经在 A7S 实机启动
> 的实现是 `boot0-A7S` 的标准 TF-A FIP 路径，不再加载 TOC1。

---

## 1. 核心结论

1. 当前 A7S 实机冷启动主链为：

   ```text
   BROM
     -> boot0 (AArch32，SRAM，初始化电源/DRAM/启动介质)
     -> TF-A FIP（主 LBA 32800，备 LBA 24576）
          |- SCP_BL2 / AR100S 固件 (RV32 RISC-V)
          |- BL31 monitor header + AArch64 EL3 runtime
          `- BL33 / U-Boot proper (AArch32)
     -> DRAM handoff stub（启动 SCP，RMR 切换至 BL31）
     -> U-Boot 重定位和板级初始化
     -> distro_bootcmd / extlinux
     -> booti
     -> BL31 SMC
     -> AArch64 Linux + DTB
   ```

2. boot0 和 U-Boot 都不是 AArch64：当前 `boot0.elf` 与 `src/u-boot` 均为 ELF32 ARM。AArch64 转换由 BL31 完成，Linux 才以 AArch64 运行。

3. DRAM 只在 boot0 阶段真正初始化。U-Boot 的 `dram_init()` 读取 boot0 写入 U-Boot 头部的容量，不重新训练 DRAM。

4. 实机已经证明 A7S 完成 UART0、AXP8191、LPDDR5 四 Pstate 训练、6GB 容量识别、SD0 初始化、FIP 加载、BL31 初始化以及跳转至 U-Boot `=>` 提示符。U-Boot 的 `mmc info` 也已识别 58.9GiB、4-bit、50MHz SD 卡。

5. FIP 读取仍使用 LBA 32800 和 24576，但它们是主/备 FIP，不是 TOC1。解析器校验 FIP TOC、UUID、偏移、长度、重复项与数据重叠；主镜像无效时才读取备镜像。

6. BL31 的卡死根因是 GIC-600 Redistributor 初始断电，清除 `GICR_WAKER.ProcessorSleep` 后 `ChildrenAsleep` 不会退出。启用 `GICV3_SUPPORT_GIC600 := 1` 后，TF-A 在访问 `WAKER` 前通过 `GICR_PWRR` 完成 Redistributor 上电，实机得以进入 U-Boot。

7. 当前尚未由本轮实机日志验证 Linux 引导、带 `HW_CONFIG` 的 FIP、AR100S DDR DFS 和深度待机。`BL31: No DTB found.` 在当前无 `HW_CONFIG` FIP 模式不是启动失败，但会限制这些运行期功能。

---

## 2. 证据等级与源码边界

| 等级 | 含义 | 本文示例 |
|---|---|---|
| 源码确认 | 可以直接沿 C/汇编调用链证明 | `boot0_main.c`、`board_f.c`、`board_r.c` |
| 构建确认 | Makefile/Kconfig 决定实际选择 | A733 -> `sun60iw2p1`、A7S -> A7A 包链接 |
| 二进制确认 | 从 ELF 头、镜像头、符号或反汇编确认 | U-Boot 是 ELF32 ARM、FIP 主/备 LBA、BL31/U-Boot 加载地址 |
| 实机确认 | 来自本次串口日志 | LPDDR5 2400MHz、6GB、FIP 加载、BL31、U-Boot 提示符与 SD 卡识别 |
| 推断 | 多项证据一致但缺少所有者源码 | BROM 的内部介质搜索和认证细节 |

boot0 的公开主流程只有包装层源码：

- `u-boot-aw2501/spl-pub/nboot/main/boot0_main.c`
- `u-boot-aw2501/spl-pub/nboot/main/boot0_head.c`
- `u-boot-aw2501/spl-pub/arch/arm/cpu/armv7/boot0_entry.S`

底层实现来自两个预编译 ELF32 ARM relocatable：

- `spl-pub/board/a733/libsun60iw2p1_sdcard.a`：MMC、TOC1 加载、跳转、部分 PMIC/时钟/板级逻辑；后缀虽为 `.a`，实际不是 archive。
- `dramlib/sun60iw2p1/spl_libdram/libdram`：真实 DRAM 控制器和 PHY 训练。

构建连接关系见：

- `u-boot-aw2501/spl-pub/board/a733/common.mk:5-10`
- `u-boot-aw2501/spl-pub/nboot/Makefile:47-77`

当前两个对象与 `boot0-A7S/blobs` 中的对象仍逐字节一致，但这只能证明复用了相同闭源对象，不能证明 boot0 已完整开源。

---

## 3. 构建目标与板级选择

### 3.1 SoC/板级映射

`spl-pub/board/a733/common.mk` 定义：

```make
SUPPORT_BOARD=a733
ARCH = arm
PLATFORM = sun60iw2p1
CFG_BOOT0_RUN_ADDR=0x44000
CFG_SYS_INIT_RAM_SIZE=0x10000
```

证据：`u-boot-aw2501/spl-pub/board/a733/common.mk:5-12`。

A7S 的 Linux/Brandy 配置仍选择统一 A733 defconfig：

```make
LICHEE_CHIP:=sun60iw2p1
LICHEE_ARCH:=arm64
LICHEE_BRANDY_DEFCONF:=sun60iw2p1_a733_defconfig
```

证据：`u-boot-aw2501/device-a733/configs/cubie_a7s/linux-5.15/BoardConfig.mk:1-6`。这里的 `LICHEE_ARCH=arm64` 描述系统/Linux 架构，不代表本树 U-Boot proper 的实际 ELF 类别。

### 3.2 A7S 没有独立的包构建目标

顶层产品只有 A5E、A7A、A7Z：`u-boot-aw2501/.github/local/Makefile.local:14-16`。A733 的 boot_package 规则也只有 A7A/A7Z：同文件 `210-224`。

Debian 安装时明确建立链接：

```text
usr/lib/u-boot/radxa-cubie-a7a -> usr/lib/u-boot/radxa-cubie-a7s
```

证据：`u-boot-aw2501/debian/u-boot-aw2501.links:1-2`。

因此当前仓库状态应理解为：A7S 板级配置存在，但发行包复用 A7A bootloader 产物。A7A 的 `sys_config.fex` 已包含多组 LPDDR5/LPDDR4 自动识别参数，而 A7S 自己的模板仍是固定 LPDDR4，二者不能混为一谈。

---

## 4. BROM 到 boot0

### 4.1 BROM 能确认什么

BROM 为片内 ROM，本仓库没有其源码。仓库能确认的是 boot0 必须提供：

- ARM 分支指令；
- `eGON.BT0` magic；
- 校验和与镜像长度；
- return/run address；
- 平台、UART、DRAM、存储 GPIO 等私有参数。

头结构定义在 `u-boot-aw2501/spl-pub/include/private_boot0.h:19-31`，头初始化在 `spl-pub/nboot/main/boot0_head.c:23-47`。

SD 卡的 boot0 写入位置由项目脚本确认是 LBA 256，即 128KiB：`u-boot-aw2501/setup/u-boot_setup-allwinner-a733.sh:21-27`。

### 4.2 运行地址存在两套事实

不要只写“boot0 在 0x44000”：

- `spl-pub` 当前源码构建配置是 `0x44000`：`u-boot-aw2501/spl-pub/board/a733/common.mk:9`。
- 仓库签入的厂商 SD boot0 头中 `ret_addr/run_addr` 都是 `0x47000`。
- 当前 `boot0-A7S/Makefile:25-31` 也按厂商头选择 `BOOT0ADDR=0x47000`。
- 当前独立构建 ELF 的 `_start` 位于 `0x47b00`，因为 `0x47000` 起始处先放约 0xB00 字节 boot0 头。

因此应区分“上游 spl-pub 默认链接地址 0x44000”和“当前可烧镜像声明的运行地址 0x47000”。

### 4.3 boot0 汇编入口

头部首条 ARM `B` 指令跳到头后的 `_start`。随后：

1. 切换到 AArch32 SVC 模式；
2. 屏蔽 IRQ/FIQ，选择 little-endian；
3. 关闭 MMU、D-cache、I-cache，保留 branch predictor；
4. 设置 SRAM 栈；
5. 清零 BSS；
6. 调用 C `main()`。

证据：`u-boot-aw2501/spl-pub/arch/arm/cpu/armv7/boot0_entry.S:9-41`。源码默认栈为 SRAM A2 顶部，见 `spl-pub/include/configs/sun60iw2p1.h:54-65`。

---

## 5. boot0 初始化主流程

主调用链来自 `u-boot-aw2501/spl-pub/nboot/main/boot0_main.c:36-145`：

```text
main
  -> sunxi_board_init_early
  -> sunxi_serial_init
  -> print_commit_log
  -> sunxi_board_init
  -> 检查 RTC FEL 标志 / 按键 / UART 热键
  -> sunxi_bootparam_load
  -> init_DRAM 或 sunxi_fpga_dram_init
  -> mmu_enable + malloc_init
  -> sunxi_board_late_init
  -> load_package
  -> load_image
  -> update_uboot_info
  -> mmu_disable
  -> 跳 monitor / OP-TEE / RTOS / U-Boot
失败 -> 清理时钟/MMU -> FEL_BASE
```

### 5.1 早期 UART0

本次日志开头：

```text
A7S BOOT0: early uart0 alive
HELLO! BOOT0 is starting!
BOOT0 commit : ec69381431
```

这些定制字符串来自当前 `boot0-A7S/src/boot0_main.c:38-54`，不是原始 `spl-pub` 主文件。定制头把 UART0 配为 PB9 TX、PB10 RX，见 `boot0-A7S/src/boot0_head.c:91-97`。

U-Boot 阶段的寄存器定义确认 UART0 基址为 `0x02500000`：`u-boot-aw2501/src/arch/arm/include/asm/arch-sunxi/plat-sun60iw2p1/cpu_autogen.h:18-23`。

### 5.2 PMIC 与 SD 供电

实机日志：

```text
A7S PMU: AXP8191 ready on TWI6
```

这是当前独立 boot0 增加的 A7S 板级实现：

1. 初始化 TWI6；
2. 在 runtime address `0x36` 读 AXP8191 chip ID；
3. 执行写保护/扩展寄存器序列；
4. 设置 AP reset 控制；
5. 打开 `DC1SW2`，给 SD 卡供电；
6. 为 DRAM 库提供 `set_ddr_voltage_ext()`。

证据：`boot0-A7S/src/platform_shims.c:181-235`、`294-335`。

注意：同文件 `338-340` 的 `sunxi_smc_en_with_glitch_workaround()` 当前为空实现。因此旧文档中“零打桩、每个接口都是厂商完整实现”的说法不成立。

### 5.3 LPDDR5 初始化

用户提供的实机结果：

| 项目 | 实机值 |
|---|---:|
| DRAM 类型 | 9 = LPDDR5 |
| DRAM VCC | 560mV |
| P0 | 2400MHz |
| P1 | 1200MHz |
| P2 | 800MHz |
| P3 | 400MHz |
| Training result | 四次均为 7 |
| 实际容量 | 6144MiB |
| para1 | `0xa10a` |
| para2 | `0x18001001` |
| dram_tpr13 | `0x10065` |

当前 A7S 独立 boot0 头提供 LPDDR5 初值：`boot0-A7S/src/boot0_head.c:57-90`。`main()` 调用 `init_DRAM()` 的位置是 `boot0-A7S/src/boot0_main.c:79-91`。

真正的控制器初始化、PHY training、Pstate 切换和容量探测位于预编译 `libdram`，不能从包装源码还原算法。日志证明该对象在当前 A7S 上完成了训练，但不证明仓库拥有其可重编译源码。

原始 `spl-pub` 还有一个容易误读的配置：`u-boot-aw2501/spl-pub/include/configs/sun60iw2p1.h:11` 直接定义了 `FPGA_PLATFORM`，所以 `u-boot-aw2501/spl-pub/nboot/main/boot0_main.c:74-78` 会选 `sunxi_fpga_dram_init()`。当前 `boot0-A7S` 删除该宏并优先链接真实 `libdram`，二者不是同一个构建结果。

### 5.4 DRAM 之后

DRAM 成功后，boot0：

- 再次检查 UART 输入：`2` 进入 FEL、`d` 增强调试、`q` 静默；
- 打开 MMU；
- 在 `0x40800000` 建立 16MiB heap；
- 初始化启动介质并读取 TOC1（原厂 `spl-pub` 历史路径）。

地址来自 `u-boot-aw2501/spl-pub/include/configs/sun60iw2p1.h:20-29`，流程来自 `spl-pub/nboot/main/boot0_main.c:91-120`。

当前 `boot0-A7S` 则在完成相同的板级初始化后读取 FIP，并不调用该 TOC1 加载路径。

---

## 6. 历史路径：SD/MMC 与 TOC1 boot_package

本节分析原厂 `u-boot-aw2501/spl-pub` 的 TOC1 加载实现和当时的故障日志；它不适用于当前已启动的 `boot0-A7S` FIP 镜像。当前 FIP 的主/备 LBA 虽相同，但格式、校验和加载逻辑不同。

### 6.1 历史日志已证明 SD 初始化完成

日志中的关键边界：

```text
card no is 0
***Try SD card 0***
HSSDR52/SDR25 4 bit
50000000 Hz
60350 MB
***SD/MMC 0 init OK!!!***
```

因此当前问题不是 UART、PMIC、DRAM 或 SD 控制器初始化失败，而是 SD 上的下一阶段内容无效。

### 6.2 boot0 实际检查两个 TOC1 位置

预编译 `libsun60iw2p1_sdcard.a` 的 `load_package()` 跳到 `load_toc1_from_sdmmc()`。其反汇编可确认：

1. 初始化 card0；
2. 调用 `sunxi_flashmap_toc1_start(1)`；
3. 调用 `sunxi_flashmap_toc1_bak_start(1)`；
4. 依次读取候选头；
5. 检查 magic、长度和加和校验；
6. 找不到合法候选则返回 4。

未被 boot0 头覆写时，两个位置是：

| 顺序 | 定义名称 | LBA | 字节偏移 |
|---|---|---:|---:|
| 1 | `UBOOT_START_SECTOR_IN_SDMMC` | 32800 (`0x8020`) | 16,793,600 |
| 2 | `UBOOT_BACKUP_START_SECTOR_IN_SDMMC` | 24576 (`0x6000`) | 12,582,912 |

常量证据：`u-boot-aw2501/spl-pub/include/spare_head.h:65-66`。项目脚本只写第二个位置：`u-boot-aw2501/setup/u-boot_setup-allwinner-a733.sh:24-26`。

### 6.3 `bad magic` / `error=4` 的精确含义

日志：

```text
error:bad magic.
error:bad magic.
Loading boot-pkg fail(error=4)
```

两次 `bad magic` 对应两个非零候选 LBA。错误码定义：

```c
E_SDMMC_FIND_BOOT1_ERR = 4
```

证据：`u-boot-aw2501/spl-pub/include/mmc_boot0.h:15-21`。

合法 `boot_package.fex` 的头部应为：

- offset `0x00`：`sunxi-package\0\0`；
- offset `0x10`：little-endian `0x89119800`，字节为 `00 98 11 89`。

结构和 magic 定义见 `u-boot-aw2501/spl-pub/include/private_toc.h:31-32`、`u-boot-aw2501/spl-pub/include/private_toc.h:58-75`。当前 `out/radxa-cubie-a7a/boot_package.fex` 符合该格式并包含 3 个 item。

### 6.4 当时最可能根因

按概率排序：

1. `boot_package.fex` 没有写到 LBA 24576；
2. 写完裸区后又执行了整盘 `dd if=card.img`，覆盖了 LBA 24576；
3. 写错了块设备；
4. 目标设备上的文件被截断或写入未完成；
5. boot0 头中的 flash map 被其他工具改成了不同 LBA。

该日志是 magic 失败而不是 checksum 失败，所以优先检查“位置/内容”，不要先怀疑 DRAM 或 TOC1 内各 item。

### 6.5 只读核验命令

将 `/dev/sdX` 替换成真实整盘设备，不要写分区节点：

```bash
# 仓库文件本身
xxd -l 32 u-boot-aw2501/out/radxa-cubie-a7a/boot_package.fex

# boot0 默认主候选
sudo dd if=/dev/sdX bs=512 skip=32800 count=1 status=none | xxd -l 32

# setup 脚本使用的备份候选
sudo dd if=/dev/sdX bs=512 skip=24576 count=1 status=none | xxd -l 32
```

至少 LBA 24576 应看到 `sunxi-package` 和 `00 98 11 89`。进一步比较：

```bash
sudo dd if=/dev/sdX of=/tmp/a7s-bootpkg-head.bin \
  bs=512 skip=24576 count=1 status=none
cmp -n 512 \
  u-boot-aw2501/out/radxa-cubie-a7a/boot_package.fex \
  /tmp/a7s-bootpkg-head.bin
```

`cmp` 返回 0 才表示首扇区一致。

---

## 7. 历史路径：TOC1 内容、加载地址与跳转

本节描述旧 TOC1 `boot_package` 的 item 布局和原厂 `boot0_jmp_monitor()`。当前 FIP handoff 的实现与入口规则见 §15。

### 7.1 历史 boot_package 内容

默认配置列出：

```ini
[package]
item=u-boot, u-boot.fex
item=monitor, monitor.fex
item=scp, scp.fex
;item=optee, optee.fex
```

证据：`u-boot-aw2501/device-a733/configs/default/boot_package.cfg:1-6`。Radxa A7A/A7Z 打包规则同样只有 U-Boot、BL31、SCP：`u-boot-aw2501/.github/local/Makefile.local:210-224`。

当前 A7A `boot_package.fex` 头显示 `items_nr=3`，内部 item 和地址为：

| item | 数据偏移 | 大小 | 运行/加载地址来源 | 地址 |
|---|---:|---:|---|---:|
| u-boot | `0x800` | 约 1.2MiB | U-Boot 私有头 offset `0x2c` | `0x4a000000` |
| monitor | `0x12d000` | 约 77KiB | monitor 私有头 offset `0x2c` | `0x48000000` |
| scp | `0x140400` | 约 115KiB | boot0 平台固定布局 | SRAM `0x44000` / DRAM `0x48100000` |

boot0 的通用 staging 区是 `0x42e00000`：`spl-pub/include/configs/sun60iw2p1.h:27-30`。

### 7.2 `load_image()` 做什么

`load_image()` 位于预编译板级对象中。其符号和 relocation-aware 反汇编确认它按 item 名处理：

- `u-boot`：读取目标地址并加载，返回 `uboot_base`；
- `monitor`：加载 BL31，返回 `monitor_base`；
- `optee`：若存在则加载并返回 `optee_base`；
- `scp`：先 assert AR100S，复制固件和 DTB/DRAM 参数，再 deassert；
- 还识别 `dtb`、`dtbo`、`logo`、`opensbi`、`rtos` 等可选 item。

item 名定义见 `u-boot-aw2501/spl-pub/include/private_toc.h:152-180`。SCP SRAM/DRAM 布局见 `spl-pub/include/configs/sun60iw2p1.h:27-51`。

### 7.3 把 boot0 结果传给 U-Boot

`update_uboot_info()` 将以下数据写入已加载 U-Boot 的私有头：

- TOC1 总长度；
- DRAM 扫描容量和 32 个 DRAM 参数；
- monitor/secure OS 是否存在；
- UART 端口和 GPIO；
- work mode、debug mode、备份标志；
- 启动介质参数。

证据：`u-boot-aw2501/spl-pub/nboot/main/boot0_main.c:148-197`。

这解释了 U-Boot 为什么不需要再次训练 DRAM，也解释了 U-Boot 如何知道启动卡号、容量和前级工作模式。

### 7.4 进入 BL31

有 monitor 时，boot0 先把 `secureos_base` 和 `nboot_base` 写进 monitor 头，然后调用 `boot0_jmp_monitor()`：`spl-pub/nboot/main/boot0_main.c:122-139`。

预编译对象的 `boot0_jmp_monitor` 反汇编显示它：

1. 把 BL31 地址写到 CPU reset-vector 低/高寄存器；
2. 设置 Reset Management Register 的 warm-reset/AArch64 位；
3. 执行 `DSB/ISB/WFI`；
4. CPU 以 AArch64 进入 `0x48000000` 的 BL31。

这是“boot0 AArch32 -> BL31 AArch64”的实际桥梁。BL31 二进制字符串还明确包含 `U-BOOT 32bit detected`，与当前 U-Boot ELF 一致。

---

## 8. U-Boot proper 初始化

### 8.1 U-Boot 是 AArch32

`src/configs/sun60iw2p1_a733_defconfig:3-17` 设置 `CONFIG_ARM=y`、`CONFIG_SYS_TEXT_BASE=0x4A000000` 和 `CONFIG_PHYS_64BIT=y`，但没有 `CONFIG_ARM64=y`。

当前产物：

```text
src/u-boot: ELF 32-bit LSB executable, ARM, EABI5
entry point: 0x4a000640
```

`CONFIG_PHYS_64BIT` 仅让物理地址类型支持 64 位，不改变指令集。U-Boot 私有头占到 `0x640`，所以镜像基址为 `0x4a000000`，真正 `_start` 为 `0x4a000640`。

### 8.2 reset 和最早期初始化

实际链路使用 ARMv7 文件，而不是 `armv8/start.S`：

```text
_start
  -> reset
  -> save_boot_params
  -> 设置 SVC32，屏蔽 IRQ/FIQ
  -> 设置 VBAR
  -> cpu_init_cp15
  -> cpu_init_crit
       -> lowlevel_init
            -> SRAM 临时栈/GD
            -> s_init
                 -> enable SMP bit
                 -> clock_init
                 -> timer_init
  -> _main
```

证据：

- `u-boot-aw2501/src/arch/arm/cpu/armv7/start.S:31-92`
- `u-boot-aw2501/src/arch/arm/cpu/armv7/lowlevel_init.S:23-69`
- `u-boot-aw2501/src/arch/arm/mach-sunxi/board.c:35-57`

A733 U-Boot 的 `clock_init_uart()` 直接返回，`clock_set_corepll()` 也直接返回：`src/arch/arm/mach-sunxi/clock_sun60iw2.c:20-23`、`63-66`。因此 U-Boot 依赖 boot0/BL31 留下的可用时钟和 UART，只读取多数 PLL 状态，不在这里重做完整 PLL 初始化。

### 8.3 `_main()`、`board_init_f()` 与重定位

`_main()`：

1. 在 SRAM 建初始栈和 global data；
2. 调用 `board_init_f(0)`；
3. 切换到 DRAM 中的新栈/GD；
4. `relocate_code()` 把 U-Boot 重定位到高端 DRAM 保留区；
5. 重定位向量，清 BSS；
6. 调用 `board_init_r()`。

证据：`u-boot-aw2501/src/arch/arm/lib/crt0.S:66-163`。

`board_init_f()` 的关键顺序来自 `src/common/board_f.c:832-979`：FDT、早期 malloc/log、CPU/SoC、DM、timer、环境、串口/console、DRAM 容量、内存保留和重定位布局。

这里的 `dram_init()` 不训练 DRAM，只读取 `uboot_spare_head.boot_data.dram_scan_size`：`src/board/sunxi/board.c:278-309`。当前代码还把 U-Boot 自己的 `gd->ram_size` 上限限制为 2048MiB；6GB 物理容量仍可通过后续 DT/boot 参数描述给内核，但 U-Boot 自身的重定位/分配视图按这段代码最多使用 2GB。

### 8.4 `board_init_r()` 与 Sunxi 板级初始化

重定位后的 `init_sequence_r` 位于 `src/common/board_r.c:915-1120`，关键步骤是：

```text
cache / relocated GD / malloc / DM
  -> board_init
  -> serial/console
  -> power_init_board
  -> Sunxi 平台 flash 初始化
  -> environment
  -> board_env_late_init
  -> board_late_init
  -> main_loop
```

其中 `board_init()`：

- 设置 Linux boot params 地址；
- 探测 secure mode/secure OS；
- 初始化 DMA；
- 探测 AXP/BMU/外部 PMU；
- 读取目标 boot clock（A733 setter 当前为空）；
- 处理 DT overlay。

证据：`u-boot-aw2501/src/board/sunxi/board.c:196-275`。

普通 `initr_mmc()` 在 Sunxi 平台被跳过，实际存储统一在 `initr_sunxi_plat()` 中通过 `sunxi_flash_init_ext()` 初始化，并处理 SPI、UFS、GPT、分区表和环境：`src/common/board_r.c:500-509`、`526-633`。

`board_late_init()` 再完成用户数据、分区信息、bootcmd、设备树替换/修正等：`src/board/sunxi/board_common.c:739-850`。

### 8.5 main loop、extlinux 与 Linux

`main_loop()`：初始化 CLI，执行 preboot，处理 bootdelay/autoboot，失败才进入交互 CLI。证据：`src/common/main.c:40-66`。

defconfig 启用 distro defaults，默认命令为 `run distro_bootcmd`。Sunxi 目标顺序为 FEL、USB、SD、NVMe、SCSI、UFS、eMMC/MMC2、PXE：

- `src/include/configs/sunxi-common.h:364-450`
- `src/include/config_distro_bootcmd.h:396-457`

每个设备分区都会查找 `/extlinux/extlinux.conf` 和 `/boot/extlinux/extlinux.conf`。仓库示例通过 `Image` 和 `sunxi.dtb` 启动：`device-a733/boot-resource/extlinux/extlinux.conf:1-4`。

PXE/extlinux 对无传统/FIT 头的 ARM64 `Image` 调用 `booti`：`src/cmd/pxe.c:888-901`。`booti` 准备 kernel、initrd、DTB 后进入 `do_bootm_states()`：`src/cmd/booti.c:19-89`。

最后 Sunxi `boot_jump_linux()`：

1. 关闭中断和设备；
2. 若配置 `CONFIG_ARISC_DEASSERT_BEFORE_KERNEL`，先通过 BL31 SMC 启动/同步 AR100S；
3. 若 monitor 存在，通过 `ARM_SVC_RUNNSOS` 把 kernel entry 和 DTB 地址交给 BL31；
4. BL31 完成异常级和执行状态切换，进入 AArch64 Linux。

证据：`u-boot-aw2501/src/arch/arm/lib/bootm.c:332-405`。

---

## 9. AR100S / SCP 在启动链中的位置

当前 `arisc/ar100s/scp.elf` 是 ELF32 RISC-V，标记为 RVC/RVE/soft-float。它不是主 CPU 的 boot0，也不串行替代 U-Boot。

生命周期是：

1. 在旧 TOC1 路径中，SCP 固件作为 `scp` item 打包；当前路径中它是 FIP 的 `SCP_BL2`；
2. 当前 FIP 解析器暂存 `SCP_BL2`，随后由 DRAM handoff stub 复制到 A733 SCP SRAM 并发布 DTB/复位向量；
3. BL31 提供 ARISC/SCP mailbox 和启动服务；
4. 当前 U-Boot 配置在进入内核前调用 BL31 的 ARISC startup SMC；
5. Linux 后续通过固件接口使用其电源、待机、唤醒等能力。

因此更准确的表述是“AR100S 与 AP 主引导协作并在运行期常驻”，不是“它与 boot0 无条件并行启动”或“只由 BL31 首次加载”。

---

## 10. 关键地址与镜像排布

### 10.1 内存地址

| 对象 | 地址 | 依据 |
|---|---:|---|
| 厂商/独立 boot0 header run addr | `0x00047000` | 当前镜像头 |
| `spl-pub` 默认 boot0 run addr | `0x00044000` | `u-boot-aw2501/spl-pub/board/a733/common.mk:9` |
| boot0 heap | `0x40800000`，16MiB | `spl-pub/include/configs/sun60iw2p1.h:23-25` |
| FIP staging | `0x42e00000` | `boot0-A7S/include/configs/sun60iw2p1.h` |
| FIP handoff stub | `0x42dff000` | FIP staging 前的 4KiB DRAM |
| BL31 monitor header | `0x48000000` | `bl31-monitor.bin` 私有头 |
| BL31 AArch64 entry | `0x48001000` | monitor header 后 `0x1000` |
| HW_CONFIG / SCP DTB 区 | `0x48100000` | FIP 可选 `HW_CONFIG` |
| U-Boot image base | `0x4a000000` | defconfig 与 U-Boot 头 |
| U-Boot `_start` | `0x4a000640` | `src/u-boot` ELF |
| U-Boot 内嵌/后备 DTB | `0x4a200000` | `src/arch/arm/mach-sunxi/board.c:170-177` |
| kernel_addr_r | `0x40080000` | `src/include/configs/sunxi-common.h:47-55` |
| fdt_addr_r | `0x4fa00000` | 同上 |
| ramdisk_addr_r | `0x4ff00000` | 同上 |

### 10.2 SD 卡裸区与分区

| LBA | 字节偏移 | 内容 | 状态 |
|---:|---:|---|---|
| 0 | 0 | MBR/GPT | 整盘镜像/分区工具 |
| 256 | 128KiB | boot0 | setup 脚本写入 |
| 2064 | 1056KiB | UFS boot0 兼容副本 | A733 setup 脚本写入 |
| 24576 | 12MiB | FIP 备镜像 | `Share/flash.sh` 写入 |
| 32800 | 16MiB + 16KiB | FIP 主镜像 | `Share/flash.sh` 写入 |

`sys_partition.fex` 的 `mbr.size=16384KiB` 说明常规分区从 16MiB 后开始：`device-a733/configs/cubie_a7s/debian/sys_partition.fex:10-14`。注意 LBA 32800 已位于 16MiB 边界之后；当前刷写脚本会同时写入主/备 FIP，并由 boot0 在主镜像无效时回退至 LBA 24576。

---

## 11. 正确烧录顺序

`build_boot.sh` 默认从相邻的 `../u-boot` 主线树构建
`cubie_a7s_defconfig`，输出放在本项目的
`build/u-boot-a733/u-boot.bin`，然后作为 BL33 封装进 FIP。U-Boot 源码树
和 boot0 项目之间只通过 `make uboot` 及该二进制产物连接，升级 U-Boot
不需要把源码复制进本项目。

单独构建并校验主线 U-Boot：

```bash
make uboot
```

复用已有构建可传 `--skip-uboot`；使用其他源码目录可传
`--uboot-dir DIR`；兼容旧厂商或其他预编译 BL33 时显式传
`--uboot FILE`。当前主线 BL33 是 ELF32 ARM，入口为 `0x4a000000`；上文
厂商 U-Boot 的 `0x4a000640` 入口和私有头布局只属于历史厂商镜像。

### 11.1 有整盘镜像时

```bash
# 1. 先写整盘镜像
sudo dd if=card.img of=/dev/sdX bs=4M conv=fsync status=progress

# 2. 构建当前 boot0、SCP、BL31、主线 U-Boot 和 FIP
cd /home/denes/Allwinner/A7S/boot0-A7S
./build_boot.sh

# 3. 先检查写入计划，再写 boot0 和两份 FIP
cd ../Share
./flash.sh --dry-run /dev/sdX \
  ../boot0-A7S/build/boot0_sdcard_sun60iw2p1.bin \
  ../boot0-A7S/build/fip.bin
sudo ./flash.sh /dev/sdX \
  ../boot0-A7S/build/boot0_sdcard_sun60iw2p1.bin \
  ../boot0-A7S/build/fip.bin
```

`flash.sh` 校验 boot0 的 `eGON.BT0` magic、FIP 的 TOC magic、512-byte 对齐和大小上限；然后将 boot0 写到 LBA 256，并把相同 FIP 写到 LBA 32800 与 LBA 24576。整盘镜像必须先写，最后再补写裸区引导链。

### 11.2 只更新 bootloader 时

```bash
cd /home/denes/Allwinner/A7S/boot0-A7S
./build_boot.sh
cd ../Share
sudo ./flash.sh /dev/sdX \
  ../boot0-A7S/build/boot0_sdcard_sun60iw2p1.bin \
  ../boot0-A7S/build/fip.bin
```

构建后的 FIP 可用实际 fiptool 检查：

```bash
/home/denes/Allwinner/A7S/arm-trusted-firmware/build/sun60i_a733/release/tools/fiptool/fiptool \
  info /home/denes/Allwinner/A7S/boot0-A7S/build/fip.bin
```

输出必须包含 `SCP_BL2`、`BL31` 和 `BL33`。FIP 文件头前四字节是小端 TOC magic `01 00 64 aa`，不是 TOC1 的 `sunxi-package`。

---

## 12. 当前工作树风险

### 12.1 A7S LPDDR5 文件当前不可烧录

`u-boot-aw2501/device-a733/bin/boot0_sdcard_sun60iw2p1_lpddr5.bin` 当前工作树版本只有 256 字节，内容是 `xxd` 文本，不是 eGON 二进制。Git 中原始版本为 131072 字节。

不要烧录当前 256 字节文件，也不要把“文件名带 `_lpddr5`”当成有效性证明。应先从可信构建产物恢复/重新生成，再检查：

```bash
stat -c '%s %n' u-boot-aw2501/device-a733/bin/boot0_sdcard_sun60iw2p1_lpddr5.bin
xxd -l 16 u-boot-aw2501/device-a733/bin/boot0_sdcard_sun60iw2p1_lpddr5.bin
```

有效 boot0 开头应包含 ARM branch 和 `eGON.BT0`，而不是字符 `00000000:`。

### 12.2 `boot0-A7S` 不是厂商 bin 的逐字节复刻

当前独立构建产物：

- `boot0-A7S/build/boot0_sdcard_sun60iw2p1.bin`：237568 字节；
- 厂商 `device-a733/bin/boot0_sdcard_sun60iw2p1.bin`：245760 字节；
- SHA256 不同，`cmp` 不同；
- 当前构建含自定义 early UART、AXP8191、SD pinmux 和一个空的 SMC workaround hook。

因此它应称为“复用厂商闭源对象并增加 A7S 适配的可链接 boot0”，不能称为“与官方 bin 逐字节一致、无打桩的完整源码重建”。

---

## 13. 串口日志的阶段判定

### 13.1 历史 TOC1 故障日志

以下日志发生在切换到 FIP 之前，保留用于解释 §6 的原厂 TOC1 路径：

```text
[66]  early UART                         成功
[74]  AXP8191/TWI6                       成功
[78]  DRAM 库启动                        成功
[85]  LPDDR5 2400MHz                     成功
[239..4516] 四 Pstate training           成功
[4525] 6GB 容量识别                      成功
[4545] DRAM simple test                  成功
[4551..4600] SD0 4-bit 50MHz             成功
[4610] LBA 32800 TOC1 magic              失败
[4618] LBA 24576 TOC1 magic              失败
[4620] E_SDMMC_FIND_BOOT1_ERR            返回 4
```

当时的最短修复路径不是改 boot0 初始化代码，而是：

1. 回读 LBA 24576；
2. 确认是否被整盘镜像覆盖；
3. 将有效 `boot_package.fex` 最后写到 LBA 24576；
4. 回读首扇区并与源文件比较；
5. 再次上电观察是否出现 `Loading boot-pkg Succeed` 和 `Jump to second Boot`。

### 13.2 当前 FIP 启动成功日志

当前镜像已经在实机上越过原 TOC1 故障点。关键边界为：

```text
FIP: primary=32800, backup=24576
FIP: loaded sector=32800, size=1383936
Jump to second Boot.
NOTICE:  BL31: Detected Allwinner A733 SoC (1903)
NOTICE:  BL31: No DTB found.
=>
=> mmc info
Device: SUNXI SD/MMC
Mode : SD High Speed (50MHz)
Capacity: 58.9 GiB
Bus Width: 4-bit
```

这证明 FIP 主镜像、BL31 到 BL33 的交接、U-Boot 入口与 SD 卡驱动均已在硬件上工作。`No DTB found.` 对应当前未在 FIP 中提供 `HW_CONFIG`，不是该次启动失败。

---

## 14. 关键文件索引

| 主题 | 文件 |
|---|---|
| boot0 头 | `u-boot-aw2501/spl-pub/nboot/main/boot0_head.c` |
| boot0 汇编入口 | `u-boot-aw2501/spl-pub/arch/arm/cpu/armv7/boot0_entry.S` |
| boot0 主流程 | `u-boot-aw2501/spl-pub/nboot/main/boot0_main.c` |
| boot0 A733 构建参数 | `u-boot-aw2501/spl-pub/board/a733/common.mk` |
| boot0 链接规则 | `u-boot-aw2501/spl-pub/nboot/Makefile` |
| 闭源板级对象 | `u-boot-aw2501/spl-pub/board/a733/libsun60iw2p1_sdcard.a` |
| 闭源 DRAM 对象 | `u-boot-aw2501/dramlib/sun60iw2p1/spl_libdram/libdram` |
| TOC1 格式 | `u-boot-aw2501/spl-pub/include/private_toc.h` |
| MMC boot0 错误码 | `u-boot-aw2501/spl-pub/include/mmc_boot0.h` |
| U-Boot defconfig | `u-boot-aw2501/src/configs/sun60iw2p1_a733_defconfig` |
| U-Boot ARM reset | `u-boot-aw2501/src/arch/arm/cpu/armv7/start.S` |
| U-Boot CRT/重定位 | `u-boot-aw2501/src/arch/arm/lib/crt0.S` |
| 早期 init 序列 | `u-boot-aw2501/src/common/board_f.c` |
| 重定位后 init 序列 | `u-boot-aw2501/src/common/board_r.c` |
| Sunxi board init | `u-boot-aw2501/src/board/sunxi/board.c` |
| A733 时钟读取/空 setter | `u-boot-aw2501/src/arch/arm/mach-sunxi/clock_sun60iw2.c` |
| late init/FDT/bootcmd | `u-boot-aw2501/src/board/sunxi/board_common.c` |
| extlinux/distro | `u-boot-aw2501/src/include/config_distro_bootcmd.h` |
| Linux 跳转/BL31 SMC | `u-boot-aw2501/src/arch/arm/lib/bootm.c` |
| A7S FEX | `u-boot-aw2501/device-a733/configs/cubie_a7s/sys_config.fex` |
| A7S Linux DTS | `u-boot-aw2501/device-a733/configs/cubie_a7s/linux-5.15/board.dts` |
| A733 烧录脚本 | `u-boot-aw2501/setup/u-boot_setup-allwinner-a733.sh` |
| A7S 定制 boot0 主流程 | `boot0-A7S/src/boot0_main.c` |
| A7S FIP 解析器 | `boot0-A7S/src/sunxi_fip.c` |
| A7S DRAM handoff 汇编 | `boot0-A7S/arch/armv7/fip_handoff.S` |
| A7S FIP 地址与限制 | `boot0-A7S/include/sunxi_fip.h`、`boot0-A7S/include/configs/sun60iw2p1.h` |
| A7S 构建与 FIP 打包 | `boot0-A7S/build_boot.sh` |
| A7S PMIC/MMC 适配 | `boot0-A7S/src/platform_shims.c` |
| A733 TF-A 平台和 GIC | `arm-trusted-firmware/plat/allwinner/sun60i_a733/platform.mk`、`arm-trusted-firmware/plat/allwinner/sun60i_a733/sunxi_gic.c` |
| GIC-600 Redistributor 电源驱动 | `arm-trusted-firmware/drivers/arm/gic/v3/gic-x00.c` |
| SD 卡安全刷写脚本 | `Share/flash.sh` |

---

## 15. 最终判断

当前 A7S 已经以 `boot0-A7S` 的 FIP 路径实机进入 U-Boot。boot0 从 LBA 32800 读取主 FIP，必要时回退到 LBA 24576；FIP 解析器加载 BL31、BL33 和可选 HW_CONFIG，并把 SCP_BL2 保留在 FIP 暂存区。DRAM handoff stub 位于 `0x42dff000`，负责将 SCP_BL2 复制到 SCP SRAM、发布启动参数，并通过 RMR 以 AArch64 从 `0x48001000` 进入 BL31。

BL31 的实机卡死根因已确认：A733 使用 GIC-600，Redistributor 在复位后处于断电状态。仅清除 `GICR_WAKER.ProcessorSleep` 会无限等待 `ChildrenAsleep`；启用 `GICV3_SUPPORT_GIC600` 后，TF-A 的 GIC-600 驱动先经 `GICR_PWRR` 完成上电，再执行通用 GICv3 初始化。该修改已经由进入 U-Boot 的实机日志验证。GIC 时钟配置保留以匹配厂商 U-Boot 早期设置，但单独设置时钟不足以解决该卡死。

仍须保持两个边界：boot0 的 DRAM 与部分板级代码仍依赖预编译对象；本轮只验证到 U-Boot 和 SD 卡识别，尚未验证 Linux 引导、带 HW_CONFIG 的 FIP、AR100S DDR DFS 或深度待机。后续回归应至少覆盖冷启动、主/备 FIP 回退、带/不带 HW_CONFIG，以及 Linux 启动和待机恢复。
