/*=========================================================================
Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
�ļ����� : flash_layout.h
�ļ����� : flash image���ֵĶ���


�޶���¼ :
         1 ���� : ������
           ���� : 2008-9-12
           ���� :

         2 �޶�: ������
            ����: 2009-3-16
            ����: �Ż���Ŀ�Ĳ���

         3 �޶�: �ĳ���
         	 ����: 2011-08-08
         	 ����: ���¶��ļ�β������,�Լ���������


=========================================================================*/
#ifndef _FLASH_LAYOUT_H_
#define _FLASH_LAYOUT_H_

/*=========================================================================
	�ײ�������Ϣ�в�Ʒ���ơ��汾��Ϣ��
	image�������к��ַ������ȵĶ���
=========================================================================*/

//#define CFG_MAX_FLASH_SECT	        (256)/* max number of sectors on one chip */
#define PRODUCT_NAME_LEN			32
#define TBS_VERSION_NAME_LEN		32
#define IMAGE_TYPES_NAME_LEN		8
#define BOARD_ID_NAME_LEN			16
#define	BOOTCODE_LENGTH		        (0x10000)/* Bootloader�̶�ռ��64KB��С,u-boot.bin */
#define	MULTILANG_LENGTH		    (0x20000)/* multilang�̶�ռ��128KB��С*/
#define	DEFLANG_LENGTH		        (0x20000)/* def lang�̶�ռ��128KB��С*/

#define WRITE_BUFFER_SIZE           (0x1000)
#define MAX_RF_DEFVALUE_SIZE        (2048)
#define SYSTEM_CONFIG_VERSION		"TBSCFG01"
#define IMAGE_TAIL_VERSION			"TBSTLV01"
#define	VERSION_LEN					12
#define MODEL_LEN					16
#define SN_LEN						20

#define F_CRC_CHECK                 0x01/* CRC��� */
#define F_PRODUCT_CHECK             0x02/* ��Ʒ�ͺż�� */
#define F_SAVE_ETHP                 0x04/* ������̫������MAC��ַ��IP��ַ�� */
#define F_SAVE_WLANP                0x08/* �������߲��� */
#define F_BUILD_CFG                 0x10/* ����ϵͳ����llconfig */
#define F_CLEAR_APP                 0x20/* ���Ӧ�ò����� */
#define F_MASK                      0xFF/* ��־���� */

#define REGION_LEN			4

#define MTD_FLASH_NAME              "sflash"
/*====================================================================
	ϵͳ������Ϣ���ݽṹ���������ڴ��ʱ��ʼ��
====================================================================*/
typedef struct sys_cfg
{
    unsigned char cfg_version[VERSION_LEN];	/*������Ϣ�汾*/
    unsigned int first_kernel_offset;
    unsigned int first_kernel_len;
    unsigned int first_rootfs_offset;
    unsigned int first_rootfs_len;
    unsigned long first_image_checksum;
    unsigned int second_kernel_offset;
    unsigned int second_kernel_len;
    unsigned int second_rootfs_offset;
    unsigned int second_rootfs_len;
    unsigned long second_image_checksum;
    unsigned int ip;
    unsigned char mac[6];
    unsigned char endian;
    unsigned char boot_img_flag;		/*0: ϵͳ����image1��1: ϵͳ����image 2*/
    unsigned char board_id[BOARD_ID_NAME_LEN];			/*�ͻ�����ǩ��*/
    unsigned char product[PRODUCT_NAME_LEN];				/*������ʶ��*/
    unsigned char version[TBS_VERSION_NAME_LEN];			/*�汾��Ϣ*/
    unsigned char tag[4];						/*  �ײ����ÿ��ʶ��'sysc'  */
    unsigned char sn[20];
}
sys_cfg_t;

/*====================================================================
	wan��������Ϣ���ݽṹ��������������ʱ��ʼ��
====================================================================*/
typedef struct
{
    unsigned int version;
    unsigned int connection_type; //WAN�������ͣ�DHCP, PPPOE
    unsigned int ip;
    unsigned int subnet_mask;
    unsigned int gateway_ip;
    unsigned int vlan;
    unsigned int dns;
    unsigned int dns2;
    char pppoe_account[64];  //PPPOE�û���
    char pppoe_password[64]; //PPPOE����
    unsigned int pppoe_auth_method;  //PPPOE��֤����
}
wan_eth_ipv4_config_t;

