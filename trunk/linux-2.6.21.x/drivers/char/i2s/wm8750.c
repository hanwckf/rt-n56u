/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: wm8750.c,v 1.7 2011-07-06 07:36:48 qwert Exp $
 *
 * Driver for WM8751 audio codec
 *
 * Based on code from the ipodlinux project - http://ipodlinux.org/
 * Adapted for Rockbox in December 2005
 *
 * Original file: linux/arch/armnommu/mach-ipod/audio.c
 *
 * Copyright (c) 2003-2005 Bernard Leach (leachbj@bouncycastle.org)
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include "wm8750.h"
#include "i2s_ctrl.h"


unsigned long wmcodec_reg_data[48];
/* Flags used in combination with settings */

/* use zero crossing to reduce clicks during volume changes */
#define LOUT1_BITS      (LOUT1_LO1ZC)
/* latch left volume first then update left+right together */
#define ROUT1_BITS      (ROUT1_RO1ZC | ROUT1_RO1VU)
#define LOUT2_BITS      (LOUT2_LO2ZC)
#define ROUT2_BITS      (ROUT2_RO2ZC | ROUT2_RO2VU)
/* We use linear bass control with 200 Hz cutoff */
#ifdef USE_ADAPTIVE_BASE
#define BASSCTRL_BITS   (BASSCTRL_BC | BASSCTRL_BB)
#else
#define BASSCTRL_BITS   (BASSCTRL_BC)
#endif
/* We use linear treble control with 4 kHz cutoff */
#define TREBCTRL_BITS   (TREBCTRL_TC)

extern void i2c_WM8751_write(unsigned int address, unsigned int data);


void audiohw_sel_input(int ain);

void wmcodec_write(int reg_addr, unsigned long reg_data)
{
	wmcodec_reg_data[reg_addr] = reg_data;
	printk("[WM875X(%02X)=0x%08X]\n",(unsigned int)reg_addr,(unsigned int)reg_data);
	i2c_WM8751_write(reg_addr, reg_data);
	return;
}

/* convert tenth of dB volume (-730..60) to master volume register value */
int tenthdb2master(int db)
{
    /* +6 to -73dB 1dB steps (plus mute == 80levels) 7bits */
    /* 1111111 ==  +6dB  (0x7f)                            */
    /* 1111001 ==   0dB  (0x79)                            */
    /* 0110000 == -73dB  (0x30)                            */
    /* 0101111..0000000 == mute  (<= 0x2f)                 */
    if (db < VOLUME_MIN)
        return 0x0;
    else
        return (db / 10) + 73 + 0x30;
}

static int tone_tenthdb2hw(int value)
{
    /* -6.0db..+0db..+9.0db step 1.5db - translate -60..+0..+90 step 15
        to 10..6..0 step -1.
    */
    value = 10 - (value + 60) / 15;

    if (value == 6)
        value = 0xf; /* 0db -> off */

    return value;
}


#ifdef USE_ADAPTIVE_BASS
static int adaptivebass2hw(int value)
{
    /* 0 to 15 step 1 - step -1  0 = off is a 15 in the register */
    value = 15 - value;

    return value;
}
#endif

/* Reset and power up the WM8751 */
void audiohw_preinit(void)
{
	int i;
	/*
	* 1. Switch on power supplies.
	*    By default the WM8751 is in Standby Mode, the DAC is
	*    digitally muted and the Audio Interface, Line outputs
	*    and Headphone outputs are all OFF (DACMU = 1 Power
	*    Management registers 1 and 2 are all zeros).
	*/
	//if(wmcodec_reg_data[RESET]!=0)
	//	return;
	
	memset(wmcodec_reg_data, 0 , sizeof(unsigned long)*48);
	
	wmcodec_write(RESET, RESET_RESET);    /*Reset*/
	
	for(i = 0; i < 1000*HZ; i++);
	
	wmcodec_reg_data[RESET] = 0xFFFF;
	/* 2. Enable Vmid and VREF. */
	wmcodec_write(PWRMGMT1, PWRMGMT1_VREF | PWRMGMT1_VMIDSEL_50K );
              
}

