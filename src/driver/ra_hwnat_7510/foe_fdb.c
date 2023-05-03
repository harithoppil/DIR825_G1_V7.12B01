/*
    Module Name:
    foe_fdb.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/delay.h>

//#include "frame_engine.h"
#include "foe_fdb.h"
#include "hwnat_ioctl.h"
#include "util.h"
extern int DebugLevel;
extern struct FoeEntry *PpeFoeBase;

#if defined (CONFIG_HNAT_V2)
/*
 * 4          2         0
 * +----------+---------+
 * |      DMAC[47:16]   |
 * +--------------------+
 * |DMAC[15:0]| 2nd VID |
 * +----------+---------+
 * 
 * 4          2         0
 * +----------+---------+
 * |      SMAC[47:16]   |
 * +--------------------+
 * |SMAC[15:0]| PPPOE ID|
 * +----------+---------+
 * 
 * Ex: 
 *
 * Mac=01:22:33:44:55:66
 *
 * 4          2         0
 * +----------+---------+
 * |     01:22:33:44    |
 * +--------------------+
 * |  55:66   | PPPOE ID|
 * +----------+---------+
 */
void FoeSetMacInfo(uint8_t * Dst, uint8_t * Src)
{
#ifdef __BIG_ENDIAN
	Dst[0] = Src[0];
	Dst[1] = Src[1];
	Dst[2] = Src[2];
	Dst[3] = Src[3];
	Dst[4] = Src[4];
	Dst[5] = Src[5];
#else
	Dst[3] = Src[0];
	Dst[2] = Src[1];
	Dst[1] = Src[2];
	Dst[0] = Src[3];
	Dst[7] = Src[4];
	Dst[6] = Src[5];
#endif	
}

void FoeGetMacInfo(uint8_t * Dst, uint8_t * Src)
{
#ifdef __BIG_ENDIAN
	Dst[0] = Src[0];
	Dst[1] = Src[1];
	Dst[2] = Src[2];
	Dst[3] = Src[3];
	Dst[4] = Src[4];
	Dst[5] = Src[5];
#else
	Dst[0] = Src[3];
	Dst[1] = Src[2];
	Dst[2] = Src[1];
	Dst[3] = Src[0];
	Dst[4] = Src[7];
	Dst[5] = Src[6];
#endif	
}

#else
/* 
 * Mac address is not continuous in foe table
 *
 * 4      2	  0
 * +------+-------+
 * |VLAN  |DMac_hi|
 * +------+-------+
 * |  Dmac_lo     |
 * +--------------+
 *
 * 4      2	  0
 * +------+-------+
 * |PPPoE |SMac_hi|
 * +------+-------+
 * |  Smac_lo     |
 * +--------------+
 *
 * Ex: 
 *
 * Mac=01:80:C2:01:23:45
 *
 * 4      2	  0
 * +------+-------+
 * |PPPoE | 01:80 |
 * +------+-------+
 * | C2:01:23:45  |
 * +--------------+
 *
 */
void FoeSetMacInfo(uint8_t * Dst, uint8_t * Src)
{
	Dst[1] = Src[0];
	Dst[0] = Src[1];
	Dst[7] = Src[2];
	Dst[6] = Src[3];
	Dst[5] = Src[4];
	Dst[4] = Src[5];
}

void FoeGetMacInfo(uint8_t * Dst, uint8_t * Src)
{
	Dst[0] = Src[1];
	Dst[1] = Src[0];
	Dst[2] = Src[7];
	Dst[3] = Src[6];
	Dst[4] = Src[5];
	Dst[5] = Src[4];
}
#endif

#if defined (CONFIG_HNAT_V2)
static int is_request_done(void)
{
	int count = 1000;

	//waiting for 1sec to make sure action was finished
	do {
		if (((RegRead(CAH_CTRL) >> 8) & 0x1) == 0) {
			return 1;
		}
		mdelay(1);
		//udelay(1000);
	} while(--count);

	return 0;
}

#define MAX_CACHE_LINE_NUM		16
int FoeDumpCacheEntry(void)
{
	int line = 0;
	int state = 0;
	int tag = 0;
	int cah_en = 0;
	int i = 0;

	cah_en = RegRead(CAH_CTRL) & 0x1;

	if(!cah_en) {
		printk("Cache is not enabled\n");
		return 0;
	}


	// cache disable
	RegModifyBits(CAH_CTRL, 0, 0, 1);

	printk(" No  |   State   |   Tag        \n");
	printk("-----+-----------+------------  \n");
	for(line = 0; line < MAX_CACHE_LINE_NUM; line++) {
	
		//set line number
		RegModifyBits(CAH_LINE_RW, line, 0, 15);

		//OFFSET_RW = 0x1F (Get Entry Number)
		RegModifyBits(CAH_LINE_RW, 0x1F, 16, 8);

		//software access cache command = read	
		RegModifyBits(CAH_CTRL, 2, 12, 2);

		//trigger software access cache request
		RegModifyBits(CAH_CTRL, 1, 8, 1);

		if (is_request_done()) {
			tag = (RegRead(CAH_RDATA) & 0xFFFF) ;
			state = ((RegRead(CAH_RDATA)>>16) & 0x3) ;
			printk("%04d | %s   | %05d\n", line, 
					(state==3) ? " Lock  " : 
					(state==2) ? " Dirty " :
					(state==1) ? " Valid " : 
						     "Invalid", tag);
		} else {
			printk("%s is timeout (%d)\n", __FUNCTION__, line);
		}
		
		//software access cache command = read	
		RegModifyBits(CAH_CTRL, 3, 12, 2);

		RegWrite(CAH_WDATA,0);

		//trigger software access cache request
		RegModifyBits(CAH_CTRL, 1, 8, 1);

		if (!is_request_done()) {
			printk("%s is timeout (%d)\n", __FUNCTION__, line);
		}

		/* dump first 16B for each foe entry */
		printk("==========<Flow Table Entry=%d >===============\n", tag);
		for(i = 0; i< 16; i++ ) {
			RegModifyBits(CAH_LINE_RW, i, 16, 8);
			
			//software access cache command = read	
			RegModifyBits(CAH_CTRL, 2, 12, 2);

			//trigger software access cache request
			RegModifyBits(CAH_CTRL, 1, 8, 1);

			if (is_request_done()) {
				printk("%02d  %08X\n", i, RegRead(CAH_RDATA));
			} else {
				printk("%s is timeout (%d)\n", __FUNCTION__, line);
			}
			
			//software access cache command = write	
			RegModifyBits(CAH_CTRL, 3, 12, 2);
			
			RegWrite(CAH_WDATA, 0);

			//trigger software access cache request
			RegModifyBits(CAH_CTRL, 1, 8, 1);

			if (!is_request_done()) {
				printk("%s is timeout (%d)\n", __FUNCTION__, line);
			}
		}
		printk("=========================================\n");
	}

	//clear cache table before enabling cache
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);

	// cache enable
	RegModifyBits(CAH_CTRL, 1, 0, 1);

	return 1;
}
#endif

