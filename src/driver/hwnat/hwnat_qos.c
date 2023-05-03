/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_qos.c#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
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
** $Log: hwnat_qos.c,v $
** Revision 1.4  2011/06/09 08:18:08  lino
** add RT65168 support
**
** Revision 1.3  2011/06/08 19:01:19  frankliao_hc
** add rt65168 support
**
** Revision 1.2  2011/06/03 02:41:46  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:43  lino
** add RT65168 support
**
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/inet.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>

#include "hwnat.h"
#include "hwnat_mac.h"
#include "hwnat_reg.h"
#include "hwnat_qos.h"
#include "hwnat_nfe.h"

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
typedef struct {
	uint8 valid;

	uint16 rxportmap;
	uint16 min_l3_pktlen;
	uint16 max_l3_pktlen;
	uint8 dest_mac[ETH_ALEN];
	uint8 dest_mac_mask[ETH_ALEN];
	uint32 min_dest_ip[4];
	uint32 max_dest_ip[4];
	uint16 min_dest_port;
	uint16 max_dest_port;
	uint8 src_mac[ETH_ALEN];
	uint8 src_mac_mask[ETH_ALEN];
	uint32 min_src_ip[4];
	uint32 max_src_ip[4];
	uint16 min_src_port;
	uint16 max_src_port;
	uint16 ether_type;
	uint8 protocol_id;
	uint16 min_vlan_id;
	uint16 max_vlan_id;
	uint16 min_8021p;
	uint16 max_8021p;

	uint8 tos_desp_sel_field;
	uint8 min_ipp;
	uint8 max_ipp;
	uint8 tos;
	uint8 min_dscp;
	uint8 max_dscp;
	uint8 min_ipv6_tc;
	uint8 max_ipv6_tc;

	uint16 min_pkt_data;
	uint16 max_pkt_data;
	uint16 min_pkt_data1;
	uint16 max_pkt_data1;

	uint8 remarking_action;
	uint8 new_ipp;
	uint8 new_tos;
	uint8 new_dscp;
	uint8 new_8021p;
	uint8 new_ipv6_tc;
	uint8 new_qid;
} qos_t;


/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_qos_tbl_write(uint16 entry, qos_t *qos);
static void hwnat_qos_tbl_read(uint16 entry, qos_t *qos);

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

qos_t qos_tbl[QOS_TBL_SIZE];

DEFINE_SPINLOCK(qos_lock);

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_qos_init(void)
{
	uint16 entry;

	for (entry = 0; entry < QOS_TBL_SIZE; entry++) {
		memset(&qos_tbl[entry], 0x0, sizeof(qos_t));

		hwnat_qos_tbl_write(entry, &qos_tbl[entry]);
	}
}

void hwnat_qos_exit(void)
{
}

int qos_init(qos_t *qos)
{
	memset(qos, 0x0, sizeof(qos_t));

	qos->valid = 0;
	qos->rxportmap = 0xf;
	qos->min_l3_pktlen = 0x0;
	qos->max_l3_pktlen = 0xffff;
	memset(qos->dest_mac, 0x0, ETH_ALEN);
	memset(qos->dest_mac_mask, 0x0, ETH_ALEN);
	memset(qos->min_dest_ip, 0x0, sizeof(qos->min_dest_ip));
	memset(qos->max_dest_ip, 0xff, sizeof(qos->max_dest_ip));
	qos->min_dest_port = 0x0;
	qos->max_dest_port = 0xffff;

	memset(qos->src_mac, 0x0, ETH_ALEN);
	memset(qos->src_mac_mask, 0x0, ETH_ALEN);
	memset(qos->min_src_ip, 0x0, sizeof(qos->min_src_ip));
	memset(qos->max_src_ip, 0xff, sizeof(qos->max_src_ip));
	qos->min_src_port = 0x0;
	qos->max_src_port = 0xffff;

	qos->ether_type = 0x0;
	qos->protocol_id = 0x0;
	qos->min_vlan_id = 0x0;
	qos->max_vlan_id = 0xfff;
	qos->min_8021p = 0x0;
	qos->max_8021p = 0x7;

	qos->tos_desp_sel_field = 0x0;
	qos->min_ipp = 0x0;
	qos->max_ipp = 0x7;
	qos->tos = 0x0;
	qos->min_dscp = 0x0;
	qos->max_dscp = 0x3f;
	qos->min_ipv6_tc = 0x0;
	qos->max_ipv6_tc = 0xff;

	qos->min_pkt_data = 0x0;
	qos->max_pkt_data = 0xffff;
	qos->min_pkt_data1 = 0x0;
	qos->max_pkt_data1 = 0xffff;

	qos->remarking_action = 0x0;
	qos->new_ipp = 0x0;
	qos->new_tos = 0x0;
	qos->new_dscp = 0x0;
	qos->new_8021p = 0x0;
	qos->new_ipv6_tc = 0x0;
	qos->new_qid = 0x0;

	return 0;
}