/* Enable DACs and audio output after a short delay */
void audiohw_postinit(int bSlave, int AIn, int AOut)
{
	int i;
	unsigned long data;
	
	if(wmcodec_reg_data[RESET]!=0xFFFF)
    	return;
    
    /* BCLKINV=0(Dont invert BCLK) MS=1(Enable Master) LRSWAP=0 LRP=0 */
    /* IWL=00(16 bit) FORMAT=10(I2S format) */
    
	if(bSlave)
	{ 
		MSG("WM8750 slave.....\n");
		wmcodec_write(AINTFCE, AINTFCE_WL_16 | AINTFCE_FORMAT_I2S);
	}	
	else
	{
		MSG("WM8750 master.....\n");
		/* AINTFCE_BCLKINV on or off depend on trying result */
		wmcodec_write(AINTFCE, AINTFCE_MS | AINTFCE_WL_16 | AINTFCE_FORMAT_I2S);	
	}
    
	wmcodec_reg_data[RESET] = 0x5A5A;
	
	/* From app notes: allow Vref to stabilize to reduce clicks */
	for(i = 0; i < 1000*HZ; i++);
    
	data = wmcodec_reg_data[PWRMGMT1];
    
	if(AIn > 0)
		wmcodec_write(PWRMGMT1, data | PWRMGMT1_ADCL | PWRMGMT1_ADCR | PWRMGMT1_AINL | PWRMGMT1_AINR |PWRMGMT1_MICB);
		//wmcodec_write(PWRMGMT1, PWRMGMT1_VREF | PWRMGMT1_VMIDSEL_50K | PWRMGMT1_ADCL | PWRMGMT1_ADCR | PWRMGMT1_AINL | PWRMGMT1_AINR );
		
	/* 3. Enable DACs as required. */
	if(AOut > 0)
	{
		data = PWRMGMT2_DACL | PWRMGMT2_DACR;
		switch(AOut)
		{
		case 1:
			data |= PWRMGMT2_LOUT1 | PWRMGMT2_ROUT1;
			break;
		case 2:
			data |= PWRMGMT2_LOUT2 | PWRMGMT2_ROUT2;
			break;
		case 3:
			data |= PWRMGMT2_OUT3;
			break;
		default:
			break;	
		}	
		wmcodec_write(PWRMGMT2,   data);
	}
	else
	{
		wmcodec_write(PWRMGMT2, 0);
	}
		 
	/* 4. Enable line and / or headphone output buffers as required. */
	wmcodec_write(ADDITIONAL1, ADDITIONAL1_TOEN | ADDITIONAL1_DMONOMIX_LLRR | ADDITIONAL1_VSEL_DEFAULT);
    
	if(AOut==2) 
	{               
		wmcodec_write(ADDITIONAL2, ADDITIONAL2_HPSWEN|ADDITIONAL2_HPSWPOL);
	}
	else
	{
		wmcodec_write(ADDITIONAL2, ADDITIONAL2_LRCM_ON);
	}
	
	wmcodec_write(ADDITIONAL3, ADDITIONAL3_ADCLRM(0));
	
	wmcodec_write(RIGHTMIX1, 0);
	wmcodec_write(LEFTMIX2, 0);
	if(AOut > 0)
	{
		wmcodec_write(LEFTMIX1, LEFTMIX1_LD2LO);
		wmcodec_write(RIGHTMIX2, RIGHTMIX2_RD2RO);
		wmcodec_write(LOUT1, 0x179);
		wmcodec_write(ROUT1, 0x179);
		wmcodec_write(LEFTGAIN, 0x1FF);
		wmcodec_write(RIGHTGAIN, 0x1FF);
	}
	else
	{
		wmcodec_write(LEFTMIX1, 0);
		wmcodec_write(RIGHTMIX2, 0);
		wmcodec_write(LOUT1, 0);
		wmcodec_write(ROUT1, 0);
	}
	wmcodec_write(MONOMIX1, 0);
	wmcodec_write(MONOMIX2, 0);
	wmcodec_write(ADCCTRL, ADCCTRL_ADCHPD);
	if(AIn>0)
	{
		//wmcodec_write(LINV, LINV_LINVOL(0x00)|LINV_LIVU);
		//wmcodec_write(RINV, RINV_RINVOL(0x00)|RINV_RIVU);
		//wmcodec_write(LINV, 0x117);
		//wmcodec_write(RINV, 0x117);
		wmcodec_write(LINV, 0x100);
		wmcodec_write(RINV, 0x100);
		
		wmcodec_write(ALC1, 0);
		wmcodec_write(NOISEGATE, 0x03);
		//wmcodec_write(ALC1, ALC1_ALCL(0x01)|ALC1_SET_MAXGAIN(0x1)|ALC1_ALCSTEREO);
		//wmcodec_write(NOISEGATE, NOISEGATE_SET_NGTH(0x1F)|NOISEGATE_SET_NGG(0x01)|NOISEGATE_NGAT_ENABLE);
		
		audiohw_sel_input(AIn);
	}
	else
	{
		wmcodec_write(LINV, LINV_LINMUTE);
		wmcodec_write(RINV, RINV_RINMUTE);
	}
	
	//wmcodec_write(LEFTGAIN, 0x1FF);
	//wmcodec_write(RIGHTGAIN, 0x1FF);
	if(AOut > 0)
		audiohw_mute(false);
	else
		audiohw_mute(true);
}
int audiohw_set_master_vol(int vol_l, int vol_r)
{
	/* +6 to -73dB 1dB steps (plus mute == 80levels) 7bits */
	/* 1111111 ==  +6dB                                    */
	/* 1111001 ==   0dB                                    */
	/* 0110000 == -73dB                                    */
	/* 0101111 == mute (0x2f)                              */
	MSG("audiohw_set_master_vol\n");
	wmcodec_write(LOUT1, LOUT1_BITS | LOUT1_LOUT1VOL(vol_l));
	wmcodec_write(ROUT1, ROUT1_BITS | ROUT1_ROUT1VOL(vol_r));
	
	wmcodec_write(LOUT2, LOUT2_BITS | LOUT2_LOUT2VOL(vol_l));
	wmcodec_write(ROUT2, ROUT2_BITS | ROUT2_ROUT2VOL(vol_r));
	
	wmcodec_write(LEFTGAIN, LEFTGAIN_LDVU | LEFTGAIN_LDACVOL(vol_l));
	wmcodec_write(RIGHTGAIN, RIGHTGAIN_LDVU | RIGHTGAIN_LDACVOL(vol_r));
	return 0;
}

