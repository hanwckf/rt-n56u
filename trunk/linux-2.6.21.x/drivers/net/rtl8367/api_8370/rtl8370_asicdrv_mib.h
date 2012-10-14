#ifndef _RTL8370_ASICDRV_MIB_H_
#define _RTL8370_ASICDRV_MIB_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_MIB_PORT_OFFSET    0x50
#define RTL8370_MIB_LEARNENTRYDISCARD_OFFSET    0x500


enum RTL8370_MIBCOUNTER{

	IfInOctets = 0,
	Dot3StatsFCSErrors,
	Dot3StatsSymbolErrors,
	Dot3InPauseFrames,
	Dot3ControlInUnknownOpcodes,		
	EtherStatsFragments,
	EtherStatsJabbers,
	IfInUcastPkts,
	EtherStatsDropEvents,
	EtherStatsOctets,

	EtherStatsUnderSizePkts,
	EtherOversizeStats,
	EtherStatsPkts64Octets,
	EtherStatsPkts65to127Octets,
	EtherStatsPkts128to255Octets,
	EtherStatsPkts256to511Octets,
	EtherStatsPkts512to1023Octets,
	EtherStatsPkts1024to1518Octets,
	EtherStatsMulticastPkts,
	EtherStatsBroadcastPkts,
	
	IfOutOctets,

	Dot3StatsSingleCollisionFrames,
	Dot3StatMultipleCollisionFrames,
	Dot3sDeferredTransmissions,
	Dot3StatsLateCollisions,
	EtherStatsCollisions,
	Dot3StatsExcessiveCollisions,
	Dot3OutPauseFrames,
	Dot1dBasePortDelayExceededDiscards,
	Dot1dTpPortInDiscards,
	IfOutUcastPkts,
	IfOutMulticastPkts,
	IfOutBroadcastPkts,
	OutOampduPkts,
	InOampduPkts,
	PktgenPkts,
	/*Device only */	
    Dot1dTpLearnEntryDiscard,
	RTL8370_MIBS_NUMBER,
	
};	

extern ret_t rtl8370_setAsicMIBsCounterReset(uint32 greset, uint32 qmreset, uint32 pmask);
extern ret_t rtl8370_getAsicMIBsCounter(uint32 port,enum RTL8370_MIBCOUNTER mibIdx,uint64* counter);
extern ret_t rtl8370_getAsicMIBsControl(uint32* mask);


#endif /*#ifndef _RTL8370_ASICDRV_MIB_H_*/

