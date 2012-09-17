/*
** $Id: proslic_ctrl.h,v 1.1 2008-12-31 02:39:58 qwert Exp $
**
** proslic_ctrl.h
** SPI driver header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for the control driver used 
** in the ProSLIC demonstration code.
**
** Dependancies:
** proslic_datatypes.h 
**
*/
#ifndef CTRL_H
#define CTRL_H

/*
** reset function pointer
**
** Description: 
** Sets the reset pin of the ProSLIC
**
** Input Parameters: 
** new reset pin status
**
** Return:
** none
*/
typedef int (*ctrl_Reset_fptr) (void *hCtrl, int status);

/*
** register write function pointer
**
** Description: 
** Writes ProSLIC registers
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** regAddr: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
typedef int (*ctrl_WriteRegister_fptr) (void *hCtrl, uInt8 channel, uInt8 regAddr, uInt8 data);

/*
** RAM write function pointer
**
** Description: 
** Writes a single ProSLIC ram
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** ramAddr: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
typedef int (*ctrl_WriteRAM_fptr) (void *hCtrl, uInt8 channel, uInt16 ramAddr, ramData data);

/*
** register read function pointer
**
** Description: 
** Reads ProSLIC registers
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** regAddr: Address of register to read
** return data: data to read from register
**
** Return:
** none
*/
typedef uInt8 (*ctrl_ReadRegister_fptr) (void *hCtrl, uInt8 channel, uInt8 regAddr);

/*
** ctrl RAM read function pointer
**
** Description: 
** Reads a single ProSLIC ram
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** ramAddr: Address of register to write
** return data: data to read from ram
**
** Return:
** none
*/
typedef ramData (*ctrl_ReadRAM_fptr) (void *hCtrl, uInt8 channel, uInt16 ramAddr);

typedef int (*ctrl_Semaphore_fptr) (void *hCtrl, int status);

#endif
/*
** $Log: proslic_ctrl.h,v $
** Revision 1.1  2008-12-31 02:39:58  qwert
** Add si3210 driver
**
** Revision 1.7  2008/03/06 23:51:23  lajordan
** updated presets
**
** Revision 1.6  2008/01/21 21:19:03  lajordan
** renaming to lower case
**
** Revision 1.4  2007/05/31 19:13:28  lajordan
** added line
**
** Revision 1.3  2007/02/21 16:53:01  lajordan
** no message
**
** Revision 1.2  2007/02/16 23:54:56  lajordan
** no message
**
** Revision 1.1  2007/02/15 23:33:27  lajordan
** no message
**
** Revision 1.2  2007/02/05 22:02:01  lajordan
** register read/ram read functions def changed
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
** Revision 1.1  2006/07/07 21:39:22  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.1  2006/06/21 22:42:26  laj
** new api style
**
**
*/

