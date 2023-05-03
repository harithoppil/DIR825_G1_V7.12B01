#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <asm/spram.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include "../../version/tcversion.h"
#include <linux/skbuff.h>

#include <asm/tc3162/TCIfSetQuery_os.h>
#include "7510ptm.h"
#include "7510ptm_lineBonding.h"

#define LINE_POLL_TIME		msecs_to_jiffies(1)
#define pause(x)					mdelay(x)


uint8 maxFragSizeEn = 0;
uint32 maxFragSizeLine0 = 0x200;
uint32 maxFragSizeLine1 = 0x200;

static uint8 isBondingInit = 0;

#ifndef FPGA_STAGE
/* defined in tcci/tcadslcmd.c ,
 * lineId==1 for line0, lineId==2 for line1*/
extern adsldev_ops* adsl_dev_ops_get(int lineId);
#endif

extern void bonding_lines_switching(int toWhere);
extern unsigned int sBonding_reg_read(unsigned int reg_offset);
extern void sBonding_reg_write(unsigned int reg_offset, unsigned int value);

typedef struct lineInfo_s
{
	unsigned int isLinkUp;
	unsigned int isBondingSupported;
	unsigned int isBacpSupported;
	unsigned int linkRate;
	
} lineInfo_t;

lineInfo_t *lineInfos[LINE_NUM];
#ifdef FPGA_STAGE
lineInfo_t *realLines[LINE_NUM];
#endif
static struct timer_list lineInfoTimer;


#if defined(CONFIG_RALINK_VDSL) || defined(TCSUPPORT_NP_BOARD)
typedef struct {
	uint32          actualBitrateNearInt;
	uint32          actualBitrateNearFast;
	uint32          actualBitrateFarInt;
	uint32          actualBitrateFarFast;
} T_adslChannelOperData;

#else
typedef struct {
	uint16          actualBitrateNearInt;
	uint16          actualBitrateNearFast;
	uint16          actualBitrateFarInt;
	uint16          actualBitrateFarFast;
} T_adslChannelOperData;
#endif


static void pafLine_Tx_close(uint8 paf, uint8 line)
{
    uint32 reg;

    printk("close bonding Tx path between PAF%d and Line%d\n", paf, line);

    if (paf == 0 && line == 0)
    {    
        //disable Tx for line0 of paf0 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG0);
        reg &= ~(0x2);
        write_reg_word(BONDING_TXPAF_CFG0, reg);
    }
    else if (paf == 4 && line == 0)
    {
        //disable Tx for line0 of paf4 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG1);
        reg &= ~(0x2);
        write_reg_word(BONDING_TXPAF_CFG1, reg);
    }
    else if (paf == 0 && line == 1)
    {
        //disable Tx for line1 of paf0 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG0);
        reg &= ~(0x1);
        write_reg_word(BONDING_TXPAF_CFG0, reg);

        //disable rbus_master for line1 Tx (in master chip)
    	reg = read_reg_word(BONDING_COMMON1);
    	reg &= ~(1<<24);
    	write_reg_word(BONDING_COMMON1, reg);
    	//wait for a while after rbus_master disable 
    	pause(100);
    }
    else if (paf == 4 && line == 1)
    {
        //disable Tx for line1 of paf4 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG1);
        reg &= ~(0x1);
        write_reg_word(BONDING_TXPAF_CFG1, reg);

        //disable rbus_master for line1 Tx (in master chip)
    	reg = read_reg_word(BONDING_COMMON1);
    	reg &= ~(1<<24);
    	write_reg_word(BONDING_COMMON1, reg);
    	//wait for a while after rbus_master disable 
    	pause(100);
    }
    else
        printk("\nError(%s): PAF%d, line%d un-supported\n", __FUNCTION__, paf, line);

    return;
}

