#ifndef _MTD_SPI_PROBE_H_
#define _MTD_SPI_PROBE_H_
#include <linux/kernel.h>

#ifdef _LINUX_KERNEL_H
#include <linux/types.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

struct spi_chip_info
{
    struct spi_flash_params *flpara;
    void (*destroy)(struct spi_chip_info *chip_info);
    int (*read)(unsigned int from, unsigned int to, size_t size);
    int (*write)(unsigned int from, unsigned int to, size_t size);
    int (*erase)(unsigned int addr);
};
#endif

#define ID_MASK                 0xffff
#define SIZE_64KiB      		0x10000
#define SIZE_64MiB              0x4000000
#define SIZE_32MiB              0x2000000
#define SIZE_16MiB              0x1000000
#define SIZE_8MiB       		0x800000
#define SIZE_4MiB       		0x400000
#define SIZE_2MiB       		0x200000

#define CMD_WREN       	        0x06     	/* Write Enable */
#define CMD_WRDI                0x04     	/* Write Disable */
#define CMD_RDSR                0x05     	/* Read Status Register */
#define CMD_RDSR2               0x35     	/* Read Status 2 */
#define CMD_WRSR       	        0x01     	/* Write Status Register */
#define CMD_READ               	0x03     	/* Read Data */
#define CMD_FASTREAD           	0x0B     	/* Fast Read Data */
#define CMD_PP                 	0x02     	/* Page Program */
#define CMD_BE                 	0xD8     	/* or 0x52 Block Erase */
#define CMD_CE                 	0xC7     	/* or 0x60 Chip Erase */
#define CMD_DP                 	0xB9     	/* Deep Power-Down Mode */
#define CMD_RES                	0xAB     	/* Read Electronic Signature */
#define CMD_RDID               	0x9F     	/* Read JEDEC ID */
#define CMD_HPM	            	0xA3     	/* High Performance Mode */
#define CMD_EN4B                0xB7        /* Enter 4byte Mode */
#define CMD_REMS                0x90
#define CMD_DREAD               0x3B        /* Dual Read */
#define CMD_SE                  0x20        /* Sector Erase */
#define CMD_RDSCUR              0x2B
#define CMD_WRSCUR              0x2F
#define CMD_ENSO                0xB1
#define CMD_EXSO                0xC1
#define CMD_REMS2               0xEF
#define CMD_CP                  0xAD
#define CMD_ESRY                0x70
#define CMD_DSRY                0x80

#define SR_WIP                  (1 << 0)    /* Write-in-Progress */
#define SR_WEL                  (1 << 1)    /* Write-enable-latch */
#define SR_BP0                  (1 << 2)    /* Block protect 0 */
#define SR_BP1                  (1 << 3)    /* Block protect 1 */
#define SR_BP2                  (1 << 4)    /* Block protect 2 */
#define SR_EPE                  (1 << 5)    /* Erase/Program error */
#define SR_SRWD                 (1 << 7)    /* Status-register-write-disable */


enum spi_mode {
	SPI_STD_RD			   =	0x0,
	SPI_STD_FAST_RD		   =	0x1,
	SPI_FAST_RD_DUAL_O	   =	0x2,
	SPI_FAST_RD_DUAL_IO	   =	0x3,
	SPI_FAST_RD_QUAD_O	   =	0x4,
	SPI_FAST_RD_QUAD_IO	   =	0x5,
	SPI_BURST_RD_QUAD_IO   =	0x6
};

struct spi_flash_params {
    char *name;
    unsigned long id;              //JEDEC ID
    unsigned long chip_size;       //in bytes
    unsigned long erasesize;       //Erase Size
    unsigned long nr_units;        //Total erase units
    unsigned long page_size;       //in bytes
    int mode;                      //Operation mode;
};

const struct spi_flash_params flash_table[]= {
	{"AT26DF161"   , 0xEF4600, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_RD},
	{"AT26DF161A"  , 0xEF4601, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_RD},
	{"AT26DF161A"  , 0xEF4602, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_RD},
	{"A25L016"     , 0x373015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_RD},
	{"EN25P16"     , 0x1C3015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_FAST_RD_QUAD_IO},
	{"EN25F16"     , 0x1C3115, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},
	{"MX25L1605D"  , 0xc22015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},
	{"MX25L1635D"  , 0xc22415, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},
	{"M25P16"      , 0x202015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},//numonyx
	{"W25X16"      , 0xEF3015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_FAST_RD_DUAL_O},
	{"W25Q16"      , 0xEF4015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_FAST_RD_QUAD_IO},
	{"S25FL016A"   , 0x010214, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},
	{"GD25Q16"     , 0xC84015, SIZE_2MiB , SIZE_64KiB, 32 , 256, SPI_STD_FAST_RD},
	{"AT25DF321"   , 0x1F4700, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"AT25DF321"   , 0x1F4701, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"A25L032"     , 0x373016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"EN25P32"     , 0x1C2016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"EN25F32"     , 0x1C3116, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"EN25Q32"     , 0x1C3316, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"EN25Q32A"    , 0x1C3016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_FAST_RD_QUAD_IO},
	{"M25P32"      , 0x202016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"MX25L3205D"  , 0xc22016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"MX25L3235D"  , 0xc22416, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"N25S32"      , 0xD53016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_RD},
	{"W25X32"      , 0xEF3016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_FAST_RD_DUAL_O},
	{"W25Q32"      , 0xEF4016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_FAST_RD_QUAD_IO},
	{"S25FL032A"   , 0x010215, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"GD25Q32"     , 0xC84016, SIZE_4MiB , SIZE_64KiB, 64 , 256, SPI_STD_FAST_RD},
	{"AT25DF641"   , 0x1F4800, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_RD},
	{"A25L064"     , 0x373017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_RD},
	{"EN25P64"     , 0x1C2017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_RD},
	{"EN25Q64"     , 0x1C3017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_FAST_RD_QUAD_IO},
	{"M25P64"      , 0x202017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_FAST_RD},
	{"MX25L6405D"  , 0xc22017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_FAST_RD},
	{"W25X64"      , 0xEF3017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_FAST_RD_DUAL_O},
	{"W25Q64"      , 0xEF4017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_FAST_RD_QUAD_IO},//FL064K
	{"S25FL064A"   , 0x010216, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_FAST_RD},
	{"GD25Q64"     , 0xC84017, SIZE_8MiB , SIZE_64KiB, 128, 256, SPI_STD_FAST_RD},
	{"GD25Q128"    , 0xC84018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_FAST_RD},
	{"AT25DF128"   , 0x1F4900, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD},
	{"A25L128"     , 0x373018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD},
	{"EN25P128"    , 0x1C2018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD},
	{"EN25Q128"    , 0x1C3018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_FAST_RD_QUAD_IO},
	{"N25Q128"     , 0x20BA18, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD},//3V
	{"N25Q128A"    , 0x20BB18, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD},//1.8V
	{"MX25L12805"  , 0xC22018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_FAST_RD},//MX25L12845
	{"W25X128"     , 0xEF3018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_FAST_RD_DUAL_O},
	{"W25Q128"     , 0xEF4018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_FAST_RD_QUAD_IO},
	{"S25FL128P"   , 0x012018, SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_FAST_RD},//S25L129P
	{"Unknown"     , 0x0     , SIZE_16MiB, SIZE_64KiB, 256, 256, SPI_STD_RD}
};

#endif