void FoeDumpEntry(uint32_t Index)
{
	struct FoeEntry *entry = &PpeFoeBase[Index];
	uint32_t *p = (uint32_t *)entry;
	uint32_t i = 0;

	NAT_PRINT("==========<Flow Table Entry=%d (%p)>===============\n", Index, entry);
#if defined (CONFIG_RA_HW_NAT_IPV6)
	for(i=0; i < 20; i++) { // 80 bytes per entry
#else
	for(i=0; i < 16; i++) { // 64 bytes per entry
#endif
		printk("%02d: %08X\n", i,*(p+i));
	}
	NAT_PRINT("-----------------<Flow Info>------------------\n");
	NAT_PRINT("Information Block 1: %08X\n", entry->ipv4_hnapt.info_blk1);

	if (IS_IPV4_HNAPT(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv4_hnapt.info_blk2);
		NAT_PRINT("Create IPv4 HNAPT entry\n");
		NAT_PRINT
		    ("IPv4 Org IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		     IP_FORMAT(entry->ipv4_hnapt.sip), entry->ipv4_hnapt.sport,
		     IP_FORMAT(entry->ipv4_hnapt.dip), entry->ipv4_hnapt.dport);
		NAT_PRINT
		    ("IPv4 New IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		     IP_FORMAT(entry->ipv4_hnapt.new_sip), entry->ipv4_hnapt.new_sport,
		     IP_FORMAT(entry->ipv4_hnapt.new_dip), entry->ipv4_hnapt.new_dport);
	} else if (IS_IPV4_HNAT(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv4_hnapt.info_blk2);
		NAT_PRINT("Create IPv4 HNAT entry\n");
		NAT_PRINT("IPv4 Org IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.sip), IP_FORMAT(entry->ipv4_hnapt.dip));
		NAT_PRINT("IPv4 New IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
			  IP_FORMAT(entry->ipv4_hnapt.new_sip), IP_FORMAT(entry->ipv4_hnapt.new_dip));
#if defined(TCSUPPORT_RA_HWNAT) && defined(CONFIG_RA_HW_NAT_L2B)
	} else if (IS_L2_RRIDGE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->l2_bridge.info_blk2);
		NAT_PRINT("Create L2 Bridge entry\n");
		NAT_PRINT("IN DMAC=%02X:%02X:%02X:%02X:%02X:%02X IN SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->l2_bridge.in_dmac_hi[0], entry->l2_bridge.in_dmac_hi[1],
		 entry->l2_bridge.in_dmac_hi[2], entry->l2_bridge.in_dmac_hi[3],
	     entry->l2_bridge.in_dmac_lo[0], entry->l2_bridge.in_dmac_lo[1],
	     entry->l2_bridge.in_smac_lo[0], entry->l2_bridge.in_smac_lo[1],
	     entry->l2_bridge.in_smac_lo[2], entry->l2_bridge.in_smac_lo[3],
	     entry->l2_bridge.in_smac_hi[0], entry->l2_bridge.in_smac_hi[1]);
#endif
#if defined (CONFIG_RA_HW_NAT_IPV6)
#if !defined(TCSUPPORT_RA_HWNAT)
	} else if (IS_IPV6_1T_ROUTE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv6_1t_route.info_blk2);
		NAT_PRINT("Create IPv6 Route entry\n");
		NAT_PRINT("Destination IPv6: %08X:%08X:%08X:%08X",
			  entry->ipv6_1t_route.ipv6_dip0, entry->ipv6_1t_route.ipv6_dip1,
			  entry->ipv6_1t_route.ipv6_dip2, entry->ipv6_1t_route.ipv6_dip3);
#endif		
#if defined (CONFIG_HNAT_V2)
	} else if (IS_IPV4_DSLITE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv4_dslite.info_blk2);
		NAT_PRINT("Create IPv4 Ds-Lite entry\n");
		NAT_PRINT
		    ("IPv4 Ds-Lite: %u.%u.%u.%u.%d->%u.%u.%u.%u:%d\n ",
		     IP_FORMAT(entry->ipv4_dslite.sip), entry->ipv4_dslite.sport,
		     IP_FORMAT(entry->ipv4_dslite.dip), entry->ipv4_dslite.dport);
		NAT_PRINT("EG DIPv6: %08X:%08X:%08X:%08X->%08X:%08X:%08X:%08X\n",
			  entry->ipv4_dslite.tunnel_sipv6_0, entry->ipv4_dslite.tunnel_sipv6_1,
			  entry->ipv4_dslite.tunnel_sipv6_2, entry->ipv4_dslite.tunnel_sipv6_3,
			  entry->ipv4_dslite.tunnel_dipv6_0, entry->ipv4_dslite.tunnel_dipv6_1,
			  entry->ipv4_dslite.tunnel_dipv6_2, entry->ipv4_dslite.tunnel_dipv6_3);
	} else if (IS_IPV6_3T_ROUTE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv6_3t_route.info_blk2);
		NAT_PRINT("Create IPv6 3-Tuple entry\n");
		NAT_PRINT
		    ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Prot=%d)\n",
		     entry->ipv6_3t_route.ipv6_sip0, entry->ipv6_3t_route.ipv6_sip1,
		     entry->ipv6_3t_route.ipv6_sip2, entry->ipv6_3t_route.ipv6_sip3,
		     entry->ipv6_3t_route.ipv6_dip0, entry->ipv6_3t_route.ipv6_dip1,
		     entry->ipv6_3t_route.ipv6_dip2, entry->ipv6_3t_route.ipv6_dip3, 
		     entry->ipv6_3t_route.prot);
	} else if (IS_IPV6_5T_ROUTE(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv6_5t_route.info_blk2);
		NAT_PRINT("Create IPv6 5-Tuple entry\n");
		if(IS_IPV6_FLAB_EBL()) {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Flow Label=%08X) \n",
				entry->ipv6_5t_route.ipv6_sip0, entry->ipv6_5t_route.ipv6_sip1,
				entry->ipv6_5t_route.ipv6_sip2, entry->ipv6_5t_route.ipv6_sip3,
				entry->ipv6_5t_route.ipv6_dip0, entry->ipv6_5t_route.ipv6_dip1,
				entry->ipv6_5t_route.ipv6_dip2, entry->ipv6_5t_route.ipv6_dip3,
				(entry->ipv6_5t_route.sport << 16) | (entry->ipv6_5t_route.dport));
		} else {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d \n",
				entry->ipv6_5t_route.ipv6_sip0, entry->ipv6_5t_route.ipv6_sip1,
				entry->ipv6_5t_route.ipv6_sip2, entry->ipv6_5t_route.ipv6_sip3,
				entry->ipv6_5t_route.sport,
				entry->ipv6_5t_route.ipv6_dip0, entry->ipv6_5t_route.ipv6_dip1,
				entry->ipv6_5t_route.ipv6_dip2, entry->ipv6_5t_route.ipv6_dip3,
				entry->ipv6_5t_route.dport);
		}
	} else if (IS_IPV6_6RD(entry)) {
		NAT_PRINT("Information Block 2: %08X\n", entry->ipv6_6rd.info_blk2);
		NAT_PRINT("Create IPv6 6RD entry\n");
		if(IS_IPV6_FLAB_EBL()) {
			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X-> %08X:%08X:%08X:%08X (Flow Label=%08X) \n",
				entry->ipv6_6rd.ipv6_sip0, entry->ipv6_6rd.ipv6_sip1,
				entry->ipv6_6rd.ipv6_sip2, entry->ipv6_6rd.ipv6_sip3,
				entry->ipv6_6rd.ipv6_dip0, entry->ipv6_6rd.ipv6_dip1, 
				entry->ipv6_6rd.ipv6_dip2, entry->ipv6_6rd.ipv6_dip3, 
				(entry->ipv6_5t_route.sport << 16) | (entry->ipv6_5t_route.dport));
		} else {

			NAT_PRINT ("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d \n",
				entry->ipv6_6rd.ipv6_sip0, entry->ipv6_6rd.ipv6_sip1,
				entry->ipv6_6rd.ipv6_sip2, entry->ipv6_6rd.ipv6_sip3,
				entry->ipv6_6rd.sport, 
				entry->ipv6_6rd.ipv6_dip0, entry->ipv6_6rd.ipv6_dip1, 
				entry->ipv6_6rd.ipv6_dip2, entry->ipv6_6rd.ipv6_dip3, 
				entry->ipv6_6rd.dport);
		}
