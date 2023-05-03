/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : flash_layout_kernel.c
 �ļ����� : flash layout��ز���

 �����б� :
				sysdata_get
				sysdata_save
				item_get
				item_save
				item_repeat
				flash_update_bin
				flash_update_img				

 �޶���¼ :
          1 ���� : ������
            ���� : 2008-7-22
            ���� :

          2 �޶�: ������
             ����: 2008-9-10
             ����: ��������Ϣ��Ϊ��Ŀ����

           3 �޶�: ������
             ����: 2009-2-18
             ����: ����BIN file����

           4 �޶�: ������
             ����: 2009-3-16
             ����: �Ż���Ŀ�Ĳ���

           5 �޶�: ������
             ����: 2009-5-18
             ����: �Ż���������

           6�޶�: ������
             ����: 2012-04-01
             ����: �������bin�ļ�ʱ����mib_rf��MAC�s��sn�Ĺ���

=========================================================================*/

#include <linux/syscalls.h>
#include <linux/vmalloc.h>
#include <linux/flash_layout_kernel.h>
#include "autoconf.h"

#define READ_ITEM_LEN ( 0x1000 )
#define MAX_FLASH_LEN (3932159)    //4M-256K

/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin start*/
#ifdef BACKUP_ITEM_MIB_RF_MAC
static char MIB_RF_PARAM[][32] = { 
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_CCK_B",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_HT40_1S_B",
	"HW_DIFF_HT40_2S",
	"HW_DIFF_HT20_A",
	"HW_DIFF_OFDM",
	"HW_11N_THER"
};
static mib_rf  mib_rf_data;
static llconfig_syscfg_t syscfg;
static void  mib_rf_config_get(  mib_rf *data);
static void  mib_rf_config_set(  mib_rf *data);
static void  llconfig_get_syscfg(  llconfig_syscfg_t *data);
static void  llconfig_set_syscfg(  llconfig_syscfg_t *data);
#endif
#ifdef CONFIG_FLASH_WRITE_PROTECT
 extern char kernal_flash_protect(char isEnable);
#endif
/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin end*/

/*=========================================================================
��Ŀ��д�ź���
=========================================================================*/

DECLARE_MUTEX(item_sem);



/*=========================================================================
 Function:		int  sysdata_get( sys_config_t* syscfg )
 Description:		��ȡ�ײ�llconfig����
 Data Accessed:
 Data Updated:
 Input:			
 Output:			
 Return:			0:�ɹ�
				1:ʧ��
 Others:
 				���ô˺���ǰ��Ϊsyscfg�����ڴ�ռ�
=========================================================================*/

int  sysdata_get(sys_config_t* syscfg)
{
	unsigned short item_len;
	int ret;

	item_len = SYSCFG_SIZE;
	ret = item_get( syscfg, LLCONFIG_NAME , &item_len );
	if( ret == ERROR_ITEM_OK )
	{
		return 0;
	}
	return 1;
}

/*=========================================================================
 Function:		int  sysdata_save(sys_config_t * syscfg)
 Description:		����ײ�llconfig����
 Data Accessed:
 Data Updated:
 Input:			
 Output:			
 Return:			0:�ɹ�
				1:ʧ��
 Others: 				
=========================================================================*/

int  sysdata_save(sys_config_t * 	syscfg)
{
	if( item_save( syscfg , LLCONFIG_NAME , SYSCFG_SIZE ) == ERROR_ITEM_OK )
	{
		return 0;
	}
	return 1;
}

/*=========================================================================
 Function:		int item_check( void )

 Description:		���ÿ����Ŀ��������
 Data Accessed:
 Data Updated:
 Input:
 Output:
 Return:			ERROR_ITEM_OK
				ERROR_CONFIG_LOST
 Others:
=========================================================================*/

int item_check( flash_info_t* flash_info_low , struct mtd_info *mtd_master  )
{
	item_t			*item;
	unsigned short	checksum;
	unsigned int		i = 0;
	unsigned int		sum = 0;
	unsigned int		retlen;
	unsigned char		*item_sdram;

		/* �ҵ�Flash��������������������ʼ��ַ*/	
	while( ( flash_info_low->start[i] - flash_info_low->start[0] ) < BOOTCODE_LENGTH )
		i++;
		
	item_sdram = vmalloc( flash_info_low->start[i+1] - flash_info_low->start[i] );
	if ( item_sdram == NULL )
	{		
		printk("item_sdram malloc wrong\n");
		
		return ERROR_ITEM_MALLOC;
	}
	
	mtd_master->read( mtd_master , flash_info_low->start[i] , (flash_info_low->start[i+1] - flash_info_low->start[i]) , &retlen, item_sdram );

	sum = CONFIG_MARK_LEN;
	while( ( sum + ITEM_HEAD_LEN ) <= ( flash_info_low->start[i+1] - flash_info_low->start[i] ) )
	{
		//mtd_master->read( mtd_master , flash_info_low->start[i] + sum , READ_ITEM_LEN , &retlen, item_sdram );
		
		item = ( item_t * ) ( item_sdram + sum );
		//item = ( item_t * ) ( item_sdram );
		
		if( item->hdr.avail == 0xff )
		{
			/*��Ŀ������,�ô���ĿΪ��Ч,���ҿ��Կ϶��������һ����Ŀ*/
			if( check_addr_null( ( unsigned int * )item , (flash_info_low->start[i+1] - flash_info_low->start[i]-sum), mtd_master) == 1 )
			{
				printk("[%s:%d]:Error item, so recopy from backup setor.\n", __FUNCTION__, __LINE__);
				vfree(item_sdram);
				return ERROR_CONFIG_LOST;
			}
			else /*�϶��������һ����Ŀ*/
				break;
		}
#if 1
		if((((item->hdr.avail != ITEM_UNAVAIL) && (item->hdr.avail != ITEM_BAD) && (item->hdr.avail != ITEM_AVAIL ))||( item->hdr.len ==0)))
		{
			printk("[%s:%d]:avail=%0x,Error item, so recopy from backup setor.\n", __FUNCTION__, __LINE__,item->hdr.avail);
			vfree(item_sdram);
			return ERROR_CONFIG_LOST;
		}
#endif
		
		if( ( item->hdr.avail == ITEM_UNAVAIL ) )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}
				
		/*��Ч��Ŀ*/
		if( item->hdr.avail == ITEM_AVAIL )
		{
			tbs_crc16( item->data , item->hdr.len ,&checksum );
			if( checksum == item->hdr.crc )		/*��Ŀ����,ָ����һ��Ŀ*/
			{
				sum += ITEM_SIZE( item->hdr.len );

				continue;
			}
			else		/*��Ŀ������,�ô���ĿΪ��Ч,���ҿ��Կ϶��������һ����Ŀ*/
			{
				printk("[%s:%d]:Error item, so recopy from backup setor.\n", __FUNCTION__, __LINE__);
				vfree(item_sdram);
				return ERROR_CONFIG_LOST;
			}
		}
	}

	vfree(item_sdram);
	return ERROR_ITEM_OK;
}


