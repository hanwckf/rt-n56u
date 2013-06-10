/*
** $Id: proslic_spi_api.c,v 1.3 2010-02-02 10:43:01 qwert Exp $
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
//#include "Proslic_Api/example_drivers/win/proslic_ppt_mb/iowrapper.h" // This is the motherboard interface library
#include "../spi.h"
#include "../../../proslic_datatypes.h"
#include "../../../proslic_ctrl.h"

//void spi_si321x_write8(unsigned char cid, unsigned char reg, unsigned char value);
//void spi_si321x_read8(unsigned char cid, unsigned char reg, unsigned short value);
IMPORT_SYMBOL(spi_si321x_master_init);
IMPORT_SYMBOL(spi_si321x_write8);
IMPORT_SYMBOL(spi_si321x_read8);

#define RAMSTAT	0x4

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
{	
	int i=0, single_bit_mask,single_bit;
    unsigned char ppbuffer;
	single_bit_mask= 0x080;
	single_bit = 0;
	/*
    while (Output_clocking[i]!=0) 
    {
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
	*/
}// byteToSpi

//*************************************************************
static uInt8 spiToByte(uInt16 portID)

{	unsigned char  ppbuffer, inputBuffer;
	int i=0,  data=0;
	unsigned short lpt1 = portID + 1;
	i=0;
	/*
	while (Input_clocking[i] !=0xff) 
	{ 
		ppbuffer= Input_clocking[i] ;
		PortOut_C(portID,ppbuffer);
		
	
		if (Input_clocking[i]==0) {
			data <<=1; // This includes one shift which is ignored
		
			inputBuffer = PortIn_C(portID+1);
			data |= (0x80 & inputBuffer)? 0:1;
	 	}

		i++;
	} // while
	*/
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

static uInt8 ReadReg (uInt16 portID, uInt8 channel, uInt8 regAddr)
{
	uInt8 regCtrl = 1<<channel;
	/*
	if (daisyChain)
		byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr|0x80);
	
	return spiToByte(portID);
	*/
	//printk("ReadReg[chid=%d,regaddr=0x%08X]\n",channel,regAddr);
	return spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(channel), (unsigned char)regAddr);
	
}

static void WriteReg (uInt16 portID, uInt8 channel, uInt8 regAddr, uInt8 data)
{
	//uInt8 regCtrl = (channel==0xff)?0xff:(1<<channel);
	/*
	if (daisyChain)
		byteToSpi(portID,regCtrl);
	byteToSpi(portID,regAddr);
	byteToSpi(portID,data);
	*/
	/*
	if (regAddr == 0){
		if (data&0x80)
			daisyChain=1;
		else
			daisyChain=0;
	}
	*/
	//printk("WriteReg[chid=%d,regaddr=0x%08X,data=0x%08X]\n",channel,regAddr,data);
	spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(channel), (unsigned char)regAddr, data);
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

static void WriteRam (uInt16 portID, uInt8 channel, uInt16 regAddr, uInt32 data)
{
	
	regAddr = possibleAddressCorrect(portID,(uInt8)channel,(uInt8)regAddr);
	if (regAddr == 0xff)
		return;
	RAMwait(portID,channel);
	WriteReg(portID,(uInt8)channel,28,(uInt8)(data & 0xFF));
	WriteReg(portID,(uInt8)channel,29,(uInt8)((data & 0xFF00)>>8));
	WriteReg(portID,(uInt8)channel,30,(uInt8)(regAddr));
}

static uInt32 ReadRam (uInt16 portID, uInt8 channel, uInt16 regAddr)
{
	
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
	//i = LoadIODLL_C();
	//PortOut_C(hSpi->portID,0x1D);
 	//return (!i);
 	spi_si321x_master_init(CONFIG_RALINK_PCM_SPICH);
	return 1;
}


/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void *hSpiGci, int status)
{
	uInt8 c;
	/*
	c = PortIn_C (((ctrl_S*)hSpiGci)->portID+2);
	if (status){ 
		PortOut_C(((ctrl_S*)hSpiGci)->portID+2,c | 0x1);
	}
	else{
		PortOut_C(((ctrl_S*)hSpiGci)->portID+2,c&0xfe);
	}
	*/
	printk("ctrl_ResetWrapper\n");
	pcm_reset_slic();
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
uInt8 ctrl_ReadRegisterWrapper (void *hSpiGci, unsigned char channel, unsigned char regAddr){

	return ReadReg(((ctrl_S *)hSpiGci)->portID,channel,regAddr);
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
int ctrl_WriteRegisterWrapper (void *hSpiGci, unsigned char channel, unsigned char regAddr, unsigned char data){
	
	WriteReg(((ctrl_S *)hSpiGci)->portID,channel,regAddr, data);
	
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
ramData ctrl_ReadRAMWrapper (void *hSpiGci, unsigned char channel, unsigned short ramAddr){
	
		return ReadRam(((ctrl_S*)hSpiGci)->portID,channel,ramAddr);
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
int ctrl_WriteRAMWrapper (void *hSpiGci, unsigned char channel, unsigned short ramAddr, ramData data){
	
	WriteRam(((ctrl_S*)hSpiGci)->portID,channel,ramAddr,data);
	return 0;
}



/*
** $Log: proslic_spi_api.c,v $
** Revision 1.3  2010-02-02 10:43:01  qwert
** Add for SPI number assignment
**
** Revision 1.2  2009-09-02 08:07:31  qwert
** Remove spi init. for sucessfully reset SLIC
**
** Revision 1.1  2008-12-31 02:39:58  qwert
** Add si3210 driver
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