static void pafLine_Rx_close(uint8 paf, uint8 line)
{
    uint32 reg;

    printk("close bonding Rx path between PAF%d and Line%d\n", paf, line);

    if (paf == 0 && line == 0)
    {
        //disable utopia rx for line0 (in master chip)
        reg = read_reg_word(BONDING_COMMON1);
        reg &= ~(COMMON1_UTOPIA_RX_EN);
        write_reg_word(BONDING_COMMON1, reg);
        
        //disable Rx for line0 of paf0 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG0);
        reg &= ~(0x202); //0x200 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG0, reg);
    }
    else if (paf == 4 && line == 0)
    {
        //disable utopia rx for line0 (in master chip)
        reg = read_reg_word(BONDING_COMMON1);
        reg &= ~(COMMON1_UTOPIA_RX_EN);
        write_reg_word(BONDING_COMMON1, reg);
        
        //disable Rx for line0 of paf4 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG1);
        reg &= ~(0x202); //0x200 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG1, reg);
    }
    else if (paf == 0 && line == 1)
    {
        //disable utopia rx for line1 (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg &= ~(COMMON1_UTOPIA_RX_EN);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);

        //disable rbus_master for line1 Rx (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg &= ~(1<<24);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
        //wait for a while after rbus_master disable 
        pause(100);

        //disable Rx for line1 of paf0 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG0);
        reg &= ~(0x101); //0x100 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG0, reg);
    }
    else if (paf == 4 && line == 1)
    {
        //disable utopia rx for line1 (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg &= ~(COMMON1_UTOPIA_RX_EN);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);

        //disable rbus_master for line1 Rx (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg &= ~(1<<24);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
        //wait for a while after rbus_master disable 
        pause(100);

        //disable Rx for line1 of paf4 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG1);
        reg &= ~(0x101); //0x100 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG1, reg);
    }
    else
        printk("\nError(%s): PAF%d, line%d un-supported\n", __FUNCTION__, paf, line);

    return;
}

static void pafLine_Tx_open(uint8 paf, uint8 line)
{
    uint32 reg;

    printk("open bonding Tx path between PAF%d and Line%d\n", paf, line);

    if (paf == 0 && line == 0)
    {
        //enable Tx for line0 of paf0 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG0);
        reg |= (0x2);
        write_reg_word(BONDING_TXPAF_CFG0, reg);
    }
    else if (paf == 4 && line == 0)
    {
        //enable Tx for line0 of paf4 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG1);
        reg |= (0x2);
        write_reg_word(BONDING_TXPAF_CFG1, reg);
    }
    else if (paf == 0 && line == 1)
    {
        //enable rbus_master for line1 Tx (in master chip)
    	reg = read_reg_word(BONDING_COMMON1);
    	reg |= (1<<24);
    	write_reg_word(BONDING_COMMON1, reg);

        //enable Tx for line1 of paf0 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG0);
        reg |= (0x1);
        write_reg_word(BONDING_TXPAF_CFG0, reg);
    }
    else if (paf == 4 && line == 1)
    {
        //enable rbus_master for line1 Tx (in master chip)
    	reg = read_reg_word(BONDING_COMMON1);
    	reg |= (1<<24);
    	write_reg_word(BONDING_COMMON1, reg);
    
        //enable Tx for line1 of paf4 (in master chip)
        reg = read_reg_word(BONDING_TXPAF_CFG1);
        reg |= (0x1);
        write_reg_word(BONDING_TXPAF_CFG1, reg);
    }
    else
        printk("\nError(%s): PAF%d, line%d un-supported\n", __FUNCTION__, paf, line);

    return;
}

