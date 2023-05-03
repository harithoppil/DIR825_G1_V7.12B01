/*
 *  TBS ioctl 命令字定义
 *
 *
 *  说明:TBS私有ioctl使用使用G字段生成的命令字
 *
 *       by Zhang Yu
 *
 *
 *	添加TBS条目的获取和保存IOCTL号
 *	2008-10-27 by xuanguanglei
 *
 */

#ifndef __LINUX_TBS_IOCTL_H_INCLUDED
#define __LINUX_TBS_IOCTL_H_INCLUDED
#include "autoconf.h"


#define TBS_IOCTL_MAGIC	 'G'

/* ioctl led命令字定义 */

#define TBS_IOCTL_LED_SET		_IOWR(TBS_IOCTL_MAGIC, 0, struct tbs_ioctl_led_parms)
#define TBS_IOCTL_LED_GET		_IOWR(TBS_IOCTL_MAGIC, 1, struct tbs_ioctl_led_parms)

#define TBS_IOCTL_ITEM_GET		_IOWR(TBS_IOCTL_MAGIC, 2, item_t)
#define TBS_IOCTL_ITEM_SAVE	_IOWR(TBS_IOCTL_MAGIC, 3, item_t)

#define TBS_IOCTL_MAC_READ		_IOWR(TBS_IOCTL_MAGIC, 4, struct mtd_mac)

#define TBS_IOCTL_CFGINFO_GET      	 _IOWR(TBS_IOCTL_MAGIC, 5,int )
#define TBS_IOCTL_REGION_READ		_IOWR(TBS_IOCTL_MAGIC, 5, region_t)
#define TBS_IOCTL_IMAGE_SYNC    _IOWR(TBS_IOCTL_MAGIC, 6, item_t) 

#define TBS_IOCTL_ITEMINFO_GET      	 _IOWR(TBS_IOCTL_MAGIC, 6,int )

#ifdef CONFIG_APPS_UPDATE_UBOOT
#define TBS_IOCTL_UPDATE_UBOOT      	 _IOWR(TBS_IOCTL_MAGIC, 7,int ) //add by wyh start 2016-01-26 to support the function that update the uboot while the kernel up
#endif


//SO_BINDDEVICE define in src/kernel/linux-2.6.36/arch/mips/include/asm/socket.h:89
#define SO_BINDDEVICE           50
//SIOCGRECVIF define in src/kernel/linux-2.6.36/include/linux/sockios.h:29
#define SIOCGRECVIF             0x8908

#endif /* __LINUX_TBS_IOCTL_H_INCLUDED */

