/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si_voice_ctrl.h,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** si_voice_ctrl.h
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
** si_voice_datatypes.h 
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


