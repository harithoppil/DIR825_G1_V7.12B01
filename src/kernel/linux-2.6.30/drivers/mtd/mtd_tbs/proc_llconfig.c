/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : proc_llconifig.c
 �ļ����� : 	��proc�ļ�ϵͳ��ϵͳ��������Ϣ���в���
 				����MAC��ַ�Ļ�ȡ���޸�
 				��ǰflash��д������image�Ĵ�С


 �����б� :

 �޶���¼ :
          1 ���� : ������
            ���� : 2008-3-27
            ���� :

          2 �޶�: ������
             ����: 2008-10-27
             ����: ɾ������Ҫ���ò���
=========================================================================*/

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/errno.h> 	/* for -EBUSY */
#include <linux/ioport.h>	/* for verify_area */
#include <linux/init.h>		/* for module_init */
#include <asm/uaccess.h>  	/* for get_user and put_user */
#include <linux/delay.h>
#include <mtd/mtd-abi.h>
#include  <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/syscalls.h>

#include <linux/flash_layout_kernel.h>
#include "autoconf.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("tbs llconfig Kernel Module");
MODULE_AUTHOR("xuanguanglei");

/*�˺����ڿ�����image.img���ռ�ļ��*/
#ifndef CONFIG_MTD_JFFS2_SIZE
#define CONFIG_MTD_JFFS2_SIZE 0x40000 //add JFFS2_SIZE 128K + 128K
#endif

unsigned char region_ttl_flg = 0;
struct proc_dir_entry *proc_llconfig = NULL;

/*=========================================================================
 Function:		static int mac_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		�����ϵͳFlash��С

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			MAC��ַ
 Others:
=========================================================================*/

static int mac_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	sys_config_t		syscfg;
	char				*p = buf;
	unsigned int		i;

	sysdata_get(&syscfg);
	for(i=0;i<6;i++)
	{
		p += sprintf(p, "%02x", syscfg.mac[i]);
	}

	return ( p - buf );
}
/*=========================================================================
 Function:		static int region_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		��ȡregion code

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			MAC��ַ
 Others:
=========================================================================*/

static int region_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	sys_config_t		syscfg;
	char				*p = buf;
	unsigned int		i;

	sysdata_get(&syscfg);
	for(i=0;i<2;i++)
	{
		p += sprintf(p, "%c", syscfg.region[i]);
	}

	return ( p - buf );
}

/*=========================================================================
 Function:		static int region_read_flg (void)
 Description:		��ȡregion code

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			MAC��ַ
 Others:
=========================================================================*/

static int region_read_flg(void)
{
	sys_config_t		syscfg;

	sysdata_get(&syscfg);
    if(!strncmp(syscfg.region, "PR", 2) || !strncmp(syscfg.region, "CN", 2))
	    region_ttl_flg = 1;
    else
        region_ttl_flg = 0;

	return region_ttl_flg;
}

/*=========================================================================
 Function:		ssize_t mac_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
 Description:		��MAC��ַд��������

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			д���MAC��ַ����
 Others:
=========================================================================*/

ssize_t region_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
{

	sys_config_t			syscfg;
	unsigned int			i;

      printk("region_write len=%d\n", len);
      
	if( ( len > 3 ) || ( len < 2 ) )
	{
		printk("your region code is not suitability\n");
		return -1;
	}
    
    	/*��ȡ�ײ���Ϣ*/
	sysdata_get(&syscfg);
       printk("region_write lhere\n");
	for(i=0;i<2;i++)
	{
		syscfg.region[i] = *(buff+i);
	}

	/*����ײ���Ϣ*/
	sysdata_save( &syscfg );

	return len;
}


