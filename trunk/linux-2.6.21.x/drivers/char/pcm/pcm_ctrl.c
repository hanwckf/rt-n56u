#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static	devfs_handle_t devfs_handle;
#endif

#include "pcm_ctrl.h"
#include "../ralink_gpio.h"
#include "../spi_drv.h"
#include "./si3220_rev1.5/dual_io.h"
#include "./si3220_rev1.5/dual.h"

#include "../ralink_gdma.h"

pcm_config_type* ppcm_config;
pcm_status_type* ppcm_status;

extern int si3220_init(void);
extern int ProSLIC_HWInit(void);
extern unsigned short si3210_init (void);
extern u8 spi_si3220_read8(int sid, unsigned char cid, unsigned char reg);
extern void spi_si3220_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value);
extern u8 spi_si321x_read8(int sid, unsigned char cid, unsigned char reg);
extern void spi_si321x_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value);
extern void spi_si3220_write16(int sid, unsigned char cid, unsigned char reg, unsigned short value);
extern void voice_release_codec(codec_data_type* pcodec);

int pcm_clock_setup(void);
int pcm_clock_enable(void);
int pcm_clock_disable(void);
int pcm_open(void);
int pcm_close(void);
int pcm_reg_setup(pcm_config_type* ptrpcm_config); 
int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config);
int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config);
int PCM_HooK_Init();
int PCM_HooK_Release();

void pcm_dma_tx_isr(u32 chid);
void pcm_dma_rx_isr(u32 chid);
void pcm_unmask_isr(u32 dma_ch);

void pcm_reset_slic (void);

void pcm_dump_reg (void);


unsigned char linear2alaw(short pcm_val);    /* 2's complement (16-bit range) */
short alaw2linear(unsigned char a_val);
unsigned char linear2ulaw(short pcm_val);    /* 2's complement (16-bit range) */
short ulaw2linear(unsigned char u_val);

static irqreturn_t pcm_irq_isr(int irq, void *irqaction);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static int pcm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif

struct file_operations pcmdrv_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl:     pcm_ioctl,
#else	
    ioctl:      pcm_ioctl,
#endif
};

static int pcmdrv_major =  233;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
static struct class *pcmmodule_class;
#endif

static dma_addr_t TxPage0[CONFIG_PCM_CH], TxPage1[CONFIG_PCM_CH];
static dma_addr_t RxPage0[CONFIG_PCM_CH], RxPage1[CONFIG_PCM_CH];

#ifdef PCM_TASKLET
struct tasklet_struct pcm_rx_tasklet;
struct tasklet_struct pcm_tx_tasklet;
#endif

unsigned int slic_type = 3226;
unsigned int idiv = CONFIG_RALINK_PCMINTDIV, cdiv=CONFIG_RALINK_PCMCOMPDIV, smode=CONFIG_RALINK_PCMSLOTMODE;
codec_data_type codec_obj[MAX_CODEC_CH];

int __init pcm_init(void)
{
#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(pcmdrv_major, PCMDRV_DEVNAME , &pcmdrv_fops)) {
		printk(KERN_WARNING " pcm: can't create device node - %s\n",PCMDRV_DEVNAME);
		return -EIO;
    }

    devfs_handle = devfs_register(NULL, PCMDRV_DEVNAME, DEVFS_FL_DEFAULT, pcmdrv_major, 0, 
	    S_IFCHR | S_IRUGO | S_IWUGO, &pcmdrv_fops, NULL);
#else
    int result=0;
    result = register_chrdev(pcmdrv_major, PCMDRV_DEVNAME, &pcmdrv_fops);
    if (result < 0) {
		printk(KERN_WARNING "pcm: can't get major %d\n",pcmdrv_major);
        return result;
    }

    if (pcmdrv_major == 0) {
		pcmdrv_major = result; /* dynamic */
    }
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
	pcmmodule_class=class_create(THIS_MODULE, PCMDRV_DEVNAME);
	if (IS_ERR(pcmmodule_class)) 
		return -EFAULT;
	device_create(pcmmodule_class, NULL, MKDEV(pcmdrv_major, 0), PCMDRV_DEVNAME);
#endif	
	MSG("PCMRST map to GPIO%d\n", CONFIG_RALINK_PCMRST_GPIO);
	MSG("Total %d PCM channel number supported\n", MAX_PCM_CH);
#if defined(CONFIG_RALINK_PCMEXTCLK) 	
	MSG("PCMCLK clock source from SoC external OSC\n");
#else
	MSG("PCMCLK clock source from SoC internal clock\n");	
#endif

#if defined(CONFIG_RALINK_PCMFRACDIV)	
	MSG("PCMCLK clock dividor Int[%d], Comp[%d]\n", idiv, cdiv);
#else
	MSG("PCMCLK clock dividor [%d]\n", CONFIG_RALINK_PCMDIV);	
#endif	
	MSG("PCM slot mode is %d\n", smode);
		
	pcm_open();	
	return 0;
}

void pcm_exit(void)
{
	pcm_close();
	
#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(pcmdrv_major, PCMDRV_DEVNAME);
    devfs_unregister(devfs_handle);
#else
    unregister_chrdev(pcmdrv_major, PCMDRV_DEVNAME);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
	device_destroy(pcmmodule_class,MKDEV(pcmdrv_major, 0));
	class_destroy(pcmmodule_class); 
#endif	
	return ;
}

int pcm_open(void)
{
	int i, data, flags;
	
	/* set pcm_config */
	ppcm_config = (pcm_config_type*)kmalloc(sizeof(pcm_config_type), GFP_KERNEL);
	if(ppcm_config==NULL)
		return PCM_OUTOFMEM;
	memset(ppcm_config, 0, sizeof(pcm_config_type));

#ifdef PCM_STATISTIC
	ppcm_status = (pcm_status_type*)kmalloc(sizeof(pcm_status_type), GFP_KERNEL);
	if(ppcm_status==NULL)
		return PCM_OUTOFMEM;
	memset(ppcm_status, 0, sizeof(pcm_status_type));
#endif
	
	ppcm_config->pcm_ch_num = CONFIG_PCM_CH;
	ppcm_config->codec_ch_num = MAX_CODEC_CH;
	ppcm_config->nch_active = 0;
	ppcm_config->extclk_en = CONFIG_PCM_EXT_CLK_EN;
	ppcm_config->clkout_en = CONFIG_PCM_CLKOUT_EN;
	ppcm_config->ext_fsync = CONFIG_PCM_EXT_FSYNC;
	ppcm_config->long_fynsc = CONFIG_PCM_LONG_FSYNC;
	ppcm_config->fsync_pol = CONFIG_PCM_FSYNC_POL;
	ppcm_config->drx_tri = CONFIG_PCM_DRX_TRI;
	ppcm_config->slot_mode = smode;//CONFIG_RALINK_PCMSLOTMODE;
	
	ppcm_config->tff_thres = CONFIG_PCM_TFF_THRES;
	ppcm_config->rff_thres = CONFIG_PCM_RFF_THRES;
		
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->lbk[i] = CONFIG_PCM_LBK;
		ppcm_config->ext_lbk[i] = CONFIG_PCM_EXT_LBK;
		ppcm_config->cmp_mode[i] = CONFIG_PCM_CMP_MODE;
#if defined(PCM_LINEAR) || defined(PCM_U2L2U) || defined(PCM_A2L2A)		
		ppcm_config->ts_start[i] = CONFIG_PCM_TS_START + i*16;	
#else
        ppcm_config->ts_start[i] = CONFIG_PCM_TS_START + i*8;
#endif		
		ppcm_config->txfifo_rd_idx[i] = 0;
		ppcm_config->txfifo_wt_idx[i] = 0;
		ppcm_config->rxfifo_rd_idx[i] = 0;
		ppcm_config->rxfifo_wt_idx[i] = 0;
		ppcm_config->bsfifo_rd_idx[i] = 0;
		ppcm_config->bsfifo_wt_idx[i] = 0;

	}

	MSG("allocate fifo buffer\n");
	/* allocate fifo buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->TxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE*MAX_PCM_FIFO, GFP_KERNEL);
		if(ppcm_config->TxFIFOBuf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}

		ppcm_config->RxFIFOBuf16Ptr[i] = kmalloc(PCM_FIFO_SIZE*MAX_PCM_FIFO, GFP_KERNEL);
		if(ppcm_config->RxFIFOBuf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		} 		
	}
	MSG("allocate page buffer\n");
	/* allocate page buffer */
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->TxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage0[i]);
		if(ppcm_config->TxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->TxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage1[i]);
		if(ppcm_config->TxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage0[i]);
		if(ppcm_config->RxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage1[i]);
		if(ppcm_config->RxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
	}
	

	/* PCM controller reset */

PCM_RESET:	
	
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data |= 0x00000800;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data &= 0xFFFFF7FF;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);
	for(i=0;i<100000;i++);
	
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data |= 0x00040000;
	pcm_outw(RALINK_SYSCTL_BASE+0x34, data);
	data = pcm_inw(RALINK_SYSCTL_BASE+0x34);
	data &= 0xFFFBFFFF;
	pcm_outw(RALINK_SYSCTL_BASE+0x34,data);

	/* Set UARTF_SHARE_MODE field */	
	data = pcm_inw(RALINK_REG_GPIOMODE);
	data &= 0xFFFFFFE1;
#if defined(CONFIG_RALINK_PCMI2S)
	data |= 0x00000008;
#endif
#if defined(CONFIG_RALINK_PCMGPIO)
	data |= 0x00000010;
