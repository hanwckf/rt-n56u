//THIS FILE CONTAINS THE LOW LEVEL IO CODE FOR THE DUAL PROSLIC

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include "dual_io.h"
#include <linux/time.h>


__s64 time0,time1,ticksPerSecond;

//#define STUBS //delete this line to use "real" functions*****************************
unsigned short lpt = 0x378;  // i guess this should be lower case

unsigned char Cid = 0;  //control ID variable (selects channel)
unsigned char broadcast = 0;

//extern unsigned char spi_si3220_old_read8();
//void spi_si3220_old_write8(unsigned char data);
void spi_si3220_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value);
void spi_si3220_write16(int sid, unsigned char cid, unsigned char reg, unsigned short value);
unsigned char spi_si3220_read8(int sid, unsigned char cid, unsigned char reg);
unsigned short spi_si3220_read16(int sid, unsigned char cid, unsigned char reg);
unsigned char init[]=
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

unsigned char Output_clocking[]=
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
  
unsigned char Input_clocking[]=
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


unsigned char interrupt_output[]=
{
0x4, //clock= 1  oe=0
0x1D,
0
};


void writeReg (int reg,unsigned char data)
{
	//write direct register
	//control bit 7 = 0  //no broadcast
	//control bit 6 = 0 //write
	//control bit 5 = 1 //direct
	//control Cid = Cid
  	/*
	if (broadcast)
	{
		broadcastReg (reg,data);
		return;
	}
 	*/

	//byteToSpi ((unsigned char)(Cid | 0x20));
	//byteToSpi((unsigned char)(reg));
	//byteToSpi(data);
	
	spi_si3220_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(Cid | 0x20), (unsigned char)reg, data);
}
/*
void writeRam (int reg,unsigned short data)
{
	//write indirect register
  	unsigned char hi,lo;
	if (broadcast)
	{
		printk("writeram:broadcast\n");
		broadcastRAM(reg,data);
		return;
	}
 
 	//printk("writeRam[%d]=%d\n",reg,data);
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	byteToSpi ((unsigned char)(Cid | 0x00));
	byteToSpi ((unsigned char)(reg));
	hi = high8bit(data);
	lo = low8bit(data);
	//printk("w: hi=%X, lo=%X\n",hi,lo);
	byteToSpi (high8bit(data));
	byteToSpi (low8bit(data));

}
*/

void writeRam (int reg,unsigned short data)
{
	//write indirect register
  	unsigned char hi,lo;
	if (broadcast)
	{
		printk("writeram:broadcast\n");
		broadcastRAM(reg,data);
		return;
	}
 
 	//printk("writeRam[%d]=%d\n",reg,data);
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	//byteToSpi ((unsigned char)(Cid | 0x00));
	//byteToSpi ((unsigned char)(reg));
	//hi = high8bit(data);
	//lo = low8bit(data);
	//printk("w: hi=%X, lo=%X\n",hi,lo);
	//byteToSpi (high8bit(data));
	//byteToSpi (low8bit(data));
	spi_si3220_write16(CONFIG_RALINK_PCM_SPICH, (unsigned char)(Cid | 0x00), (unsigned char)(reg), data);
}
/*
void writeRam (int reg,unsigned short data)
{
	//write indirect register
  	unsigned char hi,lo;
	if (broadcast)
	{
		printk("writeram:broadcast\n");
		broadcastRAM(reg,data);
		return;
	}
 
 	
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	
	writeReg(RAMADDR,(unsigned char)reg) ;
	//printk("reg=%d\n",reg);	
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	
	hi = high8bit(data);
	lo = low8bit(data);
	writeReg(RAMHI, hi);
	writeReg(RAMLO, lo);

}
*/

unsigned char readReg (int reg)
{
	
	//read register space register
	//printk("readReg CID=%d, reg=%d\n",Cid,reg);
	//byteToSpi ((unsigned char)(Cid|0x60)); //0x60 sets control byte
	//byteToSpi((unsigned char)(reg));

	//return spiToByte();

	return spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(Cid|0x60), (unsigned char)reg);

}


