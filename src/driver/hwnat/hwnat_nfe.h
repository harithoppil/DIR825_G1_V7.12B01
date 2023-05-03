/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_nfe.h#1 $
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
** $Log: hwnat_nfe.h,v $
** Revision 1.2  2011/06/03 02:41:45  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:43  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_NFE_H
#define __HWNAT_NFE_H

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern void hwnat_nfe_init(void);
extern void hwnat_nfe_exit(void);
extern void hwnat_nfe_get_stats(struct pktflow_stats *stats, int port);
extern void hwnat_nfe_clear_stats(int port);
extern void hwnat_nfe_qos_set(uint8 port, uint8 policy, uint16 w0, uint16 w1, uint16 w2, uint16 w3);
extern void hwnat_nfe_qos_get(uint8 port, uint8 *policy, uint16 *w0, uint16 *w1, uint16 *w2, uint16 *w3);
extern int do_hwnat_nfe(int argc, char *argv[], void *p);

#endif /* __HWNAT_NFE_H */
