#ifndef _RTL8367B_ASICDRV_FC_H_
#define _RTL8367B_ASICDRV_FC_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_PAGE_NUMBER    0x400


enum FLOW_CONTROL_TYPE
{
    FC_EGRESS = 0,
    FC_INGRESS, 
};

enum FC_JUMBO_SIZE
{
    FC_JUMBO_SIZE_3K = 0,
    FC_JUMBO_SIZE_4K,
    FC_JUMBO_SIZE_6K,
    FC_JUMBO_SIZE_9K,
    FC_JUMBO_SIZE_END,
    
};


extern int rtl8367b_setAsicFlowControlSelect(unsigned int select);
extern int rtl8367b_getAsicFlowControlSelect(unsigned int *pSelect);
extern int rtl8367b_setAsicFlowControlJumboMode(unsigned int enabled);
extern int rtl8367b_getAsicFlowControlJumboMode(unsigned int* pEnabled);
extern int rtl8367b_setAsicFlowControlJumboModeSize(unsigned int size);
extern int rtl8367b_getAsicFlowControlJumboModeSize(unsigned int* pSize);
extern int rtl8367b_setAsicFlowControlQueueEgressEnable(unsigned int port, unsigned int qid, unsigned int enabled);
extern int rtl8367b_getAsicFlowControlQueueEgressEnable(unsigned int port, unsigned int qid, unsigned int* pEnabled);
extern int rtl8367b_setAsicFlowControlDropAll(unsigned int dropall);
extern int rtl8367b_getAsicFlowControlDropAll(unsigned int* pDropall);
extern int rtl8367b_setAsicFlowControlPauseAllThreshold(unsigned int threshold);
extern int rtl8367b_getAsicFlowControlPauseAllThreshold(unsigned int *pThreshold);
extern int rtl8367b_setAsicFlowControlSystemThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSystemThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlSharedThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSharedThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortPrivateThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortPrivateThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlSystemDropThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSystemDropThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlSharedDropThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSharedDropThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortDropThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortDropThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortPrivateDropThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortPrivateDropThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlSystemJumboThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSystemJumboThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlSharedJumboThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlSharedJumboThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortJumboThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortJumboThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);
extern int rtl8367b_setAsicFlowControlPortPrivateJumboThreshold(unsigned int onThreshold, unsigned int offThreshold);
extern int rtl8367b_getAsicFlowControlPortPrivateJumboThreshold(unsigned int *pOnThreshold, unsigned int *pOffThreshold);

extern int rtl8367b_setAsicEgressFlowControlPortDropGap(unsigned int gap);
extern int rtl8367b_getAsicEgressFlowControlPortDropGap(unsigned int *pGap);
extern int rtl8367b_setAsicEgressFlowControlQueueDropGap(unsigned int gap);
extern int rtl8367b_getAsicEgressFlowControlQueueDropGap(unsigned int *pGap);
extern int rtl8367b_setAsicEgressFlowControlPortDropThreshold(unsigned int port, unsigned int threshold);
extern int rtl8367b_getAsicEgressFlowControlPortDropThreshold(unsigned int port, unsigned int *pThreshold);
extern int rtl8367b_setAsicEgressFlowControlQueueDropThreshold(unsigned int qid, unsigned int threshold);
extern int rtl8367b_getAsicEgressFlowControlQueueDropThreshold(unsigned int qid, unsigned int *pThreshold);
extern int rtl8367b_getAsicEgressQueueEmptyPortMask(unsigned int *pPortmask);
extern int rtl8367b_getAsicTotalPage(unsigned int *pPageCount);
extern int rtl8367b_getAsicPulbicPage(unsigned int *pPageCount);
extern int rtl8367b_getAsicMaxTotalPage(unsigned int *pPageCount);
extern int rtl8367b_getAsicMaxPulbicPage(unsigned int *pPageCount);
extern int rtl8367b_getAsicPortPage(unsigned int port, unsigned int *pPageCount);
extern int rtl8367b_getAsicPortPageMax(unsigned int port, unsigned int *pPageCount);
extern int rtl8367b_setAsicFlowControlEgressPortIndep(unsigned int port, unsigned int enable);
extern int rtl8367b_getAsicFlowControlEgressPortIndep(unsigned int port, unsigned int *pEnable);

#endif /*_RTL8367B_ASICDRV_FC_H_*/

