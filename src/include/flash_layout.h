/*=========================================================================
Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
文件名称 : flash_layout.h
文件描述 : flash image布局的定义


修订记录 :
         1 创建 : 轩光磊
           日期 : 2008-9-12
           描述 :

         2 修订: 轩光磊
            日期: 2009-3-16
            描述: 优化条目的操作

         3 修订: 夏超仁
         	 日期: 2011-08-08
         	 描述: 重新对文件尾部定义,以简化升级流程


=========================================================================*/
#ifndef _FLASH_LAYOUT_H_
#define _FLASH_LAYOUT_H_

/*=========================================================================
	底层配置信息中产品名称、版本信息、
	image类别和序列号字符串长度的定义
=========================================================================*/

//#define CFG_MAX_FLASH_SECT	        (256)/* max number of sectors on one chip */
#define PRODUCT_NAME_LEN			32
#define TBS_VERSION_NAME_LEN		32
#define IMAGE_TYPES_NAME_LEN		8
#define BOARD_ID_NAME_LEN			16
#define	BOOTCODE_LENGTH		        (0x10000)/* Bootloader固定占用64KB大小,u-boot.bin */
#define	MULTILANG_LENGTH		    (0x20000)/* multilang固定占用128KB大小*/
#define	DEFLANG_LENGTH		        (0x20000)/* def lang固定占用128KB大小*/

#define WRITE_BUFFER_SIZE           (0x1000)
#define MAX_RF_DEFVALUE_SIZE        (2048)
#define SYSTEM_CONFIG_VERSION		"TBSCFG01"
#define IMAGE_TAIL_VERSION			"TBSTLV01"
#define	VERSION_LEN					12
#define MODEL_LEN					16
#define SN_LEN						20

#define F_CRC_CHECK                 0x01/* CRC检查 */
#define F_PRODUCT_CHECK             0x02/* 产品型号检查 */
#define F_SAVE_ETHP                 0x04/* 保留以太网参数MAC地址和IP地址等 */
#define F_SAVE_WLANP                0x08/* 保留无线参数 */
#define F_BUILD_CFG                 0x10/* 更新系统配置llconfig */
#define F_CLEAR_APP                 0x20/* 清除应用层配置 */
#define F_MASK                      0xFF/* 标志掩码 */

#define REGION_LEN			4

#define MTD_FLASH_NAME              "sflash"
/*====================================================================
	系统配置信息数据结构，数据项在打包时初始化
====================================================================*/
typedef struct sys_cfg
{
    unsigned char cfg_version[VERSION_LEN];	/*配置信息版本*/
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
    unsigned char boot_img_flag;		/*0: 系统引导image1；1: 系统引导image 2*/
    unsigned char board_id[BOARD_ID_NAME_LEN];			/*客户电子签名*/
    unsigned char product[PRODUCT_NAME_LEN];				/*方案标识符*/
    unsigned char version[TBS_VERSION_NAME_LEN];			/*版本信息*/
    unsigned char tag[4];						/*  底层配置块标识符'sysc'  */
    unsigned char sn[20];
}
sys_cfg_t;

/*====================================================================
	wan侧配置信息数据结构，数据项在升级时初始化
====================================================================*/
typedef struct
{
    unsigned int version;
    unsigned int connection_type; //WAN连接类型：DHCP, PPPOE
    unsigned int ip;
    unsigned int subnet_mask;
    unsigned int gateway_ip;
    unsigned int vlan;
    unsigned int dns;
    unsigned int dns2;
    char pppoe_account[64];  //PPPOE用户名
    char pppoe_password[64]; //PPPOE密码
    unsigned int pppoe_auth_method;  //PPPOE认证类型
}
wan_eth_ipv4_config_t;

/*====================================================================
	升级配置信息数据结构，数据项在升级时初始化
====================================================================*/
typedef struct
{
    char url[512];       //升级URL
    char account[64];    //用户名
    char password[64];   //密码
}
upg_config_t;

