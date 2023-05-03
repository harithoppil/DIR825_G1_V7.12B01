/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8822B_SUPPORT == 1)  

/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

BOOLEAN
phydm_rfe_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					channel
	)
{
	/* Efuse is not wrote now */
	/* Need to check RFE type finally */
	/*if (pDM_Odm->RFEType == 1) {*/
		if (channel <= 14) {
			/* signal source */
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x704570);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x704570);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x45);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x45);
		} else if (channel > 35) {
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x174517);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x174517);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x45);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x45);
		} else
			return FALSE;
	
		/* chip top mux */
		ODM_SetBBReg(pDM_Odm, 0x64, BIT29|BIT28, 0x3);
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT26|BIT25, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x40, BIT2, 0x1);

		/* from s0 or s1 */
		ODM_SetBBReg(pDM_Odm, 0x1990, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x30);
		ODM_SetBBReg(pDM_Odm, 0x1990, (BIT11|BIT10), 0x3);

		/* input or output */
		ODM_SetBBReg(pDM_Odm, 0x974, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x3f);
		ODM_SetBBReg(pDM_Odm, 0x974, (BIT11|BIT10), 0x3);

		/* delay 400ns for PAPE */
		ODM_SetBBReg(pDM_Odm, 0x810, bMaskByte3|BIT20|BIT21|BIT22|BIT23, 0x211);

		/* antenna switch table */
		ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa555);
		ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa555);

		/* inverse or not */
		ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x0);
	/*}*/

	return TRUE;
}

VOID
phydm_CcaParByBw_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		reg82c;
	
	reg82c = ODM_GetBBReg(pDM_Odm, 0x82c, bMaskDWord);
	
	if (bandwidth == ODM_BW20M) {
		/* 82c[15:12] = 4 */
		/* 82c[27:24] = 6 */
		
		reg82c &= (~(0x0f00f000));
		reg82c |= ((0x4) << 12);
		reg82c |= ((0x6) << 24);
	} else if (bandwidth == ODM_BW40M) {
		/* 82c[19:16] = 9 */
		/* 82c[27:24] = 6 */
	
		reg82c &= (~(0x0f0f0000));
		reg82c |= ((0x9) << 16);
		reg82c |= ((0x6) << 24);
	} else if (bandwidth == ODM_BW80M) {
		/* 82c[15:12] 7 */
		/* 82c[19:16] b */
		/* 82c[23:20] d */
		/* 82c[27:24] 3 */
	
		reg82c &= (~(0x0ffff000));
		reg82c |= ((0xdb7) << 12);
		reg82c |= ((0x3) << 24);
	}

	ODM_SetBBReg(pDM_Odm, 0x82c, bMaskDWord, reg82c);
}

VOID
phydm_CcaParByRxPath_8822b(
	IN	PDM_ODM_T				pDM_Odm
	)
{
	if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B)) {
		/* Preamble parameters for ch-D */
		/*ODM_SetBBReg(pDM_Odm, 0x834, 0x70000, 0x4);*/

		/* 838[7:4] = 8 */
		/* 838[11:8] = 7 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 7 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x777678);
	} else {
		/* Preamble parameters for ch-D */
		/*ODM_SetBBReg(pDM_Odm, 0x834, 0x70000, 0x6);*/

		/* 838[7:4] = 3 */
		/* 838[11:8] = 3 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 6 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x776633);
	}

}

VOID
phydm_RxDfirParByBw_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	if (bandwidth == ODM_BW40M) {
		/* RX DFIR for BW40 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x0);
	} else if (bandwidth == ODM_BW80M) {
		/* RX DFIR for BW80 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x0);
	} else {
		/* RX DFIR for BW20, BW10 and BW5*/
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x1);
	}
}

BOOLEAN
phydm_write_txagc_1byte_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc), i;
	u1Byte	rate_offset = (HwRate & 0x3);
	u4Byte	rate_mask = (0xff << (rate_offset << 3));
	u4Byte	txagc_content = 0x0;

	/* For debug command only!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_write_txagc_1byte_8822b(): unsupported path (%d)\n", Path));
		return FALSE;
	}

#if 1
	/* For HW limitation, We can't write TXAGC once a byte. */
	for (i = 0; i < 4; i++) {
		if (i != rate_offset)
			txagc_content = txagc_content|(config_phydm_read_txagc_8822b(pDM_Odm, Path, rate_idx + i) << (i << 3));
		else
			txagc_content = txagc_content|((PowerIndex & 0x3f) << (i << 3));
	}
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, txagc_content);
#else
	ODM_Write1Byte(pDM_Odm, (offset_txagc[Path] + HwRate), (PowerIndex & 0x3f));
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_write_txagc_1byte_8822b(): Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

