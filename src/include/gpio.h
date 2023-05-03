#ifndef __GPIO_H__
#define __GPIO_H__

/* GPIO mode */
#define GPIO_IN				   0x0
#define GPIO_OUT			   0x1
#define GPIO_ALT_IN			   0x2
#define GPIO_ALT_OUT		   0x3
#define GPIO_E_DIS			   0x0
#define GPIO_E_RISE			   0x1
#define GPIO_E_FALL			   0x2
#define GPIO_E_BOTH			   0x3
#define GPIO_L_DIS			   0x0
#define GPIO_L_HIGH			   0x1
#define GPIO_L_LOW			   0x2
#define GPIO_L_BOTH			   0x3

#define REG8(reg)              (*((volatile unsigned char *)(reg)))
#define REGWRITE8(reg,value)   (*(volatile unsigned char *)(reg) = (volatile unsigned char)(value))
#define REG32(reg)             (*((volatile unsigned int *)(reg)))
#define REGWRITE32(reg, value) (*(volatile unsigned int *)(reg) = (volatile unsigned int)(value))

#define reg_read8(reg)          (*((volatile unsigned char *)(reg)))
#define reg_write8(reg,value)   (*(volatile unsigned char *)(reg) = (volatile unsigned char)(value))
#define reg_read32(reg)         (*((volatile unsigned int *)(reg)))
#define reg_write32(reg, value) (*(volatile unsigned int *)(reg) = (volatile unsigned int)(value))

//添加组合按键
/* GPIO number */
typedef enum {
	GPIO00 =  0, GPIO01, GPIO02, GPIO03, GPIO04, GPIO05, GPIO06, GPIO07,
	GPIO08 =  8, GPIO09, GPIO10, GPIO11, GPIO12, GPIO13, GPIO14, GPIO15,
	GPIO16 = 16, GPIO17, GPIO18, GPIO19, GPIO20, GPIO21, GPIO22, GPIO23,
	GPIO24 = 24, GPIO25, GPIO26, GPIO27, GPIO28, GPIO29, GPIO30, GPIO31,
	GPIO32 = 32, GPIO33, GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39,
	GPIO40 = 40, GPIO41, GPIO42, GPIO43, GPIO44, GPIO45, GPIO46, GPIO47,
	GPIO48 = 48, GPIO49, GPIO50, GPIO51, GPIO52, GPIO53, GPIO54, GPIO55, 
	GPIO56 = 56, GPIO57, GPIO58, GPIO59, GPIO60, GPIO61, GPIO62, GPIO63,
	GPIO64 = 64, GPIO65, GPIO66, GPIO67, GPIO68, GPIO69, GPIO70, GPIO71,
	GPIO72 = 72, GPIO73, GPIO74, GPIO75, GPIO76, GPIO77, GPIO78, GPIO79,
	GPIO80 = 80, GPIO81, GPIO82, GPIO83, GPIO84, GPIO85, GPIO86, GPIO87,
	GPIO88 = 88, GPIO89, GPIO90, GPIO91, GPIO92, GPIO93, GPIO94, GPIO95,
	GPIO96 = 96, GPIO97, GPIO98, GPIO99,
	GPIO_GROUPS = 160,
	GPIO_END
} gpio_idx;

/* GPIO status */
typedef enum {
	GPIO_LEVEL_LOW = 0,
	GPIO_LEVEL_HIGH
} gpio_level;


enum wlan_gpio_action {
	WLAN_LED_OFF = 0,
	WLAN_LED_ON,
	WLAN_LED_OEN,
	WLAN_LED_IEN
};

typedef enum {
	GPIO_GROUP0 = 0,
	GPIO_GROUP1 = 1,
	GPIO_GROUP2 = 2,
	GPIO_GROUP3 = 3,
	GPIO_GROUP4 = 4,
	GPIO_GROUP5 = 5,
	GPIO_GROUP6 = 6,
	GPIO_GROUP7 = 7,	
	GPIO_GROUP_END
} gpio_group;

/*构成组合按键映射表*/
struct gpio_map {
    char gpioname[36];//最多支持4个按键同时按下
    int gpio[4];
   
    
};

struct gpio_dev {
	gpio_group group;            /* GPIO group */ 
	unsigned int ctrl_addr;    /* Control regisger address */
	unsigned int dir_addr;     /* Direction register address */
	unsigned int data_addr;    /* Data register address */
	unsigned int edge_addr;    /* Edge control register address */
	unsigned int isr_addr;     /* Interrupt status register address */
};

extern void gpio_config(int gpio_num, int gpio_func);
extern int gpio_get_ins(int gpio_num);
extern void gpio_set_edge(int gpio_num, unsigned char edge);
extern void gpio_write(int gpio_num, gpio_level level);
extern gpio_level gpio_read(int gpio_num);

#endif
