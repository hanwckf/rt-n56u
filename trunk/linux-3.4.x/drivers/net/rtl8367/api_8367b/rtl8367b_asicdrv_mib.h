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

extern ret_t rtl8367b_setAsicMIBsCounterReset(rtk_uint32 greset, rtk_uint32 qmreset, rtk_uint32 pmask);
extern ret_t rtl8367b_getAsicMIBsCounter(rtk_uint32 port,RTL8367B_MIBCOUNTER mibIdx, rtk_uint64* pCounter);
extern ret_t rtl8367b_getAsicMIBsLogCounter(rtk_uint32 index, rtk_uint32 *pCounter);
extern ret_t rtl8367b_getAsicMIBsControl(rtk_uint32* pMask);

extern ret_t rtl8367b_setAsicMIBsResetValue(rtk_uint32 value);
extern ret_t rtl8367b_getAsicMIBsResetValue(rtk_uint32* value);

extern ret_t rtl8367b_setAsicMIBsUsageMode(rtk_uint32 mode);
extern ret_t rtl8367b_getAsicMIBsUsageMode(rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicMIBsTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicMIBsTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32 mode);
extern ret_t rtl8367b_getAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32 type);
extern ret_t rtl8367b_getAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32* pType);
extern ret_t rtl8367b_setAsicMIBsResetLoggingCounter(rtk_uint32 index);

#endif /*#ifndef _RTL8367B_ASICDRV_MIB_H_*/

