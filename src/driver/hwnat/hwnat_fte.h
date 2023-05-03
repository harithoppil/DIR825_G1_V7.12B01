/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_fte.h#1 $
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
** $Log: hwnat_fte.h,v $
** Revision 1.4  2011/06/30 12:07:32  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.3  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:44  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:41  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_FTE_H
#define __HWNAT_FTE_H

#define FLOW_TUPLE_LEN			20			
#define FLOW_ARG_LEN			16

#define FLOW_MAX				256

#define FLOW_DEF_TIME			15

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern void hwnat_fte_init(void);
extern void hwnat_fte_exit(void);
extern int hwnat_flow_add(Pktflow_t *cp);
extern void hwnat_flow_remove(uint16 entry);
extern void hwnat_flow_remove_by_fdb(struct net_bridge_fdb_entry *fdb);
extern void hwnat_flow_remove_by_oldest(void);
extern void hwnat_flow_update_ipv4_mc(__be32 group, unsigned int portmap);
extern void hwnat_flow_update_ip6_mc(struct in6_addr group, unsigned int portmap);

extern int do_hwnat_hash(int argc, char *argv[], void *p);
extern int do_hwnat_flow(int argc, char *argv[], void *p);

#endif /* __HWNAT_FTE_H */