/*
unsigned short readRam (unsigned char reg)
{
	//read ram space address
	unsigned char hi,lo;
	  
	unsigned short lpt=0;
	
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	byteToSpi ((unsigned char)(0x40|Cid)); //0x40 Read RAM No broadcast
	byteToSpi ((unsigned char)reg);
	hi=spiToByte (); //dummy data
	lo=spiToByte ();
	//while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	
	//byteToSpi ((unsigned char)(0x40|Cid));
	//byteToSpi ((unsigned char)reg); //dummy reg
	//hi = spiToByte ();
	//lo = spiToByte ();
	return (SixteenBit (hi,lo));
}
*/


unsigned short readRam (unsigned char reg)
{
	//read ram space address
	unsigned char hi,lo;
	  
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	
	writeReg(RAMADDR,reg) ;
	
	
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	
	
	hi = readReg(RAMHI);
	lo = readReg(RAMLO);

	return (SixteenBit (hi,lo));
}


void broadcastReg (int reg,unsigned char data)
{
	//write .register
	//control bit 7 = 1  //broadcast
	//control bit 6 = 0 //write
	//control bit 5 = 1 //direct
	//control Cid = Cid
	  
	byteToSpi ((unsigned char)(Cid | 0x20 | 0x80));
	byteToSpi((unsigned char)(reg));
	byteToSpi(data);
}

void broadcastRAM (int reg,unsigned short data)
{
	//write ram space address BROADCAST MODE
  
	while (readReg (RAMSTAT)&0x01);//wait for indirect registers
	byteToSpi ((unsigned char)(Cid|0x80));
	byteToSpi ((unsigned char)(reg));
	byteToSpi (high8bit(data));
	byteToSpi (low8bit(data));

}

void reInit (unsigned short lpt)//LINC// Now it works!
{ 
	/*
	int i=0, single_bit_mask,single_bit;
	unsigned char ppbuffer;
	  
	single_bit_mask= 0x080;
	single_bit = 0;
	while (init[i]!=0) 
	{
		// if ((init [i] & CLOCK_MASK) == 0)  single_bit = (0x7f & single_bit_mask) ? 2 :0;  // This is bit 1 of //port
		ppbuffer =  init [i];
		
		_asm { mov dx,lpt
		mov al,ppbuffer
		out dx,al
		}
 
		// if ((clocking [i] & CLOCK_MASK) == 0) single_bit_mask >>=1;
 		// When the clock is low change the data
		i++;
	} // while
	*/
}//reInit

void byteToSpi (unsigned char byte )
{	
	//spi_si3220_write8((unsigned char)(Cid | 0x20), (unsigned char)reg, data);
}// byteToSpi

unsigned char  spiToByte()
{	
	//return spi_si3220_read8((unsigned char)(Cid|0x60), (unsigned char)reg);
}// spiToByte

/*
void byteToSpi (unsigned char byte )
{	
	int i=0, single_bit_mask,single_bit;
	unsigned char ppbuffer;
	single_bit_mask= 0x080;
	single_bit = 0;
	while (Output_clocking[i]!=0) 
	{
		if (Output_clocking [i] ==8) 
		{
		
			single_bit = (byte & single_bit_mask) ? 2 :0;  // This is bit 1 of //port
			single_bit_mask >>=1;
		}
		ppbuffer = single_bit | Output_clocking [i];
		_asm {	mov dx,lpt
			mov al,ppbuffer
			out dx,al
		}
	
	
		// When the clock is low change the data
		i++;
	} // while
}// byteToSpi


unsigned char  spiToByte()
{	
	unsigned char  ppbuffer, inputBuffer;
	int i=0,  data=0;
	unsigned short lpt1 = lpt + 1;
	i=0;
	while (Input_clocking[i] !=0xff) 
	{ 
		ppbuffer= Input_clocking[i] ;
		_asm {	mov dx,lpt
				mov al,ppbuffer
				out dx,al
				}
	
		if (Input_clocking[i]==0) 
		{
			data <<=1; // This includes one shift which is ignored
			_asm {		mov dx,lpt1
						in al,dx
						mov inputBuffer, al
					}
			data |= (0x80 & inputBuffer)? 0:1;
		}
	
		i++;
	} // while
	
	return data;
}// spiToByte
*/

