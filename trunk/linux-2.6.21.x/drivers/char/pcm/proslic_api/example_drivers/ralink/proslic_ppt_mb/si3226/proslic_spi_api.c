/*
** $Id: proslic_spi_api.c,v 1.3 2012-01-09 12:56:42 qwert Exp $
**
** spi.h
** SPI driver implementation file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the SPI driver used 
** in the ProSLIC demonstration code. It calls the library
** that initializes and talks to the proslic motherboard.
**
** Dependancies:
** 
**
*/
//#include "../iowrapper.h" // This is the motherboard interface library
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "../spi.h"

#define spi_si3226_master_init spi_si3220_master_init
#define spi_si3226_write8 spi_si3220_write8
#define spi_si3226_read8 spi_si3220_read8

extern void spi_si3220_master_init(int ch);
extern void spi_si3220_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value);
extern u8 spi_si3220_read8(int sid, unsigned char cid, unsigned char reg);
extern void pcm_reset_slic (void);


#ifdef PROFILE

typedef struct {
	int regAccess;
	int ramAccess;
} profileType;

profileType profile = {0,0};

#endif

#define PPTADDR 0x378 //parallel port address 

static unsigned char init[]=
{
0x8,
0x4,
0x8,
0x4,
0x19,
0x1D,
0x19,
0x1D,
0x1D,
0
};

static unsigned char Output_clocking[]=
{
0x1C,  /* Padding CS enable time */
0x8,0xC,  
0x8,0xC,  
0x8,0xC,    
0x8,0xC,  

0x8,0xC,
0x8,0xC,
0x8,0xC,
0x8,0xC,
0x1D,
0
};
  
static unsigned char Input_clocking[]=
{
0x1C,  /* Padding CS enable time */	
0x0,0x4, 
0x0,0x4, 
0x0,0x4, 
0x0,0x4, 

0x0,0x4,
0x0,0x4,
0x0,0x4,
0x0,0x4,

0x1D,
0xFF
};

int lastChan=0;

////////////////////////////////////////////////////////////////////////////////
static void byteToSpi (uInt16 portID,uInt8 byte )

{	int i=0, single_bit_mask,single_bit;
    unsigned char ppbuffer;
	single_bit_mask= 0x080;
	single_bit = 0;
    while (Output_clocking[i]!=0) {
	if (Output_clocking [i] ==8) 
	{
		
		single_bit = (byte & single_bit_mask) ? 2 :0;  // This is bit 1 of //port
			single_bit_mask >>=1;
	}
		ppbuffer = single_bit | Output_clocking [i] ;
		//PortOut_C (portID,ppbuffer);
		
	// When the clock is low change the data
i++;
} // while
}// byteToSpi

//*************************************************************
static uInt8 spiToByte(uInt16 portID)

{	unsigned char  ppbuffer, inputBuffer;
	int i=0,  data=0;
	unsigned short lpt1 = portID + 1;
	i=0;
	while (Input_clocking[i] !=0xff) { 
	ppbuffer= Input_clocking[i] ;
	//PortOut_C(portID,ppbuffer);
	

if (Input_clocking[i]==0) {
	data <<=1; // This includes one shift which is ignored
	
	//inputBuffer = PortIn_C(portID+1);
	data |= (0x80 & inputBuffer)? 0:1;
 }

i++;
} // while
	
return data;
}// spiToByte

#define CNUM_TO_CID_QUAD(channelNumber)   (((channelNumber<<4)&0x10)|((channelNumber<<2)&0x8)|((channelNumber>>2)&0x2)|((channelNumber>>4)&0x1)|(channelNumber&0x4))

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr){
	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x60;
#ifdef PROFILE
	profile.regAccess++;
#endif
	//byteToSpi(portID,regCtrl);
	//byteToSpi(portID,regAddr);
	//return spiToByte(portID);
	return spi_si3226_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(regCtrl), (unsigned char)regAddr);
}

