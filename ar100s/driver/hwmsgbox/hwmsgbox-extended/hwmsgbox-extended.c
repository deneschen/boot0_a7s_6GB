/**
 * driver\hwmsgbox\hwmsgbox-extended\hwmsgbox-extended.c
 *
 * Descript: hardware message-box driver for new IP.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#include "hwmsgbox-extended.h"


/*
*********************************************************************************************************
*                                           CLEAR PENDING
*
* Description:  clear the receiver interrupt pending of message-queue.
*
* Arguments  :  queue   : the number of message-queue which we want to clear.
*               user    : the user which we want to clear.
*
* Returns    :  OK if clear pending succeeded, others if failed.
*********************************************************************************************************
*/
#define hwmsgbox_clear_receiver_pending(queue)\
	writel((0x1 << (queue * 2)), MSGBOX_ARM_TO_RISC_IRQ_STATUS_REG)

/* upper bound for best-effort drain of a partial frame on receive timeout */
#define HWMSGBOX_RX_DRAIN_MAX		(128)


/*
*********************************************************************************************************
*                                           INITIALIZE HWMSGBOX
*
* Description:  initialize hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_init(void)
{
	/*enable msgbox clock and set reset as de-assert state.*/
	ccu_set_mclk_onoff(CCU_MOD_CLK_MSGBOX, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_MSGBOX, CCU_CLK_NRESET);

	return OK;
}

/*
*********************************************************************************************************
*                                           EXIT HWMSGBOX
*
* Description:  exit hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if exit hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_exit(void)
{
	/* disable msgbox clock and set reset as assert state. */
	ccu_set_mclk_reset(CCU_MOD_CLK_MSGBOX, CCU_CLK_RESET);
	ccu_set_mclk_onoff(CCU_MOD_CLK_MSGBOX, CCU_CLK_OFF);

	return OK;
}

s32 hwmsgbox_wait_queue_not_full(u32 queue, u32 timeout)
{
	while (readl(MSGBOX_RISC_TO_ARM_MSG_STATUS_REG(queue)) == 8) {
		/*
		 * message-queue fifo is full,
		 * wait 1ms for message-queue process.
		 */
		if (timeout == 0) {
			return -ETIMEOUT;
		}
		time_mdelay(1);
		timeout--;
	}
	return OK;
}

s32 hwmsgbox_wait_queue_not_empty(u32 queue, u32 timeout)
{
	while (readl(MSGBOX_ARM_TO_RISC_MSG_STATUS_REG(queue)) == 0) {
		/*
		 * message-queue fifo is empty,
		 * wait 1ms for message-queue process.
		 */
		if (timeout == 0) {
			return -ETIMEOUT;
		}
		time_mdelay(1);
		timeout--;
	}
	return OK;
}

/*
*********************************************************************************************************
*                                       SEND MESSAGE BY HWMSGBOX
*
* Description:  send one message to another processor by hwmsgbox.
*
* Arguments  :  pmessage    : the pointer of sended message frame.
*               timeout     : the wait time limit when message fifo is full.
*
* Returns    :  OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_send_message(struct message *pmessage, u32 timeout)
{
	s32 ret;
	u32 i;
	u32 value;
	u32 response_capacity;
	u32 response_count;

	if (!pmessage || pmessage->count > MESSAGE_PARA_MAX ||
	    (pmessage->count && !pmessage->paras))
		return -EINVAL;
	response_capacity = pmessage->count;

	if (pmessage->attr & MESSAGE_ATTR_HARDSYN) {
		/* use ar100 hwsyn transmit channel */
		INF("send syn message\n");

		/* first send message header and misc */
		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_SYN_TX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->state | (pmessage->attr << 8) |
			(pmessage->type << 16) | (pmessage->result << 24);
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_SYN_TX_CH));

		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_SYN_TX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->count;
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_SYN_TX_CH));

		/* then send message paras */
		for (i = 0; i < pmessage->count; i++) {
			ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_SYN_TX_CH, timeout);
			if (ret != OK)
				return ret;
			writel(pmessage->paras[i], MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_SYN_TX_CH));
		}

		/* after send, wait feedback, hwsyn messsage must feedback use syn rx channel */

		/* first receive message header and misc */
		ret = hwmsgbox_wait_queue_not_empty(HWMSGBOX_RISC_SYN_RX_CH, timeout);
		if (ret != OK)
			return ret;
		value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(HWMSGBOX_RISC_SYN_RX_CH));
		pmessage->state = value & 0xff;
		pmessage->attr = (value >> 8) & 0xff;
		pmessage->type = (value >> 16) & 0xff;
		pmessage->result = (value >> 24) & 0xff;

		ret = hwmsgbox_wait_queue_not_empty(HWMSGBOX_RISC_SYN_RX_CH, timeout);
		if (ret != OK)
			return ret;
		value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(HWMSGBOX_RISC_SYN_RX_CH));
		response_count = value & 0xff;
		pmessage->count = response_count;

		/* then receive message paras */
		for (i = 0; i < response_count; i++) {
			ret = hwmsgbox_wait_queue_not_empty(HWMSGBOX_RISC_SYN_RX_CH, timeout);
			if (ret != OK)
				return ret;
			value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(HWMSGBOX_RISC_SYN_RX_CH));
			if (i < response_capacity)
				pmessage->paras[i] = value;
		}
		if (response_count > response_capacity)
			return -E2BIG;
		return OK;
	} else {
		/* asyn message use asyn tx channel */
		INF("send asyn message\n");

		/* first send message header and misc */
		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_ASYN_TX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->state | (pmessage->attr << 8) |
			(pmessage->type << 16) | (pmessage->result << 24);
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_ASYN_TX_CH));

		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_ASYN_TX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->count;
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_ASYN_TX_CH));

		/* then send message paras */
		for (i = 0; i < pmessage->count; i++) {
			ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_RISC_ASYN_TX_CH, timeout);
			if (ret != OK)
				return ret;
			writel(pmessage->paras[i], MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_RISC_ASYN_TX_CH));
		}
		return OK;
	}
}