int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r)
{
	MSG("audiohw_set_lineout_vol\n");
	switch(Aout)
	{
	case 1:
	wmcodec_write(LOUT1, LOUT1_BITS | LOUT1_LOUT1VOL(vol_l));
    wmcodec_write(ROUT1, ROUT1_BITS | ROUT1_ROUT1VOL(vol_r));
		break;
	case 2:
    wmcodec_write(LOUT2, LOUT2_BITS | LOUT2_LOUT2VOL(vol_l));
    wmcodec_write(ROUT2, ROUT2_BITS | ROUT2_ROUT2VOL(vol_r));
		break;
	case 3:
		wmcodec_write(MONOOUT, MONOOUT_MOZC | MONOOUT_MONOOUTVOL(vol_l));	
		break;
	}	
    return 0;
}

int audiohw_set_linein_vol(int vol_l, int vol_r)
{
	MSG("audiohw_set_linein_vol\n");
    wmcodec_write(LINV, LINV_LINVOL(vol_l)|LINV_LIVU);
	wmcodec_write(RINV, RINV_RINVOL(vol_r)|RINV_RIVU);
    return 0;
}

void audiohw_set_bass(int value)
{
	wmcodec_write(BASSCTRL, BASSCTRL_BITS |

#ifdef USE_ADAPTIVE_BASS
	BASSCTRL_BASS(adaptivebass2hw(value)));
#else
	BASSCTRL_BASS(tone_tenthdb2hw(value)));
#endif
}

void audiohw_set_treble(int value)
{
    wmcodec_write(TREBCTRL, TREBCTRL_BITS | TREBCTRL_TREB(tone_tenthdb2hw(value)));
}

void audiohw_mute(bool mute)
{
    /* Mute:   Set DACMU = 1 to soft-mute the audio DACs. */
    /* Unmute: Set DACMU = 0 to soft-un-mute the audio DACs. */
    wmcodec_write(DACCTRL, mute ? DACCTRL_DACMU : 0);
}

/* Nice shutdown of WM8751 codec */
void audiohw_close(void)
{
	/* 1. Set DACMU = 1 to soft-mute the audio DACs. */
	audiohw_mute(true);
	
	wmcodec_write(LADCVOL, 0);
	wmcodec_write(RADCVOL, 0);
	
	/* 2. Disable all output buffers. */
	wmcodec_write(PWRMGMT2, 0x0);
	
	/* 3. Switch off the power supplies. */
	wmcodec_write(PWRMGMT1, 0x0);
	
	memset(wmcodec_reg_data, 0 , sizeof(unsigned long)*48);
}

