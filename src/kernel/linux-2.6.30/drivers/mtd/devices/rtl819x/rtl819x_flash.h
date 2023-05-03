#ifndef __RTL819X_FLASH_H__
#define __RTL819X_FLASH_H__


typedef struct
{
    unsigned int device_id;
    unsigned short int ext_device_id;
}spi_flash_id;


#define RTL819X_FLASH_MAX_BANKS  1

/*TBS TAG:START
changed by cairong 2012-12-10 
for img大于4M,不能升级的问题
*/
//#define  RTL819X_FLASH_BASE 0xbfc00000
#define  RTL819X_FLASH_BASE 0xbd000000
/*TBS TAG:END*/

#define RTL819X_FLASH_SIZE_2MB          (2*1024*1024)
#define RTL819X_FLASH_SIZE_4MB          (4*1024*1024)
#define RTL819X_FLASH_SIZE_8MB          (8*1024*1024)
#define RTL819X_FLASH_SIZE_16MB         (16*1024*1024)
#define RTL819X_FLASH_SECTOR_SIZE_64KB  (64*1024)
#define RTL819X_FLASH_SECTOR_SIZE_256KB (256*1024)
#define RTL819X_FLASH_PG_SIZE_256B       256
#define RTL819X_FLASH_NAME               "spi_flash"
#define RTL819X_FLASH_CHIP			0

/*
 * Macro Definition
 */
#define SPI_CS(i)           ((i) << 30)   /* 0: CS0 & CS1   1: CS0   2: CS1   3: NONE */
#define SPI_LENGTH(i)       ((i) << 28)   /* 0 ~ 3 */
#define SPI_READY(i)        ((i) << 27)   /* 0: Busy  1: Ready */

#define SPI_CLK_DIV(i)      ((i) << 29)   /* 0: DIV_2  1: DIV_4  2: DIV_6 ... 7: DIV_16 */
#define SPI_RD_ORDER(i)     ((i) << 28)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_WR_ORDER(i)     ((i) << 27)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_RD_MODE(i)      ((i) << 26)   /* 0: Fast-Mode  1: Normal Mode */
#define SPI_SFSIZE(i)       ((i) << 23)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS(i)          ((i) << 19)   /* 0 ~ 15 */
#define SPI_RD_OPT(i)       ((i) << 18)   /* 0: No-Optimization  1: Optimized for Sequential Access */

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)

#endif /*__RTL819X_FLASH_H__*/

