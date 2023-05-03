/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : low_level.c
 文件描述 :获取指定端口的MAC地址


 函数列表 :
				tbs_read_mac

 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-7-22
            描述 :

          2 修订: 轩光磊
             日期: 2008-9-10
             描述: 增加WAN WLAN USB端口的MAC地址操作
=========================================================================*/
#include "autoconf.h"
#include <linux/vmalloc.h>
#include <linux/flash_layout_kernel.h>

/*=========================================================================
 Function:		int tbs_read_mac( int id , int offset , unsigned char *mac )

 Description:		获取指定端口的MAC地址
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			id:		指定端口
 				offset:	在指定端口上的偏移量
 				
 Output:			*mac:	指定端口的MAC地址
 
 Return:			0:	获取成功
 				-1:	获取失败
 Others:
 		调用此函数请先为*mac分配内存空间
=========================================================================*/

int tbs_read_mac( int id , int offset , unsigned char *mac )
{	
	sys_config_t		*syscfg;
	unsigned int 		mac_value = 0;
	char				* byte;
	unsigned char		i;

	syscfg = vmalloc( sizeof(sys_config_t) );		
	if ( syscfg == NULL )
	{
		printk(KERN_EMERG"sys_config_t malloc wrong\n");

		return -1;
	}

	memset (syscfg,0,sizeof(sys_config_t));		
		
	if( sysdata_get( syscfg ) !=0 )
	{
		printk(KERN_EMERG"Can't get low config\n");
		vfree( syscfg );

		return -1;
	}
	
	mac_value = ( syscfg->mac[3] << 16 ) | ( syscfg->mac[4] << 8 ) | syscfg->mac[5] ;
	switch ( id ) 
	{
		case LAN_MAC:
		{
#ifdef CONFIG_LAN_MAC_ADDRESS_OFFSET
			mac_value += CONFIG_LAN_MAC_ADDRESS_OFFSET;
#endif
			break;
		}
		case WAN_MAC:
		{
#ifdef CONFIG_WAN_MAC_ADDRESS_OFFSET
			mac_value += CONFIG_WAN_MAC_ADDRESS_OFFSET;
#endif
			break;
		}
		case WLAN_MAC:
		{
#ifdef CONFIG_WLAN_MAC_ADDRESS_OFFSET
			mac_value += CONFIG_WLAN_MAC_ADDRESS_OFFSET;
#endif
			break;
		}
		case USB_MAC:
		{
#ifdef CONFIG_USB_MAC_ADDRESS_OFFSET
			mac_value += CONFIG_USB_MAC_ADDRESS_OFFSET;
#endif
			break;
		}
		default:
		{
			printk(KERN_EMERG"Don't have your MAC\n");
			vfree( syscfg );

			return -1;
		}
	}

	mac_value += offset;
		
	byte = ( char * ) &mac_value;
#ifdef __BIG_ENDIAN
	syscfg->mac[3] = byte[1];
	syscfg->mac[4] = byte[2];
	syscfg->mac[5] = byte[3];
#else
	syscfg->mac[3] = byte[2];
	syscfg->mac[4] = byte[1];
	syscfg->mac[5] = byte[0];
#endif

	for( i = 0 ; i < 6 ; i++ )
	{
		mac[i] = syscfg->mac[i];
	}
	vfree( syscfg );

	return 0;
}

int tbs_read_region(unsigned char *region)
{
    sys_config_t		*syscfg;
    	unsigned char		i;

    	syscfg = vmalloc( sizeof(sys_config_t) );		
	if ( syscfg == NULL )
	{
		printk(KERN_EMERG"sys_config_t malloc wrong\n");

		return -1;
	}

    	memset (syscfg,0,sizeof(sys_config_t));		
		
	if( sysdata_get( syscfg ) !=0 )
	{
		printk(KERN_EMERG"Can't get low config\n");
		vfree( syscfg );

		return -1;
	}

    	for( i = 0 ; i < 2 ; i++ )
	{
		region[i] = syscfg->region[i];
	}
	vfree( syscfg );

	return 0;
    

}

#ifdef PINANDOTHER_ENABLED
int tbs_read_pinandother(unsigned char *pinandother)
{
    sys_config_t        *syscfg;
    unsigned char       i;

    syscfg = vmalloc( sizeof(sys_config_t) );       
    if ( syscfg == NULL )
    {
        printk(KERN_EMERG"sys_config_t malloc wrong\n");

        return -1;
    }

        memset (syscfg,0,sizeof(sys_config_t));     
        
    if( sysdata_get( syscfg ) !=0 )
    {
        printk(KERN_EMERG"Can't get low config\n");
        vfree( syscfg );

        return -1;
    }

        for( i = 0 ; i < PINANDOTHER_LEN; i++ )
    {
        pinandother[i] = syscfg->pinandother[i];
        //printk("pinandother[%d]=%d\n", i, pinandother[i]);
    }
    vfree( syscfg );

    return 0;
    
}
#endif

EXPORT_SYMBOL(tbs_read_mac);
EXPORT_SYMBOL(tbs_read_region);

#ifdef PINANDOTHER_ENABLED
EXPORT_SYMBOL(tbs_read_pinandother);
#endif



