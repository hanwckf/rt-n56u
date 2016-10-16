/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 *
  Module Name:
  ralink_gdma.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2009-03-24      Support RT3883
 *
 */
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/addrspace.h>

#include <asm/rt2880/surfboardint.h>

#include <ralink/ralink_gdma.h>

/*
 * RT305x:
 * Ch0 : Pcm0_Rx0 | Pcm0_Rx0 | ALL
 * Ch1 : Pcm0_Rx1 | Pcm0_Rx1 | ALL
 * Ch2 : Pcm0_Tx0 | Pcm0_Tx0 | ALL
 * Ch3 : Pcm0_Tx1 | Pcm0_Tx1 | ALL
 * Ch4 : Pcm1_Rx0 | I2S_Tx0  | ALL
 * Ch5 : Pcm1_Rx1 | I2S_Tx1  | ALL
 * Ch6 : Pcm1_Tx0 |  ALL     | ALL
 * Ch7 : Pcm1_Tx1 |  ALL     | ALL
 *
 * RT3883:
 * Ch0  : Pcm0_Rx0 | Pcm0_Rx0 | ALL
 * Ch1  : Pcm0_Rx1 | Pcm0_Rx1 | ALL
 * Ch2  : Pcm0_Tx0 | Pcm0_Tx0 | ALL
 * Ch3  : Pcm0_Tx1 | Pcm0_Tx1 | ALL
 * Ch4  : Pcm1_Rx0 | I2S_Tx0  | ALL
 * Ch5  : Pcm1_Rx1 | I2S_Tx1  | ALL
 * Ch6  : Pcm1_Tx0 | I2S_Rx0  | ALL
 * Ch7  : Pcm1_Tx1 | I2S_Rx1  | ALL
 * Ch8  : ALL	   |  ALL     | ALL
 * Ch9  : ALL	   |  ALL     | ALL
 * Ch10 : ALL	   |  ALL     | ALL
 * Ch11 : ALL	   |  ALL     | ALL
 * Ch12 : ALL	   |  ALL     | ALL PCI TX
 * Ch13 : ALL	   |  ALL     | ALL PCI RX
 * Ch14 : ALL	   |  ALL     | ALL
 * Ch15 : ALL	   |  ALL     | ALL
 *
 */

spinlock_t  gdma_lock;
spinlock_t  gdma_int_lock;
void (*GdmaDoneIntCallback[MAX_GDMA_CHANNEL])(uint32_t);
void (*GdmaUnMaskIntCallback[MAX_GDMA_CHANNEL])(uint32_t);


/**
 * @brief Get free GDMA channel
 *
 * @param  ChNum   GDMA channel number
 * @retval 1  	   channel is available
 * @retval 0  	   channels are all busy
 */
int _GdmaGetFreeCh(uint32_t *ChNum)
{
    unsigned long flags;
    uint32_t Data=0;
    uint32_t Ch=0;
#if defined (CONFIG_GDMA_DEBUG)
    static uint32_t Ch_RR=0;
#endif

    spin_lock_irqsave(&gdma_lock, flags);

#if defined (CONFIG_GDMA_PCM_ONLY)
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
    for(Ch=14; Ch<MAX_GDMA_CHANNEL;Ch++)  //channel 14~max_channe, channel 0~13 be usedl
#else
    for(Ch=MAX_GDMA_CHANNEL; Ch<MAX_GDMA_CHANNEL;Ch++)  //no free channel
#endif
#elif defined (CONFIG_GDMA_PCM_I2S_OTHERS)
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
    for(Ch=14; Ch<MAX_GDMA_CHANNEL;Ch++)  //channel 14~max_channe, channel 0~13 be usedl
#else
    for(Ch=6; Ch<MAX_GDMA_CHANNEL;Ch++)  //channel 6~max_channel
#endif
#elif defined (CONFIG_GDMA_EVERYBODY)
    for(Ch=0; Ch<MAX_GDMA_CHANNEL;Ch++)  //all channel
#elif defined (CONFIG_GDMA_DEBUG)
    for(Ch=(Ch_RR++)%MAX_GDMA_CHANNEL; Ch<MAX_GDMA_CHANNEL;Ch++)  //round robin
#endif
    {
	Data=GDMA_READ_REG(GDMA_CTRL_REG(Ch));

	/* hardware will reset this bit if transaction is done.
	 * It means channel is free */
	if((Data & (0x01<<CH_EBL_OFFSET))==0) { 
	    *ChNum = Ch;
	    spin_unlock_irqrestore(&gdma_lock, flags);
	    return 1; //Channel is free
	}
    }

    spin_unlock_irqrestore(&gdma_lock, flags);
    return 0; // Channels are all busy

}