/* ======================================================================== */

/* ======================================================================== */
/* These following functions can be used by driver*/

u4Byte
config_phydm_read_rf_reg_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask
	)
{
	u4Byte	Readback_Value, Direct_Addr;
	u4Byte	offset_readRF[2] = {0x2800, 0x2c00};
	u4Byte	power_RF[2] = {0x1c, 0xec};

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): unsupported path (%d)\n", RFPath));
		return INVALID_RF_DATA;
	}

	/*  Error handling. Check if RF power is enable or not */
	/*  0xffffffff means RF power is disable */
	if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): Read fail, RF is disabled\n"));
		return INVALID_RF_DATA;
	}

	/* Calculate offset */
	RegAddr &= 0xff;
	Direct_Addr = offset_readRF[RFPath] + (RegAddr << 2);

	/* RF register only has 20bits */
	BitMask &= bRFRegOffsetMask;

	/* Read RF register directly */
	Readback_Value = ODM_GetBBReg(pDM_Odm, Direct_Addr, BitMask);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): RF-%d 0x%x = 0x%x, bit mask = 0x%x\n", 
		RFPath, RegAddr, Readback_Value, BitMask));
	return Readback_Value;
}

BOOLEAN
config_phydm_write_rf_reg_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask,
	IN	u4Byte					Data
	)
{
	u4Byte	DataAndAddr = 0, Data_original = 0;
	u4Byte	offset_writeRF[2] = {0xc90, 0xe90};
	u4Byte	power_RF[2] = {0x1c, 0xec};
	u1Byte	BitShift;

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): unsupported path (%d)\n", RFPath));
		return FALSE;
	}

	/* Read RF register content first */
	RegAddr &= 0xff;
	BitMask = BitMask & bRFRegOffsetMask;

	if (BitMask != bRFRegOffsetMask) {
		Data_original = config_phydm_read_rf_reg_8822b(pDM_Odm, RFPath, RegAddr, bRFRegOffsetMask);

		/* Error handling. RF is disabled */
		if (config_phydm_read_rf_check_8822b(Data_original) == FALSE) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): Write fail, RF is disable\n"));
			return FALSE;
		}

		/* check bit mask */
		if (BitMask != 0xfffff) {
			for (BitShift = 0; BitShift <= 19; BitShift++) {
				if (((BitMask >> BitShift) & 0x1) == 1)
					break;
			}
			Data = ((Data_original) & (~BitMask)) | (Data << BitShift);
		}
	} else if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): Write fail, RF is disabled\n"));
		return FALSE;
	}

	/* Put write addr in [27:20]  and write data in [19:00] */
	DataAndAddr = ((RegAddr<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	/* Write Operation */
	ODM_SetBBReg(pDM_Odm, offset_writeRF[RFPath], bMaskDWord, DataAndAddr);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): RF-%d 0x%x = 0x%x (original: 0x%x), bit mask = 0x%x\n", 
		RFPath, RegAddr, Data, Data_original, BitMask));
	return TRUE;
}

BOOLEAN
config_phydm_write_txagc_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc), i;
	u4Byte	txagc_content = 0x0;

	/* Input need to be HW rate index, not driver rate index!!!! */

        if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): unsupported path (%d)\n", Path));
		return FALSE;
	}

	/* driver need to construct a 4-byte power index */
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, PowerIndex);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

u1Byte
config_phydm_read_txagc_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		Path,
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;

	/* Input need to be HW rate index, not driver rate index!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8822b(): unsupported path (%d)\n", Path));
		return INVALID_TXAGC_DATA;
	}

	/* Disable TX AGC report */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x0);							/* need to check */

	/* Set data rate index (bit0~6) and path index (bit7) */
	ODM_SetBBReg(pDM_Odm, 0x1998, bMaskByte0, (HwRate|(Path << 7)));

	/* Enable TXAGC report */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x1);

	/* Read TX AGC report */
	readBack_data = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xd30, 0x7f0000);

	/* Driver have to disable TXAGC report after reading TXAGC (ref. user guide v11) */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x0);	
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8822b(): Path-%d rate index 0x%x = 0x%x\n", Path, HwRate, readBack_data));
	return readBack_data;
}