/*====================================================================
	����������Ϣ���ݽṹ��������������ʱ��ʼ��
====================================================================*/
typedef struct
{
    char url[512];       //����URL
    char account[64];    //�û���
    char password[64];   //����
}
upg_config_t;

/*=========================================================================
	flash image���ֵľ��嶨�壬�������ڴ��ʱ��ʼ��
=========================================================================*/
#define		ZONE_BOOTLOADER			0
#define		ZONE_KERNEL_FIRST		1
#define		ZONE_ROOTFS_FIRST		2
#define		ZONE_MULTI_LANG		3

#define		ZONE_KERNEL_SECOND		3
#define		ZONE_ROOTFS_SECOND		4
#define		ZONE_EXTFS				5
#define		ZONE_MAX_ITEM			6


typedef struct
{
	unsigned int 			zone_offset[ZONE_MAX_ITEM];
} flash_layout_t;

typedef struct
{
	flash_layout_t		layout;
	unsigned int		ip;
	unsigned int		first_image_len;
	unsigned int		first_image_checksum;
	unsigned int		second_image_len;
	unsigned int		second_image_checksum;
	unsigned char 		mac[6];
	unsigned char		endian;
	unsigned char		image_mark;								/*0: ϵͳ����image1��1: ϵͳ����image 2*/
	unsigned char 		board_id[BOARD_ID_NAME_LEN];			/*�ͻ�����ǩ��*/     
	unsigned char		product[PRODUCT_NAME_LEN];				/*������ʶ��*/
	unsigned char		version[TBS_VERSION_NAME_LEN];			/*�汾��Ϣ*/
    
    //the following 3 items for netgear
    unsigned char 		model_name[MODEL_LEN]; /*ģ����*/
    unsigned char  		region[REGION_LEN];       /*����*/
    //swversion������汾�ţ���1.0.6.2 ȥ�����еĵ����1062, version��TBSϵͳ�汾����R2
    unsigned char  		swversion[VERSION_LEN]; /*����汾*/
    unsigned char 		sn[20]; //gxw / 2014-11-04 / add
    #ifdef PINANDOTHER_ENABLED
    unsigned char		pinandother[PINANDOTHER_LEN]; /*PIN���Լ�����������Ԥ����ռ䣬������*/
    #endif 
    
	unsigned char		tag[4];									/*  �ײ����ÿ��ʶ��'sysc'  */
} sys_config_t;

#define		SYSCFG_SIZE			( sizeof( sys_config_t ) )


/*=========================================================================
	image.imgͷ����Ϣ���ݽṹ���������ڴ��ʱ��ʼ��
=========================================================================*/
typedef struct
{
	unsigned int 			image_checksum;					/*imageУ���*/
	unsigned int			kernel_offset;
	unsigned int 			rootfs_offset;
	unsigned int 			kernel_size;
	unsigned int 			rootfs_size;
	unsigned int			image_len;
	unsigned char			img_type[IMAGE_TYPES_NAME_LEN]; 	/*�����ļ���ʶ����imageΪ:imgs    ˫����Ϊ:imgd*/
	unsigned char 			board_id[BOARD_ID_NAME_LEN];		/*�ͻ�����ǩ��*/    
	unsigned char			product[PRODUCT_NAME_LEN];			/*������ʶ��*/
	unsigned char			version[TBS_VERSION_NAME_LEN];		/*�汾��Ϣ*/
	//the following 3 items for netgear
	unsigned char			model_name[MODEL_LEN]; /*ģ����*/
	unsigned char			region[REGION_LEN];    /*����*/
	unsigned char			swversion[VERSION_LEN];/*����汾*/
} update_hdr_t;

/*=========================================================================
	imgd: ��ʾϵͳ��˫image��ϵͳ
	imgs: ��ʾϵͳ�ǵ�image��ϵͳ
	�����Ҳ���Ա��ⵥ��˫imageϵͳ�������������Ĵ���
	bin:��ʶ�ļ�ΪBIN�ļ�
=========================================================================*/
#ifdef CONFIG_DOUBLE_BACKUP
#define	IMAGE_TYPES			"imgd"
#else
#define	IMAGE_TYPES			"imgs"
#endif