#endif
#if defined(CONFIG_RALINK_PCMUARTF)
	data |= 0x00000004;
#endif	
	pcm_outw(RALINK_REG_GPIOMODE, data);
	MSG("RALINK_REG_GPIOMODE=0x%08X\n",data);

	if(pcm_reg_setup(ppcm_config)!=PCM_OK)
		MSG("PCM:pcm_reg_setup() failed\n");
	
	pcm_clock_setup();
	
	spin_lock_irqsave(&ppcm_config->lock, flags);

	/* Set to SP1_CS1_MODE mode and SPI_GPIO_MODE to spi mode */
	data = pcm_inw(RALINK_REG_GPIOMODE);
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855)
	data &= 0xFF9FFFFD;
#elif defined(CONFIG_RALINK_RT6352)
	data &= ~(1<<12);
#else
	data &= 0xFFFFFFFD;	
#endif	
	pcm_outw(RALINK_REG_GPIOMODE, data);
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)	
	/* SPI_CS1 set to tristate */
	RT2880_REG(RT2880_SPI_ARB_REG) |= 0x80000000; 
	RT2880_REG(RT2880_SPI1_CFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128 |
									SPICFG_SPICLKPOL;							
	mdelay(300);
#else
	/* SPI_CS0 set to tristate */
	RT2880_REG(RT2880_SPI0_CFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128;
	mdelay(300);	
#endif	

    data = pcm_inw(PCM_GLBCFG);
    data |= REGBIT(0x1, PCM_EN);
    pcm_outw(PCM_GLBCFG, data);
	
	pcm_clock_enable();
		
#if defined(PCM_INLOOP)
#else	
	MSG("SLIC %d initialization....\n",slic_type);
	
	if(slic_type==3220)
	{
		pcm_reset_slic();
		if(si3220_init()!=0)
		{
			for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
			spin_unlock_irqrestore(&ppcm_config->lock, flags);	
			goto PCM_RESET;
		}
	}
	else if(slic_type==3210)
	{
		/* slic reset in si3210_init() */
#if defined(SI321X)		
		if(si3210_init()==0)
		{
			for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
			spin_unlock_irqrestore(&ppcm_config->lock, flags);	
			goto PCM_RESET;
		}
#endif		
	}
	else if((slic_type==3227)||(slic_type=3226))
	{
#if defined(SI322X)			
		if(ProSLIC_HWInit()==0)
		{
			for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
		
			spin_unlock_irqrestore(&ppcm_config->lock, flags);
			goto PCM_RESET;
		}
#endif		
	}	
	else
	{
		MSG("slic type not supported\n");
		for ( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ppcm_config);
				
		spin_unlock_irqrestore(&ppcm_config->lock, flags);	
		goto PCM_RESET;
	}
#endif
	spin_unlock_irqrestore(&ppcm_config->lock, flags);

	MSG("pcm_open done...\n");

	return PCM_OK;
}

int pcm_reg_setup(pcm_config_type* ptrpcm_config)
{
	unsigned int data = 0;
	int i;	
	/* set GLBCFG's threshold fields */

	data = pcm_inw(PCM_GLBCFG);
#if defined (CONFIG_RALINK_RT6352)
	data |= REGBIT(CONFIG_PCM_LBK, PCM_LBK);
#else
#endif	
	data |= REGBIT(ptrpcm_config->tff_thres, TFF_THRES);
	data |= REGBIT(ptrpcm_config->rff_thres, RFF_THRES);
	MSG("PCM_GLBCFG=0x%08X\n",data);
	pcm_outw(PCM_GLBCFG, data);
	
	/* set PCMCFG */
	data = pcm_inw(PCM_PCMCFG);
	data |= REGBIT(ptrpcm_config->ext_fsync, PCM_EXT_FSYNC);
	data |= REGBIT(ptrpcm_config->long_fynsc, PCM_LONG_FSYNC);
	data |= REGBIT(ptrpcm_config->fsync_pol, PCM_FSYNC_POL);
	data |= REGBIT(ptrpcm_config->drx_tri, PCM_DRX_TRI);
	data &=  ~REGBIT(0x7, PCM_SLOTMODE);
	data |= REGBIT(ptrpcm_config->slot_mode, PCM_SLOTMODE);
	//data |= REGBIT(ptrpcm_config->clkout_en, PCM_CLKOUT);
	MSG("PCM_PCMCFG=0x%08X\n",data);
	pcm_outw(PCM_PCMCFG, data);
#if defined(CONFIG_RALINK_RT6352)
	for (i = 0; i < CONFIG_PCM_CH; i++)
	{
		data = pcm_inw(PCM_CH_CFG(i));
		data &= ~REGBIT(0x7, PCM_CMP_MODE);
		data &= ~REGBIT(0x3FF, PCM_TS_START);
		data |= REGBIT(ptrpcm_config->cmp_mode[i], PCM_CMP_MODE);
		data |= REGBIT(ptrpcm_config->ts_start[i], PCM_TS_START);
		MSG("PCM_CH_CFG(%d)=0x%08X\n",i,data);
		pcm_outw(PCM_CH_CFG(i), data);
	}  
#else	
	/* set CH0/1_CFG */	
	data = pcm_inw(PCM_CH0_CFG);
	data |= REGBIT(ptrpcm_config->lbk[0], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[0], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[0], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[0], PCM_TS_START);
	MSG("PCM_CH0_CFG=0x%08X\n",data);
	pcm_outw(PCM_CH0_CFG, data);

	data = pcm_inw(PCM_CH1_CFG);
	data |= REGBIT(ptrpcm_config->lbk[1], PCM_LBK);
	data |= REGBIT(ptrpcm_config->ext_lbk[1], PCM_EXT_LBK);
	data |= REGBIT(ptrpcm_config->cmp_mode[1], PCM_CMP_MODE);
	data |= REGBIT(ptrpcm_config->ts_start[1], PCM_TS_START);
	MSG("PCM_CH1_CFG=0x%08X\n",data);
	pcm_outw(PCM_CH1_CFG, data);
#endif

#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) \
	|| defined(CONFIG_RALINK_RT6352)
	data = pcm_inw(PCM_DIGDELAY_CFG);
	data = 0x00008484;
	MSG("PCM_DIGDELAY_CFG=0x%08X\n",data);
	pcm_outw(PCM_DIGDELAY_CFG, data);
#endif
	
	return PCM_OK;
}

int pcm_clock_setup(void)
{
	unsigned long data;

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT5350) \
	|| defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6352)
	MSG("PCM: enable fractinal PCM_CLK\n");
	pcm_outw(PCM_DIVINT_CFG, idiv);
	pcm_outw(PCM_DIVCOMP_CFG, cdiv|0x80000000);
#else	
	/* System controller PCMCLK_DIV set */
	data = pcm_inw(RALINK_SYSCTL_BASE+0x30);

#if defined(CONFIG_RALINK_PCMEXTCLK)
	data |= REGBIT(1, PCM_CLK_SEL);
#else
	data &= ~REGBIT(1, PCM_CLK_SEL);
#endif	
	data |= REGBIT(1, PCM_CLK_EN);	
	data &= 0xFFFFFFC0;
	data |= REGBIT(CONFIG_RALINK_PCMDIV, PCM_CLK_DIV);
	data |= 0x00000080;
	
	pcm_outw(RALINK_SYSCTL_BASE+0x30, data);
	MSG("RALINK_SYSCTL_BASE+0x30=0x%08X\n",(u32)data);	
#endif	
	
	/* set PCMCFG external PCMCLK control bit */
	data = pcm_inw(PCM_PCMCFG);
#if defined(CONFIG_RALINK_PCMEXTCLK)
	data |= REGBIT(1, PCM_EXT_CLK_EN);
#else
	data &= ~REGBIT(1, PCM_EXT_CLK_EN);
#endif	
	pcm_outw(PCM_PCMCFG, data);
	MSG("PCM_PCMCFG=0x%08X\n",(u32)data);	
	
	return 0;	
}

int pcm_clock_enable(void)
{
	unsigned long data;
	/* set PCMCFG clock out bit */
	data = pcm_inw(PCM_PCMCFG);	
	data |= REGBIT(1,  PCM_CLKOUT);
	pcm_outw(PCM_PCMCFG, data);
	MSG("PCM_PCMCFG=0x%08X\n",(u32)data);
	
	return 0;	
}

int pcm_clock_disable(void)
{
	unsigned long data;
	/* set PCMCFG clock out bit */
	data = pcm_inw(PCM_PCMCFG);	
	data &= ~REGBIT(1,  PCM_CLKOUT);
	pcm_outw(PCM_PCMCFG, data);
	MSG("PCM_PCMCFG=0x%08X\n",(u32)data);
	
	return 0;	
}
	
int pcm_close(void)
{
	int i;
		
	MSG("pcm_close\n");	

	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
		pcm_disable(i, ppcm_config);
	
#ifdef PCM_STATISTIC
	kfree(ppcm_status);
#endif
	
	/* free buffer */
	for( i = 0 ; i < ppcm_config->pcm_ch_num ; i ++ )
	{
		if(ppcm_config->TxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->TxPage0Buf16Ptr[i], TxPage0[i]);
		if(ppcm_config->TxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->TxPage1Buf16Ptr[i], TxPage1[i]);	
		if(ppcm_config->RxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->RxPage0Buf16Ptr[i], RxPage0[i]);
		if(ppcm_config->RxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->RxPage1Buf16Ptr[i], RxPage1[i]);					
		if(ppcm_config->TxFIFOBuf16Ptr[i])
			kfree(ppcm_config->TxFIFOBuf16Ptr[i]);	
		if(ppcm_config->RxFIFOBuf16Ptr[i])
			kfree(ppcm_config->RxFIFOBuf16Ptr[i]);
#ifdef PCM_SW_G729AB			
		if(ppcm_config->BSFIFOBuf16Ptr[i])
			kfree(ppcm_config->BSFIFOBuf16Ptr[i]);
#endif						
	}

	kfree(ppcm_config);
	ppcm_config = NULL;
	
	return PCM_OK;
}

