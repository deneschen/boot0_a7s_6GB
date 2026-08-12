#!/usr/bin/env python3
"""Fail-closed validation for the A733 AR100S firmware image."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


EXPECTED_BLOB_SHA256 = (
    "e1d0b91d4a3c8c4b67b65a03b901de5eb01b46b32743d245f287e9f1d57069e8"
)
SCP_ORIGIN = 0x40004000
SCP_FILE_MAX = 0x00028000
SCP_LINK_END = 0x40034000


def fail(message: str) -> None:
    raise SystemExit(f"AR100S verify: {message}")


def run(tool: Path | str, *args: str) -> str:
    result = subprocess.run(
        [str(tool), *args],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    if result.returncode != 0:
        fail(f"{' '.join([str(tool), *args])} failed:\n{result.stdout}")
    return result.stdout


def find_tool(root: Path, name: str) -> Path | str:
    local = root / "ar100s/tools/riscv64-elf-x86_64-20201104/bin" / name
    if local.is_file() and local.stat().st_mode & 0o111:
        return local
    system = shutil.which(name)
    if system:
        return system
    fail(f"required tool not found: {name}")


def symbol_values(nm_output: str) -> dict[str, int]:
    values: dict[str, int] = {}
    for line in nm_output.splitlines():
        fields = line.split()
        if len(fields) >= 3 and re.fullmatch(r"[0-9a-fA-F]+", fields[0]):
            values[fields[-1]] = int(fields[0], 16)
    return values


def disassembly_instructions(output: str) -> list[str]:
    instructions = []
    for line in output.splitlines():
        if not re.match(r"^\s*[0-9a-fA-F]+:\s", line):
            continue
        fields = line.split("\t", 2)
        if len(fields) == 3:
            instructions.append(fields[2].strip())
    return instructions


def c_function(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        fail(f"source function not found: {signature}")
    opening = source.find("{", start)
    if opening < 0:
        fail(f"source function has no body: {signature}")

    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    fail(f"source function has no closing brace: {signature}")


def integer_define(source: str, name: str) -> int:
    match = re.search(
        rf"^\s*#define\s+{re.escape(name)}\s+\(?\s*"
        rf"(0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*\s*\)?(?:\s|/|$)",
        source,
        re.MULTILINE,
    )
    if not match:
        fail(f"integer source constant not found: {name}")
    return int(match.group(1), 0)


def verify_source_contracts(root: Path) -> None:
    crt0 = (root / "ar100s/arch/riscv/cpu/e907/crt0.S").read_text()
    trace_markers = (
        "0xe9020001",
        "0xe9020002",
        "0xe9020003",
        "0xe9020004",
        "0xe90200ee",
    )
    if any(crt0.count(marker) != 1 for marker in trace_markers):
        fail("E902 early-boot breadcrumb sequence is missing or ambiguous")
    if crt0.count("0x0709011c") != len(trace_markers):
        fail("E902 breadcrumbs do not target RTC GP7")

    messages = (root / "ar100s/include/messages.h").read_text()
    fifo_words = integer_define(messages, "HWMSGBOX_FIFO_DEPTH")
    header_words = integer_define(messages, "MESSAGE_FRAME_HEADER_WORDS")
    startup_words = integer_define(messages, "STARTUP_NOTIFY_PARA_WORDS")
    if header_words + startup_words > fifo_words:
        fail("startup notification exceeds the hardware message-box FIFO")

    daemon = (root / "ar100s/system/daemon/daemon.c").read_text()
    notify = c_function(daemon, "static s32 startup_state_notify")
    if not re.search(r"struct\s+message\s+message\s*=\s*\{\s*0\s*\}", notify):
        fail("startup notification header is not zero-initialized")
    if not re.search(
        r"arisc_version\s*\[\s*STARTUP_NOTIFY_PARA_WORDS\s*\]", notify
    ):
        fail("startup notification payload is not bound to its FIFO contract")

    startup = c_function(daemon, "void startup_entry")
    intc_init = startup.find("intc_status = interrupt_init();")
    intc_check = startup.find("if (intc_status != OK)", intc_init)
    intc_failure_return = startup.find("return;", intc_check)
    global_enable = startup.find("cpu_enable_global_int();")
    if not 0 <= intc_init < intc_check < intc_failure_return < global_enable:
        fail("CLIC initialization failure does not keep global MIE disabled")

    hwmsgbox = (
        root
        / "ar100s/driver/hwmsgbox/hwmsgbox-extended/hwmsgbox-extended.c"
    ).read_text()
    sender = c_function(hwmsgbox, "s32 hwmsgbox_send_message")
    reservation = sender.find(
        "hwmsgbox_wait_queue_space(queue, frame_words, timeout)"
    )
    first_write = sender.find("writel(")
    if reservation < 0 or (first_write >= 0 and reservation > first_write):
        fail("single-FIFO messages are not reserved before transmission")

    receiver_query = c_function(hwmsgbox, "s32 hwmsgbox_query_message")
    if receiver_query.count(">=\n\t    MESSAGE_FRAME_HEADER_WORDS") < 2:
        fail("message receiver can consume a partial two-word header")

    clic = (root / "ar100s/driver/intc/clic.c").read_text()
    group_config = c_function(clic, "s32 intc_set_group_config")
    if "R_INTC_REG_BASE + 0x10 + reg_os" not in group_config:
        fail("group interrupt configuration uses the wrong register offset")
    if "readl(reg)" not in group_config or "writel(" not in group_config:
        fail("group interrupt configuration is not a 32-bit access")
    if "readb(" in group_config or "writeb(" in group_config:
        fail("group interrupt configuration still contains a byte access")

    clear_pending = c_function(clic, "s32 intc_interrupt_clear_pending")
    if re.search(r"\|\s*0x1", clear_pending) or not re.search(
        r"&\s*\(\s*~\s*0x1\s*\)", clear_pending
    ):
        fail("CLIC pending clear does not write CLICINTIP[0] low")

    clic_irq_max = integer_define(clic, "CLIC_IRQ_MAX")
    clic_info_mask = integer_define(clic, "CLIC_INFO_NUM_INTERRUPT_MASK")
    clic_reset = integer_define(clic, "CLIC_INT_RESET_VALUE")
    if clic_irq_max != 80 or clic_info_mask != 0x1FFF:
        fail("CLIC_INFO interrupt-count bounds do not match the A733 contract")
    if clic_reset & 0x101:
        fail("CLIC warm-reset value leaves CLICINTIE or CLICINTIP set")

    intc_initialize = c_function(clic, "s32 intc_init")
    compact_init = re.sub(r"\s+", " ", intc_initialize)
    if not re.search(
        r"irq_count = readl\(CLIC_INFO\) & CLIC_INFO_NUM_INTERRUPT_MASK",
        compact_init,
    ) or not re.search(
        r"irq_count < IRQ_SOUCE_MAX \|\| irq_count > CLIC_IRQ_MAX",
        compact_init,
    ):
        fail("intc_init does not validate the implemented CLIC interrupt count")
    reset_loop = re.search(
        r"for \(intno = 0; intno < irq_count; intno\+\+\)", compact_init
    )
    reset_write = intc_initialize.find(
        "writel(CLIC_INT_RESET_VALUE, CLIC_INT_REG(intno));"
    )
    cfg_reset = intc_initialize.find("writel(CLIC_CFG_RESET_VALUE, CLIC_CFG);")
    threshold_reset = intc_initialize.find("writel(0, CLIC_MINTTHRESH);")
    if (
        not reset_loop
        or reset_write < 0
        or cfg_reset < reset_write
        or threshold_reset < cfg_reset
    ):
        fail("intc_init does not sanitize the complete CLIC warm-reset state")

    intc_manager = (root / "ar100s/driver/intc/intc_manager.c").read_text()
    manager_init = c_function(intc_manager, "s32 interrupt_init")
    controller_call = manager_init.find("ret = intc_init();")
    controller_check = manager_init.find("if (ret != OK)", controller_call)
    controller_return = manager_init.find("return ret;", controller_check)
    table_init = manager_init.find("for (index = 0;", controller_return)
    if not 0 <= controller_call < controller_check < controller_return < table_init:
        fail("interrupt_init discards a CLIC initialization failure")

    # Model both valid hardware variants and corrupt/unsupported CLIC_INFO
    # values independently of the target compiler's loop lowering.
    for reported_count in (0, 34, 35, 64, 80, 81, clic_info_mask):
        accepted = 35 <= reported_count <= clic_irq_max
        registers = (
            [0xE0801000 + 4 * intno for intno in range(reported_count)]
            if accepted
            else []
        )
        if accepted and (
            len(registers) != reported_count
            or registers[0] != 0xE0801000
            or registers[-1] > 0xE080113C
        ):
            fail("CLIC_INFO model produces an invalid warm-reset register walk")
        if not accepted and registers:
            fail("CLIC_INFO model does not fail closed on an invalid count")

    pmu = (root / "ar100s/driver/pmu/pmu_axp8191.c").read_text()
    unchecked_twi = [
        line.strip()
        for line in pmu.splitlines()
        if re.search(r"\bpmu_reg_(?:read|write)\s*\(", line)
        and "ret =" not in line
        and not re.search(r"\bif\s*\(", line)
    ]
    if unchecked_twi:
        fail(f"AXP8191 contains unchecked TWI access: {unchecked_twi[0]}")

    softset = c_function(pmu, "static s32 _axp8191_pmu_softset")
    softset_write = softset.find("ret = pmu_reg_write(")
    softset_check = softset.find("if (ret != OK)", softset_write)
    softset_loop = softset.find("while (1)")
    if (
        "u8 data = 0;" not in softset
        or "ret = pmu_reg_read(" not in softset
        or softset_write < 0
        or not softset_write < softset_check < softset_loop
        or "return ret;" not in softset[softset_check:softset_loop]
    ):
        fail("AXP8191 soft-set can hang after an unhandled TWI failure")

    set_voltage = c_function(pmu, "static s32 axp8191_pmu_set_voltage_state")
    required_voltage_checks = (
        "type >= AXP8191_POWER_MAX",
        "state > POWER_VOL_ON",
        "u8 data = 0;",
        "ret = pmu_reg_read(",
        "ret = pmu_reg_write(",
    )
    if any(check not in set_voltage for check in required_voltage_checks):
        fail("AXP8191 rail state path lacks bounds or TWI error handling")

    reset = c_function(pmu, "static s32 axp8191_pmu_reset")
    charging_reset = c_function(pmu, "static s32 axp8191_pmu_charging_reset")
    if "return ret;" not in reset or "return _axp8191_pmu_softset(1);" not in reset:
        fail("AXP8191 reset does not propagate TWI or soft-set failures")
    if (
        "return ret;" not in charging_reset
        or "return _axp8191_pmu_softset(1);" not in charging_reset
    ):
        fail("AXP8191 charging reset does not propagate TWI failures")

    pmu_wrapper = (
        root / "ar100s/driver/pmu/pmu-sun60iw2p1/pmu.c"
    ).read_text()
    wrapper_returns = (
        ("s32 pmu_shutdown", "return pmu_ops_p->pmu_shutdown();"),
        ("s32 pmu_reset", "return pmu_ops_p->pmu_reset();"),
        ("s32 pmu_charging_reset", "return pmu_ops_p->pmu_charging_reset();"),
    )
    for signature, expected_return in wrapper_returns:
        if expected_return not in c_function(pmu_wrapper, signature):
            fail(f"PMU wrapper does not propagate {signature} result")

    wakeup = (root / "ar100s/service/standby/wakeup_source.c").read_text()
    irq_exit = c_function(wakeup, "static int irq_wakesource_exit")
    irq_exit_steps = (
        "interrupt_set_mask(",
        "interrupt_disable(",
        "interrupt_clear_pending(",
        "uninstall_isr(",
    )
    step_positions = [irq_exit.find(step) for step in irq_exit_steps]
    if any(position < 0 for position in step_positions) or step_positions != sorted(
        step_positions
    ):
        fail("wakeup IRQ exit does not mask, disable, clear, then uninstall")
    if re.search(r"(?<!un)install_isr\s*\(", irq_exit):
        fail("wakeup IRQ exit installs an ISR instead of removing it")

    group_exit = c_function(wakeup, "static int group_irq_exit")
    if "return interrupt_set_group_config(group_irq_num, FALSE);" not in group_exit:
        fail("wakeup group IRQ exit hides controller configuration failures")

    clear_wakeup = c_function(wakeup, "s32 clear_wakeup_src")
    required_timer_clear = (
        "wakeup_timer_stop();",
        "wakeup_timer.cycle = 0;",
        "wakeup_timer.expires = 0;",
        "default_wakeup_handler",
    )
    if any(check not in clear_wakeup for check in required_timer_clear):
        fail("wakeup source clear leaves timer or IRQ state installed")

    standby = (
        root
        / "ar100s/service/standby/standby-sun60iw2p1/plat_standby.c"
    ).read_text()
    dts_parse = c_function(standby, "static s32 standby_dts_parse")
    if 'fdt_path_offset(fdt, "/standby_param")' not in dts_parse:
        fail("standby parameters are not resolved by absolute DT path")

    rail_restore = c_function(standby, "static s32 dm_restore_off_rails")
    if (
        "dm_off_order[--dm_off_count]" not in rail_restore
        or "failed_order" not in rail_restore
    ):
        fail("standby rail rollback is not reverse-order and retryable")
    rail_off = c_function(standby, "static s32 dm_power_off_mask")
    if "ret = pmu_set_voltage_state(" not in rail_off or "return ret;" not in rail_off:
        fail("standby rail shutdown ignores PMU failures")
    suspend = c_function(standby, "static s32 dm_suspend")
    if "goto rollback;" not in suspend or "dm_restore_off_rails()" not in suspend:
        fail("standby rail shutdown has no rollback path")

    process_init = c_function(standby, "static s32 standby_process_init")
    process_exit = c_function(standby, "static s32 standby_process_exit")
    standby_entry = c_function(standby, "static s32 standby_entry")
    if "ret = dm_suspend();" not in process_init or "return ret;" not in process_init:
        fail("standby process does not propagate rail shutdown failure")
    if (
        "ret = dm_resume();" not in process_exit
        or "first_error = ret;" not in process_exit
        or "return first_error;" not in process_exit
    ):
        fail("standby process does not propagate rail restore failure")
    if standby_entry.count("standby_process_exit(pmessage,") < 2:
        fail("failed standby entry does not restore the suspended system")

    pll_restore = c_function(standby, "static s32 all_pll_set")
    if (
        "pll_restore[i].value = readl(pll_restore[i].addr);" not in pll_restore
        or "STANDBY_HW_POLL_LIMIT" not in pll_restore
        or "return -ETIMEOUT;" not in pll_restore
    ):
        fail("PLL restore lacks a complete register snapshot or lock timeout")
    bus_restore = c_function(standby, "static void bus_clock_ctl")
    if (
        "bus_ctl_value[bus_tick] = reg_val;" not in bus_restore
        or "writel(bus_ctl_value[bus_tick], bus_ctl_addr[bus_tick]);"
        not in bus_restore
    ):
        fail("standby bus clocks are not restored from complete register snapshots")
    ppu_resume = c_function(standby, "static s32 ppu_resume")
    cpu_pll_on = c_function(standby, "static s32 cpu_pll_on")
    if "return -ETIMEOUT;" not in ppu_resume:
        fail("PPU resume can wait forever")
    if cpu_pll_on.count("return -ETIMEOUT;") < 3:
        fail("CPU PLL restore lacks bounded update and lock waits")

    timer_delay = (
        root
        / "ar100s/driver/timer/timer-extended/timer-extended-delay.c"
    ).read_text()
    delay_wait = c_function(timer_delay, "static s32 delay_wait_pending")
    mdelay = c_function(timer_delay, "void time_mdelay")
    if (
        "cpucfg_counter_read() >= deadline" not in delay_wait
        or "(u64)(ms + 1U) * 24000U" not in mdelay
    ):
        fail("millisecond delay timeout is not tied to the 24 MHz counter")

    uart = (root / "ar100s/driver/uart/uart.c").read_text()
    uart_putc = c_function(uart, "s32 uart_putc")
    uart_clock = c_function(uart, "s32 uart_clkchangecb")
    uart_puts = c_function(uart, "s32 uart_puts")
    if "UART_POLL_LIMIT" not in uart_putc or "return -ETIMEOUT;" not in uart_putc:
        fail("UART transmit can wait forever")
    if (
        "UART_POLL_LIMIT" not in uart_clock
        or "ret = uart_set_baudrate(uart_rate);" not in uart_clock
        or "return ret;" not in uart_clock
    ):
        fail("UART clock-change callback hides timeout or baud-rate failures")
    if uart_puts.count("if (ret != OK)") < 2:
        fail("UART string output discards character transmit failures")

    sys_op = c_function(standby, "int sys_op")
    required_sys_op_returns = (
        "ret = pmu_charging_reset();",
        "return system_shutdown();",
        "return system_reset();",
    )
    if any(check not in sys_op for check in required_sys_op_returns):
        fail("system power operation hides PMU failures from its caller")

    fake_poweroff = c_function(standby, "s32 fake_poweroff")
    get_pmu_irq = c_function(standby, "s32 get_pmu_irq")
    if "return -EFAIL;" not in fake_poweroff or "return -EFAIL;" not in get_pmu_irq:
        fail("unimplemented standby operations report false success")


def verify_trap_context(objdump: Path | str, elf: Path) -> None:
    output = run(objdump, "-d", "--disassemble=__synchronous_exception", str(elf))
    instructions = disassembly_instructions(output)
    call_index = next(
        (index for index, insn in enumerate(instructions) if "<hadle_trap>" in insn),
        -1,
    )
    if call_index < 0:
        fail("trap entry does not call hadle_trap")

    caller_saved = ("ra", "t0", "t1", "t2", "a0", "a1", "a2", "a3", "a4", "a5")
    offsets = set()
    for register in caller_saved:
        save = next(
            (
                (index, int(match.group(1), 0))
                for index, insn in enumerate(instructions)
                if (match := re.fullmatch(rf"sw\s+{register},(-?(?:0x)?[0-9a-f]+)\(sp\)", insn))
            ),
            None,
        )
        restore = next(
            (
                (index, int(match.group(1), 0))
                for index, insn in enumerate(instructions)
                if (match := re.fullmatch(rf"lw\s+{register},(-?(?:0x)?[0-9a-f]+)\(sp\)", insn))
            ),
            None,
        )
        if not save or not restore:
            fail(f"trap entry does not save and restore RV32E register {register}")
        if save[0] >= call_index or restore[0] <= call_index or save[1] != restore[1]:
            fail(f"trap entry corrupts the saved slot for RV32E register {register}")
        if save[1] in offsets:
            fail("trap entry aliases two caller-saved register slots")
        offsets.add(save[1])

    stack_down = next(
        (
            int(match.group(1), 0)
            for insn in instructions
            if (match := re.fullmatch(r"addi\s+sp,sp,-((?:0x)?[0-9a-f]+)", insn))
        ),
        None,
    )
    stack_up = next(
        (
            int(match.group(1), 0)
            for insn in instructions
            if (match := re.fullmatch(r"addi\s+sp,sp,((?:0x)?[0-9a-f]+)", insn))
        ),
        None,
    )
    if not stack_down or stack_down != stack_up or stack_down % 16:
        fail("trap entry stack frame is not balanced and 16-byte aligned")
    if not any(insn == "mret" for insn in instructions):
        fail("trap entry does not return with mret")
    forbidden = ("a6", "a7", "t3", "t4", "t5", "t6")
    if any(re.search(rf"\b{register}\b", "\n".join(instructions)) for register in forbidden):
        fail("trap entry uses registers which do not exist in RV32E")


def verify_interrupt_contracts(objdump: Path | str, elf: Path) -> None:
    start_output = run(objdump, "-d", "--disassemble=_start", str(elf))
    start = disassembly_instructions(start_output)
    if not start or not re.fullmatch(r"(?:csrc|csrci)\s+mstatus,8", start[0]):
        fail("_start does not clear mstatus.MIE as its first instruction")
    if any(re.fullmatch(r"(?:csrs|csrsi)\s+mstatus,.*", insn) for insn in start):
        fail("_start enables mstatus.MIE before interrupt_init")
    lowered_start = start_output.lower()
    if "709011c" not in lowered_start or any(
        f"e902000{stage}" not in lowered_start for stage in range(1, 5)
    ):
        fail("built _start is missing the E902 RTC breadcrumb sequence")

    trap_output = run(
        objdump, "-d", "--disassemble=__synchronous_exception", str(elf)
    ).lower()
    if "709011c" not in trap_output or "e90200ee" not in trap_output:
        fail("built trap entry is missing the E902 exception breadcrumb")

    startup = run(objdump, "-d", "--disassemble=startup_entry", str(elf))
    intc_init = startup.find("<interrupt_init>")
    global_enable = startup.find("<cpu_enable_global_int>")
    if intc_init < 0 or global_enable < intc_init:
        fail("built startup_entry does not enable MIE after interrupt_init")

    enable = disassembly_instructions(
        run(objdump, "-d", "--disassemble=cpu_enable_global_int", str(elf))
    )
    if not any(
        re.fullmatch(r"(?:csrs|csrsi)\s+mstatus,8", insn) for insn in enable
    ):
        fail("cpu_enable_global_int does not set mstatus.MIE")


def verify_group_interrupt_config(objdump: Path | str, elf: Path) -> None:
    output = run(objdump, "-d", "--disassemble=intc_set_group_config", str(elf))
    instructions = disassembly_instructions(output)
    if "7024010" not in output.lower():
        fail("built group interrupt configuration address is not 0x07024010")
    if not any(re.match(r"lw\s", insn) for insn in instructions):
        fail("built group interrupt configuration has no 32-bit read")
    if not any(re.match(r"sw\s", insn) for insn in instructions):
        fail("built group interrupt configuration has no 32-bit write")
    if any(re.match(r"(?:lbu|lb|sb)\s", insn) for insn in instructions):
        fail("built group interrupt configuration contains a byte access")


def verify_clic_initialization(objdump: Path | str, elf: Path) -> None:
    output = run(objdump, "-d", "--disassemble=intc_init", str(elf))
    instructions = disassembly_instructions(output)
    lowered = output.lower()
    if "0x1fc00" not in lowered or not any(
        re.fullmatch(r"lw\s+[^,]+,4\([^)]*\)", insn) for insn in instructions
    ):
        fail("built intc_init does not use CLIC_INFO for its reset range")
    if not any(re.fullmatch(r"sw\s+zero,8\([^)]*\).*", insn) for insn in instructions):
        fail("built intc_init does not clear CLIC_MINTTHRESH")
    if not any(re.match(r"bltu\s", insn) for insn in instructions) or not any(
        re.match(r"bne\s", insn) for insn in instructions
    ):
        fail("built intc_init does not validate and walk the CLIC_INFO range")

    manager = disassembly_instructions(
        run(objdump, "-d", "--disassemble=interrupt_init", str(elf))
    )
    intc_call = next(
        (index for index, insn in enumerate(manager) if "<intc_init>" in insn), -1
    )
    error_branch = next(
        (
            index
            for index, insn in enumerate(manager)
            if index > intc_call and re.match(r"bnez\s+a0,", insn)
        ),
        -1,
    )
    if intc_call < 0 or error_branch < intc_call:
        fail("built interrupt_init discards an intc_init failure")


def verify(args: argparse.Namespace) -> None:
    root = Path(__file__).resolve().parent.parent
    elf = Path(args.elf)
    binary = Path(args.binary)
    blob = Path(args.blob)
    staged = Path(args.staged) if args.staged else None

    for path in (elf, binary, blob):
        if not path.is_file():
            fail(f"missing file: {path}")

    readelf = find_tool(root, "riscv64-unknown-elf-readelf")
    nm = find_tool(root, "riscv64-unknown-elf-nm")
    objdump = find_tool(root, "riscv64-unknown-elf-objdump")
    objcopy = find_tool(root, "riscv64-unknown-elf-objcopy")
    ar = find_tool(root, "riscv64-unknown-elf-ar")

    verify_source_contracts(root)

    header = run(readelf, "-h", str(elf))
    if "ELF32" not in header or "little endian" not in header or "RISC-V" not in header:
        fail("ELF is not 32-bit little-endian RISC-V")
    entry_match = re.search(r"Entry point address:\s*(0x[0-9a-fA-F]+)", header)
    if not entry_match or int(entry_match.group(1), 16) != SCP_ORIGIN:
        fail("ELF entry point is not _start at 0x40004000")
    if "RVE" not in header or "RVC" not in header:
        fail("ELF flags do not advertise the required RV32E/RVC ABI")

    attributes = run(readelf, "-A", str(elf)).lower()
    arch_match = re.search(r'tag_riscv_arch:\s*"([^"]+)"', attributes)
    if not arch_match:
        fail("ELF has no Tag_RISCV_arch attribute")
    arch_tokens = arch_match.group(1).split("_")
    base_isa = arch_tokens[0]
    if not base_isa.startswith("rv32e"):
        fail(f"ELF base ISA is not RV32E: {base_isa}")
    for extension in ("m", "c"):
        if not any(re.fullmatch(rf"{extension}[0-9].*", token) for token in arch_tokens[1:]):
            fail(f"ELF attributes are missing ISA extension {extension}")
    for extension in ("zicsr", "zifencei"):
        if not any(token.startswith(extension) for token in arch_tokens[1:]):
            fail(f"ELF attributes are missing ISA extension {extension}")

    symbols = symbol_values(run(nm, "-n", str(elf)))
    required = (
        "_start",
        "__bss_end",
        "metal_segment_stack_begin",
        "metal_segment_stack_end",
    )
    missing = [name for name in required if name not in symbols]
    if missing:
        fail(f"missing linker symbols: {', '.join(missing)}")
    if symbols["_start"] != SCP_ORIGIN:
        fail("_start does not match the CPUX/E902 SRAM alias contract")
    if symbols["__bss_end"] > symbols["metal_segment_stack_begin"]:
        fail("allocated image overlaps the runtime stack")
    if not (
        symbols["metal_segment_stack_begin"]
        < symbols["metal_segment_stack_end"]
        <= SCP_LINK_END
    ):
        fail("runtime stack is outside the reserved SRAM window")

    program_headers = run(readelf, "-W", "-l", str(elf))
    load_segments = []
    for line in program_headers.splitlines():
        fields = line.split()
        if not fields or fields[0] != "LOAD" or len(fields) < 8:
            continue
        vaddr = int(fields[2], 16)
        file_size = int(fields[4], 16)
        memory_size = int(fields[5], 16)
        flags = "".join(fields[6:-1])
        if "W" in flags and "E" in flags:
            fail("ELF contains a writable and executable load segment")
        load_segments.append((vaddr, file_size, memory_size))
    if not load_segments:
        fail("ELF has no load segments")
    if min(segment[0] for segment in load_segments) != SCP_ORIGIN:
        fail("first ELF load segment does not start at 0x40004000")
    file_end = max(vaddr + file_size for vaddr, file_size, _ in load_segments)
    memory_end = max(vaddr + memory_size for vaddr, _, memory_size in load_segments)
    if file_end - SCP_ORIGIN != binary.stat().st_size:
        fail("raw image size does not match the ELF file-backed address span")
    if binary.stat().st_size > SCP_FILE_MAX:
        fail("raw image exceeds the boot0 SRAM copy window")
    if memory_end > symbols["metal_segment_stack_begin"]:
        fail("ELF load memory crosses the runtime stack boundary")

    if run(nm, "-u", str(elf)).strip():
        fail("ELF contains undefined symbols")

    set_sp = run(objdump, "-d", "--disassemble=__set_SP", str(elf))
    instructions = [
        line
        for line in set_sp.splitlines()
        if re.match(r"^\s*[0-9a-fA-F]+:\s", line)
    ]
    if (
        len(instructions) != 2
        or not re.search(r"\bmv\s+sp,a0\s*$", instructions[0])
        or not re.search(r"\bret\s*$", instructions[1])
    ):
        fail("__set_SP contains a compiler-generated stack frame")

    verify_trap_context(objdump, elf)
    verify_interrupt_contracts(objdump, elf)
    verify_group_interrupt_config(objdump, elf)
    verify_clic_initialization(objdump, elf)

    build_dir = root / "build"
    build_dir.mkdir(exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=build_dir) as raw:
        run(objcopy, "-O", "binary", str(elf), raw.name)
        if Path(raw.name).read_bytes() != binary.read_bytes():
            fail("scp.bin is stale or differs from scp.elf")

    blob_hash = hashlib.sha256(blob.read_bytes()).hexdigest()
    if blob_hash != EXPECTED_BLOB_SHA256:
        fail(f"unexpected libar100s.a SHA256: {blob_hash}")
    members = [line for line in run(ar, "t", str(blob)).splitlines() if line]
    if members != ["obj-in.o"]:
        fail(f"unexpected libar100s.a members: {members}")

    if staged is not None:
        if not staged.is_file() or staged.read_bytes() != binary.read_bytes():
            fail("staged build/scp.bin is missing or stale")

    headroom = symbols["metal_segment_stack_begin"] - symbols["__bss_end"]
    print(
        "AR100S verify: OK "
        f"entry=0x{SCP_ORIGIN:08x} file={binary.stat().st_size} "
        f"stack_headroom={headroom} blob={blob_hash[:12]}"
    )


def main() -> None:
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser()
    parser.add_argument("--elf", default=root / "ar100s/scp.elf")
    parser.add_argument("--binary", default=root / "ar100s/scp.bin")
    parser.add_argument("--blob", default=root / "blobs/libar100s.a")
    parser.add_argument("--staged")
    parser.add_argument("--source-only", action="store_true")
    args = parser.parse_args()
    if args.source_only:
        verify_source_contracts(root)
        print("AR100S source verify: OK")
        return
    verify(args)


if __name__ == "__main__":
    main()
