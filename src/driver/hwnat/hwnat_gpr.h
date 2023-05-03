/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_gpr.h#1 $
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
** $Log: hwnat_gpr.h,v $
** Revision 1.1.2.1  2011/04/01 09:16:41  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_GPR_H
#define __HWNAT_GPR_H

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern void hwnat_gpr_init(void);
extern void hwnat_gpr_exit(void);
extern int do_hwnat_gpr(int argc, char *argv[], void *p);

#endif /* __HWNAT_GPR_H */