int pcm_enable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int GLBCFG_Data=0, int_en;
	
	if(ptrpcm_config->nch_active>=ptrpcm_config->pcm_ch_num)
	{
		MSG("There are %d channels already enabled\n",ptrpcm_config->nch_active);
		return PCM_OK;
	}
	int_en = pcm_inw(PCM_INT_EN);
	GLBCFG_Data = pcm_inw(PCM_GLBCFG);

	pcm_outw(PCM_INT_STATUS, 0x0);
	
	if(ptrpcm_config->nch_active==0)
		GLBCFG_Data |= REGBIT(0x1, DMA_EN);
#if defined (CONFIG_RALINK_RT6352)
	ptrpcm_config->nch_active++;
	GLBCFG_Data |= REGBIT(0x1, CH_EN+chid);
#else
	switch(chid)
	{
		case 0:
			MSG("PCM:enable CH0\n");
			GLBCFG_Data |= REGBIT(0x1, CH0_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH0_RX_EN);
			
			int_en |= REGBIT(0x1, CH0T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH0R_DMA_FAULT);
			 
			ptrpcm_config->nch_active++;
			break;
		case 1:
			MSG("PCM:enable CH1\n");
			GLBCFG_Data |= REGBIT(0x1, CH1_TX_EN);
			GLBCFG_Data |= REGBIT(0x1, CH1_RX_EN);
			
			int_en |= REGBIT(0x1, CH1T_DMA_FAULT);
			int_en |= REGBIT(0x1, CH1R_DMA_FAULT);
 
			ptrpcm_config->nch_active++;
			break;
		default:
			break;
	}
#endif	

	//GLBCFG_Data |= REGBIT(0x1, PCM_EN);
	pcm_outw(PCM_INT_EN, int_en);
	pcm_outw(PCM_GLBCFG, GLBCFG_Data);
	
	return PCM_OK;
}