static void pafLine_Rx_open(uint8 paf, uint8 line)
{
    uint32 reg;

    printk("open bonding Rx path between PAF%d and Line%d\n", paf, line);

    if (paf == 0 && line == 0)
    {
        //enable Rx for line0 of paf0 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG0);
        reg |= 0x202; //0x200 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG0, reg);
        
        //enable utopia rx for line0 (in master chip)
        reg = read_reg_word(BONDING_COMMON1);
        reg |= (COMMON1_UTOPIA_RX_EN);
        write_reg_word(BONDING_COMMON1, reg);
    }
    else if (paf == 4 && line == 0)
    {
        //enable Rx for line0 of paf4 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG1);
        reg |= 0x202; //0x200 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG1, reg);
        
        //enable utopia rx for line0 (in master chip)
        reg = read_reg_word(BONDING_COMMON1);
        reg |= (COMMON1_UTOPIA_RX_EN);
        write_reg_word(BONDING_COMMON1, reg);
    }
    else if (paf == 0 && line == 1)
    {
        //enable Rx for line1 of paf0 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG0);
        reg |= (0x101); //0x100 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG0, reg);

        //enable rbus_master for line1 Rx (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg |= (1<<24);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
        
        //enable utopia rx for line1 (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg |= (COMMON1_UTOPIA_RX_EN);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
    }
    else if (paf == 4 && line == 1)
    {
        //enable Rx for line1 of paf4 (in master chip)
        reg = read_reg_word(BONDING_RXPAF_CFG1);
        reg |= (0x101); //0x100 is for CRC issue
        write_reg_word(BONDING_RXPAF_CFG1, reg);

        //enable rbus_master for line1 Rx (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg |= (1<<24);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
        
        //enable utopia rx for line1 (in slave chip)
        reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
        reg |= (COMMON1_UTOPIA_RX_EN);
        sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
    }
    else
        printk("\nError(%s): PAF%d, line%d un-supported\n", __FUNCTION__, paf, line);

    return;
}


#ifdef FPGA_STAGE
static int bonding_realLineInfoSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[32];
	unsigned char i;

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	for (i = 0; i < LINE_NUM; i++)
		memset(realLines[i], 0, sizeof(lineInfo_t));


	sscanf(valString, "%d %d %d %d %d %d %d %d", 
			&realLines[0]->isLinkUp, 
			&realLines[0]->isBondingSupported, 
			&realLines[0]->isBacpSupported,
			&realLines[0]->linkRate,
			&realLines[1]->isLinkUp, 
			&realLines[1]->isBondingSupported, 
			&realLines[1]->isBacpSupported,
			&realLines[1]->linkRate);

    if ((!realLines[0]->isBondingSupported && realLines[0]->isBacpSupported) ||
        (!realLines[1]->isBondingSupported && realLines[1]->isBacpSupported)
    )
        printk("\nError: BACP_supported should be based on Bonding_supported\n");

    if ((realLines[0]->isBondingSupported != realLines[1]->isBondingSupported) ||
        (realLines[0]->isBacpSupported != realLines[1]->isBacpSupported)
    )
        printk("\nError: Both lines should support Bonding/BACP at the same time\n");
    
	printk("\nL0 linkUp:%d, Bonding:%d, BACP:%d, linkRate:%d\n"
			 "L1 linkUp:%d, Bonding:%d, BACP:%d, linkRate:%d\n\n", 
			realLines[0]->isLinkUp, 
			realLines[0]->isBondingSupported, 
			realLines[0]->isBacpSupported,
			realLines[0]->linkRate,
			realLines[1]->isLinkUp, 
			realLines[1]->isBondingSupported, 
			realLines[1]->isBacpSupported,
			realLines[1]->linkRate);


	return count;
}
#endif


static int lineLinkStatus_get(unsigned char lineId)
{
#ifdef FPGA_STAGE
	if (realLines[lineId])
		return realLines[lineId]->isLinkUp;
	else
		return 0;

#else
	uint8 modemst = 0;
	adsldev_ops *ops = NULL;

	if (lineId == 0 || lineId == 1)
		ops = adsl_dev_ops_get(lineId+1);
	else
		return 0;

	if (ops == NULL)
		return 0;

	ops->query(ADSL_QUERY_STATUS, &modemst, NULL );

	if (modemst == ADSL_MODEM_STATE_UP)
		return 1;
	else
		return 0;
#endif
}


