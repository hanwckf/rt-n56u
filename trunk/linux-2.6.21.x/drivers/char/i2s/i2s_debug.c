#include <linux/init.h>
#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include "i2s_ctrl.h"

#if defined(CONFIG_I2S_WM8750)
#include "wm8750.h"
#endif
#if defined(CONFIG_I2S_WM8751)
#include "wm8751.h"
#endif

extern unsigned long i2sSlave_exclk_12p288Mhz[11];
extern unsigned long i2sSlave_exclk_12Mhz[11]; 

extern void audiohw_loopback(int fsel);
extern void audiohw_bypass(void);

int i2s_debug_cmd(unsigned int cmd, unsigned long arg)
{
	unsigned long data, index;
	unsigned long *pTable;
	int i;
	
	switch(cmd)
	{
		case I2S_DEBUG_CLKGEN:
			MSG("I2S_DEBUG_CLKGEN\n");
#if defined(CONFIG_RALINK_RT3052)
			*(unsigned long*)(0xB0000060) = 0x00000016;
			*(unsigned long*)(0xB0000030) = 0x00009E00;
			*(unsigned long*)(0xB0000A00) = 0xC0000040;
#elif defined(CONFIG_RALINK_RT3350)		
			*(unsigned long*)(0xB0000060) = 0x00000018;
			*(unsigned long*)(0xB000002C) = 0x00000100;
			*(unsigned long*)(0xB0000030) = 0x00009E00;
			*(unsigned long*)(0xB0000A00) = 0xC0000040;			
#elif defined(CONFIG_RALINK_RT3883)	
			*(unsigned long*)(0xB0000060) = 0x00000018;
			*(unsigned long*)(0xB000002C) = 0x00003000;
			*(unsigned long*)(0xB0000A00) = 0xC1104040;
			*(unsigned long*)(0xB0000A24) = 0x00000027;
			*(unsigned long*)(0xB0000A20) = 0x80000020;
#elif (defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)) || defined (CONFIG_RALINK_RT6855)
			*(unsigned long*)(0xB0000060) = 0x00000018;
			*(unsigned long*)(0xB000002C) = 0x00000300;
			*(unsigned long*)(0xB0000A00) = 0xC1104040;
			*(unsigned long*)(0xB0000A24) = 0x00000027;
			*(unsigned long*)(0xB0000A20) = 0x80000020;			
#elif defined(CONFIG_RALINK_RT6855A)
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x860) = 0x00008080;
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
			*(unsigned long*)(RALINK_I2S_BASE+0x00) = 0xC1104040;
			*(unsigned long*)(RALINK_I2S_BASE+0x24) = 0x00000027;
			*(unsigned long*)(RALINK_I2S_BASE+0x20) = 0x80000020;	
#else
//#error "I2S debug mode not support this Chip"			
#endif			
			break;
		case I2S_DEBUG_INLBK:
			MSG("I2S_DEBUG_INLBK\n");
#if defined(CONFIG_RALINK_RT3052)
			break;
#endif
#if defined(CONFIG_RALINK_RT6855A)
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x834) |= 0x00020000;
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x834) &= 0xFFFDFFFF;	
			*(unsigned long*)(RALINK_I2S_BASE+0x0) &= 0x7FFFFFFF;	//Rest I2S to default vaule	
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x860) |= 0x00008080;
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
#else						
			*(unsigned long*)(0xB0000034) |= 0x00020000;
			*(unsigned long*)(0xB0000034) &= 0xFFFDFFFF;	
			*(unsigned long*)(0xB0000A00) &= 0x7FFFFFFF;	//Rest I2S to default vaule	
			*(unsigned long*)(0xB0000060) = 0x00000018;

#if defined(CONFIG_RALINK_RT3883)			
			*(unsigned long*)(0xB000002C) = 0x00003000;
#else
			*(unsigned long*)(0xB000002C) = 0x00000300;