int pcm_disable(unsigned int chid, pcm_config_type* ptrpcm_config)
{
	unsigned int data, int_en;

	if(ptrpcm_config->nch_active<=0)
	{ 
		MSG("No channels needed to disable\n");
		return PCM_OK;
	}
	ppcm_config->txfifo_rd_idx[chid] = 0;
	ppcm_config->txfifo_wt_idx[chid] = 0;
	ppcm_config->rxfifo_rd_idx[chid] = 0;
	ppcm_config->rxfifo_wt_idx[chid] = 0;
	ppcm_config->bsfifo_rd_idx[chid] = 0;
	ppcm_config->bsfifo_wt_idx[chid] = 0;
	
	int_en = pcm_inw(PCM_INT_EN);
	data = pcm_inw(PCM_GLBCFG);
	
#if defined (CONFIG_RALINK_RT6352)
	data &= ~REGBIT(0x1, CH_EN+chid);
	ptrpcm_config->nch_active--;
#else
	switch(chid)
	{
		case 0:
			MSG("PCM:disable CH0\n");
			data &= ~REGBIT(0x1, CH0_TX_EN);
			data &= ~REGBIT(0x1, CH0_RX_EN);
			int_en &= ~REGBIT(0x1, CH0T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH0R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		case 1:
			MSG("PCM:disable CH1\n");
			data &= ~REGBIT(0x1, CH1_TX_EN);
			data &= ~REGBIT(0x1, CH1_RX_EN);
			int_en &= ~REGBIT(0x1, CH1T_DMA_FAULT);
			int_en &= ~REGBIT(0x1, CH1R_DMA_FAULT);
			pcm_outw(PCM_INT_EN, int_en);
			ptrpcm_config->nch_active--;

			break;
		default:
			break;
	}
#endif	
	if(ptrpcm_config->nch_active<=0)
	{
		//data &= ~REGBIT(0x1, PCM_EN);
		data &= ~REGBIT(0x1, DMA_EN);
	}
	pcm_outw(PCM_GLBCFG, data);
	return PCM_OK;
}

void pcm_dma_tx_isr(u32 dma_ch)
{
	int i;
	int chid=0;
	int page=0;
	char* p8PageBuf=NULL, *p8FIFOBuf=NULL, *p8Data;
	u32 pcm_status;
	u32* pPCM_FIFO=NULL;

	if(ppcm_config->nch_active<=0)
	{
		MSG("No Active Channel for DMA[%d]\n", dma_ch);
		return;
	}	

	ppcm_config->tx_isr_cnt++;

	chid = (dma_ch>>2);
	page = (dma_ch&0x03)-2;
	pPCM_FIFO = (u32*)(PCM_CH_FIFO(chid));

	if(page==0)
		p8PageBuf = ppcm_config->TxPage0Buf8Ptr[chid];
	else
		p8PageBuf = ppcm_config->TxPage1Buf8Ptr[chid];

	if((chid>=CONFIG_PCM_CH)||(page>=2))
	{
		MSG("Invalid TX dma=%d chid=%d page=%d\n", dma_ch, chid, page);
		return;
	}	
	//if((ppcm_config->tx_isr_cnt%170==11)||(ppcm_config->tx_isr_cnt%170==90))
	//{ 
		//printk("ti dc=%d c=%d p=%d i=%d\n",dma_ch,chid,page, ppcm_config->tx_isr_cnt);	
	//}
	
	p8FIFOBuf = ppcm_config->TxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;
#if !defined(PCM_INLOOP)
	//if(ppcm_config->tx_isr_cnt>(CONFIG_PCM_CH*4))
#endif	
	{
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			if(ppcm_config->txfifo_rd_idx[chid]==ppcm_config->txfifo_wt_idx[chid])
			{
				/* tx fifo empty */
				printk("TFE[%d](%d) (r=%d,w=%d)(i=%d)\n",chid,dma_ch, ppcm_config->txfifo_rd_idx[chid], 
						ppcm_config->txfifo_wt_idx[chid], ppcm_config->tx_isr_cnt);
				break;
			}
			
			p8Data = p8FIFOBuf + (ppcm_config->txfifo_rd_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			memcpy((void*)(p8PageBuf+ppcm_config->pos), p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);	
			
			ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
			ppcm_config->txfifo_rd_idx[chid] = (ppcm_config->txfifo_rd_idx[chid]+1)%MAX_PCM_FIFO;
		}
	}
	
	ppcm_config->txcurchid = chid;
#if defined(PCM_INLOOP)	
	if(chid==(CONFIG_PCM_CH-1))	
		tasklet_hi_schedule(&pcm_tx_tasklet);	
#endif		
	GdmaPcmTx((u32)p8PageBuf, (u32)pPCM_FIFO, chid, page, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);	
	GdmaUnMaskChannel(GDMA_PCM_TX(chid, 1-page)); 
	
	return;
	
}

void pcm_dma_rx_isr(u32 dma_ch)
{
	int i;
	int chid=0; 
	int page=0;

	char* p8PageBuf=NULL, *p8FIFOBuf=NULL, *p8Data;

	u32 pcm_status=0;
	u32* pPCM_FIFO=NULL;
	
	if(ppcm_config->nch_active<=0)
	{
		MSG("No Active Channel for DMA[%d]\n", dma_ch);
		return;
	}	
	
	ppcm_config->rx_isr_cnt++;
	chid = (dma_ch>>2);
	page = (dma_ch&0x03);
	pPCM_FIFO = (u32*)(PCM_CH_FIFO(chid));
	if(page==0)
		p8PageBuf = ppcm_config->RxPage0Buf8Ptr[chid];
	else
		p8PageBuf = ppcm_config->RxPage1Buf8Ptr[chid];
	
	if((chid>=CONFIG_PCM_CH)||(page>=2))
	{
		MSG("Invalid TX dma=%d chid=%d page=%d\n", dma_ch, chid, page);
		return;
	}	
	
	//if((ppcm_config->rx_isr_cnt%170==50)||(ppcm_config->rx_isr_cnt%170==129))
	//{ 
	//	printk("ri dc=%d c=%d p=%d i=%d\n",dma_ch,chid,page, ppcm_config->rx_isr_cnt);	
	//}

	p8FIFOBuf = ppcm_config->RxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;
	
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
		if(((ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO)==ppcm_config->rxfifo_rd_idx[chid])
		{
			/* rx fifo full */
			printk("RFF[%d](%d) (r=%d,w=%d)(i=%d)",chid,dma_ch,ppcm_config->rxfifo_rd_idx[chid], 
					ppcm_config->rxfifo_wt_idx[chid], ppcm_config->rx_isr_cnt);
			break;
		}
		ppcm_config->rxfifo_wt_idx[chid] = (ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO;
		p8Data = p8FIFOBuf + (ppcm_config->rxfifo_wt_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		memcpy((void*)p8Data, (void*)(p8PageBuf+ppcm_config->pos), PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;	
	}
	
	ppcm_config->curchid = chid;

	if(chid==(CONFIG_PCM_CH-1))
		tasklet_hi_schedule(&pcm_rx_tasklet);
		
	GdmaPcmRx((u32)pPCM_FIFO, (u32)p8PageBuf, chid, page, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	GdmaUnMaskChannel(GDMA_PCM_RX(chid, 1-page));	
	return;
}

#define MAX_SESSION		2
#define MAX_HOOK_FIFO	12
unsigned short* txhook_fifo[MAX_SESSION][MAX_HOOK_FIFO];
unsigned short* rxhook_fifo[MAX_SESSION][MAX_HOOK_FIFO];
int txhook_rd_idx[MAX_SESSION],txhook_wt_idx[MAX_SESSION],rxhook_rd_idx[MAX_SESSION],rxhook_wt_idx[MAX_SESSION];

int PCM_HooK_Init()
{
	int i, j;
	for(i = 0 ; i < MAX_SESSION ; i++)
	{
		txhook_rd_idx[i] = 0;
		txhook_wt_idx[i] = 0;
	
		rxhook_rd_idx[i] = 0;
		rxhook_wt_idx[i] = 0;
	}
	
	for(i = 0 ; i < MAX_SESSION ; i++)
	{
		for(j = 0 ; j < MAX_HOOK_FIFO ; j++)
		{
			txhook_fifo[i][j] = (short*)kmalloc(PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE, GFP_KERNEL);
			rxhook_fifo[i][j] = (short*)kmalloc(PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE, GFP_KERNEL);
		}
	}
	init_waitqueue_head(&(ppcm_config->pcm_qh));
}

int PCM_HooK_Release()
{
	int i, j;
	for(i = 0 ; i < MAX_SESSION ; i++)
	{
		txhook_rd_idx[i] = 0;
		txhook_wt_idx[i] = 0;
	
		rxhook_rd_idx[i] = 0;
		rxhook_wt_idx[i] = 0;
	}

	for(i = 0 ; i < MAX_SESSION ; i++)
	{
		for(j = 0 ; j < MAX_HOOK_FIFO ; j++)
		{
			kfree(txhook_fifo[i][j]);
			kfree(rxhook_fifo[i][j]);
		}
	}
}
	
int PCM_TX_Hook(int sid, short* pPCM)
{
	if(txhook_rd_idx[sid]!=txhook_wt_idx[sid])
	{
		memcpy((char*)pPCM, (char*)txhook_fifo[sid][txhook_rd_idx[sid]], PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE);
		txhook_rd_idx[sid] = (txhook_rd_idx[sid]+1) % MAX_HOOK_FIFO;
		wake_up_interruptible(&(ppcm_config->pcm_qh));
	}
}

int PCM_RX_Hook(int sid, short* pPCM)
{
	int index = (rxhook_wt_idx[sid]+1) % MAX_HOOK_FIFO;
	if(index!=rxhook_rd_idx[sid])
	{
		memcpy((char*)rxhook_fifo[sid][index], (char*)pPCM, PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE);
		rxhook_wt_idx[sid] = index;
		wake_up_interruptible(&(ppcm_config->pcm_qh));
	}
}		

int PCM_UserPutData_Hook(int sid, short* pPCM)
{
	int index ;
	local_irq_disable();
	index = (txhook_wt_idx[sid]+1) % MAX_HOOK_FIFO;
	local_irq_enable();
	while(index==txhook_rd_idx[sid])
	{
		//printk("PD wt=%d\n",index);
		interruptible_sleep_on(&(ppcm_config->pcm_qh));
	}
			
	copy_from_user((char*)txhook_fifo[sid][index], (char*)pPCM, PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE);
	
	local_irq_disable();
	txhook_wt_idx[sid] = index;
	local_irq_enable();
	return 0;
}

int PCM_UserGetData_Hook(int sid, short* pPCM)
{

	while(rxhook_rd_idx[sid]==rxhook_wt_idx[sid])
	{
		//printk("GD rd=%d\n",rxhook_rd_idx[sid]);
		interruptible_sleep_on(&(ppcm_config->pcm_qh));
	}
	
	copy_to_user((char*)pPCM, (char*)rxhook_fifo[sid][rxhook_rd_idx[sid]], PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE);
	
	local_irq_disable();
	rxhook_rd_idx[sid] = (rxhook_rd_idx[sid]+1) % MAX_HOOK_FIFO;
	local_irq_enable();
	return 0;
}		

void pcm_tx_getdata_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,ch; 
	int i, sid;
	short* pTx16Data;
	char* pTx8Data;
	char* p8Data = NULL;
	unsigned int flags;
	
	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		txch = ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			int tx_index;
			int nTry = 3;

			do{
				nTry--;
				local_irq_disable();
	
				tx_index = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
				if(tx_index==ptrpcm_config->txfifo_rd_idx[txch])
				{
					/* tx fifo full */
					printk("TTFF(%d)[%d] ",i ,txch);
					pTx8Data = NULL;
				}
				else
				{	 
					pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (tx_index*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				}
	
				pTx16Data = (short*)pTx8Data;
	
				if(pTx16Data==NULL)
				{
					local_irq_enable();
				}
				else
					break;
			}while(nTry > 0);
			
			if(pTx16Data)
				ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;

			local_irq_enable();
			
			if(pTx16Data)
				PCM_TX_Hook(txch, pTx16Data);
		}
	}			
}


void pcm_rx_putdata_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch; 
	int i;
	short* pTx16Data;
	short* pRx16Data;
	char* pTx8Data;
	char* pRx8Data;
	char* p8Data = NULL;
	unsigned int flags;
	
	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		rxch = ch;
			
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			int nTry = 3;
			local_irq_disable();
			do{
				nTry--;
				if(ptrpcm_config->rxfifo_rd_idx[rxch]==ptrpcm_config->rxfifo_wt_idx[rxch])
				{
					/* rx fifo empty */
					printk("TRFE(%d)[%d] ",i ,rxch);
					pRx8Data = NULL;
				}
				else
				{		
					pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + 
							(ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				}	
				pRx16Data = (short*)pRx8Data;
	
				if(pRx16Data==NULL)
				{
					local_irq_enable();
				}
				else
					break;
			}while(nTry > 0);
			
			if(pRx16Data)
				ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
				
			local_irq_enable();
			
			if(pRx16Data)
				PCM_RX_Hook(rxch, pRx16Data);
			
		}
	}			
}	
	
void pcm_tx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch; 
	int i;
	short* pTx16Data;
	short* pRx16Data;
	char* pTx8Data;
	char* pRx8Data;
	char* p8Data = NULL;
	unsigned int flags;
	
	/* handle rx->tx fifo buffer */
	spin_lock_irqsave(&ptrpcm_config->txlock, flags);

	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		rxch = ch;
		//txch = (CONFIG_PCM_CH-1)-ch;
		txch = ch;
			
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
 			int tx_index;

			tx_index = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			if(tx_index==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				printk("TTFF(%d)[%d] ",i ,txch);
				pTx8Data = NULL;
			}
			else
			{	 
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (tx_index*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}
            pTx16Data = (short*)pTx8Data;
#if !defined(PCM_INLOOP)
			if(ptrpcm_config->rxfifo_rd_idx[rxch]==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				printk("TRFE(%d)[%d] ",i ,rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}	
			pRx16Data = (short*)pRx8Data;
#endif

#if !defined(PCM_INLOOP)
			if((pRx16Data==NULL)||(pTx16Data==NULL))
#else
			if(pTx16Data==NULL)
#endif				
			{
				continue;
			}
			
			ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
#if !defined(PCM_INLOOP)			
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
#endif
			
#ifdef PCM_RECORD 
			if(ptrpcm_config->iRecordCH==rxch)
			{
				if(ptrpcm_config->bStartRecord)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_rd_idx)
						{
							ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE;
						}
						else
						{
							printk("DR\n");
						}		
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)p8Data, (void*)pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*sizeof(short);
					p8Data+=PCM_8KHZ_SAMPLES*sizeof(short);
				}
				
			}
#endif			
			if(codec_obj[rxch].type)
			{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				memcpy((short*)(codec_obj[rxch].pPCMBuf16), pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
				voice_encode_frame(&codec_obj[rxch]);
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);		
				memcpy(codec_obj[txch].pBitBuf, codec_obj[rxch].pBitBuf, codec_obj[rxch].BitBufByteLen);
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				voice_decode_frame(&codec_obj[txch]);
				memcpy(pTx16Data, (short*)(codec_obj[txch].pPCMBuf16), PCM_8KHZ_SAMPLES*sizeof(short));
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			}
#ifdef PCM_PLAYBACK 
			if(ptrpcm_config->iPlaybackCH==txch)
			{
				if(ptrpcm_config->bStartPlayback)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_wt_idx)
						{
							ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)pTx8Data, (void*)p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					continue;
				}
				
			}