/*=========================================================================
 Function:		int item_get( void *data , char *item_name ,unsigned short *len ) 

 Description:		��ȡָ����Ŀ������
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		��Ŀ���ݵĴ��ָ��
 Output:			*len:		��Ŀ��Ч���ݵĳ���
 
 Return:			ERROR_ITEM_OK 
				ERROR_MALLOC
				ERROR_NOT_FIND
				ERROR_BIG
				ERROR_FLASH_BUSY
 Others:
 		�ϲ���ô˺���ʱ*len��������ֵ���ϲ��ܽ��յ���󳤶ȡ�
 ��� ��Ч����ʵ�ʳ��ȴ���*len��data�Ͳ���ֵ�����ҷ��ش��󣬲���
 ��Ŀ�� Ч���ݵĳ���ͨ��*len�����ϲ㡣
 �����Ч����ʵ�ʳ��Ȳ�����*len���ͽ����ݸ���data������ʵ�ʳ���ͨ
 ��*len�����ϲ�
=========================================================================*/

int item_get( void *data , char *item_name ,unsigned short *len )
{
	item_t				*item;
	unsigned char			*item_sdram;
	unsigned int 			i = 0;
	unsigned int			sum = 0;
	mm_segment_t		fs;
	flash_info_t			*flash_info_low;
	struct mtd_info		*mtd;
	struct mtd_part		*part ;
	unsigned int			retlen;
	
	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡMTD������Ϣ*/
	mtd = get_mtd_device( NULL, 0 );
	part = PART(mtd);
		
	flash_info_low = vmalloc( sizeof( flash_info_t ) );
	if ( flash_info_low == NULL )
	{
		printk(KERN_EMERG"flash_info_low malloc wrong\n");
		set_fs (fs);
		
		return ERROR_ITEM_MALLOC;
	}

	/* ��ȡFlash�����б�*/
	flash_info_get( flash_info_low ,  part->master );

	/* �ҵ�Flash��������������������ʼ��ַ*/
	while( ( flash_info_low->start[i] - flash_info_low->start[0] ) < BOOTCODE_LENGTH )
		i++;

	item_sdram = vmalloc( READ_ITEM_LEN );
	if ( item_sdram == NULL )
	{
		set_fs (fs);
		vfree( flash_info_low );
		
		printk("item_sdram malloc wrong\n");
		
		return ERROR_ITEM_MALLOC;
	}

      if(down_interruptible(&item_sem))
      {
              set_fs (fs);
              vfree(flash_info_low);
              vfree(item_sdram);
              printk("Item get fail,because flash is busy.\n");
              
              return ERROR_FLASH_BUSY;
      }
      
	sum = CONFIG_MARK_LEN;

	/* ������Ŀ*/
	while( ( sum + ITEM_HEAD_LEN ) <= ( flash_info_low->start[i+1] - flash_info_low->start[i] ) )
	{
		part->master->read( part->master , flash_info_low->start[i] + sum , READ_ITEM_LEN , &retlen, item_sdram );
		item = ( item_t * ) item_sdram;

		/*���ÿռ�Ѱ�����*/
		if( item->hdr.avail == 0xff )
		{
			break;
		}

		/*��Ŀ��Ч,ָ����һ��Ŀ*/
		if( item->hdr.avail == ITEM_UNAVAIL )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		/*��Ŀ�Ѿ��ǻ���,ָ����һ��Ŀ*/
		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		/* �ҵ���Ч��Ŀ*/
		if( item->hdr.avail == ITEM_AVAIL )
		{
			/* �Ա���Ŀ����,���ж��Ƿ�ΪҪѰ�ҵ�*/
			if( strcmp( item->data , item_name ) == 0 )
			{
				/* �����Ŀ�е����ݳ����Ƿ�����ϲ������ڴ�ռ�*/
				if( ( item->hdr.len - strlen( item_name ) - 1 )  >  *len )
				{
					/* �������,����ʧ�ܲ���ʵ����Ҫ�ĳ���֪ͨ�ϲ�*/
					*len = item->hdr.len - strlen( item_name ) - 1;
					set_fs (fs);
					vfree(flash_info_low);
					vfree(item_sdram);
					up(&item_sem);
                    
					return ERROR_ITEM_BIG;					
				}
				else
				{
					/* �����ݶ�ȡ����*/
					*len = item->hdr.len - strlen( item_name ) - 1;
					
					if( ITEM_SIZE( item->hdr.len ) > READ_ITEM_LEN )
					{
						part->master->read( part->master , flash_info_low->start[i] + sum + ITEM_HEAD_LEN + strlen( item_name ) + 1 , *len , &retlen, ( unsigned char * )data );
					}
					else
					{
						memcpy( data, item->data + strlen( item_name ) + 1 , *len );						
					}

					set_fs (fs);
					vfree(flash_info_low);
					vfree(item_sdram);
                                   up(&item_sem);

					return ERROR_ITEM_OK;

				}
			}

			/*�������Ҫ�ҵ���Ŀ,����Ѱ��*/
			sum += ITEM_SIZE( item->hdr.len );
				
			continue;
		}	
#if 1
		if((((item->hdr.avail != ITEM_UNAVAIL) && (item->hdr.avail != ITEM_BAD) && (item->hdr.avail != ITEM_AVAIL ))||( item->hdr.len == 0)))
		{
			printk("[%s:%d]:name=%s,avail=%0x,Error item\n", __FUNCTION__, __LINE__,item_name, item->hdr.avail);
			vfree(item_sdram);
			return ERROR_CONFIG_LOST;
		}
#endif
	}

	set_fs (fs);
	vfree(flash_info_low);
	vfree(item_sdram);
	up(&item_sem);
    
	return ERROR_ITEM_NOT_FIND;
}

