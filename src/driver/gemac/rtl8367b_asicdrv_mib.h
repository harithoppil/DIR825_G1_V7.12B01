#ifndef _RTL8367B_ASICDRV_MIB_H_
#define _RTL8367B_ASICDRV_MIB_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_MIB_PORT_OFFSET                (0x7C)
#define RTL8367B_MIB_LEARNENTRYDISCARD_OFFSET   (0x420)

#define RTL8367B_MIB_MAX_LOG_CNT_IDX            (32-1)
#define RTL8367B_MIB_LOG_CNT_OFFSET             (0x3E0)
#define RTL8367B_MIB_MAX_LOG_MODE_IDX           (16-1)

typedef enum RTL8367B_MIBCOUNTER_E{

    /* RX */
	ifInOctets = 0,

	dot3StatsFCSErrors,
	dot3StatsSymbolErrors,
	dot3InPauseFrames,
	dot3ControlInUnknownOpcodes,	
	
	etherStatsFragments,
	etherStatsJabbers,
	ifInUcastPkts,
	etherStatsDropEvents,

    ifInMulticastPkts,
    ifInBroadcastPkts,
    inMldChecksumError,
    inIgmpChecksumError,
    inMldSpecificQuery,
    inMldGeneralQuery,
    inIgmpSpecificQuery,
    inIgmpGeneralQuery,
    inMldLeaves,
    inIgmpLeaves,

    /* TX/RX */
	etherStatsOctets,

	etherStatsUnderSizePkts,
	etherOversizeStats,
	etherStatsPkts64Octets,
	etherStatsPkts65to127Octets,
	etherStatsPkts128to255Octets,
	etherStatsPkts256to511Octets,
	etherStatsPkts512to1023Octets,
	etherStatsPkts1024to1518Octets,

    /* TX */
	ifOutOctets,

	dot3StatsSingleCollisionFrames,
	dot3StatMultipleCollisionFrames,
	dot3sDeferredTransmissions,
	dot3StatsLateCollisions,
	etherStatsCollisions,
	dot3StatsExcessiveCollisions,
	dot3OutPauseFrames,
    ifOutDiscards,

    /* ALE */
	dot1dTpPortInDiscards,
	ifOutUcastPkts,
	ifOutMulticastPkts,
	ifOutBroadcastPkts,
	outOampduPkts,
	inOampduPkts,

    inIgmpJoinsSuccess,
    inIgmpJoinsFail,
    inMldJoinsSuccess,
    inMldJoinsFail,
    inReportSuppressionDrop,
    inLeaveSuppressionDrop,
    outIgmpReports,
    outIgmpLeaves,
    outIgmpGeneralQuery,
    outIgmpSpecificQuery,
    outMldReports,
    outMldLeaves,
    outMldGeneralQuery,
    outMldSpecificQuery,
    inKnownMulticastPkts,

	/*Device only */	
	dot1dTpLearnedEntryDiscards,
	RTL8367B_MIBS_NUMBER,
	
}RTL8367B_MIBCOUNTER;	

extern int rtl8367b_setAsicMIBsCounterReset(unsigned int greset, unsigned int qmreset, unsigned int pmask);
extern int rtl8367b_getAsicMIBsCounter(unsigned int port,RTL8367B_MIBCOUNTER mibIdx, unsigned long long* pCounter);
extern int rtl8367b_getAsicMIBsLogCounter(unsigned int index, unsigned int *pCounter);
extern int rtl8367b_getAsicMIBsControl(unsigned int* pMask);

extern int rtl8367b_setAsicMIBsResetValue(unsigned int value);
extern int rtl8367b_getAsicMIBsResetValue(unsigned int* value);

extern int rtl8367b_setAsicMIBsUsageMode(unsigned int mode);
extern int rtl8367b_getAsicMIBsUsageMode(unsigned int* pMode);
extern int rtl8367b_setAsicMIBsTimer(unsigned int timer);
extern int rtl8367b_getAsicMIBsTimer(unsigned int* pTimer);
extern int rtl8367b_setAsicMIBsLoggingMode(unsigned int index, unsigned int mode);
extern int rtl8367b_getAsicMIBsLoggingMode(unsigned int index, unsigned int* pMode);
extern int rtl8367b_setAsicMIBsLoggingType(unsigned int index, unsigned int type);
extern int rtl8367b_getAsicMIBsLoggingType(unsigned int index, unsigned int* pType);
extern int rtl8367b_setAsicMIBsResetLoggingCounter(unsigned int index);

#endif /*#ifndef _RTL8367B_ASICDRV_MIB_H_*/