int hwmsgbox_feedback_message(struct message *pmessage, u32 timeout)
{
	s32 ret;
	u32 i;
	u32 value;

	if (!pmessage || pmessage->count > MESSAGE_PARA_MAX ||
	    (pmessage->count && !pmessage->paras))
		return -EINVAL;

	if (pmessage->attr & MESSAGE_ATTR_HARDSYN) {
		/* use ac327 hard syn receiver channel */
		INF("send feedback message\n");

		/* first send message header and misc */
		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_ARM_SYN_RX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->state | (pmessage->attr << 8) |
			(pmessage->type << 16) | (pmessage->result << 24);
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_ARM_SYN_RX_CH));

		ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_ARM_SYN_RX_CH, timeout);
		if (ret != OK)
			return ret;
		value = pmessage->count;
		writel(value, MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_ARM_SYN_RX_CH));

		/* then send message paras */
		for (i = 0; i < pmessage->count; i++) {
			ret = hwmsgbox_wait_queue_not_full(HWMSGBOX_ARM_SYN_RX_CH, timeout);
			if (ret != OK)
				return ret;
			writel(pmessage->paras[i], MSGBOX_RISC_TO_ARM_MSG_REG(HWMSGBOX_ARM_SYN_RX_CH));
		}
		INF("feedback hard syn message : %x\n", pmessage->type);
		return OK;
	}

	/* invalid syn message */
	return -ESRCH;
}


/*
*********************************************************************************************************
*                                        QUERY MESSAGE
*
* Description:  query message of hwmsgbox syn channel by hand, mainly for.
*
* Arguments  :  none.
*
* Returns    :  the point of message, NULL if timeout.
*********************************************************************************************************
*/
static s32 hwmsgbox_receive_message(u32 queue, struct message *pmessage,
				    u32 para_capacity, u32 timeout)
{
	u32 i;
	u32 value;
	u32 count;
	s32 ret;

	ret = hwmsgbox_wait_queue_not_empty(queue, timeout);
	if (ret != OK)
		goto drain;
	value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(queue));
	pmessage->state = value & 0xff;
	pmessage->attr = (value >> 8) & 0xff;
	pmessage->type = (value >> 16) & 0xff;
	pmessage->result = (value >> 24) & 0xff;

	ret = hwmsgbox_wait_queue_not_empty(queue, timeout);
	if (ret != OK)
		goto drain;
	value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(queue));
	count = value & 0xff;
	pmessage->count = count;

	/* Always drain the frame so an oversized message cannot desync the FIFO. */
	for (i = 0; i < count; i++) {
		ret = hwmsgbox_wait_queue_not_empty(queue, timeout);
		if (ret != OK)
			goto drain;
		value = readl(MSGBOX_ARM_TO_RISC_MSG_REG(queue));
		if (i < para_capacity)
			pmessage->paras[i] = value;
	}

	hwmsgbox_clear_receiver_pending(queue);
	if (count > para_capacity)
		return -E2BIG;

	return OK;

drain:
	/* Best effort: remove any partial frame so a later query
	 * starts at a frame boundary. */
	for (i = 0; i < HWMSGBOX_RX_DRAIN_MAX &&
	     readl(MSGBOX_ARM_TO_RISC_MSG_STATUS_REG(queue)); i++)
		readl(MSGBOX_ARM_TO_RISC_MSG_REG(queue));
	hwmsgbox_clear_receiver_pending(queue);
	return ret;
}

s32 hwmsgbox_query_message(struct message *pmessage, u32 para_capacity,
			   u32 timeout)
{
	if (!pmessage || para_capacity > MESSAGE_PARA_MAX ||
	    (para_capacity && !pmessage->paras))
		return -EINVAL;

	/* query ar100 asyn received channel */
	if (!!readl(MSGBOX_ARM_TO_RISC_MSG_STATUS_REG(HWMSGBOX_RISC_ASYN_RX_CH))) {
		LOG("query asyn msg\n");
		return hwmsgbox_receive_message(HWMSGBOX_RISC_ASYN_RX_CH,
						pmessage, para_capacity, timeout);
	}

	/* query ar100 syn received channel */
	if (!!readl(MSGBOX_ARM_TO_RISC_MSG_STATUS_REG(HWMSGBOX_ARM_SYN_TX_CH))) {
		LOG("query syn msg\n");
		return hwmsgbox_receive_message(HWMSGBOX_ARM_SYN_TX_CH,
						pmessage, para_capacity, timeout);
	}

	/* no valid message */
	return FAIL;
}


s32 hwmsgbox_super_standby_init(void)
{

	hwmsgbox_exit();

	return OK;
}

s32 hwmsgbox_super_standby_exit(void)
{
	hwmsgbox_init();

	return OK;
}
