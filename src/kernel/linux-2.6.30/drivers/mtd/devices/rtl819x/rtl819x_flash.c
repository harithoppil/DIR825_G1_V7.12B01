/*
 * This file contains glue for Atheros ar7100 spi flash interface
 * Primitives are ar7100_spi_*
 * mtd flash implements are ar7100_flash_*
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/delay.h>
#include <asm/io.h>
#include "rtl819x_flash.h"

//tylo, for ic ver. detect
#define SCCR	0xb8003200
unsigned char ICver=0;
#define IC8672 	0
#define IC8671B 	1
#define IC8671B_costdown 	2
/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;

/*
 * statics
 */

static const char *part_probes[] __initdata = {"cmdlinepart", "RedBoot", NULL};

 void spi_ready(void)
{
	while (1)
	{
		if ( (*(volatile unsigned int *) SFCSR) & READY(1))
		break;
	}
}


void spi_pio_init(void)
{
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

void read_id(spi_flash_id * flash_id)
{
	unsigned int temp;
	int cnt =0;

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

	/* One More Toggle (May not Necessary) */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

	/* RDID Command */
	*(volatile unsigned int *) SFDR = 0x9F << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
	temp = *(volatile unsigned int *) SFDR;

	flash_id->device_id = (temp >> 8) & 0xFFFFFF;
	flash_id->ext_device_id= 0;

	spi_ready();

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

/*
 The sector Erase function
*/
static int do_spi_sector_erase(u32 addr)
{
	spi_pio_init();

	/* WREN Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);

	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* SE Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		/* RDSR Command */
		if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
		{
			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	return 0;
}

void rtl819x_spi_write_page(unsigned int addr, unsigned char *data, int len)
{
	unsigned char *cur_addr;
	int cur_size;
	int k;
	unsigned int temp;
	unsigned int cnt;

	cur_addr = data;
	cur_size = len;


	/* WREN Command */
	spi_ready();

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* PP Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = (0x02 << 24) | addr;

	for (k = 0; k < 64; k++) {
		spi_ready();
		if (cur_size >= 4) {/* 长度超过4字节时按4字节读取和写入 */
			temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
			cur_size -= 4;
			cur_addr += 4;
		} else {/* 长度小于4字节时按实际字节数取值 */
			temp = 0;/* 取值初始化 */
			switch(cur_size) {
				case 3:
					temp |= (*(cur_addr + 2)) << 8;/* 取第三字节 */
				case 2:
					temp |= (*(cur_addr + 1)) << 16;/* 取第二字节 */
				default:
					temp |= (*(cur_addr)) << 24;/* 取第一字节 */
					break;				
				}
			*(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1) | READY(1);/* 根据数据长度设置写入长度 */
			cur_size = 0;
		}
		*(volatile unsigned int *) SFDR = temp;/* 写入FLASH */
		if (cur_size == 0) {/* 所有数据全部写完 */
			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	cnt = 0;
	while (1)
	{
		unsigned int status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ((status & 0x01000000) == 0x00000000)
		{
			break;
		}

		if (cnt > 200000)
		{
			printk("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, addr);
			return;
		}
		cnt++;
	}

	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

}


static int rtl819x_flash_erase(struct mtd_info *mtd,struct erase_info *instr)
{

	int nsect, s_curr, s_last;
	unsigned long int flags;
    local_irq_save(flags);
    if((unsigned int)(instr)<0x80000000)
    {
         printk("instr addr 222:0x%x\n",(unsigned int)(instr));
    }
    
	if (instr->addr + instr->len > mtd->size)
	{
	    local_irq_restore(flags);
		return (-EINVAL);
	}

	nsect = ((uint32_t)instr->len)/mtd->erasesize;
	if ((uint32_t)instr->len % mtd->erasesize)
	{
		nsect ++;
	}

	s_curr = (uint32_t)instr->addr/mtd->erasesize;
	s_last  = s_curr + nsect;

	do
	{
		do_spi_sector_erase(s_curr * mtd->erasesize);
		//printk(".X.");
		__udelay(1000);
	} while (++s_curr < s_last);

	printk("\n");

	if (instr->callback)
	{
		instr->state |= MTD_ERASE_DONE;
		instr->callback (instr);
	}
    
    local_irq_restore(flags);

	return 0;
}

static int rtl819x_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
                  size_t *retlen ,u_char *buf)
{
	unsigned int address = (uint32_t)from + RTL819X_FLASH_BASE;

	if (!len)
	{
		return (0);
	}

	if (from + len > mtd->size)
	{
		return (-EINVAL);
	}

	memcpy( buf,  ( unsigned int *)address, len );
	*retlen = len;

	return 0;
}

static int rtl819x_flash_write (struct mtd_info *mtd, loff_t to, size_t len,
                    size_t *retlen, const u_char *buf)
{

	int total = 0, len_this_lp, bytes_this_page;
	uint32_t addr = 0;
	u_char *mem;

	while(total < len)
	{
		mem            = ( u_char * ) ( buf + total );
		addr             = to  + total;
		bytes_this_page  = RTL819X_FLASH_PG_SIZE_256B - (addr % RTL819X_FLASH_PG_SIZE_256B);
		len_this_lp      = min(((int)len - total), bytes_this_page);

		rtl819x_spi_write_page(addr, mem, len_this_lp);
		total += len_this_lp;
	}

	*retlen = len;

	return 0;
}

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */

static int __init rtl819x_flash_init (void)
{
	int np;
	unsigned int sccr;
	struct mtd_info *mtd;
	struct mtd_partition *mtd_parts;
	spi_flash_id  flash_id;
	int ret_val = -EIO;

	sccr=*(volatile unsigned int*)SCCR;
	if((sccr & 0x00100000) == 0) {
		ICver = IC8671B_costdown;
		SFCR = 0xB8001200;
		SFCSR = 0xB8001208;
		SFDR = 0xB800120C;
	} else {
		ICver = IC8671B;
		SFCR = 0xB8001200;
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
	}
	spi_pio_init();

	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);//
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));
	read_id(&flash_id);
	mtd =  kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!mtd) {
		printk("Cant allocate mtd stuff\n");
		goto out;
	}
	if(flash_id.device_id == 0x12018) /*spansion spi flash:S25FL128P */
	{
		if(flash_id.ext_device_id == 0x300)	{/* 256k byte sector */
			mtd->size           =   RTL819X_FLASH_SIZE_16MB;
			mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_256KB;
			printk("Found flash 25FL128P with 256K sector!\n");
		} else {/* 64k byte sector */
			mtd->size = RTL819X_FLASH_SIZE_16MB;
			mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
			printk("Found flash 25FL128P with 64K sector!\n");
		}
	} else if(flash_id.device_id == 0x10215) {/*spansion spi flash:S25FL032P */
		mtd->size      =   RTL819X_FLASH_SIZE_4MB;
		mtd->erasesize =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash SPANSION S25FL032P:4M size,64K sector!\n");
	} else if(flash_id.device_id == 0x10216) {/*spansion spi flash:S25FL64A */
		mtd->size      =   RTL819X_FLASH_SIZE_8MB;
		mtd->erasesize =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash SPANSION S25FL64A:8M size,64K sector!\n");
	} else if(flash_id.device_id == 0xc22015) {/*Mx spi flash:M25L1605D*/
		mtd->size           =   RTL819X_FLASH_SIZE_2MB;
		mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash M25L1605D:2M size,64K sector!\n");
	} else if(flash_id.device_id == 0xc22016)	{		/*Mx spi flash:M25L3205D*/
		mtd->size           =   RTL819X_FLASH_SIZE_4MB;
		mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash M25L3205D:4M size,64K sector!\n");
	} else if(flash_id.device_id == 0xc22017) {/*Mx spi flash:M25L6405D*/
		mtd->size           =   RTL819X_FLASH_SIZE_8MB;
		mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash M25L6405D:8M size,64K sector!\n");
	} else if(flash_id.device_id == 0xc22018) {/*Mx spi flash:25L128AD */
		mtd->size           =   RTL819X_FLASH_SIZE_16MB;
		mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash Mx 25L128AD:16M size,64K sector!\n");
	} else if(flash_id.device_id == 0x202018) {/*Mx spi flash:M25P128 */
		mtd->size           =   RTL819X_FLASH_SIZE_16MB;
		mtd->erasesize    =   RTL819X_FLASH_SECTOR_SIZE_256KB;
		printk("Found flash Mx M25P128:16M size,256K sector!\n");
	} else if(flash_id.device_id == 0xc25e16) {/*Mx spi flash:MX25L3235D*/
		mtd->size = RTL819X_FLASH_SIZE_4MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash MX25L3235D:4MB size,64K sector!\n");
	} else if(flash_id.device_id == 0xC22415) {/*Mx spi flash:M25L1635DM*/
		mtd->size = RTL819X_FLASH_SIZE_2MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash M25L1635DM:2MB size,64K sector!\n");
	} else if(flash_id.device_id == 0xef4016) {/*WinBond spi flash:W25Q32*/
		mtd->size = RTL819X_FLASH_SIZE_4MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash W25Q32:4MB size,64K sector!\n");
    } else if(flash_id.device_id == 0xef4017) {/*WinBond spi flash:W25Q64*/
		mtd->size = RTL819X_FLASH_SIZE_8MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash W25Q64:8MB size,64K sector!\n");
	}else if(flash_id.device_id == 0xef4018) {/*WinBond spi flash:W25Q128FVSIG*/
		mtd->size = RTL819X_FLASH_SIZE_16MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash W25Q128FVSIG:16MB size,64K sector!\n");
	} else if(flash_id.device_id == 0xc84016){ /* GD spi flash: GD25Q32B*/
		mtd->size = RTL819X_FLASH_SIZE_4MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash GD25Q32B:4M size,64K sector!\n");
	}else if(flash_id.device_id == 0xc84017){ /* GD spi flash: GD25Q64BSIG*/
		mtd->size = RTL819X_FLASH_SIZE_8MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash GD25Q64BSIG:8M size,64K sector!\n");	
	}else if(flash_id.device_id == 0xc84018){ /* GD spi flash: GD25Q128B*/
		mtd->size = RTL819X_FLASH_SIZE_16MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash GD25Q128B:16M size,64K sector!\n");		
	}else if(flash_id.device_id == 0x207017){ /* xmc spi flash: XM25QH64A*/
		mtd->size = RTL819X_FLASH_SIZE_8MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Found flash XM25QH64A:8M size,64K sector!\n");
	}else {
		mtd->size = RTL819X_FLASH_SIZE_16MB;
		mtd->erasesize = RTL819X_FLASH_SECTOR_SIZE_64KB;
		printk("Unknow flash ID:0x%x.\n",flash_id.device_id);
	}
	mtd->name               =   RTL819X_FLASH_NAME;
	mtd->type               =   MTD_NORFLASH;
	mtd->flags              =   (MTD_CAP_NORFLASH|MTD_WRITEABLE);
	mtd->writesize		    =   1;
	mtd->owner              =   THIS_MODULE;
	mtd->erase              =   rtl819x_flash_erase;
	mtd->read               =   rtl819x_flash_read;
	mtd->write              =   rtl819x_flash_write;
	np = parse_mtd_partitions(mtd, part_probes, &mtd_parts, 0);
	if(np > 0) {
		ret_val = add_mtd_partitions(mtd, mtd_parts, np);
	} else {
		printk("No partitions found!\n");
	}
out:
	return ret_val;
}

static void __exit rtl819x_flash_exit(void)
{
    /*
     * nothing to do
     */
}

module_init(rtl819x_flash_init);
module_exit(rtl819x_flash_exit);
