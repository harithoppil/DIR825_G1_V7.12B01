/*
** $Id: //BBN_Linux/Branch/Branch_for_MT7520_20120528/tclinux_phoenix/modules/private/raeth/femac.h#6 $
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
unsigned int fe_reg_read(unsigned int reg_offset);
void fe_reg_write(unsigned int reg_offset, unsigned int value);
void fe_reg_modify_bits(unsigned int reg_offset, unsigned int Data, unsigned int Offset, unsigned int Len);