/*=========================================================================
 Function:		int item_save( void *data , char *item_name ,unsigned short len ) 

 Description:		��ȡָ����Ŀ������

 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		Ҫ��ŵ���Ŀ���ݵ�ָ�� 				
				len:		��Ŀ��Ч���ݵĳ���
 Output:			��
 Return:			ERROR_ITEM_OK 
				ERROR_ITEM_MALLOC				
				ERROR_ITEM_BIG
				ERROR_ITEM_REPEAT_FAIL
				ERROR_FLASH_BUSY
 Others:
=========================================================================*/
int item_save( void *data , char *item_name ,unsigned short len )
{
	item_t				*item;
	unsigned char			*item_sdram;
	unsigned int 			i = 0;
	unsigned int 			j = 0;
	unsigned int			sum = 0;
	item_t				*item_data;
	unsigned short		checksum;	
	char					avail;
	unsigned int			item_repeat_addr[2];

	mm_segment_t		fs;
	flash_info_t			*flash_info_low;
	struct mtd_info		*mtd;
	struct mtd_part		*part ;
	unsigned int			retlen;

	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡMTD������Ϣ*/
	mtd = get_mtd_device( NULL, 0 );
	part = PART(mtd);
		
	flash_info_low = vmalloc( sizeof( flash_info_t ) );
	if ( flash_info_low == NULL )
	{
		printk(KERN_EMERG"flash_info_low malloc wrong\n");
		set_fs (fs);
		
		return ERROR_ITEM_MALLOC;
	}

	/* ��ȡFlash�����б�*/
	flash_info_get( flash_info_low ,  part->master );

	/* �ҵ�Flash��������������������ʼ��ַ*/
	while( ( flash_info_low->start[i] - flash_info_low->start[0] ) < BOOTCODE_LENGTH )
		i++;

	item_sdram = vmalloc(READ_ITEM_LEN );
	if ( item_sdram == NULL )
	{
		set_fs (fs);
		vfree( flash_info_low );
		
		printk("item_sdram malloc wrong\n");
		
		return ERROR_ITEM_MALLOC;
	}

      if(down_interruptible(&item_sem))
      {
              set_fs (fs);
              vfree(flash_info_low);
              vfree(item_sdram);
              printk("Item save fail,because flash is busy.\n");

          return ERROR_FLASH_BUSY;
     }
	//add by wyh start at 2016-01-30:У����������Ŀ����Ч�ԣ���֤��������ȷ������²Ž�����ת
	if (item_check(flash_info_low, part->master) == ERROR_CONFIG_LOST)
	{
		part->master->read( part->master , flash_info_low->start[i+1], CONFIG_MARK_LEN , &retlen, item_sdram );

		if (!strcmp(item_sdram, CONFIG_MARK))
		{
			printk("[%s:%d]:Do the recopy operation from backup setor.\n", __FUNCTION__, __LINE__);
			/* ����������*/ 
			kernel_erase( part->master , flash_info_low , flash_info_low->start[i] , flash_info_low->start[i+1] - flash_info_low->start[i] );
			/* ����Ŀд��������*/
			kernel_write( part->master , ( char * )( flash_info_low->start[i+1] + CONFIG_MARK_LEN ) , flash_info_low->start[i] + CONFIG_MARK_LEN , flash_info_low->start[i+1] - flash_info_low->start[i]-CONFIG_MARK_LEN );
			kernel_write( part->master , CONFIG_MARK , flash_info_low->start[i] , CONFIG_MARK_LEN );	
		}
		else
		{
			printk("[%s:%d]:ERR---Repeat fail\n", __FUNCTION__, __LINE__);
			set_fs (fs);
            vfree(flash_info_low);
            vfree(item_sdram);
			up(&item_sem);
			return ERROR_ITEM_REPEAT_FAIL;
		}
	}
	//add by wyh end
      
	sum = CONFIG_MARK_LEN;

	/* Ѱ�ҿ��õ����ÿռ�*/
	while( ( sum + ITEM_HEAD_LEN ) <= ( flash_info_low->start[i+1] - flash_info_low->start[i] ) )
	{
		part->master->read( part->master , flash_info_low->start[i] + sum , READ_ITEM_LEN , &retlen, item_sdram );
		item = ( item_t * ) item_sdram;

		/*�ҵ�FLASH�յ�ַ*/
		if( item->hdr.avail == 0xff )
		{
			break;
		}

		if( ( item->hdr.avail == ITEM_UNAVAIL ) )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		if( item->hdr.avail == ITEM_AVAIL )
		{
			/*���ֽ�Ҫ��Ϊ��Ч����Ŀ,���������ĵ�ַ*/
			if( strcmp( item->data , item_name ) == 0 )
			{
				item_repeat_addr[j++] = flash_info_low->start[i] + sum + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;

				/*���������ظ�����Ŀ,��ǰһ����Ŀ��Ϊ��Ч*/
				if( j == 2 )
				{
					avail = ITEM_UNAVAIL;
				#ifdef CONFIG_FLASH_WRITE_PROTECT
					kernal_flash_protect(0);
				#endif
					kernel_write( part->master , ( char * )&avail , item_repeat_addr[0] , 0x1 );
				#ifdef CONFIG_FLASH_WRITE_PROTECT
					kernal_flash_protect(1);
				#endif
					item_repeat_addr[0] = item_repeat_addr[1];
					j = 1;
				}
			}
			
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}		
	}
	
	/*��������Ϣ�ϳ���Ŀ*/
	item_data = vmalloc( ITEM_HEAD_LEN + strlen( item_name ) + 1 + len );
	if ( item_data == NULL )
	{
		set_fs (fs);
		vfree( flash_info_low );
		vfree( item_sdram );
		up(&item_sem);
		printk("item_data malloc wrong\n");
		
		return ERROR_ITEM_MALLOC;
	}

	item_data->hdr.avail = ITEM_AVAIL;	
	memcpy( item_data->data , item_name , strlen( item_name ) );
	memcpy( item_data->data + strlen( item_name ) , "\0" , 1 );	
	memcpy( item_data->data + strlen( item_name ) + 1 , data , len );	
	item_data->hdr.len = strlen( item_name ) + 1 + len;
	tbs_crc16( item_data->data , item_data->hdr.len ,&checksum );
	item_data->hdr.crc = checksum;

	/*�ж�����Ŀ�Ĵ�С�Ƿ񳬳���������ʣ��ռ�*/
	if( ITEM_SIZE( strlen( item_name ) + 1 + len ) > ( flash_info_low->start[i+1] - flash_info_low->start[i] - sum ) )
	{
		/*��,ͨ����ת�ķ�ʽ���������е���Ч�ͻ���Ŀ����*/
		if( item_repeat( flash_info_low , part->master ) != ERROR_ITEM_REPEAT_OK )
		{
			set_fs (fs);
			vfree( flash_info_low );
			vfree( item_sdram );
			vfree( item_data );
			up(&item_sem);
            
			return ERROR_ITEM_REPEAT_FAIL;
		}
		
		j = 0;
		sum = CONFIG_MARK_LEN;

		/* ��ת���ٴ�Ѱ�ҿ��õ����ÿռ�*/
		while( ( sum + ITEM_HEAD_LEN ) <= ( flash_info_low->start[i+1] - flash_info_low->start[i] ) )
		{
			part->master->read( part->master , flash_info_low->start[i] + sum , READ_ITEM_LEN , &retlen, item_sdram );
			item = ( item_t * ) item_sdram;		

			/*�ҵ�FLASH�յ�ַ*/
			if( item->hdr.avail == 0xff )
			{
				break;
			}

			if( item->hdr.avail == ITEM_AVAIL )
			{
				/*���ֽ�Ҫ��Ϊ��Ч����Ŀ,���������ĵ�ַ*/
				if( strcmp( item->data , item_name ) ==0 )
				{
					item_repeat_addr[j++] = flash_info_low->start[i] + sum + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;
				}
				
				sum += ITEM_SIZE( item->hdr.len );
				
				continue;
			}			
		}
		
		/*�ٴ��ж��Ƿ񳬳��ռ�*/
		if( ITEM_SIZE( item_data->hdr.len ) > ( flash_info_low->start[i+1] - flash_info_low->start[i] - sum ) )
		{
			/*��Ŀ̫��,�޷�����,���ش���*/
			set_fs (fs);
			vfree( flash_info_low );
			vfree( item_sdram );
			vfree( item_data );
			up(&item_sem);
            
			return ERROR_ITEM_BIG;
		}
	}

	/* ���ϳɵ�������Ŀд��Flash*/
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	kernel_write( part->master , ( unsigned char * ) item_data , flash_info_low->start[i] + sum , ITEM_HEAD_LEN + item_data->hdr.len );
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	/* ������Ŀ��Ϊ��Ч*/
	if( j == 1 )
	{
		avail = ITEM_UNAVAIL;
	#ifdef CONFIG_FLASH_WRITE_PROTECT
		kernal_flash_protect(0);
	#endif
		kernel_write( part->master , ( char * )&avail , item_repeat_addr[0] , 0x1 );
	#ifdef CONFIG_FLASH_WRITE_PROTECT
		kernal_flash_protect(1);
	#endif	
	}

	set_fs (fs);
	vfree( flash_info_low );
	vfree( item_sdram );
	vfree( item_data );
	up(&item_sem);
    
	return ERROR_ITEM_OK;
}