/**
 * @brief Set channel is masked
 *
 * When channel is masked, the GDMA transaction will stop. 
 * When GDMA controller comes back from another channel (chain feature)
 *
 * >> Channel Mask=0: It's strange, and turns on related bit in GDMA interrupt
 * status register (16:23 Unmasked)
 *
 * >> Channel Mask=1: It'll start GDMA transation, and clear this bit. 
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaMaskChannel(uint32_t ChNum)
{
    uint32_t Data=0;

    Data=GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data |= ( 0x01 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);
    GDMA_PRINT("%s: Write %0X to %X\n", __FUNCTION__, Data, GDMA_CTRL_REG1(ChNum));

    return 1;
}

/**
 * @brief Set channel is unmasked
 *
 * You can unmask the channel to start GDMA transaction. 
 *
 * When GDMA controller comes back from another channel (chain feature)
 *
 * >> Channel Mask=0: It's strange, and turns on related bit in GDMA interrupt
 * status register (16:23 Unmasked)
 *
 * >> Channel Mask=1: It'll start GDMA transation, and clear this bit. 
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaUnMaskChannel(uint32_t ChNum)
{
    uint32_t Data=0;

    Data=GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data &= ~( 0x01 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);
    GDMA_PRINT("%s: Write %0X to %X\n", __FUNCTION__, Data, GDMA_CTRL_REG1(ChNum));

    return 1;
}

/**
 * @brief Insert new GDMA entry to start GDMA transaction
 *
 * @param  ChNum   	GDMA channel number
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaReqQuickIns(uint32_t ChNum)
{
    uint32_t Data=0;

    //Mask Channel
    Data = GDMA_READ_REG(GDMA_CTRL_REG1(ChNum));
    Data |= ( 0x1 << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(ChNum), Data);

    //Channel Enable
    Data = GDMA_READ_REG(GDMA_CTRL_REG(ChNum));
    Data |= (0x01<<CH_EBL_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG(ChNum), Data);

    return 1;

}

int _GdmaReqEntryIns(GdmaReqEntry *NewEntry)
{
    uint32_t Data=0;

    GDMA_PRINT("== << GDMA Control Reg (Channel=%d) >> ===\n", NewEntry->ChNum);
    GDMA_PRINT(" Channel Source Addr = %x \n", NewEntry->Src);
    GDMA_PRINT(" Channel Dest Addr = %x \n", NewEntry->Dst);
    GDMA_PRINT(" Transfer Count=%d\n", NewEntry->TransCount);
    GDMA_PRINT(" Source DMA Req= DMA_REQ%d\n", NewEntry->SrcReqNum);
    GDMA_PRINT(" Dest DMA Req= DMA_REQ%d\n", NewEntry->DstReqNum);
    GDMA_PRINT(" Source Burst Mode=%s\n", NewEntry->SrcBurstMode ? "Fix" : "Inc");
    GDMA_PRINT(" Dest Burst Mode=%s\n", NewEntry->DstBurstMode ? "Fix" : "Inc");
    GDMA_PRINT(" Burst Size=%s\n", NewEntry->BurstSize ==0 ? "1 transfer" : \
	    NewEntry->BurstSize ==1 ? "2 transfer" :\
	    NewEntry->BurstSize ==2 ? "4 transfer" :\
	    NewEntry->BurstSize ==3 ? "8 transfer" :\
	    NewEntry->BurstSize ==4 ? "16 transfer" :\
	    "Error");
    GDMA_PRINT(" Hardware/Software Mode = %s\n", NewEntry->SoftMode ?
	    "Soft" : "Hw");
    GDMA_PRINT("== << GDMA Control Reg1 (Channel=%d) >> =\n", NewEntry->ChNum);
    GDMA_PRINT("Channel Done Interrput=%s\n", (NewEntry->DoneIntCallback!=NULL) ? 
	    "Enable" : "Disable");
    GDMA_PRINT("Channel Unmasked Int=%s\n", (NewEntry->UnMaskIntCallback!=NULL) ? 
	    "Enable" : "Disable");
#if !defined (CONFIG_RALINK_RT3052) && !defined (CONFIG_RALINK_RT3883)
    GDMA_PRINT("Coherent Interrupt =%s\n", (NewEntry->CoherentIntEbl==1)?
	    "Enable" : "Disable");
#endif
    GDMA_PRINT("Next Unmasked Channel=%d\n", NewEntry->NextUnMaskCh);
    GDMA_PRINT("Channel Mask=%d\n", NewEntry->ChMask);
    GDMA_PRINT("========================================\n");

    GDMA_WRITE_REG(GDMA_SRC_REG(NewEntry->ChNum), NewEntry->Src);
    GDMA_PRINT("SrcAddr: Write %0X to %X\n", \
	    NewEntry->Src, GDMA_SRC_REG(NewEntry->ChNum));

    GDMA_WRITE_REG(GDMA_DST_REG(NewEntry->ChNum), NewEntry->Dst);
    GDMA_PRINT("DstAddr: Write %0X to %X\n", \
	    NewEntry->Dst, GDMA_DST_REG(NewEntry->ChNum));

    Data |= ( (NewEntry->NextUnMaskCh) << NEXT_UNMASK_CH_OFFSET); 
    Data |= ( NewEntry->ChMask << CH_MASK_OFFSET); 
#if !defined (CONFIG_RALINK_RT3052) && !defined (CONFIG_RALINK_RT3883)
    Data |= ( NewEntry->CoherentIntEbl << COHERENT_INT_EBL_OFFSET); 
#endif

    if(NewEntry->UnMaskIntCallback!=NULL) {
	Data |= (0x01<<CH_UNMASKINT_EBL_OFFSET); 
	GdmaUnMaskIntCallback[NewEntry->ChNum] = NewEntry->UnMaskIntCallback;
    }

#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
    Data |= (NewEntry->SrcReqNum << SRC_DMA_REQ_OFFSET); 
    Data |= (NewEntry->DstReqNum << DST_DMA_REQ_OFFSET); 
#endif

    GDMA_WRITE_REG(GDMA_CTRL_REG1(NewEntry->ChNum), Data);
    GDMA_PRINT("CTRL1: Write %08X to %8X\n", Data, GDMA_CTRL_REG1(NewEntry->ChNum));

    Data = ((NewEntry->TransCount) << TRANS_CNT_OFFSET); 
#if defined (CONFIG_RALINK_RT3052)
    Data |= (NewEntry->SrcReqNum << SRC_DMA_REQ_OFFSET); 
    Data |= (NewEntry->DstReqNum << DST_DMA_REQ_OFFSET); 
#endif
    Data |= (NewEntry->SrcBurstMode << SRC_BRST_MODE_OFFSET); 
    Data |= (NewEntry->DstBurstMode << DST_BRST_MODE_OFFSET); 
    Data |= (NewEntry->BurstSize << BRST_SIZE_OFFSET); 

    if(NewEntry->DoneIntCallback!=NULL) {
	Data |= (0x01<<CH_DONEINT_EBL_OFFSET); 
	GdmaDoneIntCallback[NewEntry->ChNum] = NewEntry->DoneIntCallback;
    }

    if(NewEntry->SoftMode) {
	Data |= (0x01<<MODE_SEL_OFFSET); 
    }

    Data |= (0x01<<CH_EBL_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG(NewEntry->ChNum), Data);
    //GDMA_READ_REG(GDMA_CTRL_REG(NewEntry->ChNum));
    GDMA_PRINT("CTRL: Write %08X to %8X\n", Data, GDMA_CTRL_REG(NewEntry->ChNum));    
     //if there is no interrupt handler, this function will 
    //return 1 until GDMA done.
    if(NewEntry->DoneIntCallback==NULL) { 
	//wait for GDMA processing done
#if defined (CONFIG_RALINK_RT3052)	
	while((GDMA_READ_REG(RALINK_GDMAISTS) & 
		    (0x1<<NewEntry->ChNum))==0); 
	//write 1 clear
	GDMA_WRITE_REG(RALINK_GDMAISTS, 1<< NewEntry->ChNum); 
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	while((GDMA_READ_REG(RALINK_GDMA_DONEINT) & 
		    (0x1<<NewEntry->ChNum))==0); 
	//write 1 clear
	GDMA_WRITE_REG(RALINK_GDMA_DONEINT, 1<< NewEntry->ChNum); 
#endif
    }

    return 1;

}

#if defined(CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
/**
 * @brief Start GDMA transaction for sending data to SPI
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address

 * @param  TransCount  	data length
 * @param  *DoneIntCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaSpiTx(
	uint32_t Src, 
	uint32_t Dst, 
	uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=FIX_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_MEM_REQ;
    Entry.DstReqNum=DMA_SPI_TX_REQ;
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChMask=0;
    Entry.CoherentIntEbl=0;
  
	//enable chain feature
	Entry.ChNum = GDMA_SPI_TX;
	Entry.NextUnMaskCh = GDMA_SPI_TX;

    return _GdmaReqEntryIns(&Entry);
}

int GdmaSpiRx(
	uint32_t Src, 
	uint32_t Dst, 
	uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=FIX_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_SPI_RX_REQ;
    Entry.DstReqNum=DMA_MEM_REQ;
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChMask=0;
    Entry.CoherentIntEbl=1;
    

	//enable chain feature
	Entry.ChNum=GDMA_SPI_RX;
	Entry.NextUnMaskCh=GDMA_SPI_RX;
    

    return _GdmaReqEntryIns(&Entry);

}
#endif


/**
 * @brief Start GDMA transaction for sending data to I2S
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address
 * @param  TxNo    	I2S Tx number 
 * @param  TransCount  	data length
 * @param  *DoneIntCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaI2sTx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t TxNo,
	uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=FIX_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_MEM_REQ;
    Entry.DstReqNum=DMA_I2S_TX_REQ;
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChMask=1;
    Entry.CoherentIntEbl=0;
   
    if(TxNo==0) { //TX0
	//enable chain feature
	Entry.ChNum=GDMA_I2S_TX0;
	Entry.NextUnMaskCh= (TransCount==4) ? GDMA_I2S_TX0 : GDMA_I2S_TX1;
    }else if(TxNo==1) { //TX1
	//enable chain feature
	Entry.ChNum=GDMA_I2S_TX1;
	Entry.NextUnMaskCh= (TransCount==4) ? GDMA_I2S_TX1 : GDMA_I2S_TX0;
    }else {
	GDMA_PRINT("I2S Tx Number %x is invalid\n", TxNo);
	return 0;
    }

    return _GdmaReqEntryIns(&Entry);

}


#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
/**
 * @brief Start GDMA transaction for receiving data to I2S
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address
 * @param  TxNo    	I2S Tx number 
 * @param  TransCount  	data length
 * @param  *DoneIntCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaI2sRx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t RxNo,
	uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=FIX_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_I2S_RX_REQ;
    Entry.DstReqNum=DMA_MEM_REQ;
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChMask=1;
    Entry.CoherentIntEbl=1;
    
    if(RxNo==0) { //RX0
	//enable chain feature
	Entry.ChNum=GDMA_I2S_RX0;
	Entry.NextUnMaskCh=(TransCount==4) ? GDMA_I2S_RX0 : GDMA_I2S_RX1;
    }else if(RxNo==1) { //RX1
	//enable chain feature
	Entry.ChNum=GDMA_I2S_RX1;
	Entry.NextUnMaskCh=(TransCount==4) ? GDMA_I2S_RX1 : GDMA_I2S_RX0;
    }else {
	GDMA_PRINT("I2S Rx Number %x is invalid\n", RxNo);
	return 0;
    }

    return _GdmaReqEntryIns(&Entry);

}

#endif

/**
 * @brief Start GDMA transaction for receiving data from PCM
 *
 * @param  *Src   	source address
 * @param  *Dst    	destination address
 * @param  TransCount   data length
 * @param  PcmNo    	PCM channel
 * @param  RxNo    	PCM Rx number 
 * @param  *DoneIntCallback  callback function when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1  	   	success
 * @retval 0  	   	fail
 */