void hwnat_qos_tbl_write(uint16 entry, qos_t *qos)
{
	uint32 table_hold[GTH_SIZE];
	int i;

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (qos->new_qid&0x7) | ((qos->new_ipv6_tc<<3)&0x7f8) | 
		((qos->new_8021p<<11)&0x3800) | ((qos->new_dscp<<14)&0xfc000) | 
		((qos->new_tos<<20)&0xf00000) | ((qos->new_ipp<<24)&0x7000000) | 
		((qos->remarking_action<<27)&0xf8000000);
	table_hold[6] = ((qos->remarking_action>>5)&0x1) | ((qos->max_pkt_data1<<1)&0x1fffe) | 
		((qos->min_pkt_data1<<17)&0xfffe0000);
	table_hold[5] = ((qos->min_pkt_data1>>15)&0x1) | ((qos->max_pkt_data<<1)&0x1fffe) | 
		((qos->min_pkt_data<<17)&0xfffe0000);
	table_hold[4] = ((qos->min_pkt_data>>15)&0x1) | ((qos->max_ipv6_tc<<1)&0x1fe) | 
		((qos->min_ipv6_tc<<9)&0x1fe00) | ((qos->max_dscp<<17)&0x7e0000) | 
		((qos->min_dscp<<23)&0x1f800000) | ((qos->tos<<29)&0xe0000000);
	table_hold[3] = ((qos->tos>>3)&0x1) | ((qos->max_ipp<<1)&0xe) | ((qos->min_ipp<<4)&0x70) |
		((qos->tos_desp_sel_field<<7)&0x380) | ((qos->max_8021p<<10)&0x1c00) | 
		((qos->min_8021p<<13)&0xe000) | ((qos->max_vlan_id<<16)&0xfff0000) |
		((qos->min_vlan_id<<28)&0xf0000000);
	table_hold[2] = ((qos->min_vlan_id>>4)&0xff) | ((qos->protocol_id<<8)&0xff00) | 
		((qos->ether_type<<16)&0xffff0000);
	table_hold[1] = ((qos->max_src_port)&0xffff) | ((qos->min_src_port<<16)&0xffff0000);
	table_hold[0] = ((qos->max_src_ip[3])&0xffffffff);
	hwnat_write_tbl(QOS0_TBL_ID, entry, table_hold, GTH_SIZE);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = ((qos->max_src_ip[2])&0xffffffff);
	table_hold[6] = ((qos->max_src_ip[1])&0xffffffff);
	table_hold[5] = ((qos->max_src_ip[0])&0xffffffff);
	table_hold[4] = ((qos->min_src_ip[3])&0xffffffff);
	table_hold[3] = ((qos->min_src_ip[2])&0xffffffff);
	table_hold[2] = ((qos->min_src_ip[1])&0xffffffff);
	table_hold[1] = ((qos->min_src_ip[0])&0xffffffff);
	table_hold[0] = ((qos->src_mac_mask[5])&0xff) | ((qos->src_mac_mask[4]<<8)&0xff00) |
		((qos->src_mac_mask[3]<<16)&0xff0000) | ((qos->src_mac_mask[2]<<24)&0xff000000);
	hwnat_write_tbl(QOS1_TBL_ID, entry, table_hold, GTH_SIZE);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = ((qos->src_mac_mask[1])&0xff) | ((qos->src_mac_mask[0]<<8)&0xff00) |
		((qos->src_mac[5]<<16)&0xff0000) | ((qos->src_mac[4]<<24)&0xff000000);
	table_hold[6] = ((qos->src_mac[3])&0xff) | ((qos->src_mac[2]<<8)&0xff00) |
		((qos->src_mac[1]<<16)&0xff0000) | ((qos->src_mac[0]<<24)&0xff000000);
	table_hold[5] = ((qos->max_dest_port)&0xffff) | ((qos->min_dest_port<<16)&0xffff0000);
	table_hold[4] = ((qos->max_dest_ip[3])&0xffffffff);
	table_hold[3] = ((qos->max_dest_ip[2])&0xffffffff);
	table_hold[2] = ((qos->max_dest_ip[1])&0xffffffff);
	table_hold[1] = ((qos->max_dest_ip[0])&0xffffffff);
	table_hold[0] = ((qos->min_dest_ip[3])&0xffffffff);
	hwnat_write_tbl(QOS2_TBL_ID, entry, table_hold, GTH_SIZE);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = ((qos->min_dest_ip[2])&0xffffffff);
	table_hold[6] = ((qos->min_dest_ip[1])&0xffffffff);
	table_hold[5] = ((qos->min_dest_ip[0])&0xffffffff);
	table_hold[4] = ((qos->dest_mac_mask[5])&0xff) | ((qos->dest_mac_mask[4]<<8)&0xff00) |
		((qos->dest_mac_mask[3]<<16)&0xff0000) | ((qos->dest_mac_mask[2]<<24)&0xff000000);
	table_hold[3] = ((qos->dest_mac_mask[1])&0xff) | ((qos->dest_mac_mask[0]<<8)&0xff00) |
		((qos->dest_mac[5]<<16)&0xff0000) | ((qos->dest_mac[4]<<24)&0xff000000);
	table_hold[2] = ((qos->dest_mac[3])&0xff) | ((qos->dest_mac[2]<<8)&0xff00) |
		((qos->dest_mac[1]<<16)&0xff0000) | ((qos->dest_mac[0]<<24)&0xff000000);
	table_hold[1] = ((qos->max_l3_pktlen)&0xffff) | ((qos->min_l3_pktlen<<16)&0xffff0000);
	table_hold[0] = ((qos->rxportmap)&0xf) | ((qos->valid<<4)&0x10);
	hwnat_write_tbl(QOS3_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_qos_tbl_read(uint16 entry, qos_t *qos)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(QOS0_TBL_ID, entry, table_hold, GTH_SIZE);
	qos->new_qid = table_hold[7]&0x7;
	qos->new_ipv6_tc = (table_hold[7]&0x7f8)>>3;
	qos->new_8021p = (table_hold[7]&0x3800)>>11;
	qos->new_dscp = (table_hold[7]&0xfc000)>>14;
	qos->new_tos = (table_hold[7]&0xf00000)>>20;
	qos->new_ipp = (table_hold[7]&0x7000000)>>24;
	qos->remarking_action = ((table_hold[7]&0xf8000000)>>27) | ((table_hold[6]&0x1)<<5);
	qos->max_pkt_data1 = (table_hold[6]&0x1fffe)>>1;
	qos->min_pkt_data1 = ((table_hold[6]&0xfffe0000)>>17) | ((table_hold[5]&0x1)<<15);
	qos->max_pkt_data = (table_hold[5]&0x1fffe)>>1;
	qos->min_pkt_data = ((table_hold[5]&0xfffe0000)>>17) | ((table_hold[4]&0x1)<<15);
	qos->max_ipv6_tc = (table_hold[4]&0x1fe)>>1;
	qos->min_ipv6_tc = (table_hold[4]&0x1fe00)>>9;
	qos->max_dscp = (table_hold[4]&0x7e0000)>>17;
	qos->min_dscp = (table_hold[4]&0x1f800000)>>23;
	qos->tos = ((table_hold[4]&0xe0000000)>>29) | ((table_hold[3]&0x1)<<3);
	qos->max_ipp = (table_hold[3]&0xe)>>1;
	qos->min_ipp = (table_hold[3]&0x70)>>4;
	qos->tos_desp_sel_field = (table_hold[3]&0x380)>>7;
	qos->max_8021p = (table_hold[3]&0x1c00)>>10;
	qos->min_8021p = (table_hold[3]&0xe000)>>13;
	qos->max_vlan_id = (table_hold[3]&0xfff0000)>>16;
	qos->min_vlan_id = ((table_hold[3]&0xf0000000)>>28) | ((table_hold[2]&0xff)<<4);
	qos->protocol_id = (table_hold[2]&0xff00)>>8;
	qos->ether_type = (table_hold[2]&0xffff0000)>>16;
	qos->max_src_port = (table_hold[1]&0xffff);
	qos->min_src_port = (table_hold[1]&0xffff0000)>>16;
	qos->max_src_ip[3] = (table_hold[0]&0xffffffff);

	hwnat_read_tbl(QOS1_TBL_ID, entry, table_hold, GTH_SIZE);
	qos->max_src_ip[2] = (table_hold[7]&0xffffffff);
	qos->max_src_ip[1] = (table_hold[6]&0xffffffff);
	qos->max_src_ip[0] = (table_hold[5]&0xffffffff);
	qos->min_src_ip[3] = (table_hold[4]&0xffffffff);
	qos->min_src_ip[2] = (table_hold[3]&0xffffffff);
	qos->min_src_ip[1] = (table_hold[2]&0xffffffff);
	qos->min_src_ip[0] = (table_hold[1]&0xffffffff);
	qos->src_mac_mask[5] = (table_hold[0]&0xff);
	qos->src_mac_mask[4] = (table_hold[0]&0xff00)>>8;
	qos->src_mac_mask[3] = (table_hold[0]&0xff0000)>>16;
	qos->src_mac_mask[2] = (table_hold[0]&0xff000000)>>24;

	hwnat_read_tbl(QOS2_TBL_ID, entry, table_hold, GTH_SIZE);
	qos->src_mac_mask[1] = (table_hold[7]&0xff);
	qos->src_mac_mask[0] = (table_hold[7]&0xff00)>>8;
	qos->src_mac[5] = (table_hold[7]&0xff0000)>>16;
	qos->src_mac[4] = (table_hold[7]&0xff000000)>>24;
	qos->src_mac[3] = (table_hold[6]&0xff);
	qos->src_mac[2] = (table_hold[6]&0xff00)>>8;
	qos->src_mac[1] = (table_hold[6]&0xff0000)>>16;
	qos->src_mac[0] = (table_hold[6]&0xff000000)>>24;
	qos->max_dest_port = (table_hold[5]&0xffff);
	qos->min_dest_port = (table_hold[5]&0xffff0000)>>16;
	qos->max_dest_ip[3] = (table_hold[4]&0xffffffff);
	qos->max_dest_ip[2] = (table_hold[3]&0xffffffff);
	qos->max_dest_ip[1] = (table_hold[2]&0xffffffff);
	qos->max_dest_ip[0] = (table_hold[1]&0xffffffff);
	qos->min_dest_ip[3] = (table_hold[0]&0xffffffff);

	hwnat_read_tbl(QOS3_TBL_ID, entry, table_hold, GTH_SIZE);
	qos->min_dest_ip[2] = (table_hold[7]&0xffffffff);
	qos->min_dest_ip[1] = (table_hold[6]&0xffffffff);
	qos->min_dest_ip[0] = (table_hold[5]&0xffffffff);
	qos->dest_mac_mask[5] = (table_hold[4]&0xff);
	qos->dest_mac_mask[4] = (table_hold[4]&0xff00)>>8;
	qos->dest_mac_mask[3] = (table_hold[4]&0xff0000)>>16;
	qos->dest_mac_mask[2] = (table_hold[4]&0xff000000)>>24;
	qos->dest_mac_mask[1] = (table_hold[3]&0xff);
	qos->dest_mac_mask[0] = (table_hold[3]&0xff00)>>8;
	qos->dest_mac[5] = (table_hold[3]&0xff0000)>>16;
	qos->dest_mac[4] = (table_hold[3]&0xff000000)>>24;
	qos->dest_mac[3] = (table_hold[2]&0xff);
	qos->dest_mac[2] = (table_hold[2]&0xff00)>>8;
	qos->dest_mac[1] = (table_hold[2]&0xff0000)>>16;
	qos->dest_mac[0] = (table_hold[2]&0xff000000)>>24;
	qos->max_l3_pktlen = (table_hold[1]&0xffff);
	qos->min_l3_pktlen = (table_hold[1]&0xffff0000)>>16;
	qos->rxportmap = (table_hold[0]&0xf);
	qos->valid = (table_hold[0]&0x10)>>4;
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_qos_policy(int argc, char *argv[], void *p);
static int do_hwnat_qos_policy_dump(int argc, char *argv[], void *p);
static int do_hwnat_qos_policy_sp(int argc, char *argv[], void *p);
static int do_hwnat_qos_policy_wrr(int argc, char *argv[], void *p);
static int do_hwnat_qos_policy_rc(int argc, char *argv[], void *p);

static int do_hwnat_qos_rd(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr(int argc, char *argv[], void *p);

static int do_hwnat_qos_wr(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_init(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_load(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_valid(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_rxportmap(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_l3pktlen(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_destmac(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_destip(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_destip6(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_destport(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_srcmac(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_srcip(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_srcip6(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_srcport(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_ethertype(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_protocolid(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_vlanid(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_8021p(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_ipp(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_tos(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_dscp(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_ipv6tc(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_pktdata(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_pktdata1(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_newipp(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_newtos(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_newdscp(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_new8021p(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_newipv6tc(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_newqid(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_disp(int argc, char *argv[], void *p);
static int do_hwnat_qos_wr_save(int argc, char *argv[], void *p);

static int do_hwnat_qos_tbl(int argc, char *argv[], void *p);
static int do_hwnat_qos_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_qos_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_qos_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_qos_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_qos_cmds[] = {
	{"policy",		do_hwnat_qos_policy,		0x12,  	0,  NULL},
	{"rd",			do_hwnat_qos_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_qos_wr,			0x12,  	0,  NULL},
	{"tbl",			do_hwnat_qos_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_qos_policy_cmds[] = {
	{"dump",		do_hwnat_qos_policy_dump,	0x02,  	0,  NULL},
	{"sp",			do_hwnat_qos_policy_sp,		0x02,  	1,  "<port>"},
	{"wrr",			do_hwnat_qos_policy_wrr,	0x02,  	5,  "<port> <w0> <w1> <w2> <w3>"},
	{"rc",			do_hwnat_qos_policy_rc,		0x02,  	5,  "<port> <rc0> <rc1> <rc2> <rc3>"},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_qos_wr_cmds[] = {
	{"init",		do_hwnat_qos_wr_init,		0x02,  	0,  NULL},
	{"load",		do_hwnat_qos_wr_load,		0x02,  	1,  "<entry>"},
	{"valid",		do_hwnat_qos_wr_valid,		0x02,  	1,  "<1|0>"},
	{"rxportmap",	do_hwnat_qos_wr_rxportmap,	0x02,  	1,  "<rxportmap>"},
	{"l3pktlen",	do_hwnat_qos_wr_l3pktlen,	0x02,  	2,  "<min> <max>"},
	{"destmac",		do_hwnat_qos_wr_destmac,	0x02,  	2,  "<destmac> <mask>"},
	{"destip",		do_hwnat_qos_wr_destip,		0x02,  	2,  "<min> <max>"},
	{"destip6",		do_hwnat_qos_wr_destip6,	0x02,  	2,  "<min> <max>"},
	{"destport",	do_hwnat_qos_wr_destport,	0x02,  	2,  "<min> <max>"},
	{"srcmac",		do_hwnat_qos_wr_srcmac,		0x02,  	2,  "<srcmac> <mask>"},
	{"srcip",		do_hwnat_qos_wr_srcip,		0x02,  	2,  "<min> <max>"},
	{"srcip6",		do_hwnat_qos_wr_srcip6,		0x02,  	2,  "<min> <max>"},
	{"srcport",		do_hwnat_qos_wr_srcport,	0x02,  	2,  "<min> <max>"},
	{"ethertype",	do_hwnat_qos_wr_ethertype,	0x02,  	1,  "<ethertype>"},
	{"protocolid",	do_hwnat_qos_wr_protocolid,	0x02,  	1,  "<protocolid>"},
	{"vlanid",		do_hwnat_qos_wr_vlanid,		0x02,  	2,  "<min> <max>"},
	{"8021p",		do_hwnat_qos_wr_8021p,		0x02,  	2,  "<min> <max>"},
	{"ipp",			do_hwnat_qos_wr_ipp,		0x02,  	2,  "<min> <max>"},
	{"tos",			do_hwnat_qos_wr_tos,		0x02,  	2,  "<1|0> <tos>"},
	{"dscp",		do_hwnat_qos_wr_dscp,		0x02,  	2,  "<min> <max>"},
	{"ipv6tc",		do_hwnat_qos_wr_ipv6tc,		0x02,  	2,  "<min> <max>"},
	{"pktdata",		do_hwnat_qos_wr_pktdata,	0x02,  	2,  "<min> <max>"},
	{"pktdata1",	do_hwnat_qos_wr_pktdata1,	0x02,  	2,  "<min> <max>"},
	{"newipp",		do_hwnat_qos_wr_newipp,		0x02,  	2,  "<1|0> <ipp>"},
	{"newtos",		do_hwnat_qos_wr_newtos,		0x02,  	2,  "<1|0> <tos>"},
	{"newdscp",		do_hwnat_qos_wr_newdscp,	0x02,  	2,  "<1|0> <dscp>"},
	{"new8021p",	do_hwnat_qos_wr_new8021p,	0x02,  	2,  "<1|0> <8021p>"},
	{"newipv6tc",	do_hwnat_qos_wr_newipv6tc,	0x02,  	2,  "<1|0> <ipv6tc>"},
	{"newqid",		do_hwnat_qos_wr_newqid,		0x02,  	2,  "<1|0> <qid>"},
	{"disp",		do_hwnat_qos_wr_disp,		0x02,  	0,  NULL},
	{"save",		do_hwnat_qos_wr_save,		0x02,  	1,  "<entry>"},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_qos_tbl_cmds[] = {
	{"dump",		do_hwnat_qos_tbl_dump,		0x02,  	0,  NULL},
	{"raw",			do_hwnat_qos_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_qos_tbl_hwraw,		0x02,  	0,  NULL},
	{"comp",		do_hwnat_qos_tbl_comp,		0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_qos(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_qos_cmds, argc, argv, p);
}

int do_hwnat_qos_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	qos_t qos;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_qos_tbl_read(entry, &qos);

	printk("/*%03d*/ ", entry);
	printk("%d %04x %u/%u\n", qos.valid, qos.rxportmap, qos.min_l3_pktlen, qos.max_l3_pktlen);
	printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u/%u\n", 
			qos.dest_mac[0], qos.dest_mac[1], qos.dest_mac[2], qos.dest_mac[3], 
			qos.dest_mac[4], qos.dest_mac[5],
			qos.dest_mac_mask[0], qos.dest_mac_mask[1], qos.dest_mac_mask[2], qos.dest_mac_mask[3], 
			qos.dest_mac_mask[4], qos.dest_mac_mask[5], qos.min_dest_port, qos.max_dest_port);

	if ((qos.min_dest_ip[0] == 0x0) && (qos.min_dest_ip[1] == 0x0) && 
					(qos.min_dest_ip[2] == 0x0)) {
		printk("\t" NIPQUAD_FMT "/" NIPQUAD_FMT "\n", 
			NIPQUAD(qos.min_dest_ip[3]), NIPQUAD(qos.max_dest_ip[3]));
	} else {
		uint8 *cp = &qos.min_dest_ip[0];
		printk("\t%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
		cp = &qos.max_dest_ip[0];
		printk("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
	}

	printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u/%u\n", 
			qos.src_mac[0], qos.src_mac[1], qos.src_mac[2], qos.src_mac[3], 
			qos.src_mac[4], qos.src_mac[5],
			qos.src_mac_mask[0], qos.src_mac_mask[1], qos.src_mac_mask[2], qos.src_mac_mask[3], 
			qos.src_mac_mask[4], qos.src_mac_mask[5], qos.min_src_port, qos.max_src_port);

	if ((qos.min_src_ip[0] == 0x0) && (qos.min_src_ip[1] == 0x0) && 
					(qos.min_src_ip[2] == 0x0)) {
		printk("\t" NIPQUAD_FMT "/" NIPQUAD_FMT "\n", 
			NIPQUAD(qos.min_src_ip[3]), NIPQUAD(qos.max_src_ip[3]));
	} else {
		uint8 *cp = &qos.min_src_ip[0];
		printk("\t%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
		cp = &qos.max_src_ip[0];
		printk("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
	}

	printk("\t%04x %02x %u/%u %u/%u\n", 
			qos.ether_type, qos.protocol_id, qos.min_vlan_id, qos.max_vlan_id, 
			qos.min_8021p, qos.max_8021p);
	printk("\t%d:%u/%u %d:%01x %d:%u/%u %u/%u %04x/%04x %04x/%04x\n", 
			qos.tos_desp_sel_field&0x1, qos.min_ipp, qos.max_ipp, 
			qos.tos_desp_sel_field&0x2, qos.tos, 
			qos.tos_desp_sel_field&0x4, qos.min_dscp, qos.max_dscp, 
			qos.min_ipv6_tc, qos.max_ipv6_tc, 
			qos.min_pkt_data, qos.max_pkt_data, qos.min_pkt_data1, qos.max_pkt_data1); 
	printk("\tIPP(%d):%01x TOS((%d):%01x DSCP(%d):%02x 802.1p(%d):%d, IPv6_TC(%d):%x QID(%d):%d\n",
			qos.remarking_action&0x1, qos.new_ipp, 
			qos.remarking_action&0x2, qos.new_tos, 
			qos.remarking_action&0x4, qos.new_dscp, 
			qos.remarking_action&0x8, qos.new_8021p, 
			qos.remarking_action&0x10, qos.new_ipv6_tc, 
			qos.remarking_action&0x20, qos.new_qid);

	return 0;
}

int do_hwnat_qos_policy(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_qos_policy_cmds, argc, argv, p);
}

int do_hwnat_qos_policy_dump(int argc, char *argv[], void *p)
{
	uint8 port;
	uint8 policy;
	uint16 w0, w1, w2, w3;

	for (port = 0; port < 4; port++) {
		hwnat_nfe_qos_get(port, &policy, &w0, &w1, &w2, &w3);
		if (policy == QUEUE_SCH_MODE_SP) {
			printk("port=%d SP\n", port);
		} else if (policy == QUEUE_SCH_MODE_RC) {
			printk("port=%d RC rc0=%d rc1=%d rc2=%d rc3=%d\n", port, w0, w1, w2, w3);
		} else {
			printk("port=%d WRR w0=%d w1=%d w2=%d w3=%d\n", port, w0, w1, w2, w3);
		}
	}
	return 0;
}

int do_hwnat_qos_policy_sp(int argc, char *argv[], void *p)
{
	uint8 port;

	port = (uint8) simple_strtoul(argv[1], NULL, 10);

	hwnat_nfe_qos_set(port, QUEUE_SCH_MODE_SP, 1, 2, 4, 8);
	return 0;
}

int do_hwnat_qos_policy_wrr(int argc, char *argv[], void *p)
{
	uint8 port;
	uint16 w0, w1, w2, w3;

	port = (uint8) simple_strtoul(argv[1], NULL, 10);
	w0 = (uint16) simple_strtoul(argv[2], NULL, 10);
	w1 = (uint16) simple_strtoul(argv[3], NULL, 10);
	w2 = (uint16) simple_strtoul(argv[4], NULL, 10);
	w3 = (uint16) simple_strtoul(argv[5], NULL, 10);

	hwnat_nfe_qos_set(port, QUEUE_SCH_MODE_WRR, w0, w1, w2, w3);
	return 0;
}

int do_hwnat_qos_policy_rc(int argc, char *argv[], void *p)
{
	uint8 port;
	uint16 rc0, rc1, rc2, rc3;

	port = (uint8) simple_strtoul(argv[1], NULL, 10);
	rc0 = (uint16) simple_strtoul(argv[2], NULL, 10);
	rc1 = (uint16) simple_strtoul(argv[3], NULL, 10);
	rc2 = (uint16) simple_strtoul(argv[4], NULL, 10);
	rc3 = (uint16) simple_strtoul(argv[5], NULL, 10);

	hwnat_nfe_qos_set(port, QUEUE_SCH_MODE_RC, rc0, rc1, rc2, rc3);
	return 0;
}

static qos_t wr_qos;

int do_hwnat_qos_wr(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_qos_wr_cmds, argc, argv, p);
}

int do_hwnat_qos_wr_init(int argc, char *argv[], void *p)
{
	qos_init(&wr_qos);
	return 0;
}

int do_hwnat_qos_wr_load(int argc, char *argv[], void *p)
{
	uint16 entry;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_qos_tbl_read(entry, &wr_qos);
	return 0;
}

int do_hwnat_qos_wr_valid(int argc, char *argv[], void *p)
{
	uint8 valid;

	valid = (uint8) simple_strtoul(argv[1], NULL, 10);
	if (valid)
		wr_qos.valid = 1;
	else
		wr_qos.valid = 0;

	return 0;
}

int do_hwnat_qos_wr_rxportmap(int argc, char *argv[], void *p)
{
	uint16 rxportmap;

	rxportmap = (uint16) simple_strtoul(argv[1], NULL, 16);
	wr_qos.rxportmap = rxportmap;

	return 0;
}

int do_hwnat_qos_wr_l3pktlen(int argc, char *argv[], void *p)
{
	wr_qos.min_l3_pktlen = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_l3_pktlen = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_destmac(int argc, char *argv[], void *p)
{
	uint8 macaddr[ETH_ALEN];

	if (hwnat_mac_addr_in_ether(argv[1], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}
	memcpy(wr_qos.dest_mac, macaddr, ETH_ALEN);

	if (hwnat_mac_addr_in_ether(argv[2], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}
	memcpy(wr_qos.dest_mac_mask, macaddr, ETH_ALEN);

	return 0;
}

int do_hwnat_qos_wr_destip(int argc, char *argv[], void *p)
{
	wr_qos.min_dest_ip[3] = in_aton(argv[1]);
	wr_qos.min_dest_ip[2] = 0x0;
	wr_qos.min_dest_ip[1] = 0x0;
	wr_qos.min_dest_ip[0] = 0x0;
	wr_qos.max_dest_ip[3] = in_aton(argv[2]);
	wr_qos.max_dest_ip[2] = 0x0;
	wr_qos.max_dest_ip[1] = 0x0;
	wr_qos.max_dest_ip[0] = 0x0;

	return 0;
}

int do_hwnat_qos_wr_destip6(int argc, char *argv[], void *p)
{
	in6_pton(argv[1], -1 /* len */,  (char *) wr_qos.min_dest_ip, -1, NULL);
	in6_pton(argv[2], -1 /* len */,  (char *) wr_qos.max_dest_ip, -1, NULL);

	return 0;
}

int do_hwnat_qos_wr_destport(int argc, char *argv[], void *p)
{
	wr_qos.min_dest_port = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_dest_port = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_srcmac(int argc, char *argv[], void *p)
{
	uint8 macaddr[ETH_ALEN];

	if (hwnat_mac_addr_in_ether(argv[1], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}
	memcpy(wr_qos.src_mac, macaddr, ETH_ALEN);

	if (hwnat_mac_addr_in_ether(argv[2], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}
	memcpy(wr_qos.src_mac_mask, macaddr, ETH_ALEN);

	return 0;
}

int do_hwnat_qos_wr_srcip(int argc, char *argv[], void *p)
{
	wr_qos.min_src_ip[3] = in_aton(argv[1]);
	wr_qos.min_src_ip[2] = 0x0;
	wr_qos.min_src_ip[1] = 0x0;
	wr_qos.min_src_ip[0] = 0x0;
	wr_qos.max_src_ip[3] = in_aton(argv[2]);
	wr_qos.max_src_ip[2] = 0x0;
	wr_qos.max_src_ip[1] = 0x0;
	wr_qos.max_src_ip[0] = 0x0;

	return 0;
}

int do_hwnat_qos_wr_srcip6(int argc, char *argv[], void *p)
{
	in6_pton(argv[1], -1 /* len */,  (char *) wr_qos.min_src_ip, -1, NULL);
	in6_pton(argv[2], -1 /* len */,  (char *) wr_qos.max_src_ip, -1, NULL);

	return 0;
}

int do_hwnat_qos_wr_srcport(int argc, char *argv[], void *p)
{
	wr_qos.min_src_port = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_src_port = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_ethertype(int argc, char *argv[], void *p)
{
	wr_qos.ether_type = (uint16) simple_strtoul(argv[1], NULL, 16);

	return 0;
}

int do_hwnat_qos_wr_protocolid(int argc, char *argv[], void *p)
{
	wr_qos.protocol_id = (uint8) simple_strtoul(argv[1], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_vlanid(int argc, char *argv[], void *p)
{
	wr_qos.min_vlan_id = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_vlan_id = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_8021p(int argc, char *argv[], void *p)
{
	wr_qos.min_8021p = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_8021p = (uint8) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_ipp(int argc, char *argv[], void *p)
{
	wr_qos.min_ipp = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_ipp = (uint8) simple_strtoul(argv[2], NULL, 10);

	if ((wr_qos.min_ipp == 0) && (wr_qos.max_ipp == 0x7)) 
		wr_qos.tos_desp_sel_field &= ~0x1;
	else
		wr_qos.tos_desp_sel_field |= 0x1;

	return 0;
}

int do_hwnat_qos_wr_tos(int argc, char *argv[], void *p)
{
	uint8 tos;

	tos = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.tos = (uint8) simple_strtoul(argv[2], NULL, 16);

	if (tos)
		wr_qos.tos_desp_sel_field |= 0x2;
	else
		wr_qos.tos_desp_sel_field &= ~0x2;

	return 0;
}

int do_hwnat_qos_wr_dscp(int argc, char *argv[], void *p)
{
	wr_qos.min_dscp = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_dscp = (uint16) simple_strtoul(argv[2], NULL, 10);

	if ((wr_qos.min_dscp == 0) && (wr_qos.max_dscp == 0x3f)) 
		wr_qos.tos_desp_sel_field &= ~0x4;
	else
		wr_qos.tos_desp_sel_field |= 0x4;

	return 0;
}

int do_hwnat_qos_wr_ipv6tc(int argc, char *argv[], void *p)
{
	wr_qos.min_ipv6_tc = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_ipv6_tc = (uint8) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_pktdata(int argc, char *argv[], void *p)
{
	wr_qos.min_pkt_data = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_pkt_data = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_pktdata1(int argc, char *argv[], void *p)
{
	wr_qos.min_pkt_data1 = (uint16) simple_strtoul(argv[1], NULL, 10);
	wr_qos.max_pkt_data1 = (uint16) simple_strtoul(argv[2], NULL, 10);

	return 0;
}

int do_hwnat_qos_wr_newipp(int argc, char *argv[], void *p)
{
	uint8 newipp;

	newipp = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_ipp = (uint8) simple_strtoul(argv[2], NULL, 10);

	if (newipp)
		wr_qos.remarking_action |= 0x1;
	else
		wr_qos.remarking_action &= ~0x1;

	return 0;
}

int do_hwnat_qos_wr_newtos(int argc, char *argv[], void *p)
{
	uint8 newtos;

	newtos = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_tos = (uint8) simple_strtoul(argv[2], NULL, 16);

	if (newtos)
		wr_qos.remarking_action |= 0x2;
	else
		wr_qos.remarking_action &= ~0x2;

	return 0;
}

int do_hwnat_qos_wr_newdscp(int argc, char *argv[], void *p)
{
	uint8 newdscp;

	newdscp = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_dscp = (uint8) simple_strtoul(argv[2], NULL, 10);

	if (newdscp)
		wr_qos.remarking_action |= 0x4;
	else
		wr_qos.remarking_action &= ~0x4;

	return 0;
}

int do_hwnat_qos_wr_new8021p(int argc, char *argv[], void *p)
{
	uint8 new8021p;

	new8021p = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_8021p = (uint8) simple_strtoul(argv[2], NULL, 10);

	if (new8021p)
		wr_qos.remarking_action |= 0x8;
	else
		wr_qos.remarking_action &= ~0x8;

	return 0;
}

int do_hwnat_qos_wr_newipv6tc(int argc, char *argv[], void *p)
{
	uint8 newipv6tc;

	newipv6tc = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_ipv6_tc = (uint8) simple_strtoul(argv[2], NULL, 10);

	if (newipv6tc)
		wr_qos.remarking_action |= 0x10;
	else
		wr_qos.remarking_action &= ~0x10;

	return 0;
}

int do_hwnat_qos_wr_newqid(int argc, char *argv[], void *p)
{
	uint8 newqid;

	newqid = (uint8) simple_strtoul(argv[1], NULL, 10);
	wr_qos.new_qid = (uint8) simple_strtoul(argv[2], NULL, 10);

	if (newqid)
		wr_qos.remarking_action |= 0x20;
	else
		wr_qos.remarking_action &= ~0x20;

	return 0;
}

int do_hwnat_qos_wr_disp(int argc, char *argv[], void *p)
{
	printk("\t%d %04x %u/%u\n", wr_qos.valid, wr_qos.rxportmap, wr_qos.min_l3_pktlen, wr_qos.max_l3_pktlen);

	printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u/%u\n", 
			wr_qos.dest_mac[0], wr_qos.dest_mac[1], wr_qos.dest_mac[2], wr_qos.dest_mac[3], 
			wr_qos.dest_mac[4], wr_qos.dest_mac[5],
			wr_qos.dest_mac_mask[0], wr_qos.dest_mac_mask[1], wr_qos.dest_mac_mask[2], wr_qos.dest_mac_mask[3], 
			wr_qos.dest_mac_mask[4], wr_qos.dest_mac_mask[5], wr_qos.min_dest_port, wr_qos.max_dest_port);

	if ((wr_qos.min_dest_ip[0] == 0x0) && (wr_qos.min_dest_ip[1] == 0x0) && 
					(wr_qos.min_dest_ip[2] == 0x0)) {
		printk("\t" NIPQUAD_FMT "/" NIPQUAD_FMT "\n", 
			NIPQUAD(wr_qos.min_dest_ip[3]), NIPQUAD(wr_qos.max_dest_ip[3]));
	} else {
		uint8 *cp = &wr_qos.min_dest_ip[0];
		printk("\t%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
		cp = &wr_qos.max_dest_ip[0];
		printk("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
	}

	printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u/%u\n", 
			wr_qos.src_mac[0], wr_qos.src_mac[1], wr_qos.src_mac[2], wr_qos.src_mac[3], 
			wr_qos.src_mac[4], wr_qos.src_mac[5],
			wr_qos.src_mac_mask[0], wr_qos.src_mac_mask[1], wr_qos.src_mac_mask[2], wr_qos.src_mac_mask[3], 
			wr_qos.src_mac_mask[4], wr_qos.src_mac_mask[5], wr_qos.min_src_port, wr_qos.max_src_port);

	if ((wr_qos.min_src_ip[0] == 0x0) && (wr_qos.min_src_ip[1] == 0x0) && 
					(wr_qos.min_src_ip[2] == 0x0)) {
		printk("\t" NIPQUAD_FMT "/" NIPQUAD_FMT "\n", 
			NIPQUAD(wr_qos.min_src_ip[3]), NIPQUAD(wr_qos.max_src_ip[3]));
	} else {
		uint8 *cp = &wr_qos.min_src_ip[0];
		printk("\t%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
		cp = &wr_qos.max_src_ip[0];
		printk("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", 
				*cp, *(cp+1), *(cp+2), *(cp+3), *(cp+4), *(cp+5), *(cp+6), *(cp+7),
				*(cp+8), *(cp+9), *(cp+10), *(cp+11), *(cp+12), *(cp+13), *(cp+14), *(cp+15));
	}

	printk("\t%04x %02x %u/%u %u/%u\n", 
			wr_qos.ether_type, wr_qos.protocol_id, wr_qos.min_vlan_id, wr_qos.max_vlan_id, 
			wr_qos.min_8021p, wr_qos.max_8021p);
	printk("\t%d:%u/%u %d:%01x %d:%u/%u %u/%u %04x/%04x %04x/%04x\n", 
			wr_qos.tos_desp_sel_field&0x1, wr_qos.min_ipp, wr_qos.max_ipp, 
			wr_qos.tos_desp_sel_field&0x2, wr_qos.tos, 
			wr_qos.tos_desp_sel_field&0x4, wr_qos.min_dscp, wr_qos.max_dscp, 
			wr_qos.min_ipv6_tc, wr_qos.max_ipv6_tc, 
			wr_qos.min_pkt_data, wr_qos.max_pkt_data, wr_qos.min_pkt_data1, wr_qos.max_pkt_data1); 
	printk("\tIPP(%d):%01x TOS(%d):%01x DSCP(%d):%02x 802.1p(%d):%d, IPv6_TC(%d):%x QID(%d):%d\n",
			wr_qos.remarking_action&0x1, wr_qos.new_ipp, 
			wr_qos.remarking_action&0x2, wr_qos.new_tos, 
			wr_qos.remarking_action&0x4, wr_qos.new_dscp, 
			wr_qos.remarking_action&0x8, wr_qos.new_8021p, 
			wr_qos.remarking_action&0x10, wr_qos.new_ipv6_tc, 
			wr_qos.remarking_action&0x20, wr_qos.new_qid);

	return 0;
}

int do_hwnat_qos_wr_save(int argc, char *argv[], void *p)
{
	uint16 entry;
	qos_t *qos;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	qos = &qos_tbl[entry];

	memcpy(qos, &wr_qos, sizeof(qos_t));

	hwnat_qos_tbl_write(entry, qos);

	return 0;
}

int do_hwnat_qos_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_qos_tbl_cmds, argc, argv, p);
}

int do_hwnat_qos_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("qos\n");
	for (entry = 0; entry < QOS_TBL_SIZE; entry++) {
		printk("/*%03d*/ ", entry);

		printk("%d %04x %u/%u\n", qos_tbl[entry].valid, qos_tbl[entry].rxportmap, qos_tbl[entry].min_l3_pktlen, qos_tbl[entry].max_l3_pktlen);
		printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u.%u.%u.%u/%u.%u.%u.%u %u/%u\n", 
				qos_tbl[entry].dest_mac[0], qos_tbl[entry].dest_mac[1], qos_tbl[entry].dest_mac[2], qos_tbl[entry].dest_mac[3], 
				qos_tbl[entry].dest_mac[4], qos_tbl[entry].dest_mac[5],
				qos_tbl[entry].dest_mac_mask[0], qos_tbl[entry].dest_mac_mask[1], qos_tbl[entry].dest_mac_mask[2], qos_tbl[entry].dest_mac_mask[3], 
				qos_tbl[entry].dest_mac_mask[4], qos_tbl[entry].dest_mac_mask[5],
				NIPQUAD(qos_tbl[entry].min_dest_ip[3]), NIPQUAD(qos_tbl[entry].max_dest_ip[3]), qos_tbl[entry].min_dest_port, qos_tbl[entry].max_dest_port);
		printk("\t%02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x %u.%u.%u.%u/%u.%u.%u.%u %u/%u\n", 
				qos_tbl[entry].src_mac[0], qos_tbl[entry].src_mac[1], qos_tbl[entry].src_mac[2], qos_tbl[entry].src_mac[3], 
				qos_tbl[entry].src_mac[4], qos_tbl[entry].src_mac[5],
				qos_tbl[entry].src_mac_mask[0], qos_tbl[entry].src_mac_mask[1], qos_tbl[entry].src_mac_mask[2], qos_tbl[entry].src_mac_mask[3], 
				qos_tbl[entry].src_mac_mask[4], qos_tbl[entry].src_mac_mask[5],
				NIPQUAD(qos_tbl[entry].min_src_ip[3]), NIPQUAD(qos_tbl[entry].max_src_ip[3]), qos_tbl[entry].min_src_port, qos_tbl[entry].max_src_port);
		printk("\t%04x %02x %u/%u %u/%u\n", 
				qos_tbl[entry].ether_type, qos_tbl[entry].protocol_id, qos_tbl[entry].min_vlan_id, qos_tbl[entry].max_vlan_id, 
				qos_tbl[entry].min_8021p, qos_tbl[entry].max_8021p);
		printk("\t%d:%u/%u %d:%01x %d:%u/%u %u/%u %04x/%04x %04x/%04x\n", 
				qos_tbl[entry].tos_desp_sel_field&0x1, qos_tbl[entry].min_ipp, qos_tbl[entry].max_ipp, 
				qos_tbl[entry].tos_desp_sel_field&0x2, qos_tbl[entry].tos, 
				qos_tbl[entry].tos_desp_sel_field&0x4, qos_tbl[entry].min_dscp, qos_tbl[entry].max_dscp, 
				qos_tbl[entry].min_ipv6_tc, qos_tbl[entry].max_ipv6_tc, 
				qos_tbl[entry].min_pkt_data, qos_tbl[entry].max_pkt_data, qos_tbl[entry].min_pkt_data1, qos_tbl[entry].max_pkt_data1); 
		printk("\tIPP(%d):%01x TOS(%d):%01x DSCP(%d):%02x 802.1p(%d):%d, IPv6_TC(%d):%x QID(%d):%d\n",
				qos_tbl[entry].remarking_action&0x1, qos_tbl[entry].new_ipp, 
				qos_tbl[entry].remarking_action&0x2, qos_tbl[entry].new_tos, 
				qos_tbl[entry].remarking_action&0x4, qos_tbl[entry].new_dscp, 
				qos_tbl[entry].remarking_action&0x8, qos_tbl[entry].new_8021p, 
				qos_tbl[entry].remarking_action&0x10, qos_tbl[entry].new_ipv6_tc, 
				qos_tbl[entry].remarking_action&0x20, qos_tbl[entry].new_qid);
	}
	return 0;
}

int do_hwnat_qos_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	qos_t *qos;
	uint16 entry, i;

	for (entry = 0; entry < QOS_TBL_SIZE; entry++) {
		qos = &qos_tbl[entry];

		printk("/*%03d*/ ", entry);

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = ((qos->min_dest_ip[2])&0xffffffff);
		table_hold[6] = ((qos->min_dest_ip[1])&0xffffffff);
		table_hold[5] = ((qos->min_dest_ip[0])&0xffffffff);
		table_hold[4] = ((qos->dest_mac_mask[5])&0xff) | ((qos->dest_mac_mask[4]<<8)&0xff00) |
			((qos->dest_mac_mask[3]<<16)&0xff0000) | ((qos->dest_mac_mask[2]<<24)&0xff000000);
		table_hold[3] = ((qos->dest_mac_mask[1])&0xff) | ((qos->dest_mac_mask[0]<<8)&0xff00) |
			((qos->dest_mac[5]<<16)&0xff0000) | ((qos->dest_mac_mask[4]<<24)&0xff000000);
		table_hold[2] = ((qos->dest_mac[3])&0xff) | ((qos->dest_mac[2]<<8)&0xff00) |
			((qos->dest_mac[1]<<16)&0xff0000) | ((qos->dest_mac[0]<<24)&0xff000000);
		table_hold[1] = ((qos->max_l3_pktlen)&0xffff) | ((qos->min_l3_pktlen<<16)&0x10000);
		table_hold[0] = ((qos->rxportmap)&0xf) | ((qos->valid<<4)&0x10);

		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = ((qos->src_mac_mask[1])&0xff) | ((qos->src_mac_mask[0]<<8)&0xff00) |
			((qos->src_mac[5]<<16)&0xff0000) | ((qos->src_mac[4]<<24)&0xff000000);
		table_hold[6] = ((qos->src_mac[3])&0xff) | ((qos->src_mac[2]<<8)&0xff00) |
			((qos->src_mac[1]<<16)&0xff0000) | ((qos->src_mac[0]<<24)&0xff000000);
		table_hold[5] = ((qos->max_dest_port)&0xffff) | ((qos->min_dest_port<<16)&0xffff0000);
		table_hold[4] = ((qos->max_dest_ip[3])&0xffffffff);
		table_hold[3] = ((qos->max_dest_ip[2])&0xffffffff);
		table_hold[2] = ((qos->max_dest_ip[1])&0xffffffff);
		table_hold[1] = ((qos->max_dest_ip[0])&0xffffffff);
		table_hold[0] = ((qos->min_dest_ip[3])&0xffffffff);

		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = ((qos->max_src_ip[2])&0xffffffff);
		table_hold[6] = ((qos->max_src_ip[1])&0xffffffff);
		table_hold[5] = ((qos->max_src_ip[0])&0xffffffff);
		table_hold[4] = ((qos->min_src_ip[3])&0xffffffff);
		table_hold[3] = ((qos->min_src_ip[2])&0xffffffff);
		table_hold[2] = ((qos->min_src_ip[1])&0xffffffff);
		table_hold[1] = ((qos->min_src_ip[0])&0xffffffff);
		table_hold[0] = ((qos->src_mac_mask[5])&0xff) | ((qos->src_mac_mask[4]<<8)&0xff00) |
			((qos->src_mac_mask[3]<<16)&0xff0000) | ((qos->src_mac_mask[2]<<24)&0xff000000);

		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (qos->new_qid&0x7) | ((qos->new_ipv6_tc<<3)&0x7f8) | 
			((qos->new_8021p<<11)&0x3800) | ((qos->new_dscp<<14)&0xfc000) | 
			((qos->new_tos<<20)&0xf00000) | ((qos->new_ipp<<24)&0x7000000) | 
			((qos->remarking_action<<27)&0xf8000000);
		table_hold[6] = ((qos->remarking_action>>5)&0x1) | ((qos->max_pkt_data1<<1)&0x1fffe) | 
			((qos->min_pkt_data1<<17)&0xfffe0000);
		table_hold[5] = ((qos->min_pkt_data1>>15)&0x1) | ((qos->max_pkt_data<<1)&0x1fffe) | 
			((qos->min_pkt_data<<17)&0xfffe0000);
		table_hold[4] = ((qos->min_pkt_data>>15)&0x1) | ((qos->max_ipv6_tc<<1)&0x1fe) | 
			((qos->min_ipv6_tc<<9)&0x1fe00) | ((qos->max_dscp<<17)&0x7e0000) | 
			((qos->min_dscp<<23)&0x1f800000) | ((qos->tos<<29)&0xe0000000);
		table_hold[3] = ((qos->tos>>3)&0x1) | ((qos->max_ipp<<1)&0xe) | ((qos->min_ipp<<4)&0x70) |
			((qos->tos_desp_sel_field<<7)&0x380) | ((qos->max_8021p<<10)&0x1c00) | 
			((qos->min_8021p<<13)&0xe000) | ((qos->max_vlan_id<<16)&0xfff0000) |
			((qos->min_vlan_id<<28)&0xf0000000);
		table_hold[2] = ((qos->min_vlan_id>>4)&0xff) | ((qos->protocol_id<<8)&0xff00) | 
			((qos->ether_type<<16)&0xffff0000);
		table_hold[1] = ((qos->max_src_port)&0xffff) | ((qos->min_src_port<<16)&0xffff0000);
		table_hold[0] = ((qos->max_src_ip[3])&0xffffffff);

		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n");

	}
	return 0;
}

int do_hwnat_qos_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;
	uint8 dump_one = 0;

	if (argc >= 2) {
		entry = simple_strtoul(argv[1], NULL, 10);
		dump_one = 1;
		goto hwraw_dump;
	}

	printk("qos\n");
	for (entry = 0; entry < QOS_TBL_SIZE; entry++) {
hwraw_dump:
		printk("/*%03d*/ ", entry);

		hwnat_read_tbl(QOS0_TBL_ID, entry, table_hold, GTH_SIZE);
		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		hwnat_read_tbl(QOS1_TBL_ID, entry, table_hold, GTH_SIZE);
		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		hwnat_read_tbl(QOS2_TBL_ID, entry, table_hold, GTH_SIZE);
		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n\t");

		hwnat_read_tbl(QOS3_TBL_ID, entry, table_hold, GTH_SIZE);
		for (i = 0; i < GTH_SIZE; i++) {
			printk("%08lx", table_hold[i]);
			if (i != GTH_SIZE - 1)
				printk(" ");
		}
		printk("\n");

		if (dump_one)
			break;
	}

	return 0;
}

int do_hwnat_qos_tbl_comp(int argc, char *argv[], void *p)
{
#if 0
	uint32 hw_table_hold[MTH_SIZE];
	uint32 table_hold[MTH_SIZE];
	qos_t *qos;
	uint16 entry, i;

	for (entry = 0; entry < QOS_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(MAC_ADDR_TBL_ID, entry, hw_table_hold, MTH_SIZE);

		qos = &qos_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[1] = (qos->addr[5]&0xff) | ((qos->addr[4]<<8)&0xff00) | 
			((qos->addr[3]<<16)&0xff0000) | ((qos->addr[2]<<24)&0xff000000);
		table_hold[0] = (qos->addr[1]&0xff) | ((qos->addr[0]<<8)&0xff00);

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%02d*/\n", entry);
			for (i = 0; i < MTH_SIZE; i++)
				printk(" %08lx", hw_table_hold[i]);
			printk("\n");
			for (i = 0; i < MTH_SIZE; i++)
				printk(" %08lx", table_hold[i]);
			printk("\n");
		}
	}
#endif
	return 0;
}