/*=========================================================================
 Function:		int item_repeat( flash_info_t* flash_info_low , struct mtd_info *mtd_master )

 Description:		���������ռ䲻��ʱͨ����ת����ת���ݣ���ת��������
 				flashһ���������ڴ�

 Data Accessed:
 Data Updated:
 Input:			
 				flash_info_low:	flash������Ϣ
 				*mtd_master:		����flash mtd���
 				 				
 Output:			��
 Return:			ERROR_ITEM_MALLOC				
				ERROR_ITEM_REPEAT_OK
 Others:
=========================================================================*/

int item_repeat( flash_info_t* flash_info_low , struct mtd_info *mtd_master )
{
	item_t			*item;
	unsigned int 		i = 0;
	unsigned int		sum = 0;
	unsigned int		len = 0;
	unsigned char		*item_sdram;
	unsigned int		addr;
	unsigned int		retlen;
#ifndef CONFIG_FLASH_TRANSFER
	unsigned char		*item_dram_repeat;
#endif

	item_sdram = vmalloc( READ_ITEM_LEN );
	if ( item_sdram == NULL )
	{		
		printk("item_sdram malloc wrong\n");
		
		return ERROR_ITEM_MALLOC;
	}

	/* �ҵ�Flash��������������������ʼ��ַ*/	
	while( ( flash_info_low->start[i] - flash_info_low->start[0] ) < BOOTCODE_LENGTH )
		i++;

#if 0
	 //add by wyh start at 2016-01-30:У����������Ŀ����Ч�ԣ���֤��������ȷ������²Ž�����ת
	if (item_check(flash_info_low, mtd_master) == ERROR_CONFIG_LOST)
	{
		mtd_master->read( mtd_master , flash_info_low->start[i+1], CONFIG_MARK_LEN , &retlen, item_sdram );

		if (!strcmp(item_sdram, CONFIG_MARK))
		{
			printk("[%s:%d]:Do the recopy operation from backup setor.\n", __FUNCTION__, __LINE__);
			/* ����������*/ 
#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(0);
#endif
				kernel_erase( mtd_master , flash_info_low , flash_info_low->start[i] , flash_info_low->start[i+1] - flash_info_low->start[i] );
#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(1);
#endif
				/* ����Ŀд��������*/
#ifdef CONFIG_FLASH_TRANSFER
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(0);
		#endif
				kernel_write( mtd_master , ( char * )( flash_info_low->start[i+1] + CONFIG_MARK_LEN ) , flash_info_low->start[i] + CONFIG_MARK_LEN , flash_info_low->start[i+1] - flash_info_low->start[i]-CONFIG_MARK_LEN );
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(1);
		#endif
#else
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(0);
		#endif
				kernel_write( mtd_master , item_dram_repeat , flash_info_low->start[i] + CONFIG_MARK_LEN , flash_info_low->start[i+1] - flash_info_low->start[i]-CONFIG_MARK_LEN );
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(1);
		#endif
#endif
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(0);
		#endif
				kernel_write( mtd_master , CONFIG_MARK , flash_info_low->start[i] , CONFIG_MARK_LEN );
		#ifdef CONFIG_FLASH_WRITE_PROTECT
				kernal_flash_protect(1);
		#endif
			vfree( item_sdram );
		
			return ERROR_ITEM_REPEAT_OK;
		}
	}
	//add by wyh end
#endif

#ifdef CONFIG_FLASH_TRANSFER
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
	#endif
	kernel_erase( mtd_master , flash_info_low , flash_info_low->start[i+1] , flash_info_low->start[i+2] - flash_info_low->start[i+1] );
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
	#endif
	addr = flash_info_low->start[i+1] + CONFIG_MARK_LEN;
#else
	item_dram_repeat = vmalloc( flash_info_low->start[i+1] - flash_info_low->start[i] );
	if ( item_dram_repeat == NULL )
	{		
		printk("item_dram_repeat malloc wrong\n");
		vfree( item_sdram );

		return ERROR_ITEM_MALLOC;
	}
	addr = ( unsigned int )item_dram_repeat;
#endif

	sum = CONFIG_MARK_LEN;

	/* ������������Ч��Ŀ��������ת��*/
	while( ( sum + ITEM_HEAD_LEN ) <= ( flash_info_low->start[i+1] - flash_info_low->start[i] ) )
	{
		mtd_master->read( mtd_master , flash_info_low->start[i] + sum , READ_ITEM_LEN , &retlen, item_sdram );
		item = ( item_t * ) item_sdram;

		if( item->hdr.avail == 0xff )
		{
#ifdef CONFIG_FLASH_TRANSFER
		#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(0);
		#endif
			kernel_write( mtd_master , CONFIG_MARK , flash_info_low->start[i+1] , CONFIG_MARK_LEN );
		#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(1);
		#endif
#endif

			break;
		}

		if( ( item->hdr.avail == ITEM_UNAVAIL ) )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		if( item->hdr.avail == ITEM_AVAIL )
		{
#ifdef CONFIG_FLASH_TRANSFER
		#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(0);
		#endif
			kernel_write( mtd_master , ( unsigned char * )( flash_info_low->start[i] + sum ) , addr , ITEM_SIZE( item->hdr.len ) );
		#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(1);
		#endif
#else
			mtd_master->read( mtd_master , flash_info_low->start[i] + sum , ITEM_SIZE( item->hdr.len ) , &retlen, ( unsigned char* )addr );
#endif
			sum += ITEM_SIZE( item->hdr.len );
			addr += ITEM_SIZE( item->hdr.len );
			len += ITEM_SIZE( item->hdr.len );
						
			continue;
		}
	}

	/* ����������*/ 
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	kernel_erase( mtd_master , flash_info_low , flash_info_low->start[i] , flash_info_low->start[i+1] - flash_info_low->start[i] );
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	/* ����Ŀд��������*/
#ifdef CONFIG_FLASH_TRANSFER
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
	#endif
	kernel_write( mtd_master , ( char * )( flash_info_low->start[i+1] + CONFIG_MARK_LEN ) , flash_info_low->start[i] + CONFIG_MARK_LEN , len );
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
	#endif
#else
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
	#endif
	kernel_write( mtd_master , item_dram_repeat , flash_info_low->start[i] + CONFIG_MARK_LEN , len );
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
	#endif
#endif
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
	#endif
	kernel_write( mtd_master , CONFIG_MARK , flash_info_low->start[i] , CONFIG_MARK_LEN );
	#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
	#endif
	vfree( item_sdram );
#ifndef CONFIG_FLASH_TRANSFER
	vfree( item_dram_repeat );
#endif

	return ERROR_ITEM_REPEAT_OK;
}
#ifdef CONFIG_APPS_UPDATE_UBOOT
/*=========================================================================
 Function:		flash_update_uboot( void )
 Description:		����ϵͳ�е�uboot file
 				
 Data Accessed:	/var/uboot.bin
 Data Updated:	flash�е�����
 
 Input:			��
 Output:			��
 Return:			0:		�����ɹ�
 				����:	����ʧ��
 Others:
=========================================================================*/
#define UBOOT_FILE  "/usr/share/uboot.bin"
int flash_update_uboot( void )
{
	unsigned int			end = 0;
	unsigned int			retlen;
	int					len = 0;
	mm_segment_t		fs;
	long					fp_uboot = -1;
	unsigned char 					*old_boot = NULL;
	unsigned char					*buff = NULL;
	flash_info_t			*info;
	struct mtd_info		*mtd;
	struct mtd_part		*part ;
	
	
	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡMTD������Ϣ*/
	mtd = get_mtd_device( NULL, 0 );
	part = PART(mtd);

	info = vmalloc( ( unsigned long )sizeof( flash_info_t ) );
	if ( info == NULL )
	{
		printk(KERN_EMERG"info malloc wrong\n");
		set_fs (fs);
		
		return EFAULT;
	}

	/* ��ȡFlash�����б�*/
	flash_info_get( info , part->master );

	/*��ȡ��ǰ�汾��uboot����Ϣ*/
	old_boot = vmalloc( BOOTCODE_LENGTH/4 );
	if ( old_boot == NULL )
	{	
		printk("item_sdram malloc wrong\n");
		set_fs (fs);
		vfree( info );
		
		return EFAULT;
	}

	part->master->read( part->master , info->start[0]+BOOTCODE_LENGTH/2, BOOTCODE_LENGTH/4 , &retlen, old_boot );

	/*��ȡ�ں˸��µ�uboot ��Ϣ*/
	if( ( fp_uboot = sys_open( UBOOT_FILE , O_RDONLY , 0 ) ) < 0 )//modify by zhengmingming 20120702
       {
		printk("\nCan't find UBOOT file\n");
		set_fs (fs);
		vfree( info );
		vfree( old_boot );
		
		return EFAULT;
	}

	buff = vmalloc(BOOTCODE_LENGTH);
	if( buff == NULL )
	{
		printk("buff malloc wrong\n");
		sys_close( fp_uboot );
		set_fs ( fs );
		vfree( info );
		vfree( old_boot );
		
		return EFAULT;
	}
	

	if( ( len = sys_read( fp_uboot , buff , BOOTCODE_LENGTH )  ) != 0 )
	{

		/*�Ƚϵ�ǰ��uboot �Ƿ����ں˸��µ�uboot��Ϣһ�£���������Ҫ���и���*/
		if (memcmp(old_boot, (buff+(BOOTCODE_LENGTH/2)), BOOTCODE_LENGTH/4))
		{
			printk("\nErasing flash");
				
			/*��Flashȫ������*/
#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(0);
#endif	
			kernel_erase( part->master , info , info->start[0] , BOOTCODE_LENGTH);
#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(1);
#endif
			printk("\nBurning flash");	

			/*��imageд��Flash*/
#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(0);
#endif

			kernel_write( part->master , buff , info->start[0] , len );	

#ifdef CONFIG_FLASH_WRITE_PROTECT
			kernal_flash_protect(1);
#endif

		}
		else
		{
			printk("\nNo need to do any update");
		}
	}

	sys_close(fp_uboot);
	set_fs (fs);
	vfree( old_boot );
	vfree(buff);
	vfree( info );

	printk("\nUboot Update Completed\n");

	return 0;
}
#endif