#ifdef PINANDOTHER_ENABLED
static int pinandother_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    sys_config_t        syscfg;
    char                *p = buf;
    unsigned int        i;
    sysdata_get(&syscfg);
    for(i=0;i<PINANDOTHER_LEN;i++)
    {
        p += sprintf(p, "%c", syscfg.pinandother[i]);
    }

    return ( p - buf );
}
/*=========================================================================
 Function:      ssize_t mac_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
 Description:       ��MAC��ַд��������

 Data Accessed:
 Data Updated:

 Input:         ��
 Output:            ��
 Return:            д���MAC��ַ����
 Others:
=========================================================================*/

ssize_t pinandother_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
{

    sys_config_t            syscfg;
    unsigned int            i;

      printk("pinandother_write len=%d\n", len);
      
   // if( ( len > 3 ) || ( len < 2 ) )
   // {
       // printk("your region code is not suitability\n");
       // return -1;
   // }
    
        /*��ȡ�ײ���Ϣ*/
    sysdata_get(&syscfg);
    printk("pinandother_write here\n");
    for(i=0;i<PINANDOTHER_LEN;i++)
    {
        syscfg.pinandother[i] = *(buff+i);
    }

    /*����ײ���Ϣ*/
    sysdata_save( &syscfg );

    return len;
}
#endif

/*=========================================================================
 Function:		ssize_t region_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
 Description:		��mac_write д��������

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			
 Others:
=========================================================================*/

ssize_t mac_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
{
	char					mac_buf[12];
	sys_config_t			syscfg;
	unsigned int			i;
	unsigned int			j;
	unsigned char			mac_right=0;

	if( ( len > 13 ) || ( len < 12 ) )
	{
		printk("your mac is not suitability\n");
		return -1;
	}

	for(i=0;i<12;i++)
	{
		mac_buf[i] = *(buff+i);
	}

	/*���ַ���ת��Ϊʮ������*/
	for( i = 0 ; i < 12 ; i++ )
	{
		if( mac_buf[i] >= 65 && mac_buf[i] <= 70 )       /*'A'<=argv[j]<='Z'*/
			mac_buf[i] -= 55;
		else if( mac_buf[i] >= 97 && mac_buf[i] <= 102 )     /*'a'<=argv[j]<='z'*/
			mac_buf[i] -= 87;
		else if( mac_buf[i] >= 48 && mac_buf[i] <= 57 )      /*'0'<=argv[j]<='9'*/
			mac_buf[i] -= 48;
		else
		{
			printk("your mac is not suitability\n");
			mac_right = 1;

			break;
			return -1;
		}
	}

	/*��ȡ�ײ���Ϣ*/
	sysdata_get(&syscfg);

	/*���µ�MACд��������Ϣ*/
	if(mac_right!=1)
	{
		for(i=0,j=0;i<12;)
		{
			syscfg.mac[j] = mac_buf[i] * 16 + mac_buf[i+1];
			j++;
			i += 2;
		}
	}

	/*����ײ���Ϣ*/
	sysdata_save( &syscfg );

	return len;
}

/*=========================================================================
 Function:		static int img_space_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		�����ϵͳFlash��С

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			Flash��С
 Others:
=========================================================================*/

static int img_space_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	long				fp_mtd0 = -1;
	sys_config_t		syscfg;
	mm_segment_t	fs;
	flash_info_t		flash_info_low;
	char				*p = buf;

	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡFlash�������*/
	if( (fp_mtd0 = sys_open("/dev/mtd0",O_RDONLY,0)) < 0)
	{
		printk(KERN_EMERG"\nUnable to Open the /dev/mtd0 partition\n");
		set_fs (fs);

		return EFAULT;
	}

	/*ͨ��IOCTRL��ȡFlash��Ϣ*/
	if (sys_ioctl(fp_mtd0, FLASHINFO, (u_long) &flash_info_low) < 0)
	{
		printk(KERN_EMERG"\nread flashinfo failure!\n");
		sys_close(fp_mtd0);
		set_fs (fs);
		return EFAULT;
	}

	/*��ȡ�ײ���Ϣ*/
	sysdata_get(&syscfg);

	/*����ϵͳ�ǲ���˫���ݷֱ����ʣ��ռ�*/
