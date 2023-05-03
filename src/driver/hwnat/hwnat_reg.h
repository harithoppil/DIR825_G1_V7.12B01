/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_reg.h#1 $
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
** $Log: hwnat_reg.h,v $
** Revision 1.1.2.1  2011/04/01 09:16:43  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_REG_H
#define __HWNAT_REG_H

/**************************
 * HWNAT table ID         *
 **************************/

#define INTERFACE_TBL_ID			(0)
#define GPR_TBL_ID					(1)
#define COMPARATOR_TBL_ID			(2)
#define POLICY_TBL_ID				(3)
#define POLICY_RESULT_TBL_ID		(4)
#define TUPLE_TBL_ID				(5)
#define HASH_TBL_ID					(14)
#define FLOW_TBL_ID					(15)
#define MAC_ADDR_TBL_ID				(16)
#define CONST_TBL_ID				(6)
#define MOD_MACRO_TBL_ID			(7)
#define MOD_VEC_TBL_ID				(8)
#define MOD_OP_TBL_ID				(9)
#define QOS0_TBL_ID					(10)
#define QOS1_TBL_ID					(11)
#define QOS2_TBL_ID					(12)
#define QOS3_TBL_ID					(13)

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/
extern uint8 hwnat_write_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
extern uint8 hwnat_read_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);

extern void hwnat_write_reg(uint32 reg, uint32 val);
extern uint32 hwnat_read_reg(uint32 reg);

extern int do_hwnat_reg(int argc, char *argv[], void *p);

#endif /* __HWNAT_REG_H */

