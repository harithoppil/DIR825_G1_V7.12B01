#ifndef _RTL8367B_ASICDRV_QOS_H_
#define _RTL8367B_ASICDRV_QOS_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_DECISIONPRIMAX    0xFF

/* enum Priority Selection Types */
enum PRIDECISION
{
	PRIDEC_PORT = 0,
	PRIDEC_ACL,
	PRIDEC_DSCP,
	PRIDEC_1Q,
	PRIDEC_1AD,
	PRIDEC_CVLAN,
	PRIDEC_DA,
	PRIDEC_SA,
	PRIDEC_END,
};

extern int rtl8367b_setAsicRemarkingDot1pAbility(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicRemarkingDot1pAbility(unsigned int port, unsigned int* pEnabled);
extern int rtl8367b_setAsicRemarkingDot1pParameter(unsigned int priority, unsigned int newPriority );
extern int rtl8367b_getAsicRemarkingDot1pParameter(unsigned int priority, unsigned int *pNewPriority );
extern int rtl8367b_setAsicRemarkingDscpAbility(unsigned int enabled);
extern int rtl8367b_getAsicRemarkingDscpAbility(unsigned int* pEnabled);
extern int rtl8367b_setAsicRemarkingDscpParameter(unsigned int priority, unsigned int newDscp );
extern int rtl8367b_getAsicRemarkingDscpParameter(unsigned int priority, unsigned int* pNewDscp );

extern int rtl8367b_setAsicPriorityDot1qRemapping(unsigned int srcpriority, unsigned int priority );
extern int rtl8367b_getAsicPriorityDot1qRemapping(unsigned int srcpriority, unsigned int *pPriority );
extern int rtl8367b_setAsicPriorityDscpBased(unsigned int dscp, unsigned int priority );
extern int rtl8367b_getAsicPriorityDscpBased(unsigned int dscp, unsigned int *pPriority );
extern int rtl8367b_setAsicPriorityPortBased(unsigned int port, unsigned int priority );
extern int rtl8367b_getAsicPriorityPortBased(unsigned int port, unsigned int *pPriority );
extern int rtl8367b_setAsicPriorityDecision(unsigned int prisrc, unsigned int decisionPri);
extern int rtl8367b_getAsicPriorityDecision(unsigned int prisrc, unsigned int* pDecisionPri);
extern int rtl8367b_setAsicPriorityToQIDMappingTable(unsigned int qnum, unsigned int priority, unsigned int qid );
extern int rtl8367b_getAsicPriorityToQIDMappingTable(unsigned int qnum, unsigned int priority, unsigned int* pQid);
extern int rtl8367b_setAsicOutputQueueMappingIndex(unsigned int port, unsigned int qnum );
extern int rtl8367b_getAsicOutputQueueMappingIndex(unsigned int port, unsigned int *pQnum );

#endif /*#ifndef _RTL8367B_ASICDRV_QOS_H_*/

