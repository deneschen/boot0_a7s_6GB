/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                daemon module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : daemon.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: daemon module.
* Update  : date                auther      ver     notes
*           2012-5-13 15:06:10  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "daemon_i.h"
#include <libfdt.h>

/* system daemon vars */
extern u32 debug_level;
u32 dtb_base;

/* the list of daemon notifier */
static struct notifier *daemon_list;

int daemon_register_service(__pNotifier_t pcb)
{
	return notifier_insert(&daemon_list, pcb);
}

static s32 startup_state_notify(s32 result)
{
	struct message message;
	u32 arisc_version[13] = {0};
	s32 ret;

	LOG("feedback startup result [%d]\n", result);

	save_state_flag(REC_HOTPULG | 0xd);

	/* initialize message */
	message.type = AR100_STARTUP_NOTIFY;
	message.attr = MESSAGE_ATTR_HARDSYN;
	message.result = result;
	message.count = sizeof(arisc_version) / sizeof(u32);
	message.paras = arisc_version;

	/* must end with '\0' */
	strncpy((char *)(arisc_version), SUB_VER, sizeof(arisc_version) - 1);

	ret = hwmsgbox_send_message(&message, STARTUP_NOTIFY_TIMEOUT);

	if (ret == OK)
		LOG("send notify succeed\n");
	else
		LOG("send notify failed\n");

	save_state_flag(REC_HOTPULG | 0xe);

	return OK;
}

/*
 * NOTE:
 * this function is not reentrant and only can be used in process context,
 * and msg_paras define as static variable to save stack.
 */
static void message_process_loop(void)
{
	s32 ret;
	struct message message;
	static u32 msg_paras[MESSAGE_PARA_MAX];

	message.paras = msg_paras;

	ret = hwmsgbox_query_message(&message, MESSAGE_PARA_MAX,
				     HWMSGBOX_QUERY_TIMEOUT);
	/* A late reply to our own startup notify is not a real message. */
	if (ret == OK && message.type != AR100_STARTUP_NOTIFY)
		message_coming_notify(&message);

	amp_msgbox_query_message();
}

static void daemon_main(void)
{
	/* initialize cpu */
	cpu_init();

	/* daemon & message & user defined task loop process */
	LOG("daemon service setup...\n");
	while (1) {
		/* message loop process */
		message_process_loop();

		/* daemon list process */
		if (((current_time_tick()) % DAEMON_ONCE_TICKS) == 0) {
			/* daemon run one time */
			printk("------------------------------\n");
			LOG("system tick:%d\n", DAEMON_ONCE_TICKS);
			LOG("debug_mask:%d\n", debug_level);
			LOG("uart_buadrate:%d\n", uart_get_baudrate());
			notifier_notify(&daemon_list, DAEMON_RUN_NOTIFY, 0);
		}

		/*
		 * maybe add user defined task process here
		 * refer to daemon list process
		 */
	}
}

static s32 dtb_base_init(void)
{
	u32 base = read_dtb_base();

	/*
	 * RTC_DTB_BASE_STORE_REG shares the RTC_RECORD_REG cell with
	 * save_state_flag(): the handoff address must be read here, before
	 * the first state flag write below overwrites it.
	 *
	 * Accept whatever E902-view DRAM address boot0 published; keep the
	 * access inside the E902-visible DRAM window so an out-of-range
	 * address cannot raise a bus fault on this MMU-less core.
	 */
	dtb_base = 0;
	if (base < ARISC_DRAM_BASE ||
	    base > (u32)(ARISC_DRAM_END - ARISC_DTS_SIZE))
		return -EINVAL;
	if (cpucfg_set_little_endian_address((void *)base,
			(void *)(base + ARISC_DTS_SIZE)) != OK)
		return -EFAIL;
	if (fdt_check_header((void *)base) != 0 ||
	    fdt_totalsize((void *)base) > ARISC_DTS_SIZE) {
		cpucfg_remove_little_endian_address((void *)base,
				(void *)(base + ARISC_DTS_SIZE));
		return -EINVAL;
	}
	dtb_base = base;
	return OK;
}

/*
*********************************************************************************************************
*                                       STARTUP ENTRY
*
* Description:  the entry of startup.
*
* Arguments  :  none.
*
* Returns    :  none.
*********************************************************************************************************
*/
void startup_entry(void)
{
	s32 dtb_status;

	/* CPUCFG owns the little-endian window used to read HW_CONFIG. */
	cpucfg_init();
	dtb_status = dtb_base_init();

	jtag_init();

	notifier_init();
	save_state_flag(REC_HOTPULG | 0x0);

	ccu_init();
	save_state_flag(REC_HOTPULG | 0x1);

	pin_init();
	save_state_flag(REC_HOTPULG | 0x2);

	save_state_flag(REC_HOTPULG | 0x3);

	interrupt_init();
	save_state_flag(REC_HOTPULG | 0x4);

	arisc_para_init();
	save_state_flag(REC_HOTPULG | 0x5);

	debugger_init();
	save_state_flag(REC_HOTPULG | 0x6);
	LOG("debugger system ok\n");

	/* DTB is optional; dependent power operations remain disabled without it. */
	if (dtb_status != OK)
		WRN("HW_CONFIG unavailable; DTB-dependent services disabled\n");
	else if (platform_dts_parse_late() != OK)
		WRN("DRAM parameters unavailable; DFS and standby disabled\n");

	twi_init();
	LOG("twi driver ok\n");
	save_state_flag(REC_HOTPULG | 0x7);

	pmu_init();
	bmu_init();
	save_state_flag(REC_HOTPULG | 0x8);
	LOG("pmu & bmu driver ok\n");

	hwmsgbox_init();
	amp_msgbox_init();
	save_state_flag(REC_HOTPULG | 0x9);
	LOG("hwmsgbox driver ok\n");

	save_state_flag(REC_HOTPULG | 0xa);
	LOG("cpucfg driver ok\n");

	message_manager_init();
	LOG("message manager ok\n");

	timer_init();
	LOG("timer driver ok\n");

	standby_init();
	LOG("standby service ok\n");

	time_ticks_init();
	LOG("time ticks ok\n");

	watchdog_init();
	save_state_flag(REC_HOTPULG | 0xc);
	LOG("watchdog ok\n");

	/* feedback the startup state to ac327 */
	startup_state_notify(OK);

	set_paras();
	save_state_flag(REC_HOTPULG | 0xf);
	LOG("startup feedback ok\n");

	LOG("ar100 firmware version : %s\n", SUB_VER);

	/* enter daemon process main. */
	daemon_main();

	/* to avoid daemon main return. */
	ERR("system daemon exit\n");
	while (1)
		;
}