static void bacpHwParsing_enable(void)
{
	unsigned int reg;

	reg = read_reg_word(PTM_BACP_FIELD_EN);
	if ((reg & 0x3f) != 0) //enabled already
		return;

	/* set values for BACP header parsing */
	write_reg_word(PTM_BACP_FIELD_0, 0x0180c200);
	write_reg_word(PTM_BACP_FIELD_1, 0x00028809);
	write_reg_word(PTM_BACP_FIELD_2, 0x0a0019a7);
	write_reg_word(PTM_BACP_FIELD_3, 0x01010000);
	/* enable BACP header parsing */
	 write_reg_word(PTM_BACP_FIELD_EN, 0x3f);

	 printk("\nBACP HW Parsing Enabled\n");
}


static void bacpHwParsing_disable(void)
{
	unsigned int reg;

	reg = read_reg_word(PTM_BACP_FIELD_EN);
	if (reg == 0) //disabled already
		return;
		
	write_reg_word(PTM_BACP_FIELD_EN, 0);
	printk("\nBACP HW Parsing Disabled\n");
}


static int bondingBacpSupport_set(unsigned char lineId)
{
#ifdef FPGA_STAGE
    if (lineInfos[lineId] && realLines[lineId])
    {
	    lineInfos[lineId]->isBondingSupported = 
				realLines[lineId]->isBondingSupported;
		lineInfos[lineId]->isBacpSupported = 
				realLines[lineId]->isBacpSupported; 
    }
	else
		return -1;
    
#else
	unsigned char result;
	adsldev_ops *ops = NULL;

	if (lineInfos[lineId] == NULL)
		return -1;

	if (lineId == 0 || lineId == 1)
		ops = adsl_dev_ops_get(lineId+1);
	else
		return -1;

	if (ops == NULL)
		return -1;

	/* VDSL2_QUERY_BONDING_BACP_SUPPORT is 0x2003, should be
	 * defined in asm/tc3162/TCIfSetQuery_os.h */
	ops->query(VDSL2_QUERY_BONDING_BACP_SUPPORT, &result, NULL );

	if (result == BONDING_OFF_BACP_OFF)
	{
		lineInfos[lineId]->isBondingSupported = 0;
		lineInfos[lineId]->isBacpSupported = 0;
	}
	else if (result == BONDING_ON_BACP_OFF)
	{
		lineInfos[lineId]->isBondingSupported = 1;
		lineInfos[lineId]->isBacpSupported = 0;
	}
	else if (result == BONDING_OFF_BACP_ON)
	{
		printk("\nBACP-supported should be based on Bonding-supported\n");
        return -1;
	}
	else if (result == BONDING_ON_BACP_ON)
	{
		lineInfos[lineId]->isBondingSupported = 1;
		lineInfos[lineId]->isBacpSupported = 1;
	}
    else
    {
        printk("\nunknow result for BACP query!\n");
        return -1;
    }
#endif

    return 0;
}


static void lineBondingResource_free(void)
{
	unsigned char i;

    if (isBondingInit == 0)
        return;

	del_timer_sync(&lineInfoTimer);

#ifdef FPGA_STAGE
	for (i = 0; i < LINE_NUM; i++)
		kfree(realLines[i]);
#endif

	for (i = 0; i < LINE_NUM; i++)
		kfree(lineInfos[i]);

    bacpHwParsing_disable();

    //disable bonding module
    bonding_lines_switching(TO_NO_BONDING);

    isBondingInit = 0;
}


