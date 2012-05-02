/*
** $Id: proslic_spiGci_parallelPort.c,v 1.1 2010-07-30 07:55:37 qwert Exp $
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
#include "../Proslic_Api/example_drivers/win/proslic_ppt_mb/iowrapper.h" // This is the motherboard interface library
#include "../Proslic_Api/example_drivers/win/proslic_ppt_mb/SPI.h"
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"


#define PPTADDR 0x378 //parallel port address 

static uInt8 daisyChain=0;

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
		PortOut_C (portID,ppbuffer);
		
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
	PortOut_C(portID,ppbuffer);
	

if (Input_clocking[i]==0) {
	data <<=1; // This includes one shift which is ignored
	
	inputBuffer = PortIn_C(portID+1);
	data |= (0x80 & inputBuffer)? 0:1;
 }

i++;
} // while
	
return data;
}// spiToByte

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr);

static int Si3215orSi3216(uInt16 portID,uInt8 chipNumber)
{
static int slicPreviouslyIdentified = 0;
static int isSi3215 = 0;

   if(!slicPreviouslyIdentified)
   {
	   isSi3215 = ((ReadReg(portID,chipNumber, 1) & 0x80) == 0x80);
	   slicPreviouslyIdentified = 1;
   }
	return (isSi3215);
}


static uInt8 possibleAddressCorrect(uInt16 portID, uInt8 chipNumber, uInt8 address)
{
if (!Si3215orSi3216(portID,chipNumber)) 
		return (address);
		
	if ((address > 12) && (address < 41))
		return (address - 13);
	if ((address == 41) || (address == 43))
		return (address + 23);
	if ((address > 87)&&(address < 99))
		return (address - 13); 
	if ((address > 98) && (address < 105))
		return (address - 30);
	if (address >= 208)
		return address;

	return 0xFF;
	
}

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr){
	uInt8 regCtrl = 1<<channel;
	if (daisyChain)
		byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr|0x80);
	return spiToByte(portID);
}

static void WriteReg (uInt16 portID, uInt8 channel, uInt8 regAddr, uInt8 data){
	uInt8 regCtrl = (channel==0xff)?0xff:(1<<channel);
	if (daisyChain)
		byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	byteToSpi(portID,data);
	if (regAddr == 0){
		if (data&0x80)
			daisyChain=1;
		else
			daisyChain=0;
	}
}

static void RAMwait (uInt16 portID,unsigned short channel)
{
	unsigned char regVal; 
	regVal = ReadReg (portID,(uInt8)channel,31);
	while (regVal)
	{
		regVal = ReadReg (portID,(uInt8)channel,31);
	}//wait for indirect registers

}

static void WriteRam (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt32 data){
	regAddr = possibleAddressCorrect(portID,(uInt8)channel,(uInt8)regAddr);
	if (regAddr == 0xff)
		return;
	RAMwait(portID,channel);
	WriteReg(portID,(uInt8)channel,28,(uInt8)(data & 0xFF));
	WriteReg(portID,(uInt8)channel,29,(uInt8)((data & 0xFF00)>>8));
	WriteReg(portID,(uInt8)channel,30,(uInt8)(regAddr));
}

static uInt32 ReadRam (uInt16 portID, uInt8 channel, uInt16 regAddr){
	uInt16 RegVal;
    regAddr = possibleAddressCorrect(portID,(uInt8)channel,(uInt8)regAddr);
	RAMwait(portID,channel);
	WriteReg(portID,(uInt8)channel,30,(uInt8)regAddr);
	RAMwait(portID,channel);
	RegVal = ReadReg(portID,channel,28) | ( ReadReg(portID,channel,29)  << 8 );
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
	hSpi->portID = PPTADDR;
	i = LoadIODLL_C();
	if(i == 0)
	{
		PortOut_C(hSpi->portID,0x1D);
	}
 	return (!i);
	
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hSpiGci, int status){
	uInt8 c;
	c = PortIn_C (hSpiGci->portID+2);
	if (status){ 
		PortOut_C(hSpiGci->portID+2,c | 0x1);
	}
	else{
		PortOut_C(hSpiGci->portID+2,c&0xfe);
	}
	daisyChain=0;
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
** num: number of reads to perform
** regAddr: Address of register to read
** addr_inc: whether to increment address after each read
** data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr){

	return ReadReg(hSpiGci->portID,channel,regAddr);
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
int ctrl_WriteRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr, uInt8 data){
	
	WriteReg(hSpiGci->portID,channel,regAddr, data);
	
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
ramData ctrl_ReadRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr){
	
		return ReadRam(hSpiGci->portID,channel,ramAddr);
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
int ctrl_WriteRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data){
	
	WriteRam(hSpiGci->portID,channel,ramAddr,data);
	return 0;
}



/*
** $Log: proslic_spiGci_parallelPort.c,v $
** Revision 1.1  2010-07-30 07:55:37  qwert
** Add si3227 driver
**
** Revision 1.12  2008/09/10 20:49:11  lajordan
** no message
**
** Revision 1.11  2007/03/22 18:53:33  lajordan
** fixed warningg
**
** Revision 1.10  2007/02/16 23:55:19  lajordan
** no message
**
** Revision 1.9  2007/02/15 23:33:34  lajordan
** no message
**
** Revision 1.8  2007/02/05 22:01:53  lajordan
** register read/ram read functions def changed
**
** Revision 1.7  2006/07/20 18:41:11  lajordan
** fixed si3215 bug in possibleADdressCorrect
**
** Revision 1.6  2006/07/20 18:39:02  lajordan
** fixed si3215 bug
**
** Revision 1.5  2006/07/20 16:15:35  lajordan
** no message
**
*/