int GdmaPcmRx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t PcmNo,
	uint8_t RxNo,
	uint16_t TransCount, 
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=FIX_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.DstReqNum=DMA_MEM_REQ; 
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0;
    Entry.ChMask=1;
    Entry.CoherentIntEbl=1;

	if((RxNo<0)||(RxNo > 2)) {
		GDMA_PRINT("PCM Rx Number %x is invalid\n", RxNo);
		return 0;
	}

	switch(PcmNo)
	{
	case 0:
		Entry.SrcReqNum=DMA_PCM_RX0_REQ;
		break;
	case 1:
		Entry.SrcReqNum=DMA_PCM_RX1_REQ;
		break;
#if defined(CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	case 2:
		Entry.SrcReqNum=DMA_PCM_RX2_REQ;
		break;
	case 3:
		Entry.SrcReqNum=DMA_PCM_RX3_REQ;
		break;
#endif
	default:
		GDMA_PRINT("PCM Channel %x is invalid\n", PcmNo);
		return 0;
	}
	Entry.ChNum=GDMA_PCM_RX(PcmNo,RxNo);
	Entry.NextUnMaskCh=GDMA_PCM_RX(PcmNo,1-RxNo);

    return _GdmaReqEntryIns(&Entry);

}

/**
 * @brief Start GDMA transaction for sending data to PCM
 *
 * @param  *Src		    source address
 * @param  *Dst		    destination address
 * @param  TransCount	    data length
 * @param  PcmNo	    PCM channel
 * @param  TxNo		    PCM Tx number 
 * @param  *DoneIntCallback  callback func when transcation is done
 * @param  *UnMaskIntCallback  callback func when ch mask field is incorrect
 * @retval 1		    success
 * @retval 0		    fail
 */