/*=========================================================================
	image.binβ����Ϣ���ݽṹ���������ڴ��ʱ��ʼ��
=========================================================================*/

#define	IMAGE_BIN				"bin"

typedef struct bin_tail
{
    unsigned char product[PRODUCT_NAME_LEN];				/*������ʶ��*/
    unsigned char bin_type[4];								/*BIN file��ʶ��*/
}
image_bin_tail_t;


/*====================================================================
	imageβ����Ϣ���ݽṹ���������ڴ��ʱ��ʼ��, ����ԭ��ͷ��
====================================================================*/
typedef struct image_tail
{
    unsigned int bootloader_len;							/*bootloader����*/
    unsigned int config_block_offset;					/*������ƫ�Ƶ�ַ*/
    unsigned int config_block_len;						/*����������*/
    unsigned int first_kernel_offset;					/*Сimage�ں�ƫ�Ƶ�ַ*/
    unsigned int first_kernel_len;						/*Сimage�ں˳���*/
    unsigned int first_rootfs_offset;					/*Сimage��������ַ*/
    unsigned int first_rootfs_len;						/*Сimage����������*/
    unsigned long first_image_checksum;					/*СimageCRCֵ*/
    unsigned int second_kernel_offset;					/*��image�ں˵�ַ*/
    unsigned int second_kernel_len;						/*��image�ں˳���*/
    unsigned int second_rootfs_offset;					/*��image��������ַ*/
    unsigned int second_rootfs_len;						/*��image����������*/
    unsigned long second_image_checksum;					/*��imageCRCֵ*/
    unsigned int boot_img_flag;		/*0: ϵͳ����image1��1: ϵͳ����image 2*/
    unsigned char board_id[BOARD_ID_NAME_LEN];			/*�ͻ�����ǩ��*/
    unsigned char version[TBS_VERSION_NAME_LEN];			/*�汾��Ϣ*/
    unsigned char product[PRODUCT_NAME_LEN];				/*������ʶ��*/
    unsigned int image_len;								/*�ļ�����*/
    unsigned int img_type; 								/*�ļ����ͱ�ʶ��*/
    unsigned char tail_version[VERSION_LEN];				/*β����Ϣ�汾*/
    unsigned long image_checksum;						    /*image�ļ�У���*/
}
image_tail_t;

struct update_parameters {
	struct image_tail tail;
	int cfg_sector;
	int f_s_sector;
	int f_e_sector;
	int s_s_sector;
	int s_e_sector;
	unsigned long dest_addr;
	int          size;
	int syscfg_version;
};


/*====================================================================
	�ļ����ͱ�ʾ�����壬�����жϽ��յ��ļ����͡�
====================================================================*/


typedef enum {
    TYPE_UNKNOWN = 0,         /* δ֪�ļ����� */
    TYPE_SINGLE_IMG = 1,      /* ��image��IMG��ʽ�ļ� */
    TYPE_SINGLE_BIN = 2,      /* ��image��BIN��ʽ�ļ� */
    TYPE_DUAL_IMG = 3,        /* ˫image��IMG��ʽ�ļ� */
    TYPE_DUAL_BIN = 4,        /* ˫image��BIN��ʽ�ļ� */
    TYPE_BACKUP_IMG = 5,      /* ��Сimage��IMG��ʽ�ļ� */
    TYPE_BACKUP_BIN = 6,      /* ��Сimage��BIN��ʽ�ļ� */
    TYPE_OLD_SIMAGE = 7,      /* �ϰ汾��image��IMG��ʽ�ļ� */
    TYPE_OLD_DIMAGE = 8,      /* �ϰ汾˫image��IMG��ʽ�ļ� */
    TYPE_OLD_BIN = 9,         /* �ϰ汾��image��BIN��ʽ�ļ� */
    TYPE_BACKUP_FIMG = 10,    /* ��Сimage�Ĵ�IMG��ʽ�ļ� */
    TYPE_BACKUP_BIMG = 11,    /* ��Сimage��СIMG��ʽ�ļ� */
    TYPE_BOOTLOADER = 12,     /* Bootloader */
    TYPE_VIRTUAL_DIMG         /* �����°汾˫IMAGE�ļ� */    
}image_type;


