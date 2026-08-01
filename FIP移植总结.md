# A733 boot0 / AR100S FIP 移植总结

## 当前状态

- boot0 使用标准 TF-A FIP，支持主、备镜像，默认位于 SD/MMC LBA `32800` 和 `24576`。
- `SCP_BL2`、`BL31`、`BL33` 是必需项；`HW_CONFIG`（DTB）是可选项。
- 未提供 DTB 时，boot0 向 RTC 参数寄存器写入 `0`，AR100S 仍完成基础驱动和消息循环初始化。
- 未提供完整 DTB 时，AR100S 明确拒绝 DDR DFS 和深度待机请求，避免使用未初始化的 DRAM、电源树参数。
- AR100S 由本目录源码以主线 `riscv64-unknown-elf-gcc` 构建，不依赖已删除的厂商工具链压缩包。

## 启动顺序

1. boot0 初始化板级时钟、PMIC、DRAM 和 SD/MMC。
2. FIP 解析器严格检查 TOC、UUID、偏移、大小、重复项、数据重叠和包尾声明。
3. boot0 将 `BL31`、`BL33` 以及可选 `HW_CONFIG` 复制到各自 DRAM 地址；`SCP_BL2` 暂留在 FIP 缓冲区。
4. DRAM 中继代码保持 E902 复位，开启 E902 特殊时钟和配置总线，清空目标 SRAM 后复制 `SCP_BL2`。
5. 中继代码发布 DTB 地址（无 DTB 时为 `0`）和 E902 复位向量，再释放 E902。
6. 中继代码关闭 ARM MMU，将应用 CPU 的复位向量设置为 `BL31`，然后唤醒应用 CPU。
7. AR100S 依次初始化 CPUCFG、CCU、引脚、中断、参数、调试、TWI、PMU/BMU、消息箱、定时器、待机框架和看门狗，最后通知应用 CPU。

这个顺序保证 E902 在 SRAM 内容、复位向量和启动参数全部可见后才运行；消息通知发生在消息箱和定时器可用之后。

## 镜像布局

| 镜像 | 加载地址 | 最大大小 | 状态 |
|---|---:|---:|---|
| SCP_BL2 | `0x00044000` | `0x28000` | 必需 |
| BL31 | `0x48000000` | `0x100000` | 必需 |
| HW_CONFIG | `0x48100000` | `0x100000` | 可选 |
| BL33 | `0x4a000000` | `0x180000` | 必需 |

FIP 暂存于 `0x42e00000`，总大小最大 `2 MiB`。AR100S 链接入口为 E902 视角的 `0x40004000`，对应 CPUX 视角的 SRAM 地址 `0x00044000`。

## 无 DTB 模式

当前阶段可以不向 FIP 添加 `HW_CONFIG`：

```bash
fiptool create --align 512 \
  --scp-fw scp.bin \
  --soc-fw bl31.bin \
  --nt-fw u-boot.bin \
  fip.bin
```

无 DTB 时：

- AR100S 不访问空 DTB 地址，也不会因缺少 DTB 停机。
- 消息箱、定时器、看门狗、TWI 和 PMU/BMU 基础初始化仍执行。
- 调试 UART 引脚保持禁用，避免在未知板级复用关系下抢占引脚。
- DDR 调频和深度待机返回错误，不会使用空的 DRAM 或电源树参数。

以后 DTB 就绪后，可增加：

```bash
fiptool create --align 512 \
  --scp-fw scp.bin \
  --soc-fw bl31.bin \
  --nt-fw u-boot.bin \
  --hw-config sun60iw2p1-ar100s.dtb \
  fip.bin
```

生产 DTB 至少需要提供 `/dram` 下完整的 `dram_para00` 到 `dram_para95`。如需深度待机，`standby_param` 必须提供 `vdd-cpu`、`vdd-cpub`、`vdd-sys`、`vcc-pll`、`vcc-io`、`vcc-efuse` 和 `osc24m-on` 单值属性；当前仅用于日志的 `vdd-usb` 可选，缺省为 `0`。必需属性缺失或长度错误会关闭待机功能。

## AR100S 关键修正