int GdmaPcmTx(
	uint32_t Src, 
	uint32_t Dst, 
	uint8_t PcmNo,
	uint8_t TxNo,
	uint16_t TransCount, 
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data)
	)
{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=FIX_MODE;
    Entry.BurstSize=BUSTER_SIZE_4B; 
    Entry.SrcReqNum=DMA_MEM_REQ; 
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=UnMaskIntCallback;
    Entry.SoftMode=0; //Hardware Mode
    Entry.ChMask=1;
    Entry.CoherentIntEbl=0;

	if((TxNo<0)||(TxNo > 2)) {
        GDMA_PRINT("PCM Tx Number %x is invalid\n", TxNo);
		return 0;
	}
	switch(PcmNo)
	{
	case 0:
		Entry.DstReqNum=DMA_PCM_TX0_REQ;
		break;
	case 1:
		Entry.DstReqNum=DMA_PCM_TX1_REQ;
		break;
#if defined(CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	case 2:
		Entry.DstReqNum=DMA_PCM_TX2_REQ;
		break;
	case 3:
		Entry.DstReqNum=DMA_PCM_TX3_REQ;
		break;
#endif
	default:
		GDMA_PRINT("PCM Channel %x is invalid\n", PcmNo);	
		return 0;
	}
	Entry.ChNum=GDMA_PCM_TX(PcmNo,TxNo);
	Entry.NextUnMaskCh=GDMA_PCM_TX(PcmNo,1-TxNo);

    return _GdmaReqEntryIns(&Entry);

}


