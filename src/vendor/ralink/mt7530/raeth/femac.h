/*
** $Id: //BBN_Linux/Branch/Branch_for_MT75xx_ASIC_20130518/tclinux_phoenix/modules/private/raeth/femac.h#3 $
*/
/************************************************************************
 *
 *	Copyright (C) 2008 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*

 */
#ifndef _FEMAC_H
#define _FEMAC_H
#include <asm/tc3162/tc3162.h>
#include <linux/version.h>
#define KERNEL_2_6_36 		(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))

#ifdef TCSUPPORT_MT7510_FE
#include "femac_7510.h"
#else
#include "femac_63365.h"
#endif

#define read_reg_word(reg) 		regRead32(reg)
#define write_reg_word(reg, wdata) 	regWrite32(reg, wdata)

#define RX_BUF_LEN 			(2048 - NET_SKB_PAD - 64 - (sizeof(struct skb_shared_info)))
extern struct sk_buff *macRT63365STagInsert(struct sk_buff *bp);
extern void macRT63365STagRemove(struct sk_buff *bp);

extern struct sk_buff *macMT7510STagInsert(struct sk_buff *bp);
extern void macMT7510STagRemove(struct sk_buff *bp);
void macDrvStart(void);
void macDrvStop(void);
void miiStationWrite(macAdapter_t *mac_p, uint32 phyReg, uint32 miiData);
uint32 miiStationRead(macAdapter_t *mac_p, uint32 phyReg);
int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev);


extern struct sk_buff * macMT7520STagInsert(struct sk_buff *bp);
extern int macMT7520STagRemove(struct sk_buff *bp);
extern int vport_enable_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data);

extern int vport_enable_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data);

#endif /* _FEMAC_H */