#endif
#endif			

			*(unsigned long*)(RALINK_I2S_BASE+0x18) = 0x80000000;
			*(unsigned long*)(RALINK_I2S_BASE+0x00) = 0xC1104040;
			*(unsigned long*)(RALINK_I2S_BASE+0x24) = 0x00000006;
			*(unsigned long*)(RALINK_I2S_BASE+0x20) = 0x80000105;

			{
				int count = 0;
				unsigned long param[4];
				unsigned long data;
				register unsigned long ff_status;
				
				copy_from_user(param, (unsigned long*)arg, sizeof(long)*2);
	
				for( i = 0 ; i < param[0] ; i ++ )
				{
					int j;
					ff_status = *(unsigned long*)(RALINK_I2S_BASE+0x0C);
							
					if((ff_status&0x0F) > 0)
					{
						*(unsigned long*)(RALINK_I2S_BASE+0x10) = param[1];
						for(j=0;j<10000;j++);
						//if(i < 16)
						//	data = *(unsigned long*)(RALINK_I2S_BASE+0x14);
					}
					else
					{
						for(j=0;j<10000;j++);
							printk("NO TX FREE FIFO ST=[0x%08X]\n",(u32)ff_status);
						//if(i < 16)
						//	data = *(unsigned long*)(RALINK_I2S_BASE+0x14);
						continue;	
					}
					
					//if(i >= 16)
					{
						ff_status = *(unsigned long*)(RALINK_I2S_BASE+0x0C);	
						if(((ff_status>>4)&0x0F) > 0)
							data = *(unsigned long*)(RALINK_I2S_BASE+0x14);
						else
							continue;
								
						if(data!=param[1])
						{
							MSG("[%d][0x%08X] vs [0x%08X]\n" , i, (u32)data , (u32)param[1]);
						}
						else
						{
							count++;
							data = 0;	
						}
					}	
				}
				MSG("Pattern match done count=%d.\n", count);
			}							
#if !defined(CONFIG_RALINK_RT3052)
			break;
#endif
		case I2S_DEBUG_EXLBK:
			MSG("I2S_DEBUG_EXLBK\n");
			switch(arg)
			{
				case 8000:
					index = 0;
					break;
				case 11025:
					index = 1;
					break;
				case 12000:
					index = 2;
					break;			
				case 16000:
					index = 3;
					break;
				case 22050:
					index = 4;
					break;
				case 24000:
					index = 5;
					break;	
				case 32000:
					index = 6;
					break;			
				case 44100:
					index = 7;
					break;
				case 48000:
					index = 8;
					break;
				case 88200:
					index = 9;
					break;	
				case 96000:
					index = 10;
					break;
				default:
					index = 7;
			}
#if defined(CONFIG_RALINK_RT3052)
			break;
#endif			
#if defined(CONFIG_RALINK_RT6855A)
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x860) = 0x00008080;
			//*(unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
#else			
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x60) = 0x00000018;
#if defined(CONFIG_RALINK_RT3883)
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x2C) = 0x00003000;			
#else
			*(unsigned long*)(RALINK_SYSCTL_BASE+0x2C) = 0x00000300;
#endif
#endif
	
			*(unsigned long*)(RALINK_I2S_BASE+0x18) = 0x40000000;
			*(unsigned long*)(RALINK_I2S_BASE+0x00) = 0x81104040;
			
		#if defined(CONFIG_I2S_MCLK_12MHZ)
			pTable = i2sSlave_exclk_12Mhz;
			data = pTable[index]|0x01;
		#else
			pTable = i2sSlave_exclk_12p288Mhz;
			data = pTable[index];
		#endif
			//*(unsigned long*)(RALINK_I2S_BASE+0x24) = 0x00000027;
			//*(unsigned long*)(RALINK_I2S_BASE+0x20) = 0x80000020;
#if defined(CONFIG_I2S_TXRX)			
			audiohw_loopback(data);
#endif		
#if !defined(CONFIG_RALINK_RT3052)
			break;
#endif
		case I2S_DEBUG_CODECBYPASS:			
#if defined(CONFIG_I2S_TXRX)
			audiohw_bypass();	/* did not work */
#endif
			break;	
		case I2S_DEBUG_FMT:
			break;
		case I2S_DEBUG_RESET:
			break;	
		default:
			MSG("Not support this debug cmd [%d]\n", cmd);	
			break;				
	}
	
	return 0;	
}
