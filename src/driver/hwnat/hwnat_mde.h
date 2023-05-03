/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_mde.h#1 $
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
** $Log: hwnat_mde.h,v $
** Revision 1.1.2.1  2011/04/01 09:16:42  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_MDE_H
#define __HWNAT_MDE_H

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern void hwnat_mde_init(void);
extern void hwnat_mde_exit(void);
extern int do_hwnat_const(int argc, char *argv[], void *p);
extern int do_hwnat_mod_macro(int argc, char *argv[], void *p);
extern int do_hwnat_mod_vec(int argc, char *argv[], void *p);
extern int do_hwnat_mod_op(int argc, char *argv[], void *p);

#endif /* __HWNAT_MDE_H */