#endif		
			if(codec_obj[rxch].type==0)
			{
#if defined(PCM_INLOOP)
					unsigned short txpat;
					unsigned char rxpat;
					unsigned short rxpat16;
					unsigned char temp;
					int j;
#if defined(PCM_LINEAR)
					txpat = 0x5948-(u16)txch;									
					rxpat16 = txpat;
#endif				
#if defined(PCM_U2L2U)
					txpat = 0x7A5B-(u16)txch;
					rxpat = linear2ulaw(txpat);
#endif
#if defined(PCM_A2L2A)		
					txpat = 0x6C2D-(u16)txch;														
					rxpat = linear2alaw(txpat);	
#endif						
#if defined(PCM_L2U2L)
					
					txpat = 0x1234-(u16)txch;
#endif
#if defined(PCM_L2A2L)
					
					txpat = 0x5678-(u16)txch;	
#endif					
#if defined(PCM_ULAW)
					txpat = 0x5A5A;	
					rxpat = linear2ulaw(txpat);
#endif	
#if defined(PCM_ALAW)			
					txpat = 0x5A5A;	
					rxpat = linear2alaw(txpat);	
#endif	
#if defined(PCM_U2L2U)||defined(PCM_A2L2A)||defined(PCM_ULAW)||defined(PCM_ALAW)			
				memset((void*)pTx8Data, (unsigned char)rxpat, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#else
				for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )		
					(unsigned short)(*(pTx16Data+j)) = (unsigned short)txpat;
 			
#endif
#else	/* else not PCM_INLOOP */	
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				
#endif 			
			}
		}
	
	}
	spin_unlock_irqrestore(&ptrpcm_config->txlock, flags);
	if((ptrpcm_config->bStartRecord)||(ptrpcm_config->bStartPlayback))
	{
		ptrpcm_config->mmappos = 0;
		wake_up_interruptible(&(ptrpcm_config->pcm_qh));
	}
	return;
}

void pcm_rx_task(unsigned long pData)
{
	pcm_config_type* ptrpcm_config = ppcm_config;
	int txch,rxch,ch;
	int i;
	short* pTx16Data;
	short* pRx16Data;
	char* pTx8Data;
	char* pRx8Data;
	char* p8Data = NULL;
	unsigned int flags;
	int bPassed = CONFIG_PCM_CH*MAX_PCM_PROC_UNIT;

	spin_lock_irqsave(&ptrpcm_config->rxlock, flags);

	/* handle rx->tx fifo buffer */
	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		rxch = ch;
		//txch = (CONFIG_PCM_CH-1)-ch;
		txch = ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{	
			int tx_index;
#if !defined(PCM_INLOOP)
			tx_index = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
				
			if(tx_index==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				printk("RTFF(%d)[%d] ", i, txch);
				pTx8Data = NULL;
			}
			else
			{	 
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (tx_index*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}
			pTx16Data = (short*)pTx8Data;
#endif
			if(ptrpcm_config->rxfifo_rd_idx[rxch]==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				printk("RRFE(%d)[%d] ",i ,rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}	
			pRx16Data = (short*)pRx8Data;
#if defined(PCM_INLOOP)
            if(pRx16Data==NULL)
#else
			if((pRx16Data==NULL)||(pTx16Data==NULL))
#endif
			{
				bPassed--;
				continue;
			}
#if !defined(PCM_INLOOP)	
			ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
#endif
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			
#ifdef PCM_RECORD 
			if(ptrpcm_config->iRecordCH==rxch)
			{
				if(ptrpcm_config->bStartRecord)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_rd_idx)
						{
							ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)p8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
				}
				
			}
#endif
			if(codec_obj[rxch].type)
			{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				memcpy((short*)(codec_obj[rxch].pPCMBuf16), pRx16Data, PCM_8KHZ_SAMPLES*sizeof(short));
				voice_encode_frame(&codec_obj[rxch]);
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);		
				memcpy(codec_obj[txch].pBitBuf, codec_obj[rxch].pBitBuf, codec_obj[rxch].BitBufByteLen);
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				voice_decode_frame(&codec_obj[txch]);
				memcpy(pTx16Data, (short*)(codec_obj[txch].pPCMBuf16), PCM_8KHZ_SAMPLES*sizeof(short));
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			}
#ifdef PCM_PLAYBACK 
			if(ptrpcm_config->iPlaybackCH==txch)
			{
				if(ptrpcm_config->bStartPlayback)
				{
					if(ptrpcm_config->mmappos==0)
					{
						if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_wt_idx)
						{
							ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE;
						}
						else
						{
							printk("DR\n");
						}
						p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;
					}
					memcpy((void*)pTx8Data, (void*)p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
					ptrpcm_config->mmappos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					p8Data+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
					continue;
				}
				
			}
#endif
			if(codec_obj[rxch].type==0)
			{
#if defined(PCM_INLOOP)
					unsigned short txpat;
					unsigned char rxpat;
					unsigned short rxpat16;
					unsigned char temp;	
					int j;
#if defined(PCM_LINEAR)
					txpat = 0x5948-(u16)rxch;									
					rxpat16 = txpat;
					if(ppcm_config->tx_isr_cnt > (20*4))
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG("[%d]PCM_INLOOP(%d) PATTERN ERROR TX[0x%04X] RX[0x%04X] \n", ch,j, 
										(unsigned short)rxpat16, (unsigned short)(*(pRx16Data+j)));
								bPassed--;
								break;
							}		
						}
					}
#endif
#if defined(PCM_U2L2U)
					txpat = 0x7A5B-(u16)rxch;									
					rxpat = linear2ulaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X]\n", (unsigned char)rxpat, 
										(unsigned char)(*(pRx8Data+j)));
								bPassed--;
								break;
							}		
						}
					}
#endif
#if defined(PCM_A2L2A)		
					txpat = 0x6C2D-(u16)rxch;								
					rxpat = linear2alaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X]\n", 
										(unsigned char)rxpat, (unsigned char)(*(pRx8Data+j)));
								bPassed--;
								break;
							}		
						}
					}
#endif						
#if defined(PCM_L2U2L)
					
					txpat = 0x1234-(u16)rxch;

					temp = linear2ulaw(txpat);
					rxpat16=ulaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > (20*4))
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG(".LBK(%d)[%d %d] [0x%04X] [0x%02X] [0x%04X]\n",ch,i,j, 
										(unsigned short)(*(pRx16Data+j)),(unsigned char)temp,
										(unsigned short)rxpat16);
								bPassed--;
								break;
							}		
						}
					}
#endif
#if defined(PCM_L2A2L)

					txpat = 0x5678-(u16)rxch;							
					temp = linear2alaw(txpat);
					rxpat16=alaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > 20)
					{	
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG(".PCM_INLOOP PATTERN ERROR [0x%04X] [0x%02X] [0x%04X]\n", 
										(unsigned short)(*(pRx16Data+j)),(unsigned char)temp,
										(unsigned short)rxpat16);
								bPassed--;
								break;
							}		
						}
					}
#endif
#if defined(PCM_ULAW)
					txpat = 0x5A5A;	
					rxpat = linear2ulaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X]\n", 
										(unsigned char)rxpat, (unsigned char)(*(pRx8Data+j)));
								bPassed--;
								break;
							}		
						}
					}
#endif	
#if defined(PCM_ALAW)			
					txpat = 0x5A5A;	
					rxpat = linear2alaw(txpat);	
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X]\n", 
										(unsigned char)rxpat, (unsigned char)(*(pRx8Data+j)));
								bPassed--;
								break;
							}		
						}
					}
#endif					
#else	
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#endif			
			}
		}
	}
	spin_unlock_irqrestore(&ptrpcm_config->rxlock, flags);
#if defined(PCM_INLOOP)	
	if(ppcm_config->rx_isr_cnt%99==98)
		MSG("RLBK(p=%d)(i=%d)\n",bPassed,ppcm_config->rx_isr_cnt);
#endif
	if((ptrpcm_config->bStartRecord)||(ptrpcm_config->bStartPlayback))
	{
		ptrpcm_config->mmappos = 0;
		wake_up_interruptible(&(ptrpcm_config->pcm_qh));
	}
	return;
}

void pcm_unmask_isr(u32 dma_ch)
{
	char* p8Data;

	MSG("umisr c=%d\n",dma_ch);

	switch(dma_ch)
	{
	case GDMA_PCM0_RX0:
		p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[0]);
		GdmaPcmRx((u32)PCM_CH0_FIFO, (u32)p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM0_RX1);
		break;
	case GDMA_PCM0_RX1:	
		p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[0]);
		GdmaPcmRx((u32)PCM_CH0_FIFO, (u32)p8Data, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
		GdmaUnMaskChannel(GDMA_PCM0_RX0);
		break;
	case GDMA_PCM0_TX0:	
		p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[0]);
		GdmaPcmTx((u32)p8Data, (u32)PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM0_TX1);
		break;
	case GDMA_PCM0_TX1:
		p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[0]);
		GdmaPcmTx((u32)p8Data, (u32)PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM0_TX0);
		break;
	case GDMA_PCM1_RX0:	
		p8Data = (char*)(ppcm_config->RxPage0Buf16Ptr[1]);
		GdmaPcmRx((u32)PCM_CH1_FIFO, (u32)p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM1_RX1);
		break;
	case GDMA_PCM1_RX1:		
		p8Data = (char*)(ppcm_config->RxPage1Buf16Ptr[1]);					
		GdmaPcmRx((u32)PCM_CH1_FIFO, (u32)p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM1_RX0);
		break;	
	case GDMA_PCM1_TX0:	
		p8Data = (char*)(ppcm_config->TxPage0Buf16Ptr[1]);
		GdmaPcmTx((u32)p8Data, (u32)PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM1_TX1);
		break;
	case GDMA_PCM1_TX1:	
		p8Data = (char*)(ppcm_config->TxPage1Buf16Ptr[1]);
		GdmaPcmTx((u32)p8Data, (u32)PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		GdmaUnMaskChannel(GDMA_PCM1_TX0);
		break;
	}	
	return; 		
}

/**
 * @brief PCM interrupt handler 
 *
 * When PCM interrupt happened , call related handler 
 * to do the remain job.
 *
 */
