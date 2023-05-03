#ifndef _RTL8367B_ASICDRV_RMA_H_
#define _RTL8367B_ASICDRV_RMA_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_RMAMAX                     0x2F

enum RTL8367B_RMAOP
{
    RMAOP_FORWARD = 0,
    RMAOP_TRAP_TO_CPU,
    RMAOP_DROP,
    RMAOP_FORWARD_EXCLUDE_CPU,
};


typedef struct  rtl8367b_rma_s{

#ifdef _LITTLE_ENDIAN
    unsigned short portiso_leaky:1; 
    unsigned short vlan_leaky:1;
    unsigned short keep_format:1;
    unsigned short trap_priority:3;
    unsigned short discard_storm_filter:1;
    unsigned short operation:2;
    unsigned short reserved:7;
#else
    unsigned short reserved:7;
    unsigned short operation:2; 
    unsigned short discard_storm_filter:1;
    unsigned short trap_priority:3;
    unsigned short keep_format:1;
    unsigned short vlan_leaky:1;
    unsigned short portiso_leaky:1; 
#endif

}rtl8367b_rma_t;


extern int rtl8367b_setAsicRma(unsigned int index, rtl8367b_rma_t* pRmacfg);
extern int rtl8367b_getAsicRma(unsigned int index, rtl8367b_rma_t* pRmacfg);

#endif /*#ifndef _RTL8367B_ASICDRV_RMA_H_*/

