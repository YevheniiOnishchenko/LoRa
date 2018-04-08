/*-
 * Copyright (c) 2018 Jian-Hong, Pan <starnight@g.ncu.edu.tw>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 */

#ifndef __LORAWAN_H__
#define __LORAWAN_H__

#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <linux/netdevice.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include "lora.h"

#define	LORAWAN_MODULE_NAME	"lorawan"

#define	LRW_UPLINK		0
#define	LRW_DOWNLINK		1

#define	LRW_INIT_SS		0
#define	LRW_XMITTING_SS		1
#define	LRW_XMITTED		2
#define	LRW_RX1_SS		3
#define	LRW_RX2_SS		4
#define	LRW_RXTIMEOUT_SS	5
#define	LRW_RXRECEIVED_SS	6
#define	LRW_RETRANSMIT_SS	7

#define	LRW_MHDR_LEN		1
#define	LRW_FHDR_MAX_LEN	22
#define	LRW_FPORT_LEN		1
#define	LRW_MIC_LEN		4

struct lrw_fhdr {
	u8 mtype;
	u8 fctrl;
	u16 fcnt;
	u8 fopts[15];
	u8 fopts_len;
};

struct lrw_session {
	struct lrw_struct *lrw_st;
	struct list_head entry;

	u8 devaddr[LRW_DEVADDR_LEN];
	u16 fcnt_up;
	u16 fcnt_down;
	u8 fport;
	struct sk_buff *tx_skb;
	struct sk_buff *rx_skb;
	struct lrw_fhdr tx_fhdr;
	struct lrw_fhdr rx_fhdr;

	bool tx_should_ack;
	u8 retry;
	u8 state;
	spinlock_t state_lock;

	struct timer_list timer;
	struct work_struct timeout_work;
	u16 rx_delay1; // seconds
	u16 rx_delay2; // seconds
	u16 rx1_window; // mini-seconds
	u16 rx2_window; // mini-seconds
	u16 ack_timeout; // seconds

	u8 *appkey;
	u8 *nwkskey;
	u8 *appskey;
};

/**
 * lrw_struct - The full LoRaWAN hardware to the LoRa device.
 *
 * @dev:		this LoRa device registed in system
 * @lora_hw:		the LoRa device of this LoRaWAN hardware
 * @ops:		handle of LoRa operations interfaces
 * @rx_skb_list:	the list of received frames
 * @ss_list:		LoRaWAN session list of this LoRaWAN hardware
 * @_cur_ss:		pointer of the current processing session
 * @rx_should_ack:	represent the current session should be acked or not
 * @role:		the role of this LoRaWAN hardware
 * @state:		the state of this LoRaWAN hardware
 * @devaddr:		the LoRaWAN device address of this LoRaWAN hardware
 * @appky:		the Application key
 * @nwkskey:		the Network session key
 * @appskey:		the Application session key
 * @nwks_shash_tfm:	the hash handler for LoRaWAN network session
 * @nwks_skc_tfm:	the crypto handler for LoRaWAN network session
 * @apps_skc_tfm:	the crypto handler for LoRaWAN application session
 * @fcnt_up:		the counter of this LoRaWAN hardware's up frame
 * @fcnt_down:		the counter of this LoRaWAN hardware's down frame
 * @xmit_task:		the xmit task for the current LoRaWAN session
 * @rx_work:		the RX work in workqueue for the current LoRaWAN session
 * @ndev:		points to the emulating network device
 */
struct lrw_struct {
	struct device dev;
	struct lora_hw hw;
	struct lora_operations *ops;
	struct sk_buff_head rx_skb_list;
	struct list_head ss_list;
	struct mutex ss_list_lock;
	struct lrw_session *_cur_ss;
	bool rx_should_ack;
	uint8_t role;
	u8 state;

	u8 devaddr[LRW_DEVADDR_LEN];
	u8 appkey[LORA_KEY_LEN];
	u8 nwkskey[LORA_KEY_LEN];
	u8 appskey[LORA_KEY_LEN];
	struct crypto_shash *nwks_shash_tfm;
	struct crypto_skcipher *nwks_skc_tfm;
	struct crypto_skcipher *apps_skc_tfm;

	u16 fcnt_up;
	u16 fcnt_down;

	struct tasklet_struct xmit_task;
	struct work_struct rx_work;

	struct net_device *ndev;
};

#define	NETDEV_2_LRW(ndev)	((struct lrw_struct *)netdev_priv(ndev))

struct lrw_session * lrw_alloc_ss(struct lrw_struct *);
void lrw_free_ss(struct lrw_session *);
void lrw_del_ss(struct lrw_session *);
int lora_start_hw(struct lrw_struct *);
void lora_stop_hw(struct lrw_struct *);
void lrw_prepare_tx_frame(struct lrw_session *);
void lrw_xmit(unsigned long);
void lrw_rx_work(struct work_struct *);

#endif