irqreturn_t pcm_irq_isr(int irq, void *irqaction)
{
	u32 pcm_status;
	
	
	pcm_status=pcm_inw(PCM_INT_STATUS);
	MSG("SR=%08X\n",pcm_status);

	/* check CH0 status */
	if(pcm_status&REGBIT(1, CH0T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txdmafault++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txovrun++;
#endif
	}
	if(pcm_status&REGBIT(1, CH0T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH0T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH0R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch0rxthres++;
#endif		
	}

	/* check CH1 status */
	if(pcm_status&REGBIT(1, CH1T_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txdmafault++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1T_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1txthres++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_DMA_FAULT))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxdmafault++;
#endif	
	}
	if(pcm_status&REGBIT(1, CH1R_OVRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxovrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_UNRUN))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxunrun++;
#endif		
	}
	if(pcm_status&REGBIT(1, CH1R_THRES))
	{
#ifdef PCM_STATISTIC
		ppcm_status->ch1rxthres++;
#endif		
	}
	pcm_outw(PCM_INT_STATUS, 0xFFFF);
	
	return IRQ_HANDLED;

}

/* Qwert : Add for slic access */

static unsigned char pcm_changeCID (unsigned char newCid) // assigns new value of Channel ID  
	  // Because Channel ID is sent in reverse order the bits are rotated here
{
	unsigned char Cid = 0;//LOADS NEW Cid FROM LABVIEW or Assigns new value in bit reversed order to Cid
	Cid|= (newCid&0x1)<<3; // Alternative code  Cid=0; for (i =0; i<4 ; i++) { Cid |= newCid & 1 ; newCid>>1;}
	Cid|= (newCid&0x2)<<1;
	Cid|= (newCid&0x4)>>1;
	Cid|= (newCid&0x8)>>3;
	
	return Cid;
} 

/* Qwert : Add for slic access */
static void slic_ramwait (unsigned char ch)
{
	unsigned char regVal, cid;
	

	cid = pcm_changeCID(ch);
	if(slic_type==3220)
	{
		regVal = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid|0x60), (unsigned char)4);
		while (regVal&0x01)
		{
			regVal = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid|0x60), (unsigned char)4);
		}//wait for indirect registers
	}
	else
	{
		regVal = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)cid, (unsigned char)31);
		while (regVal&0x01)
		{
			regVal = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)cid, (unsigned char)31);
		}//wait for indirect registers
	}
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
int pcm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int i, Ret;
	char* p8Data;
	unsigned long flags;
	unsigned char ch, addr, cid;
	unsigned long data;
	
	pcm_config_type* ptrpcm_config = ppcm_config;
	pcm_record_type* ptrpcm_record;
	pcm_playback_type* ptrpcm_playback;
	
	switch(cmd)
	{
		case PCM_SET_RECORD:
			MSG("iocmd=PCM_SET_RECORD\n");
			ptrpcm_config->mmapbuf = kmalloc(PCM_PAGE_SIZE*MAX_PCMMMAP_PAGE, GFP_KERNEL);
			if(ptrpcm_config->mmapbuf==NULL)
				return -1;
			ptrpcm_config->mmap_rd_idx = ptrpcm_config->mmap_wt_idx = 0;		
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->bStartRecord = 1;
			ptrpcm_config->iRecordCH = arg;
			init_waitqueue_head(&(ptrpcm_config->pcm_qh));
			break;
		case PCM_SET_UNRECORD:
			MSG("iocmd=PCM_SET_UNRECORD\n");
			ptrpcm_config->bStartRecord = 0;
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->iRecordCH = -1;
			kfree(ptrpcm_config->mmapbuf);
			break;
		case PCM_SET_PLAYBACK:
			MSG("iocmd=PCM_SET_RECORD\n");
			ptrpcm_config->mmapbuf = kmalloc(PCM_PAGE_SIZE*MAX_PCMMMAP_PAGE, GFP_KERNEL);
			if(ptrpcm_config->mmapbuf==NULL)
				return -1;
			ptrpcm_config->mmap_rd_idx = ptrpcm_config->mmap_wt_idx = 0;	
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->bStartPlayback = 1;
			ptrpcm_config->iPlaybackCH = arg;
			init_waitqueue_head(&(ptrpcm_config->pcm_qh));
			break;
		case PCM_SET_UNPLAYBACK:
			MSG("iocmd=PCM_SET_UNRECORD\n");
			ptrpcm_config->bStartPlayback = 0;
			ptrpcm_config->mmappos = 0;
			ptrpcm_config->iPlaybackCH = -1;
			kfree(ptrpcm_config->mmapbuf);
			break;			
		case PCM_READ_PCM:
			ptrpcm_record = (pcm_record_type*)arg;
			if(ptrpcm_config->nch_active <= 0)
				return -1;
			do{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				if((ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_wt_idx)
				{
					ptrpcm_config->mmap_rd_idx = (ptrpcm_config->mmap_rd_idx+1)%MAX_PCMMMAP_PAGE;
					p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_rd_idx*PCM_PAGE_SIZE;	
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					break;
				}
				else
				{
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					interruptible_sleep_on(&(ptrpcm_config->pcm_qh));
				}
			}while(1);
			
			copy_to_user(ptrpcm_record->pcmbuf, p8Data, PCM_PAGE_SIZE);
			ptrpcm_record->size = PCM_PAGE_SIZE;
			ptrpcm_config->mmappos = 0;
			break;
		case PCM_WRITE_PCM:
			ptrpcm_playback = arg;
			if(ptrpcm_config->nch_active <= 0)
				return -1;
			do{
				spin_lock_irqsave(&ptrpcm_config->lock, flags);	
				if((ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE!=ptrpcm_config->mmap_rd_idx)
				{
					ptrpcm_config->mmap_wt_idx = (ptrpcm_config->mmap_wt_idx+1)%MAX_PCMMMAP_PAGE;
					p8Data = ptrpcm_config->mmapbuf+ptrpcm_config->mmap_wt_idx*PCM_PAGE_SIZE;	
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					break;
				}
				else
				{
					spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
					interruptible_sleep_on(&(ptrpcm_config->pcm_qh));
				}
			}while(1);
			
			copy_from_user(p8Data, ptrpcm_playback->pcmbuf, PCM_PAGE_SIZE);
			ptrpcm_playback->size = PCM_PAGE_SIZE;
			ptrpcm_config->mmappos = 0;
			break;
		case PCM_PUTDATA:
			PCM_UserPutData_Hook(0, arg);
			break;
		case PCM_GETDATA:
			PCM_UserGetData_Hook(0, arg);	
			break;	
				
		case PCM_SET_CODEC_TYPE:
			{
				long param[2];
				
				copy_from_user(param, (long*)arg, sizeof(long)*2);
				MSG("iocmd=PCM_SET_CODEC_TYPE CH=%d, Type=%d\n",(u32)param[0],(u32)param[1]);
				switch(param[1])
				{
					case G711ULAW_CODEC:
					case G711ALAW_CODEC:
					case G729AB_CODEC: 				
					case G723A_CODEC: 		
						ptrpcm_config->codec_type[param[0]] = param[1];
						break;
					default:
						ptrpcm_config->codec_type[param[0]] = 0;	
						break;
				}
				break;
			}	
		case PCM_START:
			MSG("iocmd=PCM_START\n");
			
			for( i = 0 ; i < ptrpcm_config->codec_ch_num ; i++ )
			{		
				codec_obj[i].type = ptrpcm_config->codec_type[i];
				codec_obj[i].ch = i ;
				voice_init_codec(&codec_obj[i]);
				MSG("CH[%d]=%d\n",i,codec_obj[i].type);
			}
			
#ifdef PCM_TASKLET
			if(arg==1)
			{
				PCM_HooK_Init();
				tasklet_init(&pcm_rx_tasklet, pcm_rx_putdata_task, (u32)ppcm_config);
				tasklet_init(&pcm_tx_tasklet, pcm_tx_getdata_task, (u32)ppcm_config);
			}
			else
			{
				tasklet_init(&pcm_rx_tasklet, pcm_rx_task, (u32)ppcm_config);
				tasklet_init(&pcm_tx_tasklet, pcm_tx_task, (u32)ppcm_config);
			}	
			MSG("pcm tasklet initialization\n");
#endif			

			for( i = 0; i < ptrpcm_config->pcm_ch_num; i++) {
				p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[i]);
				GdmaPcmRx((u32)PCM_CH_FIFO(i), (u32)p8Data, i, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
				p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[i]);
				GdmaPcmRx((u32)PCM_CH_FIFO(i), (u32)p8Data, i, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
				GdmaUnMaskChannel(GDMA_PCM_RX(i,0));
				p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[i]);
				GdmaPcmTx((u32)p8Data, (u32)PCM_CH_FIFO(i), i, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
				p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[i]);
				GdmaPcmTx((u32)p8Data, (u32)PCM_CH_FIFO(i), i, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
				GdmaUnMaskChannel(GDMA_PCM_TX(i,0));
			
			}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)			
			Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, IRQF_DISABLED, "Ralink_PCM", NULL);
#else
			Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, SA_INTERRUPT, "Ralink_PCM", NULL);