//actual IO functions  ^^^

unsigned char high8bit (unsigned short data)
{//return most significant 8 bits of 16 bit number
//return ((unsigned char)(((data & 0xFF00)/16)/16));
return ((data>>8)&(0x0FF));
}

unsigned char low8bit (unsigned short data)
{//return least significant 8 bits of 16 bit number
return ((unsigned char)(data & 0x00FF));
}

static unsigned short SixteenBit (unsigned char hi, unsigned char lo)
{//returns 16 bit composition of hi and lo (8 bits)
//return (lo + (16*16*hi));
	unsigned short data = hi;
	data = (data<<8)|lo;
	return data;
}


void changeCID (unsigned char newCid) // assigns new value of Channel ID  
	  // Because Channel ID is sent in reverse order the bits are rotated here
{
	Cid = 0;//LOADS NEW Cid FROM LABVIEW or Assigns new value in bit reversed order to Cid
	Cid|= (newCid&0x1)<<3; // Alternative code  Cid=0; for (i =0; i<4 ; i++) { Cid |= newCid & 1 ; newCid>>1;}
	Cid|= (newCid&0x2)<<1;
	Cid|= (newCid&0x4)>>1;
	Cid|= (newCid&0x8)>>3;
} 

void changeBroadcast (unsigned char newbroad)
{
	broadcast = newbroad;  //CHANGES IN AND OUT OF BROADCAST MODE
}


/*  This simple routine reads back the Interrupt from the ProSLIC */


unsigned char  Interrupt(void )

{	
	int i=0,  data;
	unsigned short lpt1;
	unsigned char  ppbuffer,inputBuffer;
	i=0;
	/*
	while (interrupt_output[i] !=0) 
	{ 
	
	
	ppbuffer = interrupt_output[i]; 
	
	_asm {	
	
	mov dx,lpt
	mov al,ppbuffer
	 out dx,al
				}
	if (interrupt_output[i]==4) {
	lpt1 = lpt + 1;
	_asm {	
	mov dx,lpt1
	in al,dx
	mov inputBuffer, al
	
	}
	
	data = (0x20 & inputBuffer)? 0:1;
	}
	i++;
	} // while
	*/
	return data;
}// Interrupt





__s64 readTSC (void)
{
	union {
	__s64 extralong;
	unsigned long longish[2];
	} t;
	unsigned long a,b;
/*	
	_asm {
	_emit 0x0f;
	_emit 0x31;
	mov a,eax;
	mov b,edx;
	}
*/
	t.longish[0]=a;t.longish[1]=b;
	return t.extralong;
}


__u64 delay( unsigned long wait)
{
	//__s64 target = readTSC() + (ticksPerSecond * wait ) /1000 ;
	__s64 target  ;
	while (readTSC() < target) ;

	return ( readTSC() - target);

}


#include <asm/delay.h>
void slic_sleep( unsigned long wait )
{
	int i;
	//int goal;
	//goal = wait + clock();
	//while( goal > clock() )
	//;
	//udelay(wait);
	for(i=0;i<wait*100;i++);
}

__s64 calibratePrecisionClock(void)
{
	slic_sleep(1);
	time0= readTSC();
	slic_sleep (1800);
	time1 = readTSC();
	
	//ticksPerSecond=((time1-time0)/1800000)*1000000;
	return ticksPerSecond;
}