static void WriteReg (uInt16 portID, uInt8 channel, uInt8 regAddr, uInt8 data){
	uInt8 regCtrl = CNUM_TO_CID_QUAD(channel)|0x20;
	/*if (lastChan != channel)
		printf ("CHANNEL %d\n",channel);
	lastChan = channel;
	if (regAddr < 5 || regAddr > 10)
		printf ("REGISTER %d = %X\n",regAddr,data);*/
#ifdef PROFILE
	profile.regAccess++;
#endif
	if (channel == 0xff)
		regCtrl = 0x20 | 0x80;
	//byteToSpi(portID,regCtrl);
	//byteToSpi(portID,regAddr);
	//byteToSpi(portID,data);
	spi_si3226_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(regCtrl), (unsigned char)regAddr, data);
}

/*
static void WriteRam16Bits (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt16 data){
	
	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	if (channel == 0xff)
		byteToSpi(portID,0x80);  // Write the control byte
	else
		byteToSpi(portID,CNUM_TO_CID_QUAD(channel));  // Write the control byte
	byteToSpi(portID,regAddr);                                 // Write the RAM address
	byteToSpi(portID,data >> 8);                               // Write the MSB of data
	byteToSpi(portID,data & 0xFF);                             // Write the LSB of data

	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,0); //clear upper address bits
}

static uInt16 ReadRam16Bits (uInt16 portID, uInt8 channel, uInt16 regAddr){
	uInt16 data;
	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits

	byteToSpi(portID,0x40 | CNUM_TO_CID_QUAD(channel));  // Write the control byte
	byteToSpi(portID,regAddr);                                 // Write the RAM address
	data = spiToByte(portID) << 8;                             // High byte RAM data
	data |= spiToByte(portID);  

	if ((regAddr>>3)&0xe0)
		WriteReg(portID,channel,5,0); //clear upper address bits
	return data;
}
*/


static void RAMwait (uInt16 portID,unsigned short channel)
{
	unsigned char regVal; 
	regVal = ReadReg (portID,channel,4);
	while (regVal&0x01)
	{
		regVal = ReadReg (portID,channel,4);
	}//wait for indirect registers

}