/* Note: Disable output before calling this function */
void audiohw_set_frequency(int fsel)
{
	MSG("audiohw_set_frequency=0x%08X\n",fsel);
	//fsel |= 0x0180;	// BCLK = MCLK/16
	//fsel |= 0x0080;
	//fsel &= 0xFFFFFFFE;
	wmcodec_write(CLOCKING, fsel);	
} 

void audiohw_set_MCLK(unsigned int bUsb)
{
	if(bUsb)
		wmcodec_write(CLOCKING, CLOCKING_SR_USB);
	else
		wmcodec_write(CLOCKING, 0);		
} 

void audiohw_sel_input(int ain)
{
	switch(ain)
	{
	case 1:
		wmcodec_write(ADCLPATH, ADCLPATH_LINSEL_IN1);
		wmcodec_write(ADCRPATH, ADCRPATH_RINSEL_IN1);
		//wmcodec_write(ADCINMODE, ADCINMODE_RDCM_EN|ADCINMODE_LDCM_EN);	
		break;
	case 2:
		wmcodec_write(ADCLPATH, ADCLPATH_LINSEL_IN2);
		wmcodec_write(ADCRPATH, ADCRPATH_RINSEL_IN2);
		//wmcodec_write(ADCLPATH, ADCLPATH_LINSEL_DS);
		//wmcodec_write(ADCRPATH, ADCRPATH_RINSEL_DS);
		//wmcodec_write(ADCINMODE, ADCINMODE_DS_IN2|ADCINMODE_RDCM_EN|ADCINMODE_LDCM_EN);						
		break;
	case 3:
		wmcodec_write(ADCLPATH, ADCLPATH_LINSEL_IN3);
		wmcodec_write(ADCRPATH, ADCRPATH_RINSEL_IN3);			
		break;	
	default:
		wmcodec_write(ADCLPATH, ADCLPATH_LINSEL_IN1);
		wmcodec_write(ADCRPATH, ADCRPATH_RINSEL_IN1);			
		break;		
	}
	return;
}

void audiohw_loopback(int fsel)
{
	int i;
	memset(wmcodec_reg_data, 0 , sizeof(unsigned long)*48);
	
	wmcodec_write(RESET, 0x000);    /*Reset*/
	  
	for(i = 0; i < 1000*HZ; i++);
	
	//wmcodec_write(ADDITIONAL2, 0x0010);
	//wmcodec_write(PWRMGMT1, 0x0FC );
	
	
	wmcodec_write(AINTFCE, AINTFCE_WL_16|AINTFCE_FORMAT_I2S|AINTFCE_MS);
	wmcodec_write(LINV, 0x117);
	wmcodec_write(RINV, 0x117);     
	wmcodec_write(LOUT1, 0x179);
	wmcodec_write(ROUT1, 0x179);
	wmcodec_write(ADCCTRL, 0);
	wmcodec_write(CLOCKING, fsel);
	wmcodec_write(LEFTGAIN, 0x01FF);   
	wmcodec_write(RIGHTGAIN, 0x01FF);   
	wmcodec_write(PWRMGMT1, 0x0FC );            
	wmcodec_write(PWRMGMT2, 0x1E0 );
	wmcodec_write(LEFTMIX1, 0x150 );
	wmcodec_write(RIGHTMIX2, 0x150 );
}	

void audiohw_bypass(void)
{
	int i;
	memset(wmcodec_reg_data, 0 , sizeof(unsigned long)*48);
	wmcodec_write(RESET, 0x000);    /*Reset*/
	  
	for(i = 0; i < 1000*HZ; i++);
	
	wmcodec_write(LINV, 0x117);
	wmcodec_write(RINV, 0x117);  
	wmcodec_write(LOUT1, 0x179);
	wmcodec_write(ROUT1, 0x179);
	wmcodec_write(DACCTRL, 0);
	wmcodec_write(AINTFCE, 0x42);
	wmcodec_write(CLOCKING, 0x23);            
	wmcodec_write(LEFTGAIN, 0x1FF);
	wmcodec_write(RIGHTGAIN, 0x1FF);
	wmcodec_write(PWRMGMT1, 0x0F0);
	wmcodec_write(PWRMGMT2, 0x060);
	wmcodec_write(LEFTMIX1, 0xA0 );
	wmcodec_write(RIGHTMIX2, 0xA0 );
}	


EXPORT_SYMBOL(audiohw_set_frequency);
EXPORT_SYMBOL(audiohw_close);
EXPORT_SYMBOL(audiohw_postinit);
EXPORT_SYMBOL(audiohw_preinit);
