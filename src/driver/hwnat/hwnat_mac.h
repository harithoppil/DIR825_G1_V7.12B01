/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_mac.h#1 $
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
** $Log: hwnat_mac.h,v $
** Revision 1.3  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:45  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:42  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_MAC_H
#define __HWNAT_MAC_H

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern void hwnat_mac_addr_init(void);
extern void hwnat_mac_addr_exit(void);
extern uint8 *hwnat_mac_addr_get(uint8 entry);
extern int hwnat_mac_addr_insert(uint8 *addr);
extern int hwnat_mac_addr_remove(uint8 entry);
extern int hwnat_mac_addr_in_ether(char *bufp, char *ptr);
extern int do_hwnat_mac_addr(int argc, char *argv[], void *p);

#endif /* __HWNAT_MAC_H */


