/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : tbs_common.c
 文件描述 : 底层公共函数

 函数列表 :
				flash_info_get
				crc16
				check_addr_null
				kernel_write
				kernel_erase

 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-7-22
            描述 :

=========================================================================*/

#include <linux/vmalloc.h>
#include <linux/flash_layout_kernel.h>

/* Table of CRC constants - implements x^16+x^12+x^5+1 */
static const uint16_t crc16_tab[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

/*=========================================================================
 Function:		void flash_info_get(flash_info_t * flash_info_low,struct mtd_info *mtd_master)
 Description:		获取底层llconfig配置
 Data Accessed:
 Data Updated:
 Input:			mtd_master:		Flash MTD分区信息
 Output:			flash_info_low:	Flash分区表
 Return:
 Others:
=========================================================================*/
void flash_info_get(flash_info_t * flash_info_low, struct mtd_info *mtd_master)
{
	unsigned int i;
	unsigned int blocknumber=0;
	unsigned int blockoffset=0;

	flash_info_low->size = mtd_master->size;

	if( (mtd_master->numeraseregions) != 0 )
	{
		for (i=0; i<mtd_master->numeraseregions;i++)
		{
			int j;

			for(j=0;j<(mtd_master->eraseregions[i].numblocks);j++)
			{
				if( blocknumber == 0)
				{
					flash_info_low->start[blocknumber++] = 0x0;
				}
				else if( j == 0 )
				{
					blockoffset = mtd_master->eraseregions[i].erasesize;
					flash_info_low->start[blocknumber++] = blockoffset;
				}
				else
				{
					blockoffset += mtd_master->eraseregions[i].erasesize;
					flash_info_low->start[blocknumber++] = blockoffset;
				}
			}
		}

		flash_info_low->sector_count = blocknumber;
	}
	else
	{
        /* 强转成32位，因为realtek编译器不支持64位的除法 */
		flash_info_low->sector_count = (uint32_t)mtd_master->size / mtd_master->erasesize;

		for(i=0;i < flash_info_low->sector_count;i++)
		{
			flash_info_low->start[i] = (mtd_master->erasesize) *  i  ;
		}
	}
}

/*=========================================================================
 Function:		int crc16(unsigned char *buf, int len , unsigned short *checksum )
 Description:		获取底层llconfig配置
 Data Accessed:
 Data Updated:
 Input:			buf:		要校验的数据指针
 				len:		要校验的数据长度
 Output:			*checksum:	校难结果
 Return:			1:成功
 Others:
=========================================================================*/

int tbs_crc16(unsigned char *buf, int len , unsigned short *checksum )
{
	int 				i;
	unsigned short	cksum;

	cksum = 0;
	for (i = 0;  i < len;  i++)
	{
		cksum = crc16_tab[((cksum>>8) ^ *buf++) & 0xFF] ^ (cksum << 8);
	}
	*checksum = cksum;

	return 1;
}

/*=========================================================================
 Function:		int check_addr_null(unsigned int *addr, unsigned int len,struct mtd_info *mtd_master)
 Description:		检查指定的Flash空间是否为空
 Data Accessed:
 Data Updated:
 Input:			*addr:			要检查的地址
 				len:				要检查的长度
 				mtd_master:		Flash MTD分区信息
 Output:			无
 Return:			0:地址空间为空
 				1:地址空间非空
 Others:
=========================================================================*/

int check_addr_null(unsigned int *addr, unsigned int len,struct mtd_info *mtd_master)
{
	unsigned int i;
	unsigned int j=0;
	char *buff_addr=NULL;
	int retlen;

	buff_addr = ( char *) vmalloc(len);
	if(buff_addr == NULL)
	{
		printk("buff malloc wrong\n");

		return EFAULT;
	}

	if( ( unsigned int ) addr >= 0 && ( unsigned int )addr <= mtd_master->size )
	{
		mtd_master->read( mtd_master , ( long )addr , len , &retlen , buff_addr );
		addr = (unsigned int *)buff_addr;
	}

	for( i = 0 ; i < len ; i++ )
	{
		if( *( ( unsigned char * )addr +  i ) == 0xff )
			j++;
	}

	vfree(buff_addr);
	if( i == j )
		return 0;
	else
		return 1;
}

/*=========================================================================
 Function:		int kernel_write(struct mtd_info *mtd_master,unsigned char *src,unsigned int addr,unsigned int len)
 Description:		Flash写接口
 Data Accessed:
 Data Updated:
 Input:			mtd_master:	Flash MTD分区信息
 				*src:		源地址
 				addr:		目的地址
 				len:			长度
 Output:
 Return:			0:		写成功
 				其它:	写失败
 Others: 			当Flash进行写操作时,防止其它进程读Flash造成系统异常,所以
 				在整个Flash写过程中中断要关掉.但如果系统启用了看门狗,
 				在中断关掉前要关闭看门狗,还原中断后再启用看门狗
=========================================================================*/

int kernel_write(struct mtd_info *mtd_master,unsigned char *src,unsigned int addr,unsigned int len)
{
	unsigned char *src_sdram=NULL;
	unsigned int i;
	unsigned int retlen;
	unsigned int ret = 0;
	unsigned long int flags;

	src_sdram = kmalloc(1024,GFP_KERNEL);
	if ( src_sdram == NULL )
	{
		printk("src_sdram malloc wrong\n");
			return EFAULT;
	}

	/*关闭看门狗*/
#ifdef CONFIG_TBS_WATCHDOG
	watchdog_close();
#endif

	/*关中断*/
	local_irq_save(flags);

	if( ( unsigned int )src >=0 && ( unsigned int )src <= mtd_master->size)
	{

		for(i=0;i<(len>>10);i++)
		{
			ret = mtd_master->read( mtd_master , ( long ) src , 1024 , &retlen , src_sdram );
			ret = mtd_master->write(mtd_master,addr,1024,&retlen,src_sdram);
			src += retlen;
			addr += retlen;
			printk(".");
		}

		if(len%1024)
		{
			ret = mtd_master->read( mtd_master , ( long ) src , len%1024 , &retlen , src_sdram );
			ret = mtd_master->write(mtd_master,addr,len%1024,&retlen,src_sdram);
		}
	}
	else
	{
		ret = mtd_master->write(mtd_master,addr,len,&retlen,src);
	}

	/*还原中断*/
	local_irq_restore(flags);

	/* 打开看门狗 */
#ifdef CONFIG_TBS_WATCHDOG
        watchdog_open();
#endif
	kfree(src_sdram);
	return ret;
}

/*=========================================================================
 Function:		int kernel_write(struct mtd_info *mtd_master,unsigned char *src,unsigned int addr,unsigned int len)
 Description:		Flash写接口
 Data Accessed:
 Data Updated:
 Input:			mtd_master:	Flash MTD分区信息
 				*info:		Flash分区表
 				addr:		目的地址
 				len:			长度
 Output:
 Return:			0:		擦除成功
 				其它:	擦除失败
 Others: 			当Flash进行擦除操作时,防止其它进程读Flash造成系统异常,所以
 				在整个Flash擦除过程中中断要关掉.但如果系统启用了看门狗,
 				在中断关掉前要关闭看门狗,还原中断后再启用看门狗
=========================================================================*/

int kernel_erase(struct mtd_info *mtd_master,flash_info_t * info,unsigned int addr,unsigned int len)
{
	struct erase_info *erase;
	int i,erase_sector_start,erase_sector_end;
	int ret;
	unsigned long int flags;

	erase = kmalloc(sizeof(struct erase_info),GFP_KERNEL);
	if ( erase == NULL )
	{
		printk("erase malloc wrong\n");
		return EFAULT;
	}
	memset (erase,0,sizeof(struct erase_info));
	erase->mtd = mtd_master;
	erase->addr = addr;

	for(i=0;info->start[i]<addr;i++);
	erase_sector_start = i;

	if( ( (addr + len ) > info->start[info->sector_count -1] ) && ( (addr + len ) <= info->size) )
	{
		erase->len = info->size - info->start[erase_sector_start];
	}
	else
	{
		for( i=0; info->start[i] < (addr + len );i++ );
		erase_sector_end = i;
		erase->len = info->start[erase_sector_end] - info->start[erase_sector_start];
	}

	/*关闭看门狗*/
#ifdef CONFIG_TBS_WATCHDOG
	watchdog_close();
#endif

	 /*关中断*/
	local_irq_save(flags);
	ret = mtd_master->erase(mtd_master,erase);
	if( ret < 0 )
		printk("erase failure\n");

	/*还原中断*/
	local_irq_restore(flags);

	/* 打开看门狗 */
#ifdef CONFIG_TBS_WATCHDOG
        watchdog_open();
#endif
	kfree(erase);
	return ret;
}

/*=========================================================================
					File End
=========================================================================*/