#endif
#endif
	}

#if defined (CONFIG_HNAT_V2)
#if defined(TCSUPPORT_RA_HWNAT) && defined(CONFIG_RA_HW_NAT_L2B)
	if (IS_IPV4_HNAPT(entry) || IS_IPV4_HNAT(entry) || IS_L2_RRIDGE(entry)) {
#else
	if (IS_IPV4_HNAPT(entry) || IS_IPV4_HNAT(entry)) {
#endif		
#ifdef __BIG_ENDIAN
		NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv4_hnapt.dmac_hi[0], entry->ipv4_hnapt.dmac_hi[1],
		 entry->ipv4_hnapt.dmac_hi[2], entry->ipv4_hnapt.dmac_hi[3],
	     entry->ipv4_hnapt.dmac_lo[0], entry->ipv4_hnapt.dmac_lo[1],
	     entry->ipv4_hnapt.smac_hi[0], entry->ipv4_hnapt.smac_hi[1],
	     entry->ipv4_hnapt.smac_hi[2], entry->ipv4_hnapt.smac_hi[3],
	     entry->ipv4_hnapt.smac_lo[0], entry->ipv4_hnapt.smac_lo[1]);
#else

	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv4_hnapt.dmac_hi[3], entry->ipv4_hnapt.dmac_hi[2],
	     entry->ipv4_hnapt.dmac_lo[1], entry->ipv4_hnapt.dmac_lo[0],
	     entry->ipv4_hnapt.dmac_lo[1], entry->ipv4_hnapt.dmac_lo[0],
	     entry->ipv4_hnapt.smac_hi[3], entry->ipv4_hnapt.smac_hi[2],
	     entry->ipv4_hnapt.smac_lo[1], entry->ipv4_hnapt.smac_lo[0],
	     entry->ipv4_hnapt.smac_lo[1], entry->ipv4_hnapt.smac_lo[0]);
#endif
	    NAT_PRINT("=========================================\n\n");
	} 
#if defined (CONFIG_RA_HW_NAT_IPV6)
	else {
#ifdef __BIG_ENDIAN
		NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv6_5t_route.dmac_hi[0], entry->ipv6_5t_route.dmac_hi[1],
	     entry->ipv6_5t_route.dmac_hi[2], entry->ipv6_5t_route.dmac_hi[3],
	     entry->ipv6_5t_route.dmac_lo[0], entry->ipv6_5t_route.dmac_lo[1],
	     entry->ipv6_5t_route.smac_hi[0], entry->ipv6_5t_route.smac_hi[1],
	     entry->ipv6_5t_route.smac_hi[2], entry->ipv6_5t_route.smac_hi[3],
	     entry->ipv6_5t_route.smac_lo[0], entry->ipv6_5t_route.smac_lo[1]);
#else
	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv6_5t_route.dmac_hi[3], entry->ipv6_5t_route.dmac_hi[2],
	     entry->ipv6_5t_route.dmac_hi[1], entry->ipv6_5t_route.dmac_hi[0],
	     entry->ipv6_5t_route.dmac_lo[1], entry->ipv6_5t_route.dmac_lo[0],
	     entry->ipv6_5t_route.smac_hi[3], entry->ipv6_5t_route.smac_hi[2],
	     entry->ipv6_5t_route.smac_hi[1], entry->ipv6_5t_route.smac_hi[0],
	     entry->ipv6_5t_route.smac_lo[1], entry->ipv6_5t_route.smac_lo[0]);
#endif
	    NAT_PRINT("=========================================\n\n");
	}
#endif

#else
	if (IS_IPV4_HNAPT(entry) || IS_IPV4_HNAT(entry) || IS_IPV6_1T_ROUTE(entry)) {

	    NAT_PRINT
		("DMAC=%02X:%02X:%02X:%02X:%02X:%02X SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
		 entry->ipv4_hnapt.dmac_hi[1], entry->ipv4_hnapt.dmac_hi[0],
	     entry->ipv4_hnapt.dmac_lo[3], entry->ipv4_hnapt.dmac_lo[2],
	     entry->ipv4_hnapt.dmac_lo[1], entry->ipv4_hnapt.dmac_lo[0],
	     entry->ipv4_hnapt.smac_hi[1], entry->ipv4_hnapt.smac_hi[0],
	     entry->ipv4_hnapt.smac_lo[3], entry->ipv4_hnapt.smac_lo[2],
	     entry->ipv4_hnapt.smac_lo[1], entry->ipv4_hnapt.smac_lo[0]);
	    NAT_PRINT("=========================================\n\n");
	} 
#endif
}
#ifdef TCSUPPORT_RA_HWNAT