/*=========================================================================
 Function:		flash_update_bin( void )
 Description:		����ϵͳ�е�BIN file
 				
 Data Accessed:	/var/image.bin
 Data Updated:	flash�е�����
 
 Input:			��
 Output:			��
 Return:			0:		�����ɹ�
 				����:	����ʧ��
 Others:
=========================================================================*/

int flash_update_bin( void )
{
	unsigned int			end = 0;
	int					len = 0;
	mm_segment_t		fs;
	long					fp_image_bin = -1;
	char					*buff = NULL;
	flash_info_t			*info;
	struct mtd_info		*mtd;
	struct mtd_part		*part ;
//modify by zhengmingming
/* add by huangshuangbang 20120602 for save mib_rf and mac when update bin start    */
	#ifdef BACKUP_ITEM_MIB_RF_MAC
		memset(&mib_rf_data, 0, sizeof(mib_rf));
		memset(&syscfg, 0, sizeof(llconfig_syscfg_t));
		/*��ȡmib_rf��������ֵ*/
		mib_rf_config_get(&mib_rf_data);
		/*��ȡmac*/
		llconfig_get_syscfg(&syscfg);
	#endif
/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin end*/
	
	
	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡMTD������Ϣ*/
	mtd = get_mtd_device( NULL, 0 );
	part = PART(mtd);

	info = vmalloc( ( unsigned long )sizeof( flash_info_t ) );
	if ( info == NULL )
	{
		printk(KERN_EMERG"info malloc wrong\n");
		return EFAULT;
	}

	/* ��ȡFlash�����б�*/
	flash_info_get( info , part->master );

	if( ( fp_image_bin = sys_open( BIN_FILE , O_RDONLY , 0 ) ) < 0 )//modify by zhengmingming 20120702
       {
		printk("\nCan't find BIN file\n");
		set_fs (fs);
		vfree( info );
		
		return EFAULT;
	}

	buff = vmalloc(1024);
	if( buff == NULL )
	{
		printk("buff malloc wrong\n");
		sys_close( fp_image_bin );
		set_fs ( fs );
		vfree( info );
		
		return EFAULT;
	}
	
	printk("\nErasing flash");

	/*��Flashȫ������*/
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif	
	kernel_erase( part->master , info , info->start[0] , info->size );
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	printk("\nBurning flash");

	/*��imageд��Flash*/
	end = info->start[0];
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	while( ( len = sys_read( fp_image_bin , buff , 1024 )  ) != 0 )
	{	
		kernel_write( part->master , buff , end , len );	
		end += len;

		if( ( end % 0x10000 ) == 0 )
			printk(".");
	}
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	sys_close(fp_image_bin);
	set_fs (fs);
	vfree(buff);
	vfree( info );
//modify by zhengmingming
/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin start*/
	#ifdef BACKUP_ITEM_MIB_RF_MAC
		/*������ϵͳǰ�������mib_rf��ֵ����д���Ӧ������*/
		mib_rf_config_set(&mib_rf_data);
		/*������ǰ��mac���û�ȥ*/
		llconfig_set_syscfg(&syscfg);
	#endif	
/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin end*/	
	printk("\nFlash Update Completed\n");

	return 0;
}