#ifdef CONFIG_DOUBLE_BACKUP

	p += sprintf(p, "%x", (unsigned int)( flash_info_low.size - CONFIG_MTD_JFFS2_SIZE - syscfg.layout.zone_offset[ZONE_KERNEL_SECOND] ) );

#else

	p += sprintf(p, "%x", (unsigned int)( flash_info_low.size - CONFIG_MTD_JFFS2_SIZE - syscfg.layout.zone_offset[ZONE_KERNEL_FIRST] ) );

#endif
	sys_close(fp_mtd0);
	set_fs (fs);

	return ( p - buf );
}

/*=========================================================================
 Function:		static int flash_size_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		�����ϵͳFlash��С

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			Flash��С
 Others:
=========================================================================*/

static int flash_size_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	long				fp_mtd0 = -1;
	mm_segment_t	fs;
	flash_info_t		flash_info_low;
	char				*p = buf;

	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡFlash�������*/
	if( (fp_mtd0 = sys_open("/dev/mtd0",O_RDONLY,0)) < 0)
	{
		printk(KERN_EMERG"\nUnable to Open the /dev/mtd0 partition\n");
		set_fs (fs);

		return EFAULT;
	}

	/*ͨ��IOCTRL��ȡFlash��Ϣ*/
	if (sys_ioctl(fp_mtd0, FLASHINFO, (u_long) &flash_info_low) < 0)
	{
		printk(KERN_EMERG"\nread flashinfo failure!\n");
		sys_close(fp_mtd0);
		set_fs (fs);
		return EFAULT;
	}
	p += sprintf(p, "%x", (unsigned int)flash_info_low.size );

	sys_close(fp_mtd0);
	set_fs (fs);

	return ( p - buf );
}


static int model_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    sys_config_t        syscfg;
    unsigned int			i;
    char                *p = buf;
    sysdata_get(&syscfg);
    // p+=sprintf(p, "%s", syscfg.model_name);
    for(i=0;i<16;i++)
    {
        p += sprintf(p, "%c", syscfg.model_name[i]);
    }
    return ( p - buf );
}

//read customer version
static int version_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    char                *p = buf;

    p += sprintf(p, "%s\n", CONFIG_SW_VERSION);
    
    return ( p - buf );
}

//don't accept setting of customer version
static ssize_t version_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
{
    printk("Don't accept setting\n");
    return -1;
}


ssize_t model_write( struct file *filp, const char __user *buff,unsigned long len, void *data )
{

    sys_config_t            syscfg;
    unsigned int			i;
    printk("Model Name Length=%d\n", len);      
    if ( len > 16 )
    {
        printk("your Model Name is too long\n");
        return -1;
    }
    
    /*��ȡ�ײ���Ϣ*/
    sysdata_get(&syscfg);
    //sprintf(syscfg.model_name, "%s", *buff);
    for(i=0;i<len;i++)
    {
        syscfg.model_name[i] = *(buff+i);
    }

    /*model_nameռ16���ֽڣ����ಹ0*/
    for(i=len;i<16;i++)
    {
        syscfg.model_name[i] = 0;
    }

    /*����ײ���Ϣ*/
    sysdata_save(&syscfg );

    return len;
}

/*=========================================================================
 Function:	sn_read
 Input:		��
 Output:	��
 Return:	MAC��ַ
 Others:	
=========================================================================*/
static int sn_read (char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	sys_config_t		syscfg;
	char				*p = buf;
	unsigned int		i,j;
	j=15;

	sysdata_get(&syscfg);
	for(i=0;i<j;i++)
	{
		p += sprintf(p, "%c", syscfg.sn[i]);
	}
	return ( p - buf );
}