static void WriteRam (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt32 data){
#ifdef PROFILE
	profile.ramAccess++;
#endif
	/*if (lastChan != channel)
		printf ("CHANNEL %d\n",channel);
	lastChan = channel;
	if (regAddr != 1359 && regAddr != 1358)
		printf ("RAM %d = %X\n",regAddr,data);*/
	if (channel == 0xff)
		RAMwait(portID,0);   
	else
		RAMwait(portID,channel);   
	WriteReg(portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	
		WriteReg (portID,channel,6,(data<<3)&0xff);
		WriteReg (portID,channel,7,(data>>5)&0xff);
		WriteReg (portID,channel,8,(data>>13)&0xff);
		WriteReg (portID,channel,9,(data>>21)&0xff);
	
		WriteReg (portID,channel,10,regAddr&0xff); //write lower address bits  
	
}

static uInt32 ReadRam (uInt16 portID, uInt8 channel, uInt16 regAddr){
	unsigned char reg; unsigned long RegVal;
#ifdef PROFILE
	profile.ramAccess++;
#endif
    RAMwait(portID,channel);
	WriteReg (portID,channel,5,(regAddr>>3)&0xe0); //write upper address bits
	
		WriteReg (portID,channel,10,regAddr&0xff); //write lower address bits
	
	RAMwait(portID,channel);
	
		reg=ReadReg (portID,channel,6);
		RegVal = reg>>3;
		reg=ReadReg(portID,channel,7);
		RegVal |= ((unsigned long)reg)<<5;
		reg=ReadReg(portID,channel,8);
		RegVal |= ((unsigned long)reg)<<13;
		reg=ReadReg(portID,channel,9);
		RegVal |= ((unsigned long)reg)<<21;
	
	return RegVal;
}

/*
** Function: SPI_Init
**
** Description: 
** Initializes the SPI interface
**
** Input Parameters: 
** none
**
** Return:
** none
*/
int SPI_Init (ctrl_S *hSpi){
	//establish link to driver
	int i;
	//hSpi->portID = PPTADDR;
	//i = LoadIODLL_C();
	//PortOut_C(hSpi->portID,0x1D);
	spi_si3226_master_init(CONFIG_RALINK_PCM_SPICH);
	
	i = 0;
	return (!i);
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void *hSpiGci, int status){
	uInt8 c;
	//c = PortIn_C (((ctrl_S*)hSpiGci)->portID+2);
	if (status){ 
	//	PortOut_C(((ctrl_S*)hSpiGci)->portID+2,c | 0x1);
	}
	else{
	//	PortOut_C(((ctrl_S*)hSpiGci)->portID+2,c&0xfe);
	}
	pcm_reset_slic();
	return 0;
}

/*
** SPI/GCI register read 
**
** Description: 
** Reads a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** regAddr: Address of register to read
** return data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (void *hSpiGci, uInt8 channel, uInt8 regAddr){
	

	return ReadReg(((ctrl_S*)hSpiGci)->portID,channel,regAddr);
}


/*
** Function: spiGci_WriteRegisterWrapper 
**
** Description: 
** Writes a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
int ctrl_WriteRegisterWrapper (void *hSpiGci, uInt8 channel, uInt8 regAddr, uInt8 data){

	WriteReg(((ctrl_S*)hSpiGci)->portID,channel,regAddr, data);
	return 0;
}


/*
** Function: SPI_ReadRAMWrapper
**
** Description: 
** Reads a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** address: Address of RAM location to read
** pData: data to read from RAM location
**
** Return:
** none
*/
ramData ctrl_ReadRAMWrapper (void *hSpiGci, uInt8 channel, uInt16 ramAddr){
	ramData data;
#ifdef ALLBITS
	data = ReadRamAllBits(((ctrl_S*)hSpiGci)->portID,channel,ramAddr);
#else
	data = ReadRam(((ctrl_S*)hSpiGci)->portID,channel,ramAddr);
#endif
	return data;
}


/*
** Function: SPI_WriteRAMWrapper
**
** Description: 
** Writes a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of RAM location to write
** data: data to write to RAM location
**
** Return:
** none
*/
int ctrl_WriteRAMWrapper (void *hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data){
#ifdef ALLBITS
	WriteRamAllBits(((ctrl_S*)hSpiGci)->portID,channel,ramAddr,data);
#else
	WriteRam(((ctrl_S*)hSpiGci)->portID,channel,ramAddr,data);
#endif
	return 0;
}



/*
** $Log: proslic_spi_api.c,v $
** Revision 1.3  2012-01-09 12:56:42  qwert
** Add for si3226
**
** Revision 1.2  2010-07-30 08:32:03  qwert
** remove debug print
**
** Revision 1.1  2010-07-30 07:55:37  qwert
** Add si3227 driver
**
** Revision 1.11  2008/07/24 21:06:16  lajordan
** no message
**
** Revision 1.10  2008/03/19 18:20:09  lajordan
** no message
**
** Revision 1.9  2007/06/04 16:35:51  lajordan
** added profiling
**
** Revision 1.8  2007/02/27 19:25:48  lajordan
** updated api
**
** Revision 1.7  2007/02/05 23:43:27  lajordan
** fixed register access func
**
** Revision 1.6  2006/11/22 21:38:19  lajordan
** broadcast added
**
** Revision 1.5  2006/07/21 20:31:26  lajordan
** fixed cant connect message
**
** Revision 1.4  2006/07/19 18:15:56  lajordan
** fixed spi init
**
** Revision 1.3  2006/07/18 21:50:16  lajordan
** removed extraneous endif
**
** Revision 1.2  2006/07/18 21:48:51  lajordan
** added 16 bit accesses example code
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.3  2006/06/29 19:11:18  laj
** lpt driver completed
**
** Revision 1.2  2006/06/21 23:46:58  laj
** register reads/writes added
**
** Revision 1.1  2006/06/21 23:18:07  laj
** no message
**
** Revision 1.1  2006/06/21 22:42:26  laj
** new api style
**
** Revision 1.3  2005/11/14 17:42:34  laj
** added SPI_init
**
** Revision 1.2  2005/11/07 23:20:54  laj
** took out extra spaces
**
*/