/*=========================================================================
	������Ϣ��Ŀ���ݽṹ
=========================================================================*/

typedef struct item_hdr {
    unsigned short crc;
    unsigned short len;
    unsigned char avail;
}
__attribute__ ((packed)) item_hdr_t;

typedef struct item {
    struct item_hdr hdr;
    unsigned char name[0];              /* ��Ŀ���ư����������� */
    unsigned char data[];				/* ����*/
}
item_t;

#define     CFG_PARTITION_SIZE          (0x10000)/* Configuration partition size */
#define     MAX_INVALID_ITEM_CNT        (50)/* prevent too slow scan speed for item locating  */
#define     MAX_ITEM_NAME_LENGTH        (32)
#define		ITEM_HEAD_LEN				(sizeof(struct item_hdr))
#define		ITEM_SIZE(data_len)			((ITEM_HEAD_LEN + (data_len)) + ((ITEM_HEAD_LEN + (data_len)) % 2))
#define		CONFIG_MARK		            "OK"
#define		CONFIG_MARK_LEN	            (strlen(CONFIG_MARK))
#define		ITEM_UNAVAIL	            0x00
#define		ITEM_AVAIL		            0x11
#define		ITEM_BAD		            0x22
#define		ITEM_NULL		            0xFF

/*=========================================================================
	�����ݽṹ����Ӧ��ӿ���ioctlʱ��ײ�������ݽ���
=========================================================================*/
typedef struct
{
	unsigned char			*name;
	unsigned char			item_error;
	unsigned short			len;
	unsigned char			*data;				/* ����*/
} item_app_t;

/*=========================================================================
	ϵͳ��ʹ�õ���Ŀ���ƣ���Ŀ��������ú궨���������
	���ڹ����ά��
=========================================================================*/
#define     LLCONFIG_NAME               "llconfig"
#define     WLAN_NAME                   "wlan_cfg"
#define     WLAN_NAME_5G                "wlan_cfg5g"
#define     TBS_APP_CFG                 "tbs_app_cfg"
#define     TBS_TR069_EVENT             "tr069_event"
#define     TBS_APP_KEY_TREE            "tbs_app_key_tree"
#define     TBS_APP_KEY_CONN            "tbs_app_key_conn"
#define     TBS_PPPOE_SSID              "pppoe_ssid"
#define     BOOT_TIMES                  "boot_times"
#define     TBS_BIT_FLAG                "tbs_bit_flag"
#define     TBS_DEFCFG_PATH             "def_cfg_path"
#define     TBS_USERCFG_ITEM            "user_cfg_item"
#define     TBS_USERCFG_PREFIX          "tbs_app_cfg_"
#define     BACKUP_NET_CFG              "backup_net_cfg"
#define     TR069_UPG                   "tr069_upg"
#define     TBS_SNMP_EVENT              "tbs_snmp"
#define     TBS_LOGIN_NAME              "login_name"
#define     TBS_LOGIN_PASSWORD          "login_password"
#define     TBS_TELECOM_PASSWORD        "telecom_password"
#define     TBS_DEVINFO_SN              "devinfo_sn"
#define     TBS_WLAN_SSID             	"wlan_2.4Gssid"
#define     TBS_WLAN_PASSWORD           "wlan_2.4Gpasswd"
#define     TBS_WLAN_PASSWORD5G         "wlan_5Gpasswd"
#define     TBS_WLAN_SSID5G            	"wlan_5Gssid"
#define     TBS_TEST_OK                 "factory_test_ok"
/*sitcom ���ƻ�*/
#define     SITCOM_SCS_ENABLE           "scs_eanble"
#define     SITCOM_LAN_IP               "lan_ip"           //for LAN IPv4
#define     SITCOM_LAN_IPV6             "lan_ipv6"         //for LAN IPv6
#define     SITCOM_FWVER                "firmware_version" //for Firmware version
#define     SITCOM_BM_NAME              "board_model_name" //for board model name
#define     SITCOM_FW_ONSERVER          "fw_on_server"     //for FWonServer
#define     SITECOM_FWACTION            "fw_action"        //for FWaction
#define     SCS_CFG                     "scs_cfg"
#define     AUTOFW_CFG                  "autofw_cfg"
#define     TBS_WiZARD_FLAG				"wizard_flag"	//����µ�Ҫ�����ָ�����ҳ��ı�ʶ by hujian 2016-3-4
#ifndef CONFIG_MEM_SAVE
#define     TOTAL_TERMINAL_NUMBER       "totalterminalnumber"
#endif
/*=========================================================================
	�ڶ���Ŀ����ʱ���صĴ�����
=========================================================================*/

