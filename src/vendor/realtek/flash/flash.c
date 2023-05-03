/******************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: flash.c
 文件描述:  产测工具
 修订记录:
         1 作者 : 陈赞辉
           日期 : 2009-08-07
           内容: 创建文件
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <flash_layout.h>

/*****************************************************************************
*                                 MACRO                                      *
******************************************************************************/

/*****************************************************************************
*                                 ENUM                                       *
******************************************************************************/

/*****************************************************************************
*                                STRUCT                                      *
******************************************************************************/

/*****************************************************************************
*                               GLOBAL VAR                                   *
******************************************************************************/


/*****************************************************************************
*                               FUNCTION                                     *
******************************************************************************/

/*****************************************************************************
功能: 产测工具 Show Flash File System Help
参数: 
返回: 无
备注:   
******************************************************************************/
static void showHelp(void)
{
	printf("Usage: flash cmd\n");
	printf("cmd:\n");
	printf("  get MIB-NAME \t\t\tget a specific mib from flash memory.\n");
	printf("  set MIB-NAME MIB-VALUE \tset a specific mib into flash memory.\n");
	printf("support mib_name:\n");
	printf("  HW_TX_POWER_CCK_A \n");
	printf("  HW_TX_POWER_CCK_B \n");
	printf("  HW_TX_POWER_HT40_1S_A \n");
	printf("  HW_TX_POWER_HT40_1S_B \n");
	printf("  HW_DIFF_HT40_2S \n");
	printf("  HW_DIFF_HT20_A \n");
	printf("  HW_DIFF_OFDM \n");
	printf("  HW_11N_THER \n");
	printf("  HW_TX_POWER_HT40_1S_A_5G \n");
	printf("  HW_TX_POWER_HT40_1S_B_5G \n");
	printf("  HW_DIFF_HT40_2S_5G \n");
	printf("  HW_DIFF_HT20_A_5G \n");
	printf("  HW_DIFF_OFDM_5G \n");
	printf("  HW_11N_THER_5G \n");
	printf("example:\n");
	printf("  HW_TX_POWER_CCK_A \"xx(14byte)\"\n");
	printf("  HW_TX_POWER_CCK_B \"xx(14byte)\"\n");
	printf("  HW_TX_POWER_HT40_1S_A \"xx(14byte)\"\n");
	printf("  HW_TX_POWER_HT40_1S_A_5G \"xx(196byte)\"\n");
	printf("  HW_DIFF_OFDM \"xx xx\"\n");
	
}

/*****************************************************************************
功能: 产测工具main函数
参数: 
返回: 无
备注:   
******************************************************************************/
int main(int argc, char** argv)
{
	int action=0;
	int argNum=1;
	char mib[100]={0};
	char mibvalue[256+1]={0};
	unsigned short Len;
	

	
	if ( argc > 1 ) 
	{
		if ( !strcmp(argv[argNum], "info") ) {
			action = 0;
			return 0;
		}
		else if ( !strcmp(argv[argNum], "get") ) {
			action = 1;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "set") ) {
			action = 2;
			argNum++;
		}else
		{
			showHelp();
			return 0;
		}
	}
	
	if(action==0)
	{
		showHelp();
		return 0;
	}
	if(action==1)
	{
		unsigned char buffer[512];
		while(argNum < argc)
		{
			memset(buffer, 0x00 , 512);
			sscanf(argv[argNum++], "%s", mib);
			Len=sizeof(buffer);
			app_item_get(buffer,mib,&Len);
			printf("%s=%s\n",mib,buffer);
		}
	}
	if(action==2)
	{
		while((argNum + 1) < argc)
		{
			sscanf(argv[argNum++], "%s", mib);
			sscanf(argv[argNum++], "%s", mibvalue);
//			printf("set %s=%s\n",mib,mibvalue);	
			app_item_save(mibvalue, mib,strlen(mibvalue));		/* 保存新条目*/
		}
	}

	return 0;
}

/*****************************************************************************
*                                 END                                        *
******************************************************************************/