#endif
			if(Ret){
				MSG("PCM: IRQ %d is not free.\n", SURFBOARDINT_PCM);
				return PCM_REQUEST_IRQ_FAILED;
			}
			for( i = 0; i < ptrpcm_config->pcm_ch_num; i++)
			{
				ptrpcm_config->txfifo_rd_idx[i] = 0;
				ptrpcm_config->txfifo_wt_idx[i] = 0;
				ptrpcm_config->rxfifo_rd_idx[i] = 0;
				ptrpcm_config->rxfifo_wt_idx[i] = 0;
				ptrpcm_config->bsfifo_rd_idx[i] = 0;
				ptrpcm_config->bsfifo_wt_idx[i] = 0;
			}
			ptrpcm_config->rx_isr_cnt = 0;
			ptrpcm_config->tx_isr_cnt = 0;
			
			ptrpcm_config->bStartRecord = 0;
			pcm_dump_reg();			
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_enable(i, ptrpcm_config);	
			/* enable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data |=0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
			break;
		case PCM_STOP:
			MSG("iocmd=PCM_STOP\n");
			spin_lock_irqsave(&ptrpcm_config->lock, flags);	
			
			/* disable system interrupt for PCM */
			data = pcm_inw(RALINK_REG_INTENA);
			data &=~0x010;
    		pcm_outw(RALINK_REG_INTENA, data);
		
			synchronize_irq(SURFBOARDINT_PCM);
			free_irq(SURFBOARDINT_PCM, NULL);
			
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ptrpcm_config);
#ifdef PCM_TASKLET
			if(arg==1)
			{
				PCM_HooK_Release();
			}
			tasklet_kill(&pcm_rx_tasklet);
			tasklet_kill(&pcm_tx_tasklet);
			MSG("pcm tasklet deinitialization\n");
#endif					
			
			for( i = 0 ; i < ptrpcm_config->codec_ch_num ; i++ )
			{
				codec_obj[i].type = 0;
				codec_obj[i].ch = 0 ;
				voice_release_codec(&codec_obj[i]);
				ptrpcm_config->codec_type[i] = 0;
			}
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			break;
			/* Qwert : Add for slic access */	
		case PCM_SLIC_DRREAD:
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			cid = pcm_changeCID(ch);
			if(slic_type==3220)
				data = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid|0x60), (unsigned char)addr);
			else
				data = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)cid, (unsigned char)addr);
			printk("slic(%u) R DR[%03u] = 0x%02X\n",ch, addr, (u32)data);
			break;
		case PCM_SLIC_IRREAD:
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			cid = pcm_changeCID(ch);
			if(slic_type==3220)
			{
				slic_ramwait(ch);
				spi_si3220_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid|0x20), 103, (unsigned char)addr);
				slic_ramwait(ch);
				data = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), 101) | spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 102) << 8;
			}
			else
			{
				slic_ramwait(ch);
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), 30, (unsigned char)addr);
				slic_ramwait(ch);
				data = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)cid, 28) | (spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, 29) << 8);
			}
			printk("slic(%u) R IR[%03u] = 0x%04X\n",ch, addr, (u32)data);
			break;
		case PCM_SLIC_DRWRITE:
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			__get_user(data, (int __user *)(long*)arg+2);
			cid = pcm_changeCID(ch);
			if(slic_type==3220)
			{
				spi_si3220_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid|0x20), (unsigned char)addr, data);
			}	
			else
			{	
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), (unsigned char)addr, data);
			}
			printk("slic(%u) W DR[%03d] = 0x%02X\n",ch, (u32)addr, (u32)data);
			break;
		case PCM_SLIC_IRWRITE:			
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			__get_user(data, (int __user *)(long*)arg+2);
			cid = pcm_changeCID(ch);
			if(slic_type==3220)
			{
				slic_ramwait(ch);
				spi_si3220_write16(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid | 0x00), (unsigned char)(addr), data);
			}
			else
			{
				slic_ramwait(ch);
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), 28, (unsigned char)(data & 0xFF));
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), 29, (unsigned char)((data & 0xFF00) >> 8));
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(cid), 30, (unsigned char)addr);
			}
			printk("slic(%u) W IR[%03d] = 0x%04X\n",ch, (u32)addr, (u32)data);
			break;
		case PCM_EXT_LOOPBACK_ON:
			MSG("external loopback on\n");
			data = pcm_inw(PCM_CH0_CFG);
			data |= 0x40000000;
			pcm_outw(PCM_CH0_CFG, data);
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_enable(i, ptrpcm_config);
			break;
		case PCM_EXT_LOOPBACK_OFF:
			MSG("external loopback off\n");
			for ( i = 0 ; i < ptrpcm_config->pcm_ch_num ; i ++ )
				pcm_disable(i, ptrpcm_config);
			data = pcm_inw(PCM_CH0_CFG);
			data &= ~0x40000000;
			pcm_outw(PCM_CH0_CFG, data);
			break;		
		default:
			break;
	}
	
	return 0;
}
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_TY6855) \
	|| defined(CONFIG_RALINK_RT6352)	
/* spi_cs pin is not always shared with gpio on all Ralink SoC */
void pcm_reset_slic_gpio(void)
{
	unsigned long data;
	unsigned long gpio_dir_reg,gpio_data_reg,gpio_bit;
	
	MSG("Reset by GPIO\n");
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* Set SPI_CS1_MODE and SPI to GPIO mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFF9FFFFF;
	data |= 0x00600002;
	pcm_outw(RALINK_REG_GPIOMODE, data);

	/* Set SPI_CS1_MODE to GPIO mode */
	data = pcm_inw(RALINK_SYSCTL_BASE+0x14); 
	data &= 0xFFFFFFF3;
	data |= 0x00000008;
	pcm_outw(RALINK_SYSCTL_BASE+0x14, data);	
	
#else	
	/* Set SPI to GPIO mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data |= 0x02;
	pcm_outw(RALINK_REG_GPIOMODE, data);
#endif

	/* CS set to low */	
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)	
#if defined(CONFIG_RALINK_RT6352)
	data = pcm_inw(0xB0000600+0x54);
	data |= (1<<(37-24));                      /* set GPIO#37 as output pin */
	pcm_outw(0xB0000600+0x54, data);
	data = pcm_inw(0xB0000600+0x50);
	data &= (~1<<(37-24));                     /* set GPIO#37 as low */
	pcm_outw(0xB0000600+0x50, data);
#else	
	data = pcm_inw(0xB0000600+0x74);
	data |= 0x020;						/* set GPIO#45 as output pin */
	pcm_outw(0xB0000600+0x74, data);
	data = pcm_inw(0xB0000600+0x70);
	data &= ~0x020;						/* set GPIO#45 as low */
	pcm_outw(0xB0000600+0x70, data);
#endif
#else
	
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(3);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
#endif	
	
	mdelay(200); 
	
	/* RESET set to low */
	gpio_dir_reg = RALINK_REG_PIODIR;
	gpio_data_reg = RALINK_REG_PIODATA;
	gpio_bit = CONFIG_RALINK_PCMRST_GPIO;	
	
	if((CONFIG_RALINK_PCMRST_GPIO >= 24) && (CONFIG_RALINK_PCMRST_GPIO <= 39))
	{
		gpio_dir_reg = RALINK_REG_PIO3924DIR;
		gpio_data_reg = RALINK_REG_PIO3924DATA;
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 24;
	}
	else if((CONFIG_RALINK_PCMRST_GPIO >= 40) && (CONFIG_RALINK_PCMRST_GPIO <= 51))
	{
#if defined (RALINK_GPIO_HAS_5124)
		gpio_dir_reg = RALINK_REG_PIO5140DIR;
		gpio_data_reg = RALINK_REG_PIO5140DATA;
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;	
#endif
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO7140DIR;
		gpio_data_reg = RALINK_REG_PIO7140DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;
#endif	
	}	
	else if((CONFIG_RALINK_PCMRST_GPIO >= 52) && (CONFIG_RALINK_PCMRST_GPIO <= 71))
	{
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO7140DIR;
		gpio_data_reg = RALINK_REG_PIO7140DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;
#endif		
	}
	else if((CONFIG_RALINK_PCMRST_GPIO >= 72) && (CONFIG_RALINK_PCMRST_GPIO <= 95))	
	{
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO9572DIR;
		gpio_data_reg = RALINK_REG_PIO9572DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 72;
#endif	
	}

	MSG("DIR=%X DR=%X BIT=%d\n",(u32)gpio_dir_reg,(u32)gpio_data_reg,(u32)gpio_bit);
	data = pcm_inw(gpio_dir_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_dir_reg, data);
	data = pcm_inw(gpio_data_reg);
	data &= ~(1<<gpio_bit);
	pcm_outw(gpio_data_reg, data);
	
	mdelay(200); 
	
	/* CS set to high */
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
#if defined(CONFIG_RALINK_RT6352)
	data = pcm_inw(0xB0000600+0x54);
	data |= (1<<(37-24));              /* set GPIO#37 as output pin */
	pcm_outw(0xB0000600+0x54, data);
	data = pcm_inw(0xB0000600+0x50);
	data |= (1<<(37-24));               /* set GPIO#37 as high */
	pcm_outw(0xB0000600+0x50, data);
#else	
	data = pcm_inw(0xB0000600+0x74);
	data |= 0x020;						/* set GPIO#45 as output pin */
	pcm_outw(0xB0000600+0x74, data);
	data = pcm_inw(0xB0000600+0x70);
	data |= 0x020;						/* set GPIO#45 as high */
	pcm_outw(0xB0000600+0x70, data);	
#endif
#else	
	data = pcm_inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(3);
	pcm_outw(RALINK_REG_PIODIR, data);
	data = pcm_inw(RALINK_REG_PIODATA);
	data |= RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