enum item_error
{
    ERROR_ITEM_OK = 0,
    ERROR_ITEM_MTD,
    ERROR_ITEM_IOCTRL,
    ERROR_ITEM_MALLOC,
    ERROR_ITEM_NOT_FIND,
    ERROR_ITEM_CRC,
    ERROR_ITEM_BIG,
    ERROR_ITEM_REPEAT_OK,
    ERROR_ITEM_REPEAT_FAIL,
    ERROR_CONFIG_LOST,
    ERROR_FLASH_BUSY
};

/*=========================================================================
	�ڶ��ļ���������ʱ���صĴ�����
=========================================================================*/

/*=========================================================================
	Ӧ�ò��ȡ�ͱ�����Ŀ�Ľӿ�
=========================================================================*/

int app_item_get( void *data , char *item_name , unsigned short *len );		/* ��ȡ��Ӧ��Ŀ��Ϣ*/
int app_item_save( void *data , char *item_name , unsigned short len );		/* ��������Ŀ*/

/*=========================================================================
	�ײ��ȡ�ͱ�����Ŀ�Ľӿ�
=========================================================================*/
int item_get( void *data , char *item_name , unsigned short *len );		/* ��ȡ��Ӧ��Ŀ��Ϣ*/
int item_save( void *data , char *item_name , unsigned short len );		/* ��������Ŀ*/
unsigned int item_recycle(unsigned char *item_sdram, struct item *new_it);

/*=========================================================================
	Ӧ�ò��ȡMACַַ�ӿ�
=========================================================================*/
typedef struct mtd_mac {
    int id;
    int	offset;
    unsigned char mac[6];
}mac_t;

typedef struct
{
	unsigned char		region[2];
} region_t;

enum device_mac
{
    LAN_MAC = 0,
    WAN_MAC,
    WLAN_MAC,
    USB_MAC
};

int tbs_read_mac(int id, int offset, unsigned char *mac);
int app_tbs_read_mac(int id, int offset, unsigned char *mac);
int tbs_read_region(unsigned char *region);
int app_tbs_read_region(unsigned char *region);
int image_file_check(void *data, unsigned int len, void *ptr);
int flash_update(void *data, unsigned int len, void *tail);
//int	is_sysdata(void *sys_data);
/*=========================================================================
	����bin�ļ���ָ��������������������
=========================================================================*/
typedef struct mib_rf_config
{
	char HW_TBS_PTEST_RIGHT_CHECK[5];
	char HW_TX_POWER_CCK_A[29];
	char HW_TX_POWER_CCK_B[29];
	char HW_TX_POWER_HT40_1S_A[29];
	char HW_TX_POWER_HT40_1S_B[29];
	char HW_DIFF_HT40_2S[29];
	char HW_DIFF_HT20_A[29];
	char HW_DIFF_OFDM[29];
    char HW_WLAN0_11N_TRSWPAPE_C9[8];
    char HW_WLAN0_11N_TRSWPAPE_CC[8];
    char HW_WLAN1_11N_TRSWPAPE_C9[8];
    char HW_WLAN1_11N_TRSWPAPE_CC[8];
	/*TBS TAG:START:
	added by cairong 2012-12-14 for 5G
	*/
	#if 1
	char HW_TX_POWER_HT40_1S_A_5G[400];
	char HW_TX_POWER_HT40_1S_B_5G[400];
	char HW_DIFF_HT40_2S_5G[400];
	char HW_DIFF_HT20_A_5G[400];
	char HW_DIFF_OFDM_5G[400];
	#endif
	/*TBS TAG:end*/
}mib_rf;

typedef struct llconfig_mac_sn
{
	unsigned char mac[6];
	unsigned char sn[20];
}llconfig_mac_sn_t;

#endif  /*_FLASH_LAYOUT_H_*/