BOOLEAN
config_phydm_switch_band_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b()======================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);

	if (central_ch <= 14) {
		/* 2.4G */

		/* Enable CCK block */
		ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0x1);

		/* Disable MAC CCK check */
		ODM_SetBBReg(pDM_Odm, 0x454, BIT7, 0x0);

		/* Disable BB CCK check */
		ODM_SetBBReg(pDM_Odm, 0xa80, BIT18, 0x0);

		/*CCA Mask*/
		ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 15); /*default value*/

		/* RF band */
		rf_reg18 = (rf_reg18 & (~(BIT16|BIT9|BIT8)));
	} else if (central_ch > 35) {
		/* 5G */

		/* Enable BB CCK check */
		ODM_SetBBReg(pDM_Odm, 0xa80, BIT18, 0x1);
		
		/* Enable CCK check */
		ODM_SetBBReg(pDM_Odm, 0x454, BIT7, 0x1);

		/* Disable CCK block */
		ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0x0);

		/*CCA Mask*/
		ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 34); /*CCA mask = 13.6us*/

		/* RF band */
		rf_reg18 = (rf_reg18 & (~(BIT16|BIT9|BIT8)));
		rf_reg18 = (rf_reg18|BIT8|BIT16);
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Fail to switch band (ch: %d)\n", central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);

	if (phydm_rfe_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Fail to switch band (ch: %d), because writing RF register is fail\n", central_ch));	
		return FALSE;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Success to switch band (ch: %d)\n", central_ch));	
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		rf_reg18, rf_regB8;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b()====================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_regB8 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_regB8);

	/* Switch band and channel */
	if (central_ch <= 14) {
		/* 2.4G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18|central_ch);

		/* 2. AGC table selection */
		ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x0);
		pDM_DigTable->agcTableIdx = 0x0;

		/* 3. Set central frequency for clock offset tracking */
		ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x96a);

		/* Fix LCK fail issue @ 5285MHz~5375MHz, 0xb8[19]=0x0 */
		rf_regB8 = rf_regB8 | BIT19;

	} else if (central_ch > 35) {
		/* 5G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18 | central_ch);

		if (central_ch >= 36 && central_ch <= 64)
			;
		else if ((central_ch >= 100) && (central_ch <= 140))
			rf_reg18 = (rf_reg18 | BIT17);
		else if (central_ch > 140)
			rf_reg18 = (rf_reg18 | BIT18);
		else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (RF18) (ch: %d)\n", central_ch));
			return FALSE;
		 }

		/* 2. AGC table selection */
		if ((central_ch >= 36) && (central_ch <= 64)) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x1);
			pDM_DigTable->agcTableIdx = 0x1;
		} else if ((central_ch >= 100) && (central_ch <= 144)) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x2);
			pDM_DigTable->agcTableIdx = 0x2;
		} else if (central_ch >= 149) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x3);
			pDM_DigTable->agcTableIdx = 0x3;
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (AGC) (ch: %d)\n", central_ch));
			return FALSE;
		}

		/* 3. Set central frequency for clock offset tracking */
		if ((central_ch >= 36) && (central_ch <= 48))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x494);
		else if ((central_ch >= 52) && (central_ch <= 64))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x453);
		else if ((central_ch >= 100) && (central_ch <= 116))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x452);
		else if ((central_ch >= 118) && (central_ch <= 177))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x412);
		else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (fc_area) (ch: %d)\n", central_ch));
			return FALSE;
		}

		/* Fix LCK fail issue @ 5285MHz~5375MHz, 0xb8[19]=0x0 */
		if ((central_ch >= 57) && (central_ch <= 75))
			rf_regB8 = rf_regB8 & (~BIT19);
		else
			rf_regB8 = rf_regB8 | BIT19;

	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch band (ch: %d)\n", central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask, rf_regB8);

	if (pDM_Odm->RFType > ODM_1T1R) {
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0xb8, bRFRegOffsetMask, rf_regB8);
	}

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (ch: %d), because writing RF register is fail\n", central_ch));
		return FALSE;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Success to switch channel (ch: %d)\n", central_ch));
	return TRUE;
}