/*=========================================================================
 Function:		int flash_update_img( void )

 Description:		��ϵͳ��������
 				
 Calls:			sys_open
 				sys_read
 				sys_close
 				kernel_erase
 				kernel_write
 				sysdata_save
 Data Accessed:	/var/image.img
 Data Updated:	flash�е�����
 
 Input:			��
 Output:			��
 Return:			0:		�����ɹ�
 				����:	����ʧ��
 Others:
=========================================================================*/

int flash_update_img( void )
{
	update_hdr_t			*update_hdr;
	unsigned int			start = 0 ;
	unsigned int			end = 0;
	int					len = 0;
	mm_segment_t		fs;
	long					fp_image_img = -1;
	char					*buff = NULL;
	flash_info_t			*info;
	struct mtd_info		*mtd;
	struct mtd_part		*part ;
	sys_config_t			*sys_data;

	fs = get_fs();
	set_fs (get_ds());

	/* ��ȡMTD������Ϣ*/
	mtd = get_mtd_device( NULL, 0 );
	part = PART(mtd);
	
	info = vmalloc( ( unsigned long )sizeof( flash_info_t ) );
	if ( info == NULL )
	{
		printk(KERN_EMERG"info malloc wrong\n");
		return EFAULT;
	}

	/* ��ȡFlash�����б�*/
	flash_info_get( info , part->master );
	
	sys_data = vmalloc( sizeof( sys_config_t ) );		
	if ( sys_data == NULL )
	{
		printk(KERN_EMERG"sys_config_t malloc wrong\n");
		vfree( info );

		return EFAULT;
	}

	/* ��ȡ�ײ�������Ϣ*/
	sysdata_get( sys_data );

	if( (fp_image_img= sys_open(IMG_FILE,O_RDONLY,0)) < 0)
       {
		printk("\nUnable to Open the /var/image.img partition\n");
		set_fs (fs);
		vfree(info);
		vfree(sys_data);
		
		return EFAULT;
	}

	buff = vmalloc(1024);
	if(buff == NULL)
	{
		printk("buff malloc wrong\n");
		sys_close(fp_image_img);
		set_fs (fs);
		vfree(info);
		vfree(sys_data);
				
		return EFAULT;
	}

	/* ��ȡ�����ļ�ͷ����Ϣ*/
	update_hdr = vmalloc(sizeof(update_hdr_t));		
	if ( update_hdr == NULL )
	{
		printk("update_hdr malloc wrong\n");
		sys_close(fp_image_img);
		vfree(buff);
		vfree(info);
		vfree(sys_data);

		return EFAULT;
	}

	sys_read( fp_image_img , buff , sizeof(update_hdr_t) ) ;
		
	memcpy( update_hdr , buff , sizeof(update_hdr_t) );
	
	strcpy( sys_data->version ,  update_hdr->version );
	strcpy( sys_data->board_id, update_hdr->board_id);
    
      strcpy( sys_data->swversion, update_hdr->swversion);
      strcpy( sys_data->model_name, update_hdr->model_name);

      //region code������
      //strcpy( sys_data->region, update_hdr->region);

#ifdef CONFIG_DOUBLE_BACKUP

	/*�ҵ�Ҫд��image��Flash��ʼ��ַ*/
	if( sys_data->image_mark == 0 )
	{
		while( sys_data->layout.zone_offset[ZONE_KERNEL_SECOND] > ( info->start[start] - info->start[0] ) )
		{
			start++;
		}
	}
	else
	{
		while( sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] > ( info->start[start] - info->start[0] ) )
		{
			start++;
		}
	}

	printk("\nErasing flash");
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	kernel_erase(part->master,info,info->start[start],update_hdr->image_len);
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	printk("\nBurning flash");

	end = info->start[start];
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	while( ( len = sys_read(fp_image_img,buff,1024) ) != 0 )
	{			
		kernel_write(part->master,buff,end,len);	
		end += len;

		if( ( end % 0x10000 ) == 0 )
			printk(".");
	}
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	/*�л�����image�ı�ʶλ*/
	if( sys_data->image_mark == 0 )
	{
		sys_data->layout.zone_offset[ZONE_ROOTFS_SECOND] = sys_data->layout.zone_offset[ZONE_KERNEL_SECOND] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
		sys_data->second_image_checksum = update_hdr->image_checksum;
		sys_data->second_image_len = update_hdr->image_len;
		sys_data->image_mark =1;
	}
	else
	{
		sys_data->layout.zone_offset[ZONE_ROOTFS_FIRST] = sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
		sys_data->first_image_checksum = update_hdr->image_checksum;
		sys_data->first_image_len = update_hdr->image_len;
		sys_data->image_mark = 0;
	}