static unsigned int linkRate_get(int lineId)
{
#ifdef FPGA_STAGE
	return realLines[lineId]->linkRate;

#else
	T_adslChannelOperData adslChannelOperData;
	adsldev_ops *ops = NULL;

	if (lineId == 0 || lineId == 1)
		ops = adsl_dev_ops_get(lineId+1);
	else
		return 0;

	if (ops == NULL)
		return 0;

	ops->query(ADSL_QUERY_CH_OP_DATA, &adslChannelOperData, NULL);

	if (adslChannelOperData.actualBitrateFarInt)
		return adslChannelOperData.actualBitrateFarInt;
	else
		return adslChannelOperData.actualBitrateFarFast;
#endif
}


void maxFragSize_set(void)
{
	uint32 linkRateL0;
	uint32 linkRateL1;
	uint32 diffValue;
	uint32 markValue;
	

	if (!lineLinkStatus_get(0) || !lineLinkStatus_get(1))
		return;

	linkRateL0 = linkRate_get(0);
	linkRateL1 = linkRate_get(1);

	if (!linkRateL0 || !linkRateL1)
		return;


	if (linkRateL0 >= linkRateL1)
	{
		markValue = linkRateL0 >> 2;
		diffValue = linkRateL0 - linkRateL1;
			
		//set fragSize is 512 for L0
		maxFragSizeLine0 = 0x200;

		if (diffValue < markValue)
		    maxFragSizeLine1 = 0x200; //512 for L0
		else if (markValue <= diffValue && diffValue < (markValue<<1))
		    maxFragSizeLine1 = 0x180; //384 for L1
		else if ((markValue<<1) <= diffValue && diffValue < (markValue*3))
		    maxFragSizeLine1 = 0x100; //256 for L1
		else
		    maxFragSizeLine1 = 0x80; //128 for L1
	}
	else // linkRateL1 > linkRateL0
	{
		markValue = linkRateL1 >> 2;
		diffValue = linkRateL1 - linkRateL0;
			
		//set fragSize is 512 for L1
		maxFragSizeLine1 = 0x200;

		if (diffValue < markValue)
		    maxFragSizeLine0 = 0x200; //512 for L0
		else if (markValue <= diffValue && diffValue < (markValue<<1))
		    maxFragSizeLine0 = 0x180; //384 for L0
		else if ((markValue<<1) <= diffValue && diffValue < (markValue*3))
		    maxFragSizeLine0 = 0x100; //256 for L0
		else
		    maxFragSizeLine0 = 0x80; //128 for L0	
	}

	//set maxFragSize in bonding_recovery().
	maxFragSizeEn = 1;

	printk("\nMax FragSize for line0:%d, line1:%d\n",
				(int)maxFragSizeLine0, (int)maxFragSizeLine0);
}