int FoeAddEntry(struct hwnat_tuple *opt)
{
	struct FoePriKey key;
	struct FoeEntry *entry;
	int32_t hash_index;

	if (DebugLevel >= 2) 
	{
		if(opt->pkt_type == L2_BRIDGE){
			printk("IN smac %x:%x:%x:%x:%x:%x \n",opt->in_smac[0],opt->in_smac[1],opt->in_smac[2],opt->in_smac[3],opt->in_smac[4],opt->in_smac[5]);
			printk("IN dmac %x:%x:%x:%x:%x:%x \n",opt->in_dmac[0],opt->in_dmac[1],opt->in_dmac[2],opt->in_dmac[3],opt->in_dmac[4],opt->in_dmac[5]);
		}else{	
			if((opt->pkt_type == IPV4_HNAPT) || (opt->pkt_type == IPV4_HNAT) || (opt->pkt_type == IPV4_DSLITE))
			{
				printk("in sip %x is dip %x eg sip %x is dip %x\n",opt->ing_sipv4,opt->ing_dipv4,opt->eg_sipv4,opt->eg_dipv4);
			}else
			{
				printk("in sip %x %x %x %x \n",opt->ing_sipv6_0,opt->ing_sipv6_1,opt->ing_sipv6_2,opt->ing_sipv6_3);
				printk("in dip %x %x %x %x \n",opt->ing_dipv6_0,opt->ing_dipv6_1,opt->ing_dipv6_2,opt->ing_dipv6_3);
				printk("eg sip %x %x %x %x \n",opt->eg_sipv6_0,opt->eg_sipv6_1,opt->eg_sipv6_2,opt->eg_sipv6_3);
				printk("eg dip %x %x %x %x \n",opt->eg_dipv6_0,opt->eg_dipv6_1,opt->eg_dipv6_2,opt->eg_dipv6_3);
			}
			printk("in ing sp %d in dp %d eg sp %d ing dp %d\n",opt->ing_sp,opt->ing_dp,opt->eg_sp,opt->eg_dp);
			printk("pkt_type %d is_udp %d eg dscp %x\n",opt->pkt_type,opt->is_udp,opt->eg_dscp);
		}
		printk("stag %x vlan_layer %d vlan1 %x vlan2 %x pppoe %x prot %x\n",opt->stag,opt->vlan_layer, opt->vlan1,opt->vlan2,opt->pppoe_id,opt->prot);
		printk("smac %x:%x:%x:%x:%x:%x dmac %x:%x:%x:%x:%x:%x \n",opt->smac[0],opt->smac[1],opt->smac[2],opt->smac[3],opt->smac[4],opt->smac[5],opt->dmac[0],opt->dmac[1],opt->dmac[2],opt->dmac[3],opt->dmac[4],opt->dmac[5]);
		printk("ag %x mg %x ipv6_flowlabel %x info2 %lx\n",opt->ag,opt->mg,opt->ipv6_flowlabel,opt->info2);
		printk("static %x tsid %x qid %x vpm %x pcp %x\n",opt->sta,opt->tsid,opt->qid,opt->vpm,opt->pcp);
		
		if(opt->pkt_type == IPV6_6RD)
			printk("tunnel header : dscp %x TTL %x IPv4_flag %x\n",opt->tunnel_dscp,opt->tunnel_TTL,opt->tunnel_IPv4_flag);
		else if (opt->pkt_type == IPV4_DSLITE)
			printk("tunnel header : TRFC %x HPOL %x\n",opt->tunnel_dscp,opt->tunnel_TTL);
	}

	
	key.pkt_type=opt->pkt_type;
	
	if(opt->pkt_type == L2_BRIDGE){
		memcpy(key.l2_bridge.in_smac, opt->in_smac, 6);
		memcpy(key.l2_bridge.in_dmac, opt->in_dmac, 6);
	}else if((opt->pkt_type == IPV4_HNAPT) || (opt->pkt_type == IPV4_HNAT) || (opt->pkt_type == IPV4_DSLITE))
	{
		key.ipv4_hnapt.is_udp=opt->is_udp;
		key.ipv4_hnapt.sip=opt->ing_sipv4;
		key.ipv4_hnapt.dip=opt->ing_dipv4;
		key.ipv4_hnapt.sport=opt->ing_sp;
		key.ipv4_hnapt.dport=opt->ing_dp;
		key.ipv4_hnapt.prot=opt->prot;
	}else{
		key.ipv6_routing.is_udp=opt->is_udp;
		key.ipv6_routing.sip0=opt->ing_sipv6_0;
		key.ipv6_routing.sip1=opt->ing_sipv6_1;
		key.ipv6_routing.sip2=opt->ing_sipv6_2;
		key.ipv6_routing.sip3=opt->ing_sipv6_3;
		key.ipv6_routing.dip0=opt->ing_dipv6_0;
		key.ipv6_routing.dip1=opt->ing_dipv6_1;
		key.ipv6_routing.dip2=opt->ing_dipv6_2;
		key.ipv6_routing.dip3=opt->ing_dipv6_3;
		key.ipv6_routing.sport=opt->ing_sp;
		key.ipv6_routing.dport=opt->ing_dp;
		key.ipv6_routing.nexth=opt->prot;
	}
   
	hash_index = FoeHashFun(&key,INVALID);
	hash_index += opt->hash_index_shift;
	if (DebugLevel >= 2) 
		printk("hash_index %d\n",hash_index);
	if(hash_index != -1) {
		opt->hash_index=hash_index;
		entry=&PpeFoeBase[hash_index];
		memset(entry, 0, sizeof(struct FoeEntry));
		entry->bfib1.sta=opt->sta; /* static entry*/
		entry->bfib1.udp=opt->is_udp; /* tcp/udp */
		entry->bfib1.state=BIND; 
		entry->bfib1.vpm = opt->vpm; /* vpm */
		entry->bfib1.ttl=1; /* TTL-1 */
		entry->bfib1.cah=1; /* always cache */
		if(opt->pppoe_id != 0){
			entry->bfib1.psn=1; /* has pppoe id */
		}else{
			entry->bfib1.psn=0; /* has pppoe id */
		}
		if(opt->vlan_layer == 0xff){
			if((opt->vlan1 != 0) || (opt->stag != 0)){
				if(opt->vlan2 != 0){
					entry->bfib1.vlan_layer=2;
				}else{
					entry->bfib1.vlan_layer=1;
				}
			}else{
				entry->bfib1.vlan_layer=0;
			}
		}else{
			entry->bfib1.vlan_layer = opt->vlan_layer;
		}
		entry->bfib1.time_stamp=RegRead(FOE_TS)&0xFFFF;
		entry->bfib1.pkt_type = opt->pkt_type;
		if((opt->pkt_type == IPV4_HNAPT) || (opt->pkt_type == IPV4_HNAT))
		{
			entry->ipv4_hnapt.sip=opt->ing_sipv4; 
			entry->ipv4_hnapt.dip=opt->ing_dipv4;
			entry->ipv4_hnapt.new_sip=opt->eg_sipv4;
			entry->ipv4_hnapt.new_dip=opt->eg_dipv4;
			if(opt->pkt_type == IPV4_HNAPT){
				entry->ipv4_hnapt.dport=opt->ing_dp;
				entry->ipv4_hnapt.sport=opt->ing_sp;
				entry->ipv4_hnapt.new_dport=opt->eg_dp;
				entry->ipv4_hnapt.new_sport=opt->eg_sp;
			}else{
				entry->ipv4_hnapt.sport=0xa5a5; // no use
				entry->ipv4_hnapt.dport=0xa500 | opt->prot; // means protocol
			}
			
			entry->ipv4_hnapt.iblk2.dscp=opt->eg_dscp;		
		}else if((opt->pkt_type == IPV6_3T_ROUTE) || (opt->pkt_type == IPV6_5T_ROUTE)|| (opt->pkt_type == IPV6_6RD))
		{
			entry->ipv6_5t_route.ipv6_sip0=opt->ing_sipv6_3;
			entry->ipv6_5t_route.ipv6_sip1=opt->ing_sipv6_2;
			entry->ipv6_5t_route.ipv6_sip2=opt->ing_sipv6_1;
			entry->ipv6_5t_route.ipv6_sip3=opt->ing_sipv6_0;
			entry->ipv6_5t_route.ipv6_dip0=opt->ing_dipv6_3;
			entry->ipv6_5t_route.ipv6_dip1=opt->ing_dipv6_2;
			entry->ipv6_5t_route.ipv6_dip2=opt->ing_dipv6_1;
			entry->ipv6_5t_route.ipv6_dip3=opt->ing_dipv6_0;
			if((opt->pkt_type == IPV6_5T_ROUTE) || (opt->pkt_type == IPV6_6RD)){
				entry->ipv6_5t_route.sport=opt->ing_sp;
				entry->ipv6_5t_route.dport=opt->ing_dp;
			}else{
				entry->ipv6_5t_route.sport=0xa5a5; // no use
				entry->ipv6_5t_route.dport=0xa500 | opt->prot; // means ipv6 next header
			}
			
			entry->ipv6_5t_route.iblk2.dscp=opt->eg_dscp;

			if(opt->pkt_type == IPV6_6RD)
			{
				entry->ipv6_6rd.bfib1.rmt=opt->RMT;
				entry->ipv6_6rd.tunnel_dipv4 = opt->eg_dipv4;
				entry->ipv6_6rd.tunnel_sipv4 = opt->eg_sipv4;
				entry->ipv6_6rd.dscp = opt->tunnel_dscp;
				entry->ipv6_6rd.ttl = opt->tunnel_TTL;
				entry->ipv6_6rd.hdr_chksum = 0;
				entry->ipv6_6rd.flag = opt->tunnel_IPv4_flag;
			}
		}else if(opt->pkt_type == IPV4_DSLITE){
			entry->ipv4_dslite.sip=opt->ing_sipv4; 
			entry->ipv4_dslite.dip=opt->ing_dipv4;
			entry->ipv4_dslite.dport=opt->ing_dp;
			entry->ipv4_dslite.sport=opt->ing_sp;

			entry->ipv4_dslite.tunnel_dipv6_0=opt->eg_dipv6_3;
			entry->ipv4_dslite.tunnel_dipv6_1=opt->eg_dipv6_2;
			entry->ipv4_dslite.tunnel_dipv6_2=opt->eg_dipv6_1;
			entry->ipv4_dslite.tunnel_dipv6_3=opt->eg_dipv6_0;
			entry->ipv4_dslite.tunnel_sipv6_0=opt->eg_sipv6_3;
			entry->ipv4_dslite.tunnel_sipv6_1=opt->eg_sipv6_2;
			entry->ipv4_dslite.tunnel_sipv6_2=opt->eg_sipv6_1;
			entry->ipv4_dslite.tunnel_sipv6_3=opt->eg_sipv6_0;
			
			entry->ipv4_dslite.bfib1.rmt=opt->RMT;
			entry->ipv4_dslite.iblk2.dscp=opt->eg_dscp;
			entry->ipv4_dslite.priority=opt->tunnel_dscp;
			entry->ipv4_dslite.hop_limit = opt->tunnel_TTL;
			
			entry->ipv4_dslite.flow_lbl[0]=opt->ipv6_flowlabel & 0xff;
			entry->ipv4_dslite.flow_lbl[1]=(opt->ipv6_flowlabel>>8) & 0xff;
			entry->ipv4_dslite.flow_lbl[2]=(opt->ipv6_flowlabel>>16) & 0xff;
		}else if(opt->pkt_type == L2_BRIDGE){
			FoeSetMacInfo(entry->l2_bridge.in_dmac_hi,opt->in_dmac);
			FoeSetMacInfo(entry->l2_bridge.in_smac_hi,opt->in_smac);
		}
		if((opt->pkt_type == IPV4_HNAPT) || (opt->pkt_type == IPV4_HNAT) || (opt->pkt_type == L2_BRIDGE))
		{
			if(opt->info2 == 0xffffffff){
				entry->ipv4_hnapt.iblk2.fpidx=opt->dst_port;
				entry->ipv4_hnapt.iblk2.pcp=opt->pcp;
				entry->ipv4_hnapt.iblk2.port_mg=opt->mg;
				entry->ipv4_hnapt.iblk2.port_ag=opt->ag;

				if(opt->dst_port == FP_QDMA_HW){
					entry->ipv4_hnapt.iblk2.fqos = 1;
					entry->ipv4_hnapt.iblk2.qid = opt->qid;
				}	
			}else{
				entry->ipv4_hnapt.info_blk2 = opt->info2;
			}	
			entry->ipv4_hnapt.etype=opt->stag;
			entry->ipv4_hnapt.vlan1=opt->vlan1;
			entry->ipv4_hnapt.vlan2=opt->vlan2;
			entry->ipv4_hnapt.pppoe_id=opt->pppoe_id;
			entry->ipv4_hnapt.ts_id=opt->tsid;

			FoeSetMacInfo(((struct _ipv4_hnapt *)entry)->dmac_hi,opt->dmac);
			FoeSetMacInfo(((struct _ipv4_hnapt *)entry)->smac_hi,opt->smac);
		}else{
			if(opt->info2 == 0xffffffff){
				entry->ipv6_5t_route.iblk2.fpidx=opt->dst_port;
				entry->ipv6_5t_route.iblk2.pcp=opt->pcp;
				entry->ipv6_5t_route.iblk2.port_mg=opt->mg;
				entry->ipv6_5t_route.iblk2.port_ag=opt->ag;
				if(opt->dst_port == FP_QDMA_HW){
					entry->ipv6_5t_route.iblk2.fqos = 1;
					entry->ipv6_5t_route.iblk2.qid = opt->qid;
				}
			}else{
				entry->ipv6_5t_route.info_blk2 = opt->info2;
			}	
			entry->ipv6_5t_route.etype=opt->stag;
			entry->ipv6_5t_route.vlan1=opt->vlan1;
			entry->ipv6_5t_route.vlan2=opt->vlan2;
			entry->ipv6_5t_route.pppoe_id=opt->pppoe_id;
			entry->ipv6_5t_route.ts_id=opt->tsid;

			FoeSetMacInfo(((struct _ipv6_5t_route *)entry)->dmac_hi,opt->dmac);
			FoeSetMacInfo(((struct _ipv6_5t_route *)entry)->smac_hi,opt->smac);
		}	
	
		return HWNAT_SUCCESS;
	}
	
	return HWNAT_FAIL;
	
}
#endif


