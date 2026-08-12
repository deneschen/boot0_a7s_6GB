#include "cpu_i.h"

void clean_bss(void)
{
	extern int __bss_start, __bss_end;

	volatile unsigned int *start = (volatile unsigned int *) &__bss_start;
	volatile unsigned int *end = (volatile unsigned int *) &__bss_end;

	while (start < end) {
		*start++ = 0;
	}
}

extern char metal_segment_stack_begin[];
extern char metal_segment_stack_end[];

static s32 cpu_stack_monitor(u32 message, u32 aux)
{
	u32 *stack = (u32 *) metal_segment_stack_begin;
	u32 size = 0;

	while ((stack < (u32 *) metal_segment_stack_end) && (*stack == 0)) {
		stack++;
		size += 4;
	}
	printk("stack free:%dbyte\n", size);
	return OK;
}

/*initialize cpu interrupts*/
void cpu_init(void)
{
	/*regiser stack status check daemon service */
	daemon_register_service(cpu_stack_monitor);
}

extern u32 c906_irq_save(void);
s32 cpu_disable_int(void)
{
	return c906_irq_save();
}

extern void c906_irq_restore(u32 irq_state);
void cpu_enable_int(s32 cpsr)
{
	c906_irq_restore(cpsr);
}

void cpu_enable_global_int(void)
{
	__asm volatile("csrsi mstatus, 8" : : : "memory");
}

/*return value by making a syscall*/
void exit(s32 i)
{
	LOG("system exit\n");
	while (1)
		;
}

void cpu_enter_doze(void)
{
	__asm volatile("wfi");
}

void time_cdelay(u32 cycles)
{
	for (; cycles > 0; cycles--)
		__asm volatile("nop");
}
