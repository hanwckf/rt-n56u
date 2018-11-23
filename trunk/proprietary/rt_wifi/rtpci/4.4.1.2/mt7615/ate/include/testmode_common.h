
#ifndef _TESTMODE_COMMON_H 
#define _TESTMODE_COMMON_H

#ifndef COMPOS_TESTMODE_WIN					
typedef enum{
	EP4QID = 0,
	EP5QID,
	EP6QID,
	EP7QID,
	EP8QID = 8,
	EP9QID,	
	EPALLQID
}EPQID;
#endif
/*test mode common*/
//
// Scheduler Register 4 (offset: 0x0594, default: 0x0000_0000) 
// Note: 
//  1. DW : double word
//
typedef union _SCHEDULER_REGISTER4
{
	struct	{
	    // DW0
	    ULONG   ForceQid:4;                                 // [3 : 0]      When force_mode = 1, queue id will adopt this index.
	    ULONG   ForceMode:1;                            // [4]      Force enable dma_scheduler work in force queue without calculate TXOP time.
	    ULONG   BypassMode:1;                           // [5]      Bypass_mode:1, for PSE loopback only.
	    ULONG   HybridMode:1;                           // [6]      Hybrid_mode:1, when assert this reg, DMA scheduler would ignore the tx op time information from LMAC, and also use FFA and RSV for enqueue cal.
	    ULONG   RgPredictNoMask:1;                      // [7]      When disable high and queue mask, force in predict mode.
	    ULONG   RgResetScheduler:1;                         // [8]      DMA Scheduler soft reset. When assert this reset, dma scheduler state machine will be in INIT state and reset all queue counter.
	    ULONG   RgDoneClearHeader:1;                    // [9]      Switch header counter clear flag. 0 : Clear by tx_eof   1 : Clear by sch_txdone
	    ULONG   SwMode:1;                               // [10]     When SwMode is 1 : DMA scheduler will ignore rate mapping & txop time threshold from LMAC, and use RgRateMap & txtime_thd_* value.
	    ULONG   Reserves0:5;                            // [15 : 11]    Reserved bits.
	    ULONG   RgRateMap:14;                           // [29 : 16]    When SwMode 1 : DMA scheduler will load rate mapping information from this register, else rate mapping information from LMAC. Rate Mapping table : {mode X [2 : 0], rate [5 : 0], frmode [1 : 0], nss [1:  0], sg_flag}
	    ULONG   Reserves1:2;                            // [31 : 30]    Reserved bits.
	}	Default;
	ULONG			word;
} SCHEDULER_REGISTER4, *PSCHEDULER_REGISTER4;

// MT7637 for Band display
#define MT7367_RO_AGC_DEBUG_2	WF_PHY_BASE + 0x0584
#define CR_ACI_HIT          WF_PHY_BASE + 0x0594
// MT7637 for Band display end


/*~test mode common*/

#endif //_TESTMODE_COMMON_H