/*=========================================================================
	flash image布局的具体定义，数据项在打包时初始化
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
	unsigned char		image_mark;								/*0: 系统引导image1；1: 系统引导image 2*/
	unsigned char 		board_id[BOARD_ID_NAME_LEN];			/*客户电子签名*/     
	unsigned char		product[PRODUCT_NAME_LEN];				/*方案标识符*/
	unsigned char		version[TBS_VERSION_NAME_LEN];			/*版本信息*/
    
    //the following 3 items for netgear
    unsigned char 		model_name[MODEL_LEN]; /*模板名*/
    unsigned char  		region[REGION_LEN];       /*地区*/
    //swversion是软件版本号，如1.0.6.2 去掉当中的点就是1062, version是TBS系统版本号如R2
    unsigned char  		swversion[VERSION_LEN]; /*软件版本*/
    unsigned char 		sn[20]; //gxw / 2014-11-04 / add
    #ifdef PINANDOTHER_ENABLED
    unsigned char		pinandother[PINANDOTHER_LEN]; /*PIN码以及其他，这里预留点空间，亡羊补牢*/
    #endif 
    
	unsigned char		tag[4];									/*  底层配置块标识符'sysc'  */
} sys_config_t;

#define		SYSCFG_SIZE			( sizeof( sys_config_t ) )


/*=========================================================================
	image.img头部信息数据结构，数据项在打包时初始化
=========================================================================*/
typedef struct
{
	unsigned int 			image_checksum;					/*image校验和*/
	unsigned int			kernel_offset;
	unsigned int 			rootfs_offset;
	unsigned int 			kernel_size;
	unsigned int 			rootfs_size;
	unsigned int			image_len;
	unsigned char			img_type[IMAGE_TYPES_NAME_LEN]; 	/*升级文件标识符单image为:imgs    双备份为:imgd*/
	unsigned char 			board_id[BOARD_ID_NAME_LEN];		/*客户电子签名*/    
	unsigned char			product[PRODUCT_NAME_LEN];			/*方案标识符*/
	unsigned char			version[TBS_VERSION_NAME_LEN];		/*版本信息*/
	//the following 3 items for netgear
	unsigned char			model_name[MODEL_LEN]; /*模板名*/
	unsigned char			region[REGION_LEN];    /*地区*/
	unsigned char			swversion[VERSION_LEN];/*软件版本*/
} update_hdr_t;

/*=========================================================================
	imgd: 表示系统是双image的系统
	imgs: 表示系统是单image的系统
	这个宏也可以避免单、双image系统互相升级带来的错误
	bin:标识文件为BIN文件
=========================================================================*/
#ifdef CONFIG_DOUBLE_BACKUP
#define	IMAGE_TYPES			"imgd"
#else
#define	IMAGE_TYPES			"imgs"
#endif

/*=========================================================================
	image.bin尾部信息数据结构，数据项在打包时初始化
=========================================================================*/

#define	IMAGE_BIN				"bin"

typedef struct bin_tail
{
    unsigned char product[PRODUCT_NAME_LEN];				/*方案标识符*/
    unsigned char bin_type[4];								/*BIN file标识符*/
}
image_bin_tail_t;