#endif	

	mdelay(200);

	/* RESET set to high */
	data = pcm_inw(gpio_dir_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_dir_reg, data);
	data = pcm_inw(gpio_data_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_data_reg, data);
	
	mdelay(500);
	
	/* CS set to low */
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	//data = pcm_inw(0xB0000600+0x70);
	//data &= ~0x020;					/* set GPIO#45 as low */
	//pcm_outw(0xB0000600+0x70, data);	
#else
	data = pcm_inw(RALINK_REG_PIODATA);
	data &= ~RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
#endif
	mdelay(200);
	
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)	
	/* Set SPI_CS1_MODE to SPI_CS1 mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFF9FFFFD;
	pcm_outw(RALINK_REG_GPIOMODE, data);

	data = pcm_inw(RALINK_SYSCTL_BASE+0x14); 
	data &= 0xFFFFFFF3;
	pcm_outw(RALINK_SYSCTL_BASE+0x14, data);
	
	data = pcm_inw(0xB0000B00+0xF0);
	data |= 0x80000002;
	pcm_outw(0xB0000B00+0xF0, data);
	
	data = pcm_inw(0xB0000B00+0x50);
	data = 0x176;
	pcm_outw(0xB0000B00+0x50, data);
	
	data = pcm_inw(0xB0000B00+0x54);
	data = 0x03;
	pcm_outw(0xB0000B00+0x54, data);

#else	
	/* Set GPIO to SPI mode */
	data = pcm_inw(RALINK_REG_GPIOMODE); 
	data &= 0xFFFFFFFD;
	pcm_outw(RALINK_REG_GPIOMODE, data);
#endif
	
	mdelay(200);
	return;
}
#else
void pcm_reset_slic_cs (void)
{
 	unsigned long data;
	unsigned long gpio_dir_reg,gpio_data_reg,gpio_bit;
	
	MSG("Reset by SPI_CS\n");
	
	/* Set to SP1_CS1_MODE mode and SPI_GPIO_MODE to spi mode */
	data = pcm_inw(RALINK_REG_GPIOMODE);
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) \
	|| defined(CONFIG_RALINK_RT6352)
	data &= 0xFF9FFFFD;
#else
	data &= 0xFFFFFFFD;	
#endif	
	pcm_outw(RALINK_REG_GPIOMODE, data);

	RT2880_REG(RT2880_SPI_ARB_REG) = 0x80000002|(1<<16);//0x80000003;
	
	/* CS set to low */
	if(CONFIG_RALINK_PCM_SPICH > 0)
		RT2880_REG(RT2880_SPI1_CTL_REG) = SPICTL_STARTRD ;
	else
		RT2880_REG(RT2880_SPI0_CTL_REG) = SPICTL_STARTRD ;

	mdelay(200);
		
	/* RESET set to low */
	gpio_dir_reg = RALINK_REG_PIODIR;
	gpio_data_reg = RALINK_REG_PIODATA;
	gpio_bit = CONFIG_RALINK_PCMRST_GPIO;	
	
	if((CONFIG_RALINK_PCMRST_GPIO >= 24) && (CONFIG_RALINK_PCMRST_GPIO <= 39))
	{
		gpio_dir_reg = RALINK_REG_PIO3924DIR;
		gpio_data_reg = RALINK_REG_PIO3924DATA;
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 24;
	}
	else if((CONFIG_RALINK_PCMRST_GPIO >= 40) && (CONFIG_RALINK_PCMRST_GPIO <= 51))
	{
#if defined (RALINK_GPIO_HAS_5124)
		gpio_dir_reg = RALINK_REG_PIO5140DIR;
		gpio_data_reg = RALINK_REG_PIO5140DATA;
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;	
#endif
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO7140DIR;
		gpio_data_reg = RALINK_REG_PIO7140DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;
#endif	
	}	
	else if((CONFIG_RALINK_PCMRST_GPIO >= 52) && (CONFIG_RALINK_PCMRST_GPIO <= 71))
	{
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO7140DIR;
		gpio_data_reg = RALINK_REG_PIO7140DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 40;
#endif		
	}
	else if((CONFIG_RALINK_PCMRST_GPIO >= 72) && (CONFIG_RALINK_PCMRST_GPIO <= 95))	
	{
#if defined (RALINK_GPIO_HAS_9524)
		gpio_dir_reg = RALINK_REG_PIO9572DIR;
		gpio_data_reg = RALINK_REG_PIO9572DATA;	
		gpio_bit = 	CONFIG_RALINK_PCMRST_GPIO - 72;
#endif	
	}

	MSG("DIR=%X DR=%X BIT=%u\n",(u32)gpio_dir_reg,(u32)gpio_data_reg,(u32)gpio_bit);
	data = pcm_inw(gpio_dir_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_dir_reg, data);
	data = pcm_inw(gpio_data_reg);
	data &= ~(1<<gpio_bit);
	pcm_outw(gpio_data_reg, data);
	
	mdelay(500);
	
	/* CS set to high */
	if(CONFIG_RALINK_PCM_SPICH > 0)
		RT2880_REG(RT2880_SPI1_CTL_REG) |= SPICTL_SPIENA_ASSERT;
	else
		RT2880_REG(RT2880_SPI0_CTL_REG) |= SPICTL_SPIENA_ASSERT;
		
	/* RESET set to high */
	data = pcm_inw(gpio_dir_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_dir_reg, data);
	data = pcm_inw(gpio_data_reg);
	data |= (1<<gpio_bit);
	pcm_outw(gpio_data_reg, data);

	mdelay(200);
	
	/* CS set to low */
	if(CONFIG_RALINK_PCM_SPICH > 0)
		RT2880_REG(RT2880_SPI1_CTL_REG) &= SPICTL_SPIENA_NEGATE;
	else
		RT2880_REG(RT2880_SPI0_CTL_REG) &= SPICTL_SPIENA_NEGATE;

	mdelay(200);

	return;
}
#endif

 
void pcm_reset_slic (void)
{
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_TY6855) \
	|| defined(CONFIG_RALINK_RT6352)	
	pcm_reset_slic_gpio();
#else	
	pcm_reset_slic_cs();
#endif	

	return;
}

void pcm_dump_reg (void)
{
	int i;
	MSG("[0x%08X]RALINK_REG_GPIOMODE=0x%08X\n", RALINK_REG_GPIOMODE, pcm_inw(RALINK_REG_GPIOMODE));
	MSG("[0x%08X]PCM_GLBCFG=0x%08X\n", PCM_GLBCFG, pcm_inw(PCM_GLBCFG));
	MSG("[0x%08X]PCM_PCMCFG=0x%08X\n", PCM_PCMCFG, pcm_inw(PCM_PCMCFG));
	MSG("[0x%08X]PCM_INT_STATUS=0x%08X\n", PCM_INT_STATUS, pcm_inw(PCM_INT_STATUS));
	MSG("[0x%08X]PCM_INT_EN=0x%08X\n", PCM_INT_EN, pcm_inw(PCM_INT_EN));
	MSG("[0x%08X]PCM_FF_STATUS=0x%08X\n", PCM_FF_STATUS, pcm_inw(PCM_FF_STATUS));
#if defined (CONFIG_RALINK_RT6352)
	for (i = 0; i < CONFIG_PCM_CH; i++) {
		MSG("[0x%08X]PCM_CH_CFG(%d)=0x%08X\n", PCM_CH_CFG(i), i,  pcm_inw(PCM_CH_CFG(i)));
		MSG("[0x%08X]PCM_CH_FIFO(%d)=0x%08X\n", PCM_CH_FIFO(i), i, pcm_inw(PCM_CH_FIFO(i)));
	}
#else	
	MSG("[0x%08X]PCM_CH0_CFG=0x%08X\n", PCM_CH0_CFG, pcm_inw(PCM_CH0_CFG));
	MSG("[0x%08X]PCM_CH1_CFG=0x%08X\n", PCM_CH1_CFG, pcm_inw(PCM_CH1_CFG));
	MSG("[0x%08X]PCM_CH0_FIFO=0x%08X\n", PCM_CH0_FIFO,pcm_inw(PCM_CH0_FIFO));
	MSG("[0x%08X]PCM_CH1_FIFO=0x%08X\n", PCM_CH1_FIFO,pcm_inw(PCM_CH1_FIFO));
#endif
#if defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) \
	|| defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6352)
	MSG("[0x%08X]PCM_FSYNC_CFG=0x%08X\n", PCM_FSYNC_CFG, pcm_inw(PCM_FSYNC_CFG));
	MSG("[0x%08X]PCM_CH_CFG2=0x%08X\n", PCM_CH_CFG2, pcm_inw(PCM_CH_CFG2));
	MSG("[0x%08X]PCM_DIVCOMP_CFG=0x%08X\n", PCM_DIVCOMP_CFG, pcm_inw(PCM_DIVCOMP_CFG));
	MSG("[0x%08X]PCM_DIVINT_CFG=0x%08X\n", PCM_DIVINT_CFG, pcm_inw(PCM_DIVINT_CFG));
#endif	
}	

module_init(pcm_init);
module_exit(pcm_exit);
module_param_named(slic, slic_type, int, S_IRUGO);
module_param_named(idiv, idiv, int, S_IRUGO);
module_param_named(cdiv, cdiv, int, S_IRUGO);
module_param_named(smode, smode, int, S_IRUGO);
MODULE_DESCRIPTION("Ralink SoC PCM Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(PCM_MOD_VERSION);
EXPORT_SYMBOL(pcm_reset_slic);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (pcmdrv_major, "i");
#else
module_param (pcmdrv_major, int, 0);
#endif
