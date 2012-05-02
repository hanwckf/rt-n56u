/*
** $Id: proslic_spiGci_usb.c,v 1.1 2010-07-30 07:55:38 qwert Exp $
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
** that initializes and talks to the voice motherboard.
**
** Dependancies:
** 
**
*/
#include "../proslic_api/example_drivers/win/usb/DLLWrapper.h" // This is the motherboard interface library
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "../proslic_api/example_drivers/win/usb/spi.h"



#define FIRMWAREPATH "voicemb.out" //path to voice motherboard firmware

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
	//establish usb link, initialize motherboard, start up clocks
	uInt8 result;
	result = openVoiceIntf (0x378,1,FIRMWAREPATH);  
	UNReset();
	return result;
}


/*
** Function: ctrl_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (ctrl_S *hctrl, int status){
	if (status)
		Reset();
	else
		UNReset();
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
uInt8 ctrl_ReadRegisterWrapper (ctrl_S * hctrl, uInt8 channel, uInt8 regAddr){
	setChannel(channel);
	
	return QuadReadRegExp(regAddr);
		
}


/*
** Function: ctrl_WriteRegisterWrapper 
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
int ctrl_WriteRegisterWrapper (ctrl_S * hctrl, uInt8 channel, uInt8 regAddr, uInt8 data){
	if (channel == 0xff){ //broadcast
		setBroadcastmode(1);
	}
	else
		setChannel(channel);
	
	QuadWriteRegExp(regAddr, data);
		
	if (channel == 0xff){
		setBroadcastmode(0);
	}
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
ramData ctrl_ReadRAMWrapper (ctrl_S * hctrl, uInt8 channel, uInt16 ramAddr){
	
	setChannel(channel);
	return QuadReadRamExp(ramAddr);
}



static void RAMwait (ctrl_S * hctrl)
{
	unsigned char regVal; 
	
		
	do
	{
		//result=0;
		//for (i=0;i<3;i++){
			regVal = ctrl_ReadRegisterWrapper(hctrl,0,4);
		//	if (regVal != 0xff)
		//		result |= regVal;
		//}
	}while (regVal&0x01);//wait for indirect registers

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
int ctrl_WriteRAMWrapper (ctrl_S * hctrl, uInt8 channel, uInt16 ramAddr, ramData data){
	uInt8 data1;
	if (channel == 0xff){//broadcast
		RAMwait(hctrl);
		data1 = (uInt8)((ramAddr>>3)&0xe0);
		ctrl_WriteRegisterWrapper(hctrl,0xff,5,data1);
		data1 = (uInt8)((data<<3)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,6,data1);
		data1 = (uInt8)((data>>5)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,7,data1);
		data1 = (uInt8)((data>>13)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,8,data1);
		data1 = (uInt8)((data>>21)&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,9,data1);
		data1 =(uInt8)(ramAddr&0xff);
		ctrl_WriteRegisterWrapper(hctrl,0xff,10,data1);
		RAMwait(hctrl);
	}
	else {
		setChannel(channel);
		QuadWriteRamExp(ramAddr,data);
	}
	return 0;
}



/*
** $Log: proslic_spiGci_usb.c,v $
** Revision 1.1  2010-07-30 07:55:38  qwert
** Add si3227 driver
**
** Revision 1.12  2008/07/24 21:06:16  lajordan
** no message
**
** Revision 1.11  2008/03/19 18:20:41  lajordan
** no message
**
** Revision 1.10  2007/03/02 16:48:48  lajordan
** no message
**
** Revision 1.9  2007/03/02 16:45:01  lajordan
** updated fcn names
**
** Revision 1.8  2007/02/06 00:20:47  lajordan
** no message
**
** Revision 1.7  2007/02/06 00:16:46  lajordan
** no message
**
** Revision 1.6  2007/02/05 23:49:25  lajordan
** fixed register access func
**
** Revision 1.5  2007/02/05 23:43:19  lajordan
** fixed register access func
**
** Revision 1.4  2007/02/02 20:31:56  lajordan
** fixed broadcast to ram
**
** Revision 1.3  2006/11/22 21:38:12  lajordan
** broadcast added
**
** Revision 1.2  2006/07/14 22:44:35  lajordan
** fixed include paths
**
**
*/