#else

	/*�ҵ�Ҫд��image��Flash��ʼ��ַ*/
	while(sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] > (info->start[start]-info->start[0]))
		start++;
//ͬ����������Ŀ��Ĭ�Ͽ���watchdog��������ʱ����رա�
#if defined(CONFIG_RTL_WTDOG)
          printk("--flash_update_img---disabe watchdog---\n");
          *((volatile unsigned long *)0xB800311C) = 0xA5000000;    // disabe watchdog
#endif

	printk("\nErasing flash");
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	kernel_erase(part->master,info,info->start[start],update_hdr->image_len);
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	printk("\nBurning flash");
	end = info->start[start];
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(0);
#endif
	while( ( len = sys_read(fp_image_img,buff,1024) ) != 0 )
	{	
		kernel_write(part->master,buff,end,len);
		end += len;

		if( ( end % 0x10000 ) == 0 )
			printk(".");
	}

	if( ( end % 0x10000 ) != 0 )
		printk(".");
			
#ifdef CONFIG_FLASH_WRITE_PROTECT
	kernal_flash_protect(1);
#endif
	sys_data->layout.zone_offset[ZONE_ROOTFS_FIRST] = sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
	sys_data->first_image_checksum = update_hdr->image_checksum;
	sys_data->first_image_len = update_hdr->image_len;

#endif

	sysdata_save(sys_data); 

	sys_close(fp_image_img);
	set_fs (fs);
	vfree(buff);
	vfree(update_hdr);
	vfree(info);
	vfree(sys_data);

	printk("\nFlash Update Completed\n");

	return 0;
}

/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin start*/
#ifdef BACKUP_ITEM_MIB_RF_MAC
/*=========================================================================
 Function:		void  mib_rf_config_get(  mib_rf *data)
 Description:		��ȡmib_rf ��������ֵ���Ա�������binʱ�ָ�����
 Data Accessed:
 Data Updated:
 Input:		
 Output:			*data:		��ȡ��mib_rf��������ֵ
 Return:			
 Others:
=========================================================================*/

static void  mib_rf_config_get(  mib_rf *data)
{
	unsigned short len = 29; //modify by zhengmingming 20120702
	
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_TX_POWER_CCK_A, MIB_RF_PARAM[0],&len))
	{
		memset(data->HW_TX_POWER_CCK_A, 0, 29 );
	}
	else
	{
		data->HW_TX_POWER_CCK_A[len] = '\0';
	}
			
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_TX_POWER_CCK_B, MIB_RF_PARAM[1],&len))
	{
		memset(data->HW_TX_POWER_CCK_B, 0, 29 );
	}
	else
	{
		data->HW_TX_POWER_CCK_B[len] = '\0';

	}
	
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_TX_POWER_HT40_1S_A, MIB_RF_PARAM[2],&len))
	{
		memset(data->HW_TX_POWER_HT40_1S_A, 0, 29 );
	}
	else
	{
		data->HW_TX_POWER_HT40_1S_A[len] = '\0';

	}
	
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_TX_POWER_HT40_1S_B, MIB_RF_PARAM[3],&len))
	{
		memset(data->HW_TX_POWER_HT40_1S_B, 0, 29 );
	}	
	else
	{
		data->HW_TX_POWER_HT40_1S_B[len] = '\0';

	}
	
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_DIFF_HT40_2S, MIB_RF_PARAM[4],&len))
	{
		memset(data->HW_DIFF_HT40_2S, 0, 29 );
	}
	else
	{
		data->HW_DIFF_HT40_2S[len] = '\0';

	}
				
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_DIFF_HT20_A, MIB_RF_PARAM[5],&len))
	{
		memset(data->HW_DIFF_HT20_A, 0, 29 );
	}
	else
	{
		data->HW_DIFF_HT20_A[len] = '\0';

	}
		
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_DIFF_OFDM, MIB_RF_PARAM[6],&len))
	{
		memset(data->HW_DIFF_OFDM, 0, 29 );
	}
	else
	{
		data->HW_DIFF_OFDM[len] = '\0';

	}

//add by zhengmingming 21020709 add HW_11N_THER 
//modify by zhengmingming 20120809 for error
	if(ERROR_ITEM_NOT_FIND == item_get(data->HW_11N_THER, MIB_RF_PARAM[7],&len))
	{
		memset(data->HW_11N_THER, 0, 29 );
	}
	else
	{
		data->HW_11N_THER[len] = '\0';

	}
		
}


/*=========================================================================
 Function:		void mib_rf_config_set(  mib_rf *data)
 Description:		�ָ�mib_rf ������������ֵ
 Data Accessed:
 Data Updated:
 Input:		       *data:	�ָ���mib_rf��������ֵ
 Output:			
 Return:			
 Others:
=========================================================================*/

