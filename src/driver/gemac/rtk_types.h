#ifndef _RTL8367B_TYPES_H_
#define _RTL8367B_TYPES_H_

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdio.h>
#endif

#ifndef _RTL_TYPES_H

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_addr_s {
	unsigned char octet[ETHER_ADDR_LEN];
} ether_addr_t;

#ifdef _LINUX_KERNEL_H
#define PRINT printk
#else
#define PRINT printf
#endif
#endif /*_RTL_TYPES_H*/

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif /* _RTL8367B_TYPES_H_ */
