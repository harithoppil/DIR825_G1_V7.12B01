#ifndef _RTL8367B_ASICDRV_SCHEDULING_H_
#define _RTL8367B_ASICDRV_SCHEDULING_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_QWEIGHTMAX    0x7F
#define RTL8367B_PORT_QUEUE_METER_INDEX_MAX    7

/* enum for queue type */
enum QUEUETYPE
{
	QTYPE_STRICT = 0,
	QTYPE_WFQ,
};
extern int rtl8367b_setAsicLeakyBucketParameter(unsigned int tick, unsigned int token);
extern int rtl8367b_getAsicLeakyBucketParameter(unsigned int *tick, unsigned int *token);
extern int rtl8367b_setAsicAprMeter(unsigned int port, unsigned int qid, unsigned int apridx);
extern int rtl8367b_getAsicAprMeter(unsigned int port, unsigned int qid, unsigned int *apridx);
extern int rtl8367b_setAsicPprMeter(unsigned int port, unsigned int qid, unsigned int ppridx);
extern int rtl8367b_getAsicPprMeter(unsigned int port, unsigned int qid, unsigned int *ppridx);
extern int rtl8367b_setAsicAprEnable(unsigned int port, unsigned int aprEnable);
extern int rtl8367b_getAsicAprEnable(unsigned int port, unsigned int *aprEnable);
extern int rtl8367b_setAsicPprEnable(unsigned int port, unsigned int pprEnable);
extern int rtl8367b_getAsicPprEnable(unsigned int port, unsigned int *pprEnable);

extern int rtl8367b_setAsicWFQWeight(unsigned int, unsigned int queueid, unsigned int weight );
extern int rtl8367b_getAsicWFQWeight(unsigned int, unsigned int queueid, unsigned int *weight );
extern int rtl8367b_setAsicWFQBurstSize(unsigned int burstsize);
extern int rtl8367b_getAsicWFQBurstSize(unsigned int *burstsize);

extern int rtl8367b_setAsicQueueType(unsigned int port, unsigned int qid, unsigned int queueType);
extern int rtl8367b_getAsicQueueType(unsigned int port, unsigned int qid, unsigned int *queueType);
extern int rtl8367b_setAsicQueueRate(unsigned int port, unsigned int qid, unsigned int ppridx, unsigned int apridx );
extern int rtl8367b_getAsicQueueRate(unsigned int port, unsigned int qid, unsigned int* ppridx, unsigned int* apridx );
extern int rtl8367b_setAsicPortEgressRate(unsigned int port, unsigned int rate);
extern int rtl8367b_getAsicPortEgressRate(unsigned int port, unsigned int *rate);
extern int rtl8367b_setAsicPortEgressRateIfg(unsigned int ifg);
extern int rtl8367b_getAsicPortEgressRateIfg(unsigned int *ifg);

#endif /*_RTL8367B_ASICDRV_SCHEDULING_H_*/