/*====================================================================
	image尾部信息数据结构，数据项在打包时初始化, 整合原有头部
====================================================================*/
typedef struct image_tail
{
    unsigned int bootloader_len;							/*bootloader长度*/
    unsigned int config_block_offset;					/*配置区偏移地址*/
    unsigned int config_block_len;						/*配置区长度*/
    unsigned int first_kernel_offset;					/*小image内核偏移地址*/
    unsigned int first_kernel_len;						/*小image内核长度*/
    unsigned int first_rootfs_offset;					/*小image根分区地址*/
    unsigned int first_rootfs_len;						/*小image根分区长度*/
    unsigned long first_image_checksum;					/*小imageCRC值*/
    unsigned int second_kernel_offset;					/*大image内核地址*/
    unsigned int second_kernel_len;						/*大image内核长度*/
    unsigned int second_rootfs_offset;					/*大image根分区地址*/
    unsigned int second_rootfs_len;						/*大image根分区长度*/
    unsigned long second_image_checksum;					/*大imageCRC值*/
    unsigned int boot_img_flag;		/*0: 系统引导image1；1: 系统引导image 2*/
    unsigned char board_id[BOARD_ID_NAME_LEN];			/*客户电子签名*/
    unsigned char version[TBS_VERSION_NAME_LEN];			/*版本信息*/
    unsigned char product[PRODUCT_NAME_LEN];				/*方案标识符*/
    unsigned int image_len;								/*文件长度*/
    unsigned int img_type; 								/*文件类型标识符*/
    unsigned char tail_version[VERSION_LEN];				/*尾部信息版本*/
    unsigned long image_checksum;						    /*image文件校验和*/
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
	文件类型标示符定义，用于判断接收的文件类型。
====================================================================*/


typedef enum {
    TYPE_UNKNOWN = 0,         /* 未知文件类型 */
    TYPE_SINGLE_IMG = 1,      /* 单image的IMG格式文件 */
    TYPE_SINGLE_BIN = 2,      /* 单image的BIN格式文件 */
    TYPE_DUAL_IMG = 3,        /* 双image的IMG格式文件 */
    TYPE_DUAL_BIN = 4,        /* 双image的BIN格式文件 */
    TYPE_BACKUP_IMG = 5,      /* 大小image的IMG格式文件 */
    TYPE_BACKUP_BIN = 6,      /* 大小image的BIN格式文件 */
    TYPE_OLD_SIMAGE = 7,      /* 老版本单image的IMG格式文件 */
    TYPE_OLD_DIMAGE = 8,      /* 老版本双image的IMG格式文件 */
    TYPE_OLD_BIN = 9,         /* 老版本单image的BIN格式文件 */
    TYPE_BACKUP_FIMG = 10,    /* 大小image的大IMG格式文件 */
    TYPE_BACKUP_BIMG = 11,    /* 大小image的小IMG格式文件 */
    TYPE_BOOTLOADER = 12,     /* Bootloader */
    TYPE_VIRTUAL_DIMG         /* 虚拟新版本双IMAGE文件 */    
}image_type;


/*=========================================================================
	配置信息条目数据结构
=========================================================================*/

typedef struct item_hdr {
    unsigned short crc;
    unsigned short len;
    unsigned char avail;
}
__attribute__ ((packed)) item_hdr_t;

typedef struct item {
    struct item_hdr hdr;
    unsigned char name[0];              /* 条目名称包含在数据中 */
    unsigned char data[];				/* 数据*/
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
	此数据结构用来应层接口做ioctl时与底层进行数据交互
=========================================================================*/
typedef struct
{
	unsigned char			*name;
	unsigned char			item_error;
	unsigned short			len;
	unsigned char			*data;				/* 数据*/
} item_app_t;

/*=========================================================================
	系统中使用的条目名称，条目名称最好用宏定义放在这里
	便于管理和维护
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
/*sitcom 客制化*/
#define     SITCOM_SCS_ENABLE           "scs_eanble"
#define     SITCOM_LAN_IP               "lan_ip"           //for LAN IPv4
#define     SITCOM_LAN_IPV6             "lan_ipv6"         //for LAN IPv6
#define     SITCOM_FWVER                "firmware_version" //for Firmware version
#define     SITCOM_BM_NAME              "board_model_name" //for board model name
#define     SITCOM_FW_ONSERVER          "fw_on_server"     //for FWonServer
#define     SITECOM_FWACTION            "fw_action"        //for FWaction
#define     SCS_CFG                     "scs_cfg"
#define     AUTOFW_CFG                  "autofw_cfg"
#define     TBS_WiZARD_FLAG				"wizard_flag"	//添加新的要跳到恢复出厂页面的标识 by hujian 2016-3-4
#ifndef CONFIG_MEM_SAVE
#define     TOTAL_TERMINAL_NUMBER       "totalterminalnumber"
#endif
/*=========================================================================
	在对条目操作时返回的错误码
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
	在对文件升级操作时返回的错误码
=========================================================================*/

/*=========================================================================
	应用层获取和保存条目的接口
=========================================================================*/

int app_item_get( void *data , char *item_name , unsigned short *len );		/* 获取相应条目信息*/
int app_item_save( void *data , char *item_name , unsigned short len );		/* 保存新条目*/

/*=========================================================================
	底层获取和保存条目的接口
=========================================================================*/
int item_get( void *data , char *item_name , unsigned short *len );		/* 获取相应条目信息*/
int item_save( void *data , char *item_name , unsigned short len );		/* 保存新条目*/
unsigned int item_recycle(unsigned char *item_sdram, struct item *new_it);

/*=========================================================================
	应用层获取MAC址址接口
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
	升级bin文件后恢复相关配置所需数据类型
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

