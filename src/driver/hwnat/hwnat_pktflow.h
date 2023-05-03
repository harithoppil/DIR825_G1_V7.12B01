/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_pktflow.h#1 $
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
** $Log: hwnat_pktflow.h,v $
** Revision 1.2  2011/06/03 02:41:46  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:43  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_PKTFLOW_H
#define __HWNAT_PKTFLOW_H

typedef enum {
        DIR_RX,           /* Receive path */
        DIR_TX,           /* Transmit path */
        DIR_MAX
} PktflowDir_t;

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern int hwnat_pktflow_init(void);
extern void hwnat_pktflow_exit(void);

extern char *pktflow_l2_get(PktflowHeader_t * bHdr_p, PktflowDir_t dir, PktflowEncap_t encap, int offset);

extern int do_hwnat_pktflow(int argc, char *argv[], void *p);

#endif /* __HWNAT_PKTFLOW_H */