static void  mib_rf_config_set(  mib_rf *data)
{
	unsigned short len;
	mib_rf data_tmp;

	
	len = strlen(data->HW_TX_POWER_CCK_A);	
	if(ERROR_ITEM_OK != item_save(data->HW_TX_POWER_CCK_A, MIB_RF_PARAM[0],len))
	{
		printk("[mib_rf_config_set]set HW_TX_POWER_CCK_A info fail!!\n");
	}

	len = strlen(data->HW_TX_POWER_CCK_B);	
	if(ERROR_ITEM_OK != item_save(data->HW_TX_POWER_CCK_B, MIB_RF_PARAM[1],len))
	{
		printk("[mib_rf_config_set]set HW_TX_POWER_CCK_B info fail!!\n");
	}
	
	len = strlen(data->HW_TX_POWER_HT40_1S_A);	
	if(ERROR_ITEM_OK != item_save(data->HW_TX_POWER_HT40_1S_A, MIB_RF_PARAM[2],len))
	{
		printk("[mib_rf_config_set]set HW_TX_POWER_HT40_1S_A info fail!!\n");
	}
	

	len = strlen(data->HW_TX_POWER_HT40_1S_B);
	if(ERROR_ITEM_OK != item_save(data->HW_TX_POWER_HT40_1S_B, MIB_RF_PARAM[3],len))
	{
		printk("[mib_rf_config_set]set HW_TX_POWER_HT40_1S_B info fail!!\n");
	}	
	

	len = strlen(data->HW_DIFF_HT40_2S);
	if(ERROR_ITEM_OK != item_save(data->HW_DIFF_HT40_2S, MIB_RF_PARAM[4],len))
	{
		printk("[mib_rf_config_set]set HW_DIFF_HT40_2S info fail!!\n");
	}
	
	len = strlen(data->HW_DIFF_HT20_A);
	if(ERROR_ITEM_OK != item_save(data->HW_DIFF_HT20_A, MIB_RF_PARAM[5],len))
	{
		printk("[mib_rf_config_set]set HW_DIFF_HT20_A info fail!!\n");
	}

	len = strlen(data->HW_DIFF_OFDM);
	if(ERROR_ITEM_OK != item_save(data->HW_DIFF_OFDM, MIB_RF_PARAM[6],len))
	{
		printk("[mib_rf_config_set]set HW_DIFF_OFDM fail!!\n");
	}
//add by zhengmingming 21020709 add HW_11N_THER 
	len = strlen(data->HW_11N_THER);
	if(ERROR_ITEM_OK != item_save(data->HW_11N_THER, MIB_RF_PARAM[7],len))
	{
		printk("[mib_rf_config_set]set HW_DIFF_OFDM fail!!\n");
	}
	
		
}


/*=========================================================================
 Function:		void  llconfig_get_syscfg(  llconfig_syscfg_t *data)
 Description:		��ȡMAC����ֵ
 Data Accessed:
 Data Updated:
 Input:		       
 Output:			*data:	��ȡ��MAC����ֵ
 Return:			
 Others:
=========================================================================*/
static void  llconfig_get_syscfg(  llconfig_syscfg_t *data)
{
	sys_config_t syscfg;
	int i;

	sysdata_get( &syscfg);
	for(i = 0; i < 6; i++)
	{
		data->mac[i] = syscfg.mac[i];
	}

	memcpy(data->region,syscfg.region,REGION_LEN);

	#ifdef PINANDOTHER_ENABLED
		memcpy(data->pinandother,syscfg.pinandother,PINANDOTHER_LEN);    
	#endif 
}




/*=========================================================================
 Function:		void  llconfig_set_syscfg(  llconfig_syscfg_t *data)
 Description:		�ָ�MAC����ֵ
 Data Accessed:
 Data Updated:
 Input:		       *data:	�ָ���MAC����ֵ
 Output:			
 Return:			
 Others:
=========================================================================*/
static void  llconfig_set_syscfg(  llconfig_syscfg_t *data)
{
	int i;
	sys_config_t syscfg_tmp;
	char mac_and = data->mac[0] & data->mac[1] & data->mac[2] & data->mac[3] & data->mac[4] & data->mac[5];
	char mac_or   = data->mac[0] | data->mac[1] | data->mac[2] | data->mac[3] | data->mac[4] | data->mac[5];

	memset(&syscfg_tmp, 0, sizeof(sys_config_t));
	sysdata_get(&syscfg_tmp);
	if(0xFF == mac_and || 0x00 == mac_or)
	{
		printk("Illegal mac address, recover fail!!!\n");
	}
	else
	{
		for(i = 0; i < 6; i++)
		{
			syscfg_tmp.mac[i] = data->mac[i];
		}
	}

	memcpy(syscfg_tmp.region,data->region,REGION_LEN);

	#ifdef PINANDOTHER_ENABLED
		memcpy(syscfg_tmp.pinandother,data->pinandother,PINANDOTHER_LEN);    
	#endif 
	
	sysdata_save( &syscfg_tmp);

}
#endif
/*add by huangshuangbang 20120602 for save mib_rf and mac when update bin end*/

//add by zhengmingming 20120608 repoint the comand line from tbsboot for starting kernel
void repoint_cmdline(char *bootargs)
{
	char *fixed = "console=ttyS0,115200  root=31:2 mtdparts=spi_flash:";
	char *tmp = NULL;
	char *str = bootargs;

	int n[4] = {0};
	int i = 1;
	
	if(NULL == bootargs) 
		return;
	
	tmp = str + strlen(fixed);
	n[0] = simple_strtoul(tmp, NULL, 10);
	tmp = tmp + 1;
	printk("%d\n", n[0]);
	
	while(NULL != (tmp = strstr(tmp + 1, ","))){
		tmp = tmp + 1;
		n[i++] = simple_strtoul(tmp, NULL, 10);
		printk("%d\n", n[i]);
	}
	n[3] = MAX_FLASH_LEN - n[0] -n[1];	
	printk("n[0] = %d, n[1] = %d, n[3] = %d\n", n[0], n[1], n[3]);
	sprintf(str,"%s%d(boot),%d(kernel),%d(rootfs),131072(multi_lang),131072(deflang)", fixed, n[0], n[1], n[3]);
	printk(KERN_EMERG"repoint_cmdline,str:%s\n", str);
}
//add by zhengmingming 20120608 repoint the comand line from tbsboot for starting kernel .end

EXPORT_SYMBOL(item_get);
EXPORT_SYMBOL(item_save);

/*=========================================================================
					File End
=========================================================================*/