/**
 * @brief Start GDMA transaction for memory to memory copy
 *
 * @param  *Src		    source address
 * @param  *Dst		    destination address
 * @param  TransCount	    data length
 * @param  *DoneIntCallback  callback function when transcation is done
 * @retval 1		    success
 * @retval 0		    fail
 */
int GdmaMem2Mem(
	uint32_t Src, 
	uint32_t Dst, 
	uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data)
	)

{
    GdmaReqEntry Entry;

    Entry.Src= (Src & 0x1FFFFFFF);
    Entry.Dst= (Dst & 0x1FFFFFFF);
    Entry.TransCount = TransCount;
    Entry.SrcBurstMode=INC_MODE;
    Entry.DstBurstMode=INC_MODE;
    Entry.BurstSize=BUSTER_SIZE_64B; 
    Entry.SrcReqNum=DMA_MEM_REQ; 
    Entry.DstReqNum=DMA_MEM_REQ; 
    Entry.DoneIntCallback=DoneIntCallback;
    Entry.UnMaskIntCallback=NULL;
    Entry.SoftMode=1;
    Entry.ChMask=0;

    Entry.CoherentIntEbl=1;

    //No reserved channel for Memory to Memory GDMA,
    //get free channel on demand
    if(!_GdmaGetFreeCh(&Entry.ChNum)) {
	GDMA_PRINT("GDMA Channels are all busy\n");
	return 0;
    }


    //set next channel to their own channel 
    //to disable chain feature
     Entry.NextUnMaskCh= Entry.ChNum;
      //printk ("ChNum = %d\n", Entry.ChNum);
    //set next channel to another channel
    //to enable chain feature
    //Entry.NextUnMaskCh= (Entry.ChNum+1) % MAX_GDMA_CHANNEL;

    return _GdmaReqEntryIns(&Entry);

}

/**
 * @brief GDMA interrupt handler 
 *
 * When GDMA transcation is done, call related handler 
 * to do the remain job.
 *
 */