int FoeGetAllEntries(struct hwnat_args *opt)
{
	struct FoeEntry *entry;
	int hash_index = 0;
	int count = 0;		/* valid entry count */

	for (hash_index = 0; hash_index < FOE_4TB_SIZ; hash_index++) {
		entry = &PpeFoeBase[hash_index];

		if (entry->bfib1.state == opt->entry_state) {
			opt->entries[count].hash_index = hash_index;
			opt->entries[count].pkt_type =
			    entry->ipv4_hnapt.bfib1.pkt_type;

			if (IS_IPV4_HNAT(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_hnapt.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_hnapt.dip;
				opt->entries[count].eg_sipv4 = entry->ipv4_hnapt.new_sip;
				opt->entries[count].eg_dipv4 = entry->ipv4_hnapt.new_dip;
				count++;
			} else if (IS_IPV4_HNAPT(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_hnapt.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_hnapt.dip;
				opt->entries[count].eg_sipv4 = entry->ipv4_hnapt.new_sip;
				opt->entries[count].eg_dipv4 = entry->ipv4_hnapt.new_dip;
				opt->entries[count].ing_sp = entry->ipv4_hnapt.sport;
				opt->entries[count].ing_dp = entry->ipv4_hnapt.dport;
				opt->entries[count].eg_sp = entry->ipv4_hnapt.new_sport;
				opt->entries[count].eg_dp = entry->ipv4_hnapt.new_dport;
				count++;
#if defined(TCSUPPORT_RA_HWNAT) && defined(CONFIG_RA_HW_NAT_L2B)
			} else if (IS_L2_RRIDGE(entry)) {
				opt->entries[count].in_dmac[5] = entry->l2_bridge.in_dmac_lo[1];
				opt->entries[count].in_dmac[4] = entry->l2_bridge.in_dmac_lo[0];
				opt->entries[count].in_dmac[3] = entry->l2_bridge.in_dmac_hi[3];
				opt->entries[count].in_dmac[2] = entry->l2_bridge.in_dmac_hi[2];
				opt->entries[count].in_dmac[1] = entry->l2_bridge.in_dmac_hi[1];
				opt->entries[count].in_dmac[0] = entry->l2_bridge.in_dmac_hi[0];
				opt->entries[count].in_smac[5] = entry->l2_bridge.in_smac_hi[1];
				opt->entries[count].in_smac[4] = entry->l2_bridge.in_smac_hi[0];
				opt->entries[count].in_smac[3] = entry->l2_bridge.in_smac_lo[3];
				opt->entries[count].in_smac[2] = entry->l2_bridge.in_smac_lo[2];
				opt->entries[count].in_smac[1] = entry->l2_bridge.in_smac_lo[1];
				opt->entries[count].in_smac[0] = entry->l2_bridge.in_smac_lo[0];
				count++;
#endif
#if defined (CONFIG_RA_HW_NAT_IPV6)
#if !defined(TCSUPPORT_RA_HWNAT)
			} else if (IS_IPV6_1T_ROUTE(entry)) {
				opt->entries[count].ing_dipv6_0 = entry->ipv6_1t_route.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_1t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_1t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_1t_route.ipv6_dip3;
				count++;
#endif				
#if defined (CONFIG_HNAT_V2)
			} else if (IS_IPV4_DSLITE(entry)) {
				opt->entries[count].ing_sipv4 = entry->ipv4_dslite.sip;
				opt->entries[count].ing_dipv4 = entry->ipv4_dslite.dip;
				opt->entries[count].ing_sp = entry->ipv4_dslite.sport;
				opt->entries[count].ing_dp = entry->ipv4_dslite.dport;
				opt->entries[count].eg_sipv6_0 = entry->ipv4_dslite.tunnel_sipv6_0;
				opt->entries[count].eg_sipv6_1 = entry->ipv4_dslite.tunnel_sipv6_1;
				opt->entries[count].eg_sipv6_2 = entry->ipv4_dslite.tunnel_sipv6_2;
				opt->entries[count].eg_sipv6_3 = entry->ipv4_dslite.tunnel_sipv6_3;
				opt->entries[count].eg_dipv6_0 = entry->ipv4_dslite.tunnel_dipv6_0;
				opt->entries[count].eg_dipv6_1 = entry->ipv4_dslite.tunnel_dipv6_1;
				opt->entries[count].eg_dipv6_2 = entry->ipv4_dslite.tunnel_dipv6_2;
				opt->entries[count].eg_dipv6_3 = entry->ipv4_dslite.tunnel_dipv6_3;
				count++;
			} else if (IS_IPV6_3T_ROUTE(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_3t_route.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_3t_route.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_3t_route.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_3t_route.ipv6_sip3;
				opt->entries[count].ing_dipv6_0 = entry->ipv6_3t_route.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_3t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_3t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_3t_route.ipv6_dip3;
				opt->entries[count].prot = entry->ipv6_3t_route.prot;
				count++;
			} else if (IS_IPV6_5T_ROUTE(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_5t_route.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_5t_route.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_5t_route.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_5t_route.ipv6_sip3;
				opt->entries[count].ing_sp = entry->ipv6_5t_route.sport;
				opt->entries[count].ing_dp = entry->ipv6_5t_route.dport;

				opt->entries[count].ing_dipv6_0 = entry->ipv6_5t_route.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_5t_route.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_5t_route.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_5t_route.ipv6_dip3;
				opt->entries[count].ipv6_flowlabel = IS_IPV6_FLAB_EBL();
				count++;
			} else if (IS_IPV6_6RD(entry)) {
				opt->entries[count].ing_sipv6_0 = entry->ipv6_6rd.ipv6_sip0;
				opt->entries[count].ing_sipv6_1 = entry->ipv6_6rd.ipv6_sip1;
				opt->entries[count].ing_sipv6_2 = entry->ipv6_6rd.ipv6_sip2;
				opt->entries[count].ing_sipv6_3 = entry->ipv6_6rd.ipv6_sip3;

				opt->entries[count].ing_dipv6_0 = entry->ipv6_6rd.ipv6_dip0;
				opt->entries[count].ing_dipv6_1 = entry->ipv6_6rd.ipv6_dip1;
				opt->entries[count].ing_dipv6_2 = entry->ipv6_6rd.ipv6_dip2;
				opt->entries[count].ing_dipv6_3 = entry->ipv6_6rd.ipv6_dip3;
				opt->entries[count].ing_sp = entry->ipv6_6rd.sport;
				opt->entries[count].ing_dp = entry->ipv6_6rd.dport;
				opt->entries[count].ipv6_flowlabel = IS_IPV6_FLAB_EBL();

				opt->entries[count].eg_sipv4 = entry->ipv6_6rd.tunnel_sipv4;
				opt->entries[count].eg_dipv4 = entry->ipv6_6rd.tunnel_dipv4;
				count++;
#endif
#endif
			}
		}
	}

	opt->num_of_entries = count;

	if (opt->num_of_entries > 0) {
		return HWNAT_SUCCESS;
	} else {
		return HWNAT_ENTRY_NOT_FOUND;
	}
}

int FoeBindEntry(struct hwnat_args *opt)
{
	struct FoeEntry *entry;

	entry = &PpeFoeBase[opt->entry_num];

	//restore right information block1
	entry->bfib1.time_stamp = RegRead(FOE_TS) & 0xFFFF;
	entry->bfib1.state = BIND;

	return HWNAT_SUCCESS;
}

int FoeUnBindEntry(struct hwnat_args *opt)
{

	struct FoeEntry *entry;

	entry = &PpeFoeBase[opt->entry_num];

	entry->ipv4_hnapt.udib1.state = UNBIND;
	entry->ipv4_hnapt.udib1.time_stamp = RegRead(FOE_TS) & 0xFF;

	return HWNAT_SUCCESS;
}

int FoeDelEntryByNum(uint32_t entry_num)
{
	struct FoeEntry *entry;

	entry = &PpeFoeBase[entry_num];
	memset(entry, 0, sizeof(struct FoeEntry));
#ifdef TCSUPPORT_RA_HWNAT

	//need to clear cache(now we use flush cashe table way)
	// cache disable
	RegModifyBits(CAH_CTRL, 0, 0, 1);
	
	//clear cache table before enabling cache
	RegModifyBits(CAH_CTRL, 1, 9, 1);
	RegModifyBits(CAH_CTRL, 0, 9, 1);
	
	// cache enable
	RegModifyBits(CAH_CTRL, 1, 0, 1);
#endif
	return HWNAT_SUCCESS;
}

void FoeTblClean(void)
{
	uint32_t FoeTblSize;

	FoeTblSize = FOE_4TB_SIZ * sizeof(struct FoeEntry);
	memset(PpeFoeBase, 0, FoeTblSize);

}

#ifdef TCSUPPORT_RA_HWNAT

int32_t FoeHashFun(struct FoePriKey *key,enum FoeEntryState TargetState)
{

	uint32_t index=0;
	uint32_t buf[3];
	uint32_t h_sd,h_32,h_16;
	struct FoeEntry *entry;

	if (DebugLevel >= 2) 
		printk("FoeHashFun pkt type %d\n",key->pkt_type);

	memset(buf, 0, sizeof(buf));
	switch (key->pkt_type){
		case IPV4_HNAT:
			buf[0] = 0x5a5a5a00 | (key->ipv4_hnapt.prot & 0xff);
			buf[1] = key->ipv4_hnapt.dip;
			buf[2] = key->ipv4_hnapt.sip;
			break;
		case IPV4_HNAPT:
		case IPV4_DSLITE:	
			buf[0] = key->ipv4_hnapt.sport<<16 | key->ipv4_hnapt.dport;
			buf[1] = key->ipv4_hnapt.dip;
			buf[2] = key->ipv4_hnapt.sip;
			break;
		case IPV6_3T_ROUTE:
			buf[0] = (0x5a5a5a00 | (key->ipv6_routing.nexth & 0xff))^(key->ipv6_routing.dip0)^(key->ipv6_routing.sip0);
			buf[1] = (key->ipv6_routing.dip3)^(key->ipv6_routing.dip1)^(key->ipv6_routing.sip1);
			buf[2] = (key->ipv6_routing.sip3)^(key->ipv6_routing.dip2)^(key->ipv6_routing.sip2);
			break;	
		case IPV6_5T_ROUTE:
		case IPV6_6RD:	
			buf[0] = (key->ipv6_routing.sport<<16 | key->ipv6_routing.dport)^(key->ipv6_routing.dip0)^(key->ipv6_routing.sip0);
			buf[1] = (key->ipv6_routing.dip3)^(key->ipv6_routing.dip1)^(key->ipv6_routing.sip1);
			buf[2] = (key->ipv6_routing.sip3)^(key->ipv6_routing.dip2)^(key->ipv6_routing.sip2);
			break;	
		case L2_BRIDGE:
			buf[0] = (key->l2_bridge.in_smac[2] << 24)^(key->l2_bridge.in_smac[3] << 16)^((key->l2_bridge.in_smac[4] << 8) | (key->l2_bridge.in_smac[5]));
			buf[1] = (key->l2_bridge.in_dmac[4] << 24)^(key->l2_bridge.in_dmac[5] << 16)^((key->l2_bridge.in_smac[0] << 8) | (key->l2_bridge.in_smac[1]));
			buf[2] = (key->l2_bridge.in_dmac[0] << 24)^(key->l2_bridge.in_dmac[1] << 16)^((key->l2_bridge.in_dmac[2] << 8) | (key->l2_bridge.in_dmac[3]));
			break;
		default :
			NAT_PRINT("Wrong MFT value\n");
			return -1;

	}
	
#if defined(CONFIG_RA_HW_NAT_HASH0)	
	h_sd = RegRead(PPE_HASH_SEED);
#elif defined(CONFIG_RA_HW_NAT_HASH1)
	h_sd = (buf[0] & buf[1]) | (~buf[0] & buf[2]);
#elif defined(CONFIG_RA_HW_NAT_HASH2)
	h_sd = buf[0] ^ (buf[1] & ~buf[2]);
#elif defined(CONFIG_RA_HW_NAT_HASH3)
	//need crc32
#endif
	
	h_32 = buf[0] ^ buf[1] ^ buf[2] ^ ((h_sd<<8) | (h_sd>>24));
	h_16 = ((h_32>>16) ^ (h_32 & 0xffff));

	switch(FOE_4TB_SIZ)
	{
		case 2048:
			index =  (h_16 & (0x3ff))*FOE_HASH_WAY;
			break;
		case 4096:
			index =  (h_16 & (0x7ff))*FOE_HASH_WAY;
			break;
		case 8192:
			index =  (h_16 & (0xfff))*FOE_HASH_WAY;
			break;
		case 16384:
			index =  (h_16 & (0x1fff))*FOE_HASH_WAY;
			break;
		case 1024:	
		default:
			index =  (h_16 & (0x1ff))*FOE_HASH_WAY;
			break;
	}

	if (DebugLevel >= 2) {
		//printk("1 %x %x %x\n",(key->ipv6_routing.sport<<16 | key->ipv6_routing.dport),(key->ipv6_routing.dip0),(key->ipv6_routing.sip0));
		//printk("2 %x %x %x\n",(key->ipv6_routing.dip3),(key->ipv6_routing.dip1),(key->ipv6_routing.sip1));
		//printk("3 %x %x %x\n",(key->ipv6_routing.sip3),(key->ipv6_routing.dip2),(key->ipv6_routing.sip2));
		printk("buf  0:%x 1:%x 2:%x\n",buf[0],buf[1],buf[2]);
		printk("h_sd %x h_32 %x h_16 %x\n",h_sd,h_32,h_16);
		printk("index  %x\n",index);
	}
	

	do {
		entry = &PpeFoeBase[index];

		//case1.want to search binding entry
		//case2.want to insert duplicate entry
		//printk("TargetState %d %d\n",TargetState,entry->bfib1.state);	
		if(TargetState==BIND || 
		  (TargetState==INVALID && entry->bfib1.state == BIND)) {
			switch (key->pkt_type){
				case IPV4_HNAT:
					if((entry->bfib1.state == BIND) && 
						(entry->ipv4_hnapt.sip == key->ipv4_hnapt.sip) &&  
						(entry->ipv4_hnapt.dip == key->ipv4_hnapt.dip) &&
						((entry->ipv4_hnapt.dport & 0xff) == (key->ipv4_hnapt.prot & 0xff)))
					{
						return index;
					}
					break;
				case IPV4_HNAPT:
				case IPV4_DSLITE:	
				
					if((entry->bfib1.state == BIND) && 
						(entry->ipv4_hnapt.sip == key->ipv4_hnapt.sip) && 
						(entry->ipv4_hnapt.dip == key->ipv4_hnapt.dip) &&
						(entry->ipv4_hnapt.dport == key->ipv4_hnapt.dport) &&
						(entry->ipv4_hnapt.sport == key->ipv4_hnapt.sport) &&
						(entry->bfib1.udp == (key->ipv4_hnapt.is_udp & 0x1)))
					{
							return index;
					}
					break;
				case IPV6_3T_ROUTE:
					if((entry->bfib1.state == BIND) && 
						(entry->ipv6_3t_route.ipv6_sip0 == key->ipv6_routing.sip3) &&
						(entry->ipv6_3t_route.ipv6_sip1 == key->ipv6_routing.sip2) &&
						(entry->ipv6_3t_route.ipv6_sip2 == key->ipv6_routing.sip1) &&
						(entry->ipv6_3t_route.ipv6_sip3 == key->ipv6_routing.sip0) &&
						(entry->ipv6_3t_route.ipv6_dip0 == key->ipv6_routing.dip3) &&
						(entry->ipv6_3t_route.ipv6_dip1 == key->ipv6_routing.dip2) &&
						(entry->ipv6_3t_route.ipv6_dip2 == key->ipv6_routing.dip1) &&
						(entry->ipv6_3t_route.ipv6_dip3 == key->ipv6_routing.dip0) &&
						(entry->ipv6_3t_route.prot == (key->ipv6_routing.nexth & 0xff)))
					{
						return index;
					}
					break;
				case IPV6_5T_ROUTE:
				case IPV6_6RD:	
					if((entry->bfib1.state == BIND) && 
						(entry->ipv6_5t_route.ipv6_sip0 == key->ipv6_routing.sip3) &&
						(entry->ipv6_5t_route.ipv6_sip1 == key->ipv6_routing.sip2) &&
						(entry->ipv6_5t_route.ipv6_sip2 == key->ipv6_routing.sip1) &&
						(entry->ipv6_5t_route.ipv6_sip3 == key->ipv6_routing.sip0) &&
						(entry->ipv6_5t_route.ipv6_dip0 == key->ipv6_routing.dip3) &&
						(entry->ipv6_5t_route.ipv6_dip1 == key->ipv6_routing.dip2) &&
						(entry->ipv6_5t_route.ipv6_dip2 == key->ipv6_routing.dip1) &&
						(entry->ipv6_5t_route.ipv6_dip3 == key->ipv6_routing.dip0) &&
						(entry->ipv6_5t_route.sport == key->ipv6_routing.sport) && 
						(entry->ipv6_5t_route.dport == key->ipv6_routing.dport) &&
						(entry->bfib1.udp == (key->ipv6_routing.is_udp & 0x1)))
					{
						return index;
					}
					break;	
				case L2_BRIDGE:
					if((entry->bfib1.state == BIND) && 
						(entry->l2_bridge.in_dmac_hi[3] == key->l2_bridge.in_dmac[5]) &&
						(entry->l2_bridge.in_dmac_hi[2] == key->l2_bridge.in_dmac[4]) &&
						(entry->l2_bridge.in_dmac_hi[1] == key->l2_bridge.in_dmac[3]) &&
						(entry->l2_bridge.in_dmac_hi[0] == key->l2_bridge.in_dmac[2]) &&
						(entry->l2_bridge.in_dmac_lo[1] == key->l2_bridge.in_dmac[1]) &&
						(entry->l2_bridge.in_dmac_hi[0] == key->l2_bridge.in_dmac[0]) &&
						(entry->l2_bridge.in_smac_hi[1] == key->l2_bridge.in_dmac[5]) &&
						(entry->l2_bridge.in_smac_hi[0] == key->l2_bridge.in_dmac[4]) &&
						(entry->l2_bridge.in_smac_hi[3] == key->l2_bridge.in_dmac[3]) &&
						(entry->l2_bridge.in_smac_hi[2] == key->l2_bridge.in_dmac[2]) &&
						(entry->l2_bridge.in_smac_hi[1] == key->l2_bridge.in_dmac[1]) &&
						(entry->l2_bridge.in_smac_hi[0] == key->l2_bridge.in_dmac[0]))
					{
						return index;
					}
					break;
				default :
					NAT_PRINT("Wrong MFT value\n");
					break;

	}			
		}else if(TargetState==INVALID) 
		{
		    // empty entry found
		    if((entry->bfib1.state == INVALID) || (entry->bfib1.state == UNBIND))
		    {
				return index;
		    }
		} 		
	} while( ((index++)%FOE_HASH_WAY) ==0 );

	//entry not found

	return -1;

}
#endif