/*=========================================================================
 Function:	sn_write
 Input:		��
 Output:	��
 Return:	MAC��ַ
 Others:	
=========================================================================*/
ssize_t sn_write( struct file *filp, const char __user *buff,unsigned long len
, void *data )
{
	char					sn_buf[20];
	sys_config_t			syscfg;
	unsigned int			i,j;

	j = len > 19 ? 19 : len;

	for(i=0; i < j; i++)
	{
		sn_buf[i] = *(buff+i);
	}

	/*��ȡ�ײ���Ϣ*/
	sysdata_get(&syscfg);

	/*���µ�snд��������Ϣ*/
	
	for(i=0; i < j; i++)
	{
		syscfg.sn[i] = sn_buf[i];
	}
	
	syscfg.sn[i] = '\0';
	
	/*����ײ���Ϣ*/
	sysdata_save( &syscfg );

	return j;
}



/*=========================================================================
 Function:		static int __init llconfig_module_init( void )
 Description:		����proc/llconfig�ļ�

 Data Accessed:
 Data Updated:

 Input:			��
 Output:			��
 Return:			0:		�����ɹ�
 				����:	����ʧ��
 Others:
=========================================================================*/

static int __init llconfig_module_init( void )
{
	struct proc_dir_entry *mac_entry;
    struct proc_dir_entry *region_entry;
    struct proc_dir_entry *model_entry;
#ifdef PINANDOTHER_ENABLED
    struct proc_dir_entry *pinandother_entry;
#endif
	struct proc_dir_entry *img_space;
	struct proc_dir_entry *flashinfo;
	struct proc_dir_entry *sn_entry;

	proc_llconfig = proc_mkdir("llconfig", NULL);

	mac_entry = create_proc_entry( "macaddr", 0644, proc_llconfig );
	mac_entry->read_proc = mac_read;
	mac_entry->write_proc= mac_write;

    region_entry = create_proc_entry( "region", 0644, proc_llconfig );
	region_entry->read_proc = region_read;
	region_entry->write_proc= region_write;

    model_entry = create_proc_entry( "model", 0644, proc_llconfig );
	model_entry->read_proc = model_read;
	model_entry->write_proc= model_write;

    model_entry = create_proc_entry( "version", 0444, proc_llconfig );
	model_entry->read_proc = version_read;
    model_entry->write_proc= version_write;
    
#ifdef PINANDOTHER_ENABLED
    pinandother_entry = create_proc_entry( "pinandother", 0644, proc_llconfig );
	pinandother_entry->read_proc = pinandother_read;
	pinandother_entry->write_proc= pinandother_write;
#endif

	img_space = create_proc_entry( "img_space", 0644, proc_llconfig );
	img_space->read_proc = img_space_read;

	flashinfo = create_proc_entry( "flash_size", 0644, proc_llconfig );
	flashinfo->read_proc = flash_size_read;

	//gxw / 2014-11-04 / sn read/set
    sn_entry = create_proc_entry( "serialnumber", 0644, proc_llconfig );
	sn_entry->read_proc = sn_read;
	sn_entry->write_proc= sn_write;

        /* 
         *��������region_ttl_flg����,û��ʵ��ʹ��,
         *����MTD����δ��ɶ�ȡ����,����Kernel panic
         */
        //region_read_flg();

	return 0 ;
}

/*=========================================================================
 Function:		void llconfig_module_clean( void )
 Description:		ע��proc/llconfig�ļ�

 Data Accessed:
 Data Updated:	proc_llconfig

 Input:			��
 Output:			��
 Return:
 Others:
=========================================================================*/

void llconfig_module_clean( void )
{
	remove_proc_entry("tbs_mac", proc_llconfig);
    remove_proc_entry("region", proc_llconfig);

#ifdef PINANDOTHER_ENABLED    
    remove_proc_entry("pinandother", proc_llconfig);
	remove_proc_entry("img_space", proc_llconfig);
	remove_proc_entry("flash_size", proc_llconfig);
#endif

	remove_proc_entry("llconfig", NULL);
	printk(KERN_INFO "llconfig_module_clean: Module unloaded.\n");
}


module_init( llconfig_module_init );
module_exit( llconfig_module_clean );

/*=========================================================================
					File End
=========================================================================*/