irqreturn_t GdmaIrqHandler(int irq, void *irqaction)
{
	u32 Ch=0;
	unsigned long flags;
#if defined (CONFIG_RALINK_RT3052)
	u32 GdmaUnMaskStatus=GDMA_READ_REG(RALINK_GDMAISTS) & 0xFF0000;
	u32 GdmaDoneStatus=GDMA_READ_REG(RALINK_GDMAISTS) & 0xFF;
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	u32 GdmaUnMaskStatus=GDMA_READ_REG(RALINK_GDMA_UNMASKINT);
	u32 GdmaDoneStatus=GDMA_READ_REG(RALINK_GDMA_DONEINT);
#endif

	//GDMA_PRINT("========================================\n");
	//GDMA_PRINT("GdmaUnMask Interrupt=%x\n",GdmaUnMaskStatus);
	//GDMA_PRINT("GdmaDone Interrupt=%x\n",GdmaDoneStatus);
	//GDMA_PRINT("========================================\n");

	spin_lock_irqsave(&gdma_int_lock, flags);

	//write 1 clear
#if defined (CONFIG_RALINK_RT3052)
	GDMA_WRITE_REG(RALINK_GDMAISTS, GdmaUnMaskStatus); 
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	GDMA_WRITE_REG(RALINK_GDMA_UNMASKINT, GdmaUnMaskStatus); 
#endif

	//UnMask error
	for(Ch=0;Ch<MAX_GDMA_CHANNEL;Ch++) {
		if(GdmaUnMaskStatus & (0x1 << (UNMASK_INT_STATUS(Ch))) ) {
			if(GdmaUnMaskIntCallback[Ch] != NULL) {
				GdmaUnMaskIntCallback[Ch](Ch);
			}
		}
	}

	//write 1 clear
#if defined (CONFIG_RALINK_RT3052)
	GDMA_WRITE_REG(RALINK_GDMAISTS, GdmaDoneStatus);
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	GDMA_WRITE_REG(RALINK_GDMA_DONEINT, GdmaDoneStatus);
#endif

	//printk("interrupt status = %x \n", GdmaDoneStatus);
	//processing done
	for(Ch=0;Ch<MAX_GDMA_CHANNEL;Ch++) {
		if(GdmaDoneStatus & (0x1<<Ch)) {
			if(GdmaDoneIntCallback[Ch] != NULL) {
				GdmaDoneIntCallback[Ch](Ch);
			}
		}
	}

	spin_unlock_irqrestore(&gdma_int_lock, flags);

	return IRQ_HANDLED;
}

int __init RalinkGdmaInit(void)
{
	uint32_t ret=0;
	uint32_t val = 0;
	printk("Enable Ralink GDMA Controller Module \n");
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	printk("GDMA IP Version=%d\n", GET_GDMA_IP_VER);
#endif

	ret = request_irq(SURFBOARDINT_DMA, GdmaIrqHandler, IRQF_DISABLED, "Ralink_DMA", NULL);
	if(ret){
		GDMA_PRINT("IRQ %d is not free.\n", SURFBOARDINT_DMA);
		return 1;
	}

	//Enable GDMA interrupt
	(*(volatile u32 *)(RALINK_INTENA)) = cpu_to_le32(RALINK_INTCTL_DMA);

	//Channel0~Channel7 are round-robin
#if defined (CONFIG_RALINK_RT3052)
	GDMA_WRITE_REG(RALINK_GDMAGCT, 0x01);
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	GDMA_WRITE_REG(RALINK_GDMA_GCT, 0x01);
#else
#error Please Choose System Type
#endif

	return 0;
}

void __exit RalinkGdmaExit(void)
{
	printk("Disable Ralink GDMA Controller Module\n");

	//Disable GDMA interrupt
	GDMA_WRITE_REG(RALINK_INTDIS, RALINK_INTCTL_DMA);

	free_irq(SURFBOARDINT_DMA, NULL);
}

module_init(RalinkGdmaInit);
module_exit(RalinkGdmaExit);

EXPORT_SYMBOL(GdmaI2sRx);
EXPORT_SYMBOL(GdmaI2sTx);
EXPORT_SYMBOL(GdmaPcmRx);
EXPORT_SYMBOL(GdmaPcmTx);
#if defined(CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
EXPORT_SYMBOL(GdmaSpiRx);
EXPORT_SYMBOL(GdmaSpiTx);
#endif
EXPORT_SYMBOL(GdmaMem2Mem);
EXPORT_SYMBOL(GdmaReqQuickIns);
EXPORT_SYMBOL(GdmaMaskChannel);
EXPORT_SYMBOL(GdmaUnMaskChannel);


MODULE_DESCRIPTION("Ralink SoC GDMA Controller API Module");
MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(MOD_VERSION);