- 修正 E902 入口、RV32E 主线工具链 ISA/CSR 兼容和 `__set_SP` 裸函数实现。
- 修正 A733 PPU 基址、系统定时器时钟选择、26 MHz DCXO 与 24 MHz SYS_CLK 的混用。
- 修正 A733 CPUS 看门狗寄存器偏移、写保护 key 和低速时钟配置。
- 修正 BSS 清零多写一个字、CPUCFG 小端窗口删除索引，以及 IOSC 校准除零/上溢路径。
- 邮箱接收 API 现在带显式缓冲容量；超长帧会被排空并拒绝，所有命令在访问参数前检查最小参数数目。
- DTB、DRAM 和待机属性按完整 FDT 头及单个 32 位单元严格校验。
- 链接脚本分离 RX/RW 段，并断言 BSS、栈和 SRAM 边界。

后续稳健性修正：

- 启动通知改为短超时（3 s）的尽力而为发送，迟到的应答在消息循环中被丢弃，避免在无 ARM 侧应答者时阻塞约 100 s。
- 邮箱接收路径改为有界等待（每字 100 ms），超时后尽力排空残余帧并清除 pending，避免部分帧导致后续协议失步。
- DTB 基址不再要求等于 `0x48100000`，改为接受 boot0 发布的 E902 视角 DRAM 地址（`0x40000000–0x7FF00000` 窗口内），仍以 FDT 头校验把关，并防止越界地址触发无 MMU 核的总线错误死循环。
- 运行时栈由 1 KB 扩大到 4 KB，上移到"仅 CPUS 可用"的 SRAM 顶区（`0x40033000–0x40034000`）。
- 系统定时器时钟源注释更正为 pll-ref（固定 24 MHz，依赖 REFPLL=24.000 MHz 前提）。
- 调试打印缓冲区加边界保护：`%s` 按剩余空间截断、整体输出限长（256 字节），并修正 `%c` 不推进指针的缺陷，杜绝未知消息 `hexdump` 路径的缓冲区溢出。

## 构建与验证

```bash
cd /home/denes/Allwinner/A7S/boot0-A7S
make clean
make verify-all
```

`verify-all` 会执行：

- boot0 交叉编译、BROM 头部/长度/校验和及链接布局检查；
- FIP 单元测试、ASan/UBSan 测试；
- 使用真实 `fiptool` 分别验证带 DTB 和无 DTB 的 FIP；
- AR100S `-Werror` 编译及 ELF32/RV32E/RVC/ISA 属性检查；
- AR100S 入口、LOAD 段权限、文件/SRAM/栈边界、未定义符号和 `__set_SP` 指令检查；
- `scp.elf` 到 `scp.bin` 的一致性、staged 副本一致性及闭源库哈希检查。

输出文件：

- `build/boot0_sdcard_sun60iw2p1.bin`
- `build/scp.bin`
- `build/scp.elf`

2026-08-01 从 `make clean` 开始的验证结果：

| 文件 | 大小 | SHA256 |
|---|---:|---|
| `build/boot0_sdcard_sun60iw2p1.bin` | 237568 | `e60bcf8586694e4fc81bad9ee20ae4e4b54e588480caf40e9a9bdb1f306a3516` |
| `build/scp.bin` | 118672 | `59cb4ef62bfcbb588dce3d0d228d4b24412a49622892139ecf149594a4c382cf` |
| `build/scp.elf` | 401824 | `dde1c5c6ba4a902e7d920837d3eb7dcc4134d677aa779a4ae3780e7e96fd1b42` |

AR100S 检查结果为入口 `0x40004000`、文件大小 `118672`、BSS 到栈底余量 `26820` 字节；ELF 为 RV32E/RVC soft-float ABI，且没有 RWX LOAD 段。

## 证据边界和剩余工作

- `blobs/libar100s.a` 是单成员 `obj-in.o` 的闭源 ELF 归档，固定 SHA256 为 `e1d0b91d4a3c8c4b67b65a03b901de5eb01b46b32743d245f287e9f1d57069e8`。其内部 DRAM DFS 行为无法通过当前源码完整审计。
- boot0 的 `libdram.o` 和 `board_sdcard.o` 同样包含厂商预编译代码；构建时出现的 executable-stack 警告来自 `board_sdcard.o`，不是新编译源码生成的段。
- 当前 TF-A 源码仍需一份经过 A733 实板验证的 BL31；能通过 FIP 格式检查不等于完整启动组合已在硬件上成立。
- 最终结论仍需要 A733 实板 UART 日志，至少覆盖冷启动、主/备 FIP、无 DTB 启动，以及 DTB 完成后的 DDR DFS/待机恢复。
