# A7S AR100S U-Boot 板测

本文只测试当前 `boot0-A7S` 已实现并可由 U-Boot 安全观测的路径。所有地址均为
A733 CPUX 视角。先刷入同一次构建生成的 boot0 和 FIP，不能混用旧产物。

## 1. 构建与刷写

```sh
cd /home/denes/Allwinner/A7S/boot0-A7S
make clean && make -j4 verify-all
./build_boot.sh --skip-scp --skip-uboot
sha256sum build/boot0_sdcard_sun60iw2p1.bin build/fip.bin
sudo ../Share/flash.sh /dev/sdX
```

`/dev/sdX` 必须替换为目标卡。刷写后完全断电再上电，不要只执行 warm reset。

## 2. 启动与固件驻留

在 U-Boot 执行：

```text
md.l 07032204 1
md.l 07050008 1
md.l 07010210 1
md.l 0701021c 1
md.l 00044000 10
md.l 07090118 2
md.l 0709010c 1
md.l 0300406c 1
```

判据：

- `0x07032204` 的入口读回应落在 `0x40004000` 对应的 E902 SRAM 区域。实板可能把
  实现位读回为 `0x40014000`，不能只按整字相等判断。
- `0x00044000` 必须是当前 `build/scp.bin` 的开头，而不是 eGON/SPL 头或全零。
- `0x07090118` 是 boot0 安全态复位读回标记：`0xb0070001` 表示 handoff 已执行且
  `0x07050008 bit0` 在安全态读回为 1。`0xb0070000` 表示 handoff 已执行，但 E902
  复位没有释放。`0x0709011c` 是 E902 入口标记：
  `0xe9020001..04` 分别表示完成首指令、栈清零、`mtvec/mtvt` 配置和 BSS 清零；
  `0xe90200ee` 表示进入过 trap handler，正常 timer/mailbox 异步中断也会写入该值，
  不能单独当作崩溃。若它与 `0x0709010c = 0x0000b00f`、queue 3 状态 `8` 同时出现，
  表示 daemon 已完成初始化并正常发出启动通知。两个寄存器都为零表示刷入的镜像
  不含该探针，或 boot0 尚未执行到 AR100S handoff，不能据此单独判断 E902 状态。
- `0x0701021c` 应为 `0x00010003`，`0x07010210 bit31`、`0x07050008 bit0` 应为 1。
  这是 A733 厂商 BL31 使用的 E902 启动序列；`0x07000400` 不在 A733 手册地址表中，
  不能用作 E902 复位寄存器。
- `0x0709010c` 正常最终状态为 `0x0000b00f`。`0xb000..0xb00e` 表示启动停在
  对应阶段；`0x0000bbXX` 表示 mandatory init 失败，低 8 位为错误码绝对值。
- CPUX mailbox queue 3 状态 `0x0300406c` 通常为 `8`：AR100S 一次性写入了完整的
  2-word header + 6-word version 启动通知，U-Boot 尚未消费它。它不能大于 8。

可在主机上直接比对 SCP 前 64 字节：

```sh
xxd -g4 -l 64 build/scp.bin
```

## 3. 读取启动通知

先确认状态是 8，再连续读取同一个 FIFO 数据寄存器 8 次。读取会弹出 FIFO 数据：

```text
md.l 0300406c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300407c 1
md.l 0300406c 1
```

期望：

- word 0 的 `type` 字段（bits 23:16）为 `0x90`，`attr`（bits 15:8）为 `0x02`。
- word 1 为 `0x00000006`。
- word 2..7 是零结尾的 `SUB_VER` 字符串，按小端 4 字节打包。
- 最后 queue 状态回到 0。

若初始状态不是 8，不要盲读 8 次；读取次数必须等于 `0x0300406c` 的低 4 位。

## 4. 安全 mailbox loopback

只有在上一节确认 CPUX queue 3 已清空后执行。向 CPUS queue 3 写入一个
`MESSAGE_LOOPBACK` hard-sync 请求，帧只有两个 word：

当前固件会等两个固定 header word 都进入 FIFO 后才开始解析，因此 U-Boot 的两次
`mw.l` 即使不是原子操作，也不会把帧拆开。旧固件可能在第一个 word 到达时立即消费，
表现为 `0x0709406c = 1` 且没有回复；遇到该状态不要继续发送，应刷入新 FIP 并冷启动。

```text
md.l 0300406c 1
md.l 0709406c 1
mw.l 0709407c 00610200
mw.l 0709407c 00000000
md.l 0709406c 1
md.l 0300406c 1
```

等待 `0x0300406c` 变为 2，然后读取回复：

```text
md.l 0300407c 1
md.l 0300407c 1
md.l 0300406c 1
```

回复判据：word 0 的 type 为 `0x61`、attr 为 `0x02`、result 为 0；word 1 为 0；
最后状态回到 0。这同时覆盖 ARM->E902 接收、daemon 分派、E902->ARM 回复和两向 FIFO。

## 5. 不要在 U-Boot 裸测的功能

不要用 `mw.l` 手工发送 `SYS_OP_REQ`、`ESSTANDBY_ENTER_REQ`、`SET_DDRFREQ` 或任意
PMIC rail 请求。它们会改变电源、DRAM 或时钟状态，且 U-Boot 没有事务级回滚能力。

当前 TF-A 只处理 ARISC startup/wait/PMU 兼容 SMC，尚未把 Linux 使用的 wakeup、DFS、
CRC 和 deep-standby SMC 映射成 mailbox RPC。因此上述启动与 loopback 测试通过，只能证明
AR100S 核心、HW_CONFIG 和 mailbox 路径正常；完整 deep standby/DFS 必须先补 TF-A RPC，
再从 Linux 做 suspend/resume、唤醒源、DRAM CRC 和非法参数测试。

## 6. 失败采集

失败时不要继续写寄存器，保存以下输出以及完整 UART 日志：

```text
md.l 0709010c 1
md.l 07090118 2
md.l 07032204 1
md.l 07050008 1
md.l 07010210 1
md.l 0701021c 1
md.l 00044000 10
md.l 0300406c 1
md.l 0709406c 1
md.l 07024004 1
md.l 07091000 20
```

同时记录所刷产物的 SHA256，避免把旧镜像行为归因到当前源码。
