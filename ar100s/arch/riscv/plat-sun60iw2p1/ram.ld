OUTPUT_ARCH("riscv")

ENTRY(_start)


MEMORY
{
	dram (rxaiw) : o = 0x40004000, l = 0x30000
}

PHDRS
{
	text PT_LOAD FLAGS(5);
	data PT_LOAD FLAGS(6);
}

SECTIONS
{
	metal_segment_stack_begin = 0x40033000;
	metal_segment_stack_end = 0x40034000;
	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	.init 		:
	{
		KEEP (*(.text.metal.init.enter))
		KEEP (*(SORT_NONE(.init)))
		*crt0.o(.text .text.*)
		*e907_com.o(.text .text.*)
		*core.o(.text .text.*)
		KEEP (*(.text.libgloss.start))
	} >dram AT>dram :text


	.text 		:
	{
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*main.o(.text .text.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	} >dram AT>dram :text

	.rodata 		:
	{
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)
		*(.eh_frame .eh_frame.*)
	} >dram AT>dram :text


	. = ALIGN(4);

	.data 		:
	{
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer = . + 0x800 );
		*(.sdata .sdata.* .sdata2.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)
	} >dram AT>dram :data

		.bss		:
	{
		PROVIDE( __bss_start = .);
		*(.bss .bss.*)
		*(.sbss .sbss.*)
		} >dram AT>dram :data
	PROVIDE(__bss_end = .);

	ASSERT(__bss_end <= metal_segment_stack_begin,
	       "AR100S image overlaps its runtime stack")
	ASSERT(metal_segment_stack_end <= ORIGIN(dram) + LENGTH(dram),
	       "AR100S stack exceeds SRAM")
}