static void lineInfo_poll_func(unsigned long data)
{
	lineInfo_t curInfos[LINE_NUM];	
	unsigned char i;


	for (i = 0; i < LINE_NUM; i++)
	{
		memset(&curInfos[i], 0, sizeof(lineInfo_t));

        //get link up status	
		curInfos[i].isLinkUp = lineLinkStatus_get(i);

        /* Only when Line is down2up, its bonding status needs
         * to be updated. */
        if (!lineInfos[i]->isLinkUp && curInfos[i].isLinkUp)
        {
            if (bondingBacpSupport_set(i))
            {
                printk("\nFAILED: getting line%d bonding info\n", i);
                lineBondingResource_free();
                return;
            }

            if ((lineInfos[i]->linkRate = linkRate_get(i)) == 0)
            {
                printk("\nFAILED: link rate is zero for line%d\n", i);
                lineBondingResource_free();
                return;
            }
        }
    }


    /* in case that no status change for L0 & L1 */
    //(L0_up2up || L0_down2down) && (L1_up2up || L1_down2down)
    if (( (lineInfos[0]->isLinkUp && curInfos[0].isLinkUp) ||
       (!lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp) ) &&
       ( (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp) ||
       (!lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp) )
    )
    {
        //L0_down2down && L1_down2down
        if ( (!lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp) &&
           (!lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp) )
           lineBondingResource_free();
        else
           mod_timer(&lineInfoTimer, jiffies + LINE_POLL_TIME);

        return;
    }


	/* in case that L0 is link up, if L0 is not bonding
	 * supported, bonding two lines is impossible, so 
	 * don't have to care L1's status, just return */
	//L0_xxx2up
	if (curInfos[0].isLinkUp)
	{
		if (!lineInfos[0]->isBondingSupported)
		{
			lineBondingResource_free();
			return;
		}
	}

	/* in case that L1 is link up, if L1 is not bonding
	 * supported, bonding two lines is impossible, so 
	 * don't have to care L0's status, just return */
	//L1_xxx2up
	if (curInfos[1].isLinkUp)
	{
		if (!lineInfos[1]->isBondingSupported)
		{
			lineBondingResource_free();
			return;
		}
	}

    /* Both lines should support or not support Bonding/BACP
     * at the same time when both lines are up */
    if (curInfos[0].isLinkUp && curInfos[1].isLinkUp)
    {
        if ((lineInfos[0]->isBondingSupported != lineInfos[1]->isBondingSupported) ||
           (lineInfos[0]->isBacpSupported != lineInfos[1]->isBacpSupported)
        )
        {
            printk("\nBoth lines should support or not support Bonding/BACP at the same time\n");
    		lineBondingResource_free();
    		return;                
        }
    }


    /* enable bacp hw parsing 
     * when either line is down2up and bacp supported */
    if ((lineInfos[0]->isLinkUp && curInfos[0].isLinkUp && lineInfos[0]->isBacpSupported) || 
        (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp && lineInfos[1]->isBacpSupported))
        bacpHwParsing_enable();


		
    /* 
     * In the following cases, we suppose both lines are
     * bonding supported! If not, that means certain line 
     * is never link up yet until now! 
     */
    

	//L0_Bonding && L1_Bonding
	if (lineInfos[0]->isBondingSupported && lineInfos[1]->isBondingSupported)
	{
	    /* both lines are bonding and bacp supported */
	    //L0_BACP && L1_BACP
		if (lineInfos[0]->isBacpSupported && lineInfos[1]->isBacpSupported)
		{
		    //L0_down2up
			if (!lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
            {
                //L1_down2up
                if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE0);
                    printk("\ndo single-PME bonding for L0!\n");
                    pafLine_Rx_open(4, 1);
                    pafLine_Tx_open(4, 1);
                    printk("\ndo single-PME-PAF4 bonding for L1!\n");
                    pafLine_Tx_close(4, 1);
                    pafLine_Rx_close(4, 1);
                    printk("\ndo PAF bonding for L1!\n");                    
                    pafLine_Rx_open(0, 1);
                    pafLine_Tx_open(0, 1);
                }
                //L1_up2down
                else if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE0);
                    printk("\ntear single-PME bonding for L1!\n");
                    printk("\ndo single-PME bonding for L0!\n");
                }                
                //L1_up2up
                else if (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {
                    pafLine_Rx_open(4, 0);
                    pafLine_Tx_open(4, 0);
                    printk("\ndo single-PME-PAF4 bonding for L0!\n");
                    pafLine_Tx_close(4, 0);
                    pafLine_Rx_close(4, 0);
                    printk("\ndo PAF bonding for L0!\n");                    
                    pafLine_Rx_open(0, 0);
                    pafLine_Tx_open(0, 0);                
                }
                else
                {
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
                }
            }
		    //L0_up2up
			else if (lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
            {
                //L1_down2up
                if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {
                    pafLine_Rx_open(4, 1);
                    pafLine_Tx_open(4, 1);
                    printk("\ndo single-PME-PAF4 bonding for L1!\n");
                    pafLine_Tx_close(4, 1);
                    pafLine_Rx_close(4, 1);
                    printk("\ndo PAF bonding for L1!\n");                    
                    pafLine_Rx_open(0, 1);
                    pafLine_Tx_open(0, 1);
                }
                //L1_up2down
                else if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
                {
                    pafLine_Tx_close(0, 1);
                    pafLine_Rx_close(0, 1);
                    printk("\ntear PAF bonding for L1!\n");
                }
                else
                {
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
                }
            }
		    //L0_down2up
			else if (lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp)
            {
                //L1_down2up
                if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE1);
                    printk("\ntear single-PME bonding for L0!\n");
                    printk("\ndo single-PME bonding for L1!\n");
                }
                //L1_up2up
                if (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE1);
                    printk("\ntear PAF bonding for L0!\n");
                }
                else
                {
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
                }
            }
		    //L0_down2down
			else
            {
                printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                lineBondingResource_free();
                return;
            }
		}
        /* BACP should be supported for both lines at the same
         * time, unless at least one line is link down */
        //L0_BACP && !L1_BACP
		else if (lineInfos[0]->isBacpSupported && !lineInfos[1]->isBacpSupported)
		{
		    //L0_down2up
            if (!lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
            {
                //L1_up2down
                if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE0);
                    printk("\ndo single-PME bonding for L0!\n");
                }
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}                    
            }
            //L0_up2down
            else if (lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp)
            {
                //L1_down2up
                if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
                {            
                    printk("\ntear single-PME bonding for L0!\n");
                    bonding_lines_switching(TO_LINE1);
                }
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}

            }
            else
            {
                printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                lineBondingResource_free();
                return;
            }
		}
        /* BACP should be supported for both lines at the same
         * time, unless at least one line is link down */
        //!L0_BACP && L1_BACP
		else if (!lineInfos[0]->isBacpSupported && lineInfos[1]->isBacpSupported)
		{
		    //L1_down2up
            if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
            {
                //L0_up2down
                if (lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp)
                {
                    bonding_lines_switching(TO_LINE1);
                    printk("\ndo single-PME bonding for L1!\n");
                }
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}                    
            }
            //L1_up2down
            else if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
            {
                //L0_down2up
                if (!lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
                {            
                    printk("\ntear single-PME bonding for L1!\n");
                    bonding_lines_switching(TO_LINE0);
                }
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}

            }
            else
            {
                printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                lineBondingResource_free();
                return;
            }
		}
		/* both lines are bonding supported and not bacp supported */
		//!L0_BACP && !L1_BACP
		else 
		{
		    //L0_down2up
			if (!lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
			{
			    //L1_down2up || L1_up2up
				if ((!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp) ||
                     (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp))
				{
					bonding_lines_switching(TO_BOTH_LINES);
				}
                //L1_up2down
				else if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
				{
					bonding_lines_switching(TO_LINE0);
				}			
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}
			}
            //L0_up2up
			else if (lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
			{
			    //L1_down2up
				if (!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp)
				{
					bonding_lines_switching(TO_BOTH_LINES);
				}
                //L1_up2down
				else if (lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp)
				{
					bonding_lines_switching(TO_LINE0);
				}
                else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}
			}
            //L0_up2down
			else if (lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp)
			{
			    //L1_down2up || L1_up2up
				if ((!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp) ||
                    (lineInfos[1]->isLinkUp && curInfos[1].isLinkUp) )
				{
					bonding_lines_switching(TO_LINE1);
				}
				else
				{
                    printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                    lineBondingResource_free();
                    return;
				}
			}
            //L0_down2down
			else
			{
                printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
                lineBondingResource_free();
                return;
			}
		}
	}
	/* In this case, L1 is never link up yet until now, 
	 * so we just have to care L0's status change */
	//L0_Bonding && !L1_Bonding
	else if (lineInfos[0]->isBondingSupported && !lineInfos[1]->isBondingSupported)
	{
	    //L0_down2up
		if (!lineInfos[0]->isLinkUp && curInfos[0].isLinkUp)
		{
            bonding_lines_switching(TO_LINE0);
        
			if (lineInfos[0]->isBacpSupported)
			{
				printk("\ndo single-PME bonding for L0!\n");
			}
		}
        //L0_up2down
		else if ((lineInfos[0]->isLinkUp && !curInfos[0].isLinkUp))
		{
			if (lineInfos[0]->isBacpSupported)
			{
				printk("\ntear single-PME bonding for L0!\n");
			}
            
            lineBondingResource_free();
            return;
		}
	}
	/* In this case, L0 is never link up yet until now, 
	 * so we just have to care L1's status change */
	//!L0_Bonding && L1_Bonding
	else if (!lineInfos[0]->isBondingSupported && lineInfos[1]->isBondingSupported)
	{
	    //L1_down2up
		if ((!lineInfos[1]->isLinkUp && curInfos[1].isLinkUp))
		{
            bonding_lines_switching(TO_LINE1);
        
			if (lineInfos[1]->isBacpSupported)
			{
				printk("\ndo single-PME bonding for L1\n");
			}
		}
        //L1_up2down
		else if ((lineInfos[1]->isLinkUp && !curInfos[1].isLinkUp))
		{
			if (lineInfos[1]->isBacpSupported)
			{
				printk("\ntear single-PME bonding for L1!\n");
			}

            lineBondingResource_free();
            return;
		}
	}
	else
    {   
		printk("\nWrong: %s shouldn't be here!\n", __FUNCTION__);
        lineBondingResource_free();
        return;
    }


	/* update line link status */
	for (i = 0; i < LINE_NUM; i++)
		lineInfos[i]->isLinkUp = curInfos[i].isLinkUp;

    //wakeup timer later
    mod_timer(&lineInfoTimer, jiffies + LINE_POLL_TIME);

    return;
}