BOOLEAN
config_phydm_switch_bandwidth_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b()===================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((bandwidth >= ODM_BW_MAX) || ((bandwidth == ODM_BW40M) && (primary_ch_idx > 2)) || ((bandwidth == ODM_BW80M) && (primary_ch_idx > 4))) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);

	/* Switch bandwidth */
	switch (bandwidth) {
	case ODM_BW20M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, ODM_BW20M);

		/* ADC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 | BIT11 | BIT10);

		break;
	}
	case ODM_BW40M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = sub-channel, rf mode([1:0]) = 40M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (((primary_ch_idx & 0xf) << 2)|ODM_BW40M));

		/* CCK primary channel */
		if (primary_ch_idx == 1)
			ODM_SetBBReg(pDM_Odm, 0xa00, BIT4, primary_ch_idx);
		else
			ODM_SetBBReg(pDM_Odm, 0xa00, BIT4, 0);

		/* ADC clock = 160M clock for BW40 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT11|BIT10), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT17, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT23|BIT22), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT29, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT11);		

		break;
	}
	case ODM_BW80M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = sub-channel, rf mode([1:0]) = 80M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (((primary_ch_idx & 0xf) << 2)|ODM_BW80M));

		/* ADC clock = 160M clock for BW80 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT13|BIT12), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT18, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT25|BIT24), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT30, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT10);

		break;
	}
	case ODM_BW5M:
	{
		/* Small BW([7:6]) = 1, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (BIT6|ODM_BW20M));

		/* ADC clock = 40M clock */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x2);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x0);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x2);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x0);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8c8, BIT31, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	case ODM_BW10M:
	{
		/* Small BW([7:6]) = 1, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (BIT7|ODM_BW20M));

		/* ADC clock = 80M clock */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x3);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x0);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x3);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x0);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8c8, BIT31, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	}

	/* Write RF register */
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d), because writing RF register is fail\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	/* Modify RX DFIR parameters */
	phydm_RxDfirParByBw_8822b(pDM_Odm, bandwidth);

	/* Modify CCA parameters */
	phydm_CcaParByBw_8822b(pDM_Odm, bandwidth);

	/* Toggle RX path to avoid RX dead zone issue */
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, 0x0);
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, (pDM_Odm->RXAntStatus|(pDM_Odm->RXAntStatus<<4)));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Success to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_bw_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u1Byte			eRFPath = 0;
	u4Byte			RFValToWR , RFTmpVal, BitShift, BitMask;

	/* Switch band */
	if (config_phydm_switch_band_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch channel */
	if (config_phydm_switch_channel_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch bandwidth */
	if (config_phydm_switch_bandwidth_8822b(pDM_Odm, primary_ch_idx, bandwidth) == FALSE)
		return FALSE;

	return TRUE;
}

BOOLEAN
config_phydm_trx_mode_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	)
{
	BOOLEAN		rf_reg_status = TRUE;
	u1Byte		IGI;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b()=====================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	if ((TxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Wrong TX setting (TX: 0x%x)\n", TxPath));
		return FALSE;
	}

	if ((RxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Wrong RX setting (RX: 0x%x)\n", RxPath));
		return FALSE;
	}

	/* RF mode of path-A and path-B */
	/* Cannot shut down path-A, beacause synthesizer will be shut down when path-A is in shut down mode */
	if ((TxPath|RxPath) & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, 0xc08, bMaskLWord, 0x3231);
	else
		ODM_SetBBReg(pDM_Odm, 0xc08, bMaskLWord, 0x1111);

	if ((TxPath|RxPath) & ODM_RF_B)
		ODM_SetBBReg(pDM_Odm, 0xe08, bMaskLWord, 0x3231);
	else
		ODM_SetBBReg(pDM_Odm, 0xe08, bMaskLWord, 0x0);

	/* Set TX antenna by Nsts */
	ODM_SetBBReg(pDM_Odm, 0x93c, (BIT19|BIT18), 0x3);
	ODM_SetBBReg(pDM_Odm, 0x80c, (BIT29|BIT28), 0x1);

	/* Control CCK TX path by 0xa07[7] */
	ODM_SetBBReg(pDM_Odm, 0x80c, BIT30, 0x1);

	/* TX logic map and TX path en for Nsts = 1, and CCK TX path*/	
	if (TxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x001);
		ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0x8);
	} else if (TxPath & ODM_RF_B) {
		ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x002);
		ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0x4);
	}
		
	/* TX logic map and TX path en for Nsts = 2*/
	ODM_SetBBReg(pDM_Odm, 0x940, 0xfff0, 0x43);

	/* TX path enable */
	ODM_SetBBReg(pDM_Odm, 0x80c, bMaskByte0, ((TxPath << 4)|TxPath));

	/* CCK RX 1st and 2nd path setting*/	
	if (RxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, 0xa04, 0x0f000000, 0x1);
	else if (RxPath & ODM_RF_B)
		ODM_SetBBReg(pDM_Odm, 0xa04, 0x0f000000, 0x4);

	/* RX path enable */
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, ((RxPath << 4)|RxPath));

	if ((RxPath == ODM_RF_A) || (RxPath == ODM_RF_B)) {
		/* 1R */

		/* Disable MRC for CCA */
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT22, 0x0);

		/* Disable MRC for barker */
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT18, 0x0);
		
		/* Disable CCK antenna diversity */
		/* ODM_SetBBReg(pDM_Odm, 0xa00, BIT15, 0x0); */

		/* Disable Antenna weighting */
		ODM_SetBBReg(pDM_Odm, 0x1904, BIT16, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x800, BIT28, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x850, BIT23, 0x0);
	} else {
		/* 2R */

		/* Enable MRC for CCA */
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT22, 0x1);

		/* Enable MRC for barker */
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT18, 0x1);

		/* Disable CCK antenna diversity */
		ODM_SetBBReg(pDM_Odm, 0xa00, BIT15, 0x0);

		/* Enable Antenna weighting */
		ODM_SetBBReg(pDM_Odm, 0x1904, BIT16, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x800, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x850, BIT23, 0x1);
	}

	/* 2T */
	if (!((TxPath == ODM_RF_A) || (TxPath == ODM_RF_B))) {
		if (bTx2Path || pDM_Odm->mp_mode) {
			/* 2Tx for OFDM */
			ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x043);

			/* 2Tx for CCK */
			ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0xc);
		}
	}

	/* Update TXRX antenna status for PHYDM */
	pDM_Odm->TXAntStatus =  (TxPath & 0x3);
	pDM_Odm->RXAntStatus =  (RxPath & 0x3);

	/* MP driver need to support path-B TX\RX */
	if (pDM_Odm->mp_mode) {
		/*	0xef 0x80000  0x33 0x00001  0x3e 0x00034  0x3f 0x4080e  0xef 0x00000    suggested by Lucas*/
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x80000);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask, 0x00001);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3e, bRFRegOffsetMask, 0x00034);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3f, bRFRegOffsetMask, 0x4080e);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x00000);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): MP mode!! support path-B TX and RX\n"));
	} else {
		/*	0xef 0x80000  0x33 0x00001  0x3e 0x00034  0x3f 0x4080c  0xef 0x00000 */
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x80000);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask, 0x00001);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3e, bRFRegOffsetMask, 0x00034);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3f, bRFRegOffsetMask, 0x4080c);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x00000);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Normal mode!! Do not support path-B TX and RX\n"));
	}

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Fail to set TRx mode setting (TX: 0x%x, RX: 0x%x), because writing RF register is fail\n", TxPath, RxPath));
		return FALSE;
	}

	/* Toggle IGI to let RF enter RX mode, because BB doesn't send 3-wire command when RX path is enable */
	IGI = (u1Byte)ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	ODM_Write_DIG(pDM_Odm, IGI - 2);
	ODM_Write_DIG(pDM_Odm, IGI);

	/* Modify CCA parameters */
	phydm_CcaParByRxPath_8822b(pDM_Odm);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Success to set TRx mode setting (TX: 0x%x, RX: 0x%x)\n", TxPath, RxPath));
	return TRUE;
}

BOOLEAN
config_phydm_parameter_init(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	)
{
	if (type == ODM_PRE_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Pre setting: disable OFDM and CCK block\n"));
	} else if (type == ODM_POST_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x3);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Post setting: enable OFDM and CCK block\n"));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Wrong type!!\n"));
		return FALSE;
	}

	return TRUE;
}

/* ======================================================================== */
#endif	/* RTL8822B_SUPPORT == 1 */