int lineBonding_proc_init(void)
{
#ifdef FPGA_STAGE
    struct proc_dir_entry *ptmProc;

	ptmProc = create_proc_entry("tc3162/bonding_realLineInfo_set", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for bonding_realLineInfo_set\n");
		return -1;
	}
	ptmProc->write_proc = bonding_realLineInfoSet_write_proc;
#endif

    return 0;
}

void lineBonding_proc_remove(void)
{
#ifdef FPGA_STAGE
    remove_proc_entry("tc3162/bonding_realLineInfo_set", 0);
#endif

    return;
}

int lineBonding_init(void)
{
	unsigned char i, j;


    if (isBondingInit)
        return 0;

	if (LINE_NUM != 2)
	{
		printk("\nWARNNING: LINE_NUM:%d is not 2, so lineBonding_init failed\n", LINE_NUM);
		return -1;
	}


	for (i = 0; i < LINE_NUM; i++)
	{
		lineInfos[i] = (lineInfo_t*) kzalloc(sizeof(lineInfo_t), GFP_KERNEL);
		if (lineInfos[i] == NULL)
		{
			printk("\nFailed: allocate lineInfo %d\n", i);
			for (j = 0; j < i; j++)
				kfree(lineInfos[j]);
			return -1;
		}
	}

#ifdef FPGA_STAGE
	for (i = 0; i < LINE_NUM; i++)
	{
		realLines[i] = (lineInfo_t*) kzalloc(sizeof(lineInfo_t), GFP_KERNEL);
		if (realLines[i] == NULL)
		{
			printk("\nFailed: allocate realLine %d\n", i);
			for (j = 0; j < i; j++)
				kfree(realLines[j]);
			return -1;
		}
	}	
#endif


	init_timer(&lineInfoTimer);
	lineInfoTimer.expires = jiffies + LINE_POLL_TIME;
	lineInfoTimer.function = lineInfo_poll_func;
	lineInfoTimer.data = (unsigned long) NULL;
	add_timer(&lineInfoTimer);

    isBondingInit = 1;

	return 0;
}
