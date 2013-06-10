#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/device.h>

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
int PCM_HooK_Init(void);
int PCM_HooK_Release(void);

void pcm_dma_tx_isr(u32 chid);
void pcm_dma_rx_isr(u32 chid);
void pcm_unmask_isr(u32 dma_ch);

void pcm_reset_slic (void);

void pcm_dump_reg (void);

static irqreturn_t pcm_irq_isr(int irq, void *irqaction);

struct file_operations pcmdrv_fops = {
    ioctl:      pcm_ioctl,
};

static int s_pcmdrv_major =  233;
static struct class *s_pcmdrv_class = NULL;
static struct device *s_pcmdrv_device = NULL;
static struct class *pcmmodule_class;

static dma_addr_t TxPage0, TxPage1;
static dma_addr_t RxPage0, RxPage1;

#ifdef PCM_TASKLET
struct tasklet_struct pcm_rx_tasklet;
struct tasklet_struct pcm_tx_tasklet;
#endif

unsigned int slic_type = 3220;

codec_data_type codec_obj[MAX_CODEC_CH];

int __init pcm_init(void)
{
#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(s_pcmdrv_major, PCMDRV_DEVNAME , &pcmdrv_fops)) {
		printk(KERN_WARNING " pcm: can't create device node - %s\n",PCMDRV_DEVNAME);
		return -EIO;
    }

    devfs_handle = devfs_register(NULL, PCMDRV_DEVNAME, DEVFS_FL_DEFAULT, s_pcmdrv_major, 0,
	    S_IFCHR | S_IRUGO | S_IWUGO, &pcmdrv_fops, NULL);
#else
    int result=0, ret = 0;
    struct class *tmp_class;
    struct device *tmp_device;

    result = register_chrdev(s_pcmdrv_major, PCMDRV_DEVNAME, &pcmdrv_fops);
    if (result < 0) {
		printk(KERN_WARNING "pcm: can't get major %d\n",s_pcmdrv_major);
        return result;
    }

    if (s_pcmdrv_major == 0) {
		s_pcmdrv_major = result; /* dynamic */
    }

	tmp_class = class_create(THIS_MODULE, PCMDRV_DEVNAME);
	if (IS_ERR(tmp_class)) {
		ret = PTR_ERR(tmp_class);
		goto err_class_create;
	}
	s_pcmdrv_class = tmp_class;
	tmp_device = device_create(s_pcmdrv_class, NULL, MKDEV(s_pcmdrv_major, 0), "%s", PCMDRV_DEVNAME);
	if (IS_ERR(tmp_device)) {
		ret = PTR_ERR(tmp_device);
		goto err_device_create;
	}
	s_pcmdrv_device = tmp_device;
#endif

	pcmmodule_class=class_create(THIS_MODULE, PCMDRV_DEVNAME);
	if (IS_ERR(pcmmodule_class)) 
		return -EFAULT;
	device_create(pcmmodule_class, NULL, MKDEV(s_pcmdrv_major, 0), NULL, PCMDRV_DEVNAME);
	
	MSG("PCMRST map to GPIO%d\n", CONFIG_RALINK_PCMRST_GPIO);
	MSG("Total %d PCM channel number supported\n", MAX_PCM_CH);
#if defined(CONFIG_RALINK_PCMEXTCLK) 	
	MSG("PCMCLK clock source from SoC external OSC\n");
#else
	MSG("PCMCLK clock source from SoC internal clock\n");	
#endif

#if defined(CONFIG_RALINK_PCMFRACDIV)
	MSG("PCMCLK clock dividor Int[%d], Comp[%d]\n", CONFIG_RALINK_PCMINTDIV, CONFIG_RALINK_PCMCOMPDIV);	
#else
	MSG("PCMCLK clock dividor [%d]\n", CONFIG_RALINK_PCMDIV);	
#endif
	MSG("PCM slot mode is %d\n", CONFIG_RALINK_PCMSLOTMODE);	
	pcm_open();	
	return 0;

err_device_create:

	class_destroy(s_pcmdrv_class);
	s_pcmdrv_class = NULL;

err_class_create:

	unregister_chrdev(s_pcmdrv_major, PCMDRV_DEVNAME);

	return ret;
}

void pcm_exit(void)
{
	pcm_close();
	
#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(s_pcmdrv_major, PCMDRV_DEVNAME);
    devfs_unregister(devfs_handle);
#else

	if (s_pcmdrv_device) {
		device_destroy(s_pcmdrv_class, MKDEV(s_pcmdrv_major, 0));
		s_pcmdrv_device = NULL;
	}
	if (s_pcmdrv_class) {
		class_destroy(s_pcmdrv_class);
		s_pcmdrv_class = NULL;
	}

    unregister_chrdev(s_pcmdrv_major, PCMDRV_DEVNAME);
#endif

	device_destroy(pcmmodule_class,MKDEV(s_pcmdrv_major, 0));
	class_destroy(pcmmodule_class); 
	
	return ;
}

int pcm_open(void)
{
	int i, data;
	unsigned long flags;
	
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
	ppcm_config->slot_mode = CONFIG_RALINK_PCMSLOTMODE;
	
	ppcm_config->tff_thres = CONFIG_PCM_TFF_THRES;
	ppcm_config->rff_thres = CONFIG_PCM_RFF_THRES;
		
	for ( i = 0 ; i < ppcm_config->pcm_ch_num; i ++ )
	{
		ppcm_config->lbk[i] = CONFIG_PCM_LBK;
		ppcm_config->ext_lbk[i] = CONFIG_PCM_EXT_LBK;
		ppcm_config->cmp_mode[i] = CONFIG_PCM_CMP_MODE;
		ppcm_config->ts_start[i] = CONFIG_PCM_TS_START + i*16;	
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
		ppcm_config->TxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage0);
		if(ppcm_config->TxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->TxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &TxPage1);
		if(ppcm_config->TxPage1Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage0Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage0);
		if(ppcm_config->RxPage0Buf16Ptr[i]==NULL)
		{
			pcm_close();
			return PCM_OUTOFMEM;
		}
		ppcm_config->RxPage1Buf16Ptr[i] = pci_alloc_consistent(NULL, PCM_PAGE_SIZE , &RxPage1);
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
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
	data &= 0xFF9FFFFD;
#else
	data &= 0xFFFFFFFD;	
#endif	
	pcm_outw(RALINK_REG_GPIOMODE, data);
#if defined(CONFIG_RALINK_MULTISPI)	
	/* SPI_CS1 set to tristate */
	RT2880_REG(RT2880_SPIARB_REG) |= 0x80000000; 
	RT2880_REG(RT2880_SPICFG1_REG) = SPICFG_MSBFIRST | 
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128 |
									SPICFG_HIZSPI;	
	mdelay(300);
#else
	/* SPI_CS0 set to tristate */
	RT2880_REG(RT2880_SPICFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV128 |
									SPICFG_HIZSPI;
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
	
	/* set GLBCFG's threshold fields */

	data = pcm_inw(PCM_GLBCFG);
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
	data |= REGBIT(ptrpcm_config->slot_mode, PCM_SLOTMODE);
	MSG("PCM_PCMCFG=0x%08X\n",data);
	pcm_outw(PCM_PCMCFG, data);

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

#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
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

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT5350)
	MSG("PCM: enable fractinal PCM_CLK\n");
	pcm_outw(PCM_DIVINT_CFG, CONFIG_RALINK_PCMINTDIV);
	pcm_outw(PCM_DIVCOMP_CFG, CONFIG_RALINK_PCMCOMPDIV|0x80000000);
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
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->TxPage0Buf16Ptr[i], TxPage0);
		if(ppcm_config->TxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->TxPage1Buf16Ptr[i], TxPage1);	
		if(ppcm_config->RxPage0Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->RxPage0Buf16Ptr[i], RxPage0);
		if(ppcm_config->RxPage1Buf16Ptr[i])
			pci_free_consistent(NULL, PCM_PAGE_SIZE, (void*)ppcm_config->RxPage1Buf16Ptr[i], RxPage1);					
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
	if((ppcm_config->tx_isr_cnt%170==11)||(ppcm_config->tx_isr_cnt%170==90))
	{ 
		i= pcm_inw(PCM_GLBCFG);
		pcm_status=pcm_inw(PCM_INT_STATUS);
		printk("ti c=%d %X %X tc=%d\n",dma_ch,pcm_status,i, ppcm_config->tx_isr_cnt);	
	}

	ppcm_config->tx_isr_cnt++;
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM0_TX1))
	{
		chid = 0;
		pPCM_FIFO = (u32*)PCM_CH0_FIFO;
		
	}
	else if((dma_ch==GDMA_PCM1_TX0)||(dma_ch==GDMA_PCM1_TX1))
	{
		chid = 1;
		pPCM_FIFO = (u32*)PCM_CH1_FIFO;
	}
	else
	{
		printk("PCM ERR : tx dma channel number is illeagle\n");
		return;
	}
	
	if((dma_ch==GDMA_PCM0_TX0)||(dma_ch==GDMA_PCM1_TX0))
	{
		page = 0;
		p8PageBuf = ppcm_config->TxPage0Buf8Ptr[chid];
	}
	if((dma_ch==GDMA_PCM0_TX1)||(dma_ch==GDMA_PCM1_TX1))
	{
		page = 1;
		p8PageBuf = ppcm_config->TxPage1Buf8Ptr[chid];
	}
	
	p8FIFOBuf = ppcm_config->TxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;

	//if(ppcm_config->tx_isr_cnt>4)
	{
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
			if(ppcm_config->txfifo_rd_idx[chid]==ppcm_config->txfifo_wt_idx[chid])
			{
				/* tx fifo empty */
				printk("TFE[%d](%d) isr=%d\n",chid,dma_ch,ppcm_config->tx_isr_cnt);
				break;
			}
			
			p8Data = p8FIFOBuf + (ppcm_config->txfifo_rd_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			memcpy((void*)(p8PageBuf+ppcm_config->pos), p8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);	
			
			ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;
			ppcm_config->txfifo_rd_idx[chid] = (ppcm_config->txfifo_rd_idx[chid]+1)%MAX_PCM_FIFO;
		}
	}
	
	GdmaPcmTx((u32)p8PageBuf, (u32)pPCM_FIFO, chid, page, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);		
	switch(chid)
	{
	case 0:
		if(page==0)    
	    	GdmaUnMaskChannel(GDMA_PCM0_TX1);
	    else                                                                  
		    GdmaUnMaskChannel(GDMA_PCM0_TX0);
		break;
	case 1:
		if(page==0)    
		    GdmaUnMaskChannel(GDMA_PCM1_TX1);
	    else                                                                  
	    	GdmaUnMaskChannel(GDMA_PCM1_TX0);
		break;
	default:
	    //FIXEME:error handling
	    break;
	}


	ppcm_config->txcurchid = chid;
	
	//if(ppcm_config->tx_isr_cnt%(CONFIG_PCM_CH*2)==0)
		tasklet_hi_schedule(&pcm_tx_tasklet);	
	
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
	
	if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM0_RX1))
	{
		chid = 0;
		pPCM_FIFO = (u32*)PCM_CH0_FIFO;	
	}
	else if((dma_ch==GDMA_PCM1_RX0)||(dma_ch==GDMA_PCM1_RX1))
	{
		chid = 1;
		pPCM_FIFO = (u32*)PCM_CH1_FIFO;
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
		return;
	}
	
	
	if((dma_ch==GDMA_PCM0_RX0)||(dma_ch==GDMA_PCM1_RX0))
	{
		page = 0;
		p8PageBuf = (char*)(ppcm_config->RxPage0Buf8Ptr[chid]);
	}
	else if((dma_ch==GDMA_PCM0_RX1)||(dma_ch==GDMA_PCM1_RX1))
	{
		page = 1;
		p8PageBuf = (char*)(ppcm_config->RxPage1Buf8Ptr[chid]);
	}
	else
	{
		MSG("PCM ERR : rx dma channel number (CH=%d) is illeagle\n", dma_ch);
	}
	
	if((ppcm_config->rx_isr_cnt%170==50)||(ppcm_config->rx_isr_cnt%170==129))
	{ 
		pcm_status=pcm_inw(PCM_INT_STATUS);
		i=pcm_inw(PCM_GLBCFG);
		printk("ri c=%d %X %X rc=%d\n",dma_ch,pcm_status,i,ppcm_config->rx_isr_cnt);
	}

	ppcm_config->rx_isr_cnt++;
	p8FIFOBuf = ppcm_config->RxFIFOBuf8Ptr[chid];
	ppcm_config->pos = 0;
	
	for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
	{
		if(((ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO)==ppcm_config->rxfifo_rd_idx[chid])
		{
			/* rx fifo full */
			printk("RFF[%d](%d) ",chid,dma_ch);
			break;
		}
		ppcm_config->rxfifo_wt_idx[chid] = (ppcm_config->rxfifo_wt_idx[chid]+1)%MAX_PCM_FIFO;
		p8Data = p8FIFOBuf + (ppcm_config->rxfifo_wt_idx[chid]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		memcpy((void*)p8Data, (void*)(p8PageBuf+ppcm_config->pos), PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
		ppcm_config->pos+=PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE;	
	}
	
	GdmaPcmRx((u32)pPCM_FIFO, (u32)p8PageBuf, chid, page, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
	switch(chid)
	{
	case 0:
		if(page==0)    
			GdmaUnMaskChannel(GDMA_PCM0_RX1);
		else                                                                  
			GdmaUnMaskChannel(GDMA_PCM0_RX0);
		break;
	case 1:
		if(page==0)    
			GdmaUnMaskChannel(GDMA_PCM1_RX1);
		else                                                                  
			GdmaUnMaskChannel(GDMA_PCM1_RX0);
		break;
	default:
		//FIXEME:error handling
		break;
	}

	ppcm_config->curchid = chid;
	//if(ppcm_config->rx_isr_cnt%(CONFIG_PCM_CH*2)==1)
		tasklet_hi_schedule(&pcm_rx_tasklet);
		
	return;
}

#define MAX_SESSION		2
#define MAX_HOOK_FIFO	12
unsigned short* txhook_fifo[MAX_SESSION][MAX_HOOK_FIFO];
unsigned short* rxhook_fifo[MAX_SESSION][MAX_HOOK_FIFO];
int txhook_rd_idx[MAX_SESSION],txhook_wt_idx[MAX_SESSION],rxhook_rd_idx[MAX_SESSION],rxhook_wt_idx[MAX_SESSION];

int PCM_HooK_Init(void)
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

int PCM_HooK_Release(void)
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
	int i;
	short* pTx16Data;
	char* pTx8Data;
	
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
	int rxch,ch;
	int i;
	short* pRx16Data;
	char* pRx8Data;
	
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
					pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
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
	unsigned long flags;
	
	/* handle rx->tx fifo buffer */

	for( ch = 0 ; ch < CONFIG_PCM_CH ; ch ++ )
	{
		rxch = ch;
		txch = (CONFIG_PCM_CH-1)-ch;
			
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{
 			int tx_index;
			spin_lock_irqsave(&ptrpcm_config->lock, flags);

			tx_index = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			if(tx_index==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				//printk("TTFF(%d)[%d] ",i ,txch);
				pTx8Data = NULL;
			}
			else
			{	 
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (tx_index*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}

			if(ptrpcm_config->rxfifo_rd_idx[rxch]==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				//printk("TRFE(%d)[%d] ",i ,rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}	
			pRx16Data = (short*)pRx8Data;
			pTx16Data = (short*)pTx8Data;

			if((pRx16Data==NULL)||(pTx16Data==NULL))
			{
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				continue;
			}
			
			ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			
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

#if defined(PCM_LINEAR)
					txpat = 0x4848;									
					rxpat16 = txpat;
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%04X] RX[0x%04X] \n", (unsigned short)(*(pTx16Data+j)), (unsigned short)(*(pRx16Data+j)));
								break;
							}		
						}
					}
#endif				
#if defined(PCM_U2L2U)
					txpat = 0x4848;									
					rxpat = linear2ulaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X] a[0x%02X]\n", (unsigned char)(*(pTx8Data+j)), (unsigned char)(*(pRx8Data+j)),(unsigned char)rxpat);
								break;
							}		
						}
					}
#endif
#if defined(PCM_A2L2A)		
					txpat = 0x2929;								
					rxpat = linear2alaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X] a[0x%02X]\n", (unsigned char)(*(pTx8Data+j)), (unsigned char)(*(pRx8Data+j)),(unsigned char)rxpat);
								break;
							}		
						}
					}
#endif						
#if defined(PCM_L2U2L)
					
					txpat = 0x3939;
													
					temp = linear2ulaw(txpat);
					rxpat16=ulaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG("PCM_INLOOP PATTERN ERROR [0x%04X] [0x%02X] [0x%04X]\n", (unsigned short)(*(pRx16Data+j)),(unsigned char)temp,(unsigned short)rxpat16);
								break;
							}		
						}
					}
#endif
#if defined(PCM_L2A2L)
					
					txpat = 0x5A5A;							
					temp = linear2alaw(txpat);
					rxpat16=alaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > 20)
					{	
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG("PCM_INLOOP PATTERN ERROR [0x%04X] [0x%02X] [0x%04X]\n", (unsigned short)(*(pRx16Data+j)),(unsigned char)temp,(unsigned short)rxpat16);
								break;
							}		
						}
					}
#endif					

#if defined(PCM_U2L2U)||defined(PCM_A2L2A)			
				memset((void*)pTx8Data, (unsigned char)rxpat, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#else
				memset((void*)pTx8Data, (char)txpat, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#endif
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
#else	/* else not PCM_INLOOP */	
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
				
#endif 			
			}
		}
	
	}

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
	unsigned long flags;
	
	/* handle rx->tx fifo buffer */
	for( ch = 0 ; ch < MAX_PCM_CH ; ch ++ )
	{
		rxch = ch;
		txch = (MAX_PCM_CH-1)-ch;
	
		for (i = 0 ; i < MAX_PCM_PROC_UNIT ; i ++ )
		{	
			int tx_index;
			spin_lock_irqsave(&ptrpcm_config->lock, flags);

			tx_index = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
				
			if(tx_index==ptrpcm_config->txfifo_rd_idx[txch])
			{
				/* tx fifo full */
				//printk("RTFF(%d)[%d] ", i, txch);
				pTx8Data = NULL;
			}
			else
			{	 
				pTx8Data = ptrpcm_config->TxFIFOBuf8Ptr[txch] + (tx_index*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}

			if(ptrpcm_config->rxfifo_rd_idx[rxch]==ptrpcm_config->rxfifo_wt_idx[rxch])
			{
				/* rx fifo empty */
				//printk("RRFE(%d)[%d] ",i ,rxch);
				pRx8Data = NULL;
			}
			else
			{		
				pRx8Data = ptrpcm_config->RxFIFOBuf8Ptr[rxch] + (ptrpcm_config->rxfifo_rd_idx[rxch]*PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
			}	
			pTx16Data = (short*)pTx8Data;
			pRx16Data = (short*)pRx8Data;

			if((pRx16Data==NULL)||(pTx16Data==NULL))
			{
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				continue;
			}
			
			ptrpcm_config->txfifo_wt_idx[txch] = (ptrpcm_config->txfifo_wt_idx[txch]+1)%MAX_PCM_FIFO;
			ptrpcm_config->rxfifo_rd_idx[rxch] = (ptrpcm_config->rxfifo_rd_idx[rxch]+1)%MAX_PCM_FIFO;
			spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
			
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
#if defined(PCM_LINEAR)
					txpat = 0x4848;									
					rxpat16 = txpat;
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%04X] RX[0x%04X] \n", (unsigned short)(*(pTx16Data+j)), (unsigned short)(*(pRx16Data+j)));
								break;
							}		
						}
					}
#endif
#if defined(PCM_U2L2U)
					txpat = 0x4848;									
					rxpat = linear2ulaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X] a[0x%02X]\n", (unsigned char)(*(pTx8Data+j)), (unsigned char)(*(pRx8Data+j)),(unsigned char)rxpat);
								break;
							}		
						}
					}
#endif
#if defined(PCM_A2L2A)		
					txpat = 0x2929;								
					rxpat = linear2alaw(txpat);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE ; j++ )
						{		
							if((unsigned char)(*(pRx8Data+j))!=(unsigned char)rxpat)
							{
								MSG(".PCM_INLOOP PATTERN ERROR TX[0x%02X] RX[0x%02X] a[0x%02X]\n", (unsigned char)(*(pTx8Data+j)), (unsigned char)(*(pRx8Data+j)),(unsigned char)rxpat);
								break;
							}		
						}
					}
#endif						
#if defined(PCM_L2U2L)
					
					txpat = 0x3939;
													
					temp = linear2ulaw(txpat);
					rxpat16=ulaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > 20)
					{
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG("PCM_INLOOP PATTERN ERROR [0x%04X] [0x%02X] [0x%04X]\n", (unsigned short)(*(pRx16Data+j)),(unsigned char)temp,(unsigned short)rxpat16);
								break;
							}		
						}
					}
#endif
#if defined(PCM_L2A2L)
					
					txpat = 0x5A5A;							
					temp = linear2alaw(txpat);
					rxpat16=alaw2linear(temp);
					if(ppcm_config->tx_isr_cnt > 20)
					{	
						for( j = 0 ; j < PCM_8KHZ_SAMPLES ; j++ )
						{		
							if((unsigned short)(*(pRx16Data+j))!=(unsigned short)rxpat16)
							{
								MSG("PCM_INLOOP PATTERN ERROR [0x%04X] [0x%02X] [0x%04X]\n", (unsigned short)(*(pRx16Data+j)),(unsigned char)temp,(unsigned short)rxpat16);
								break;
							}		
						}
					}
#endif					

#if defined(PCM_U2L2U)||defined(PCM_A2L2A)		
				memset((void*)pTx8Data, (unsigned char)rxpat, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#else
				memset((void*)pTx8Data, (char)txpat, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#endif
				spin_unlock_irqrestore(&ptrpcm_config->lock, flags);
				if((ppcm_config->tx_isr_cnt%100)==99)
					MSG("RXTSK LBK PASS[%d]\n",ppcm_config->tx_isr_cnt);
#else	
				memcpy((void*)pTx8Data, (void*)pRx8Data, PCM_8KHZ_SAMPLES*PCM_SAMPLE_SIZE);
#endif			
			}
		}
	}
	
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
static void slic_ramwait (unsigned char ch)
{
	unsigned char regVal; 

	if(slic_type==3220)
	{
		regVal = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)4);
		while (regVal&0x01)
		{
			regVal = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)4);
		}//wait for indirect registers
	}
	else
	{
		regVal = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)31);
		while (regVal&0x01)
		{
			regVal = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)31);
		}//wait for indirect registers
	}
}

int pcm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i, Ret;
	char* p8Data;
	unsigned long flags;
	unsigned char ch, addr;
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
			ptrpcm_playback = (pcm_playback_type*)arg;
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
			PCM_UserPutData_Hook(0, (short*)arg);
			break;
		case PCM_GETDATA:
			PCM_UserGetData_Hook(0, (short*)arg);
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
			p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[0]);
			GdmaPcmRx((u32)PCM_CH0_FIFO, (u32)p8Data, 0, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[0]);
			GdmaPcmRx((u32)PCM_CH0_FIFO, (u32)p8Data, 0, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);	
			
			GdmaUnMaskChannel(GDMA_PCM0_RX0);
			
			p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[0]);
			GdmaPcmTx((u32)p8Data, (u32)PCM_CH0_FIFO, 0, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
			
			p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[0]);
			GdmaPcmTx((u32)p8Data, (u32)PCM_CH0_FIFO, 0, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
		
			GdmaUnMaskChannel(GDMA_PCM0_TX0);
			
			if(ptrpcm_config->pcm_ch_num>=2)
			{
				p8Data = (char*)(ptrpcm_config->TxPage0Buf16Ptr[1]);
				GdmaPcmTx((u32)p8Data, (u32)PCM_CH1_FIFO, 1, 0, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
				
				p8Data = (char*)(ptrpcm_config->TxPage1Buf16Ptr[1]);
				GdmaPcmTx((u32)p8Data, (u32)PCM_CH1_FIFO, 1, 1, PCM_PAGE_SIZE, pcm_dma_tx_isr, pcm_unmask_isr);
	
				GdmaUnMaskChannel(GDMA_PCM1_TX0);
				
				p8Data = (char*)(ptrpcm_config->RxPage0Buf16Ptr[1]);
				GdmaPcmRx((u32)PCM_CH1_FIFO, (u32)p8Data, 1, 0, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
					
				p8Data = (char*)(ptrpcm_config->RxPage1Buf16Ptr[1]);					
				GdmaPcmRx((u32)PCM_CH1_FIFO, (u32)p8Data, 1, 1, PCM_PAGE_SIZE, pcm_dma_rx_isr, pcm_unmask_isr);
				
				GdmaUnMaskChannel(GDMA_PCM1_RX0);
			}
			
			Ret = request_irq(SURFBOARDINT_PCM, pcm_irq_isr, IRQF_DISABLED, "Ralink_PCM", NULL);
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
			if(slic_type==3220)
				data = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)addr);
			else
				data = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, (unsigned char)addr);
			printk("slic(%u) R DR[%03u] = 0x%02X\n",ch, addr, (u32)data);
			break;
		case PCM_SLIC_IRREAD:
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			if(slic_type==3220)
			{
				slic_ramwait(ch);
				spi_si3220_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 103, (unsigned char)addr);
				slic_ramwait(ch);
				data = spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 101) | spi_si3220_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 102) << 8;
			}
			else
			{
				slic_ramwait(ch);
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 30, (unsigned char)addr);
				slic_ramwait(ch);
				data = spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, 28) | (spi_si321x_read8(CONFIG_RALINK_PCM_SPICH, (unsigned char)ch, 29) << 8);
			}
			printk("slic(%u) R IR[%03u] = 0x%04X\n",ch, addr, (u32)data);
			break;
		case PCM_SLIC_DRWRITE:
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			__get_user(data, (int __user *)(long*)arg+2);
			if(slic_type==3220)
			{
				spi_si3220_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), (unsigned char)addr, data);
			}	
			else
			{	
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), (unsigned char)addr, data);
			}
			printk("slic(%u) W DR[%03d] = 0x%02X\n",ch, (u32)addr, (u32)data);
			break;
		case PCM_SLIC_IRWRITE:			
			__get_user(ch, (int __user *)(long*)arg);
			__get_user(addr, (int __user *)(long*)arg+1);
			__get_user(data, (int __user *)(long*)arg+2);
			if(slic_type==3220)
			{
				slic_ramwait(ch);
				spi_si3220_write16(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch | 0x00), (unsigned char)(addr), data);
			}
			else
			{
				slic_ramwait(ch);
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 28, (unsigned char)(data & 0xFF));
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 29, (unsigned char)((data & 0xFF00) >> 8));
				spi_si321x_write8(CONFIG_RALINK_PCM_SPICH, (unsigned char)(ch), 30, (unsigned char)addr);
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

void pcm_reset_slic_cs (void)
{
 	unsigned long data;
	unsigned long gpio_dir_reg,gpio_data_reg,gpio_bit;
	
	MSG("Reset by SPI_CS\n");
	
	/* Set to SP1_CS1_MODE mode and SPI_GPIO_MODE to spi mode */
	data = pcm_inw(RALINK_REG_GPIOMODE);
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
	data &= 0xFF9FFFFD;
#else
	data &= 0xFFFFFFFD;	
#endif	
	pcm_outw(RALINK_REG_GPIOMODE, data);

	RT2880_REG(RT2880_SPIARB_REG) = 0x80000003;
	
	/* CS set to low */
	if(CONFIG_RALINK_PCM_SPICH > 0)
		RT2880_REG(RT2880_SPICTL1_REG) = SPICTL_STARTRD ;
	else
		RT2880_REG(RT2880_SPICTL_REG) = SPICTL_STARTRD ;

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
		RT2880_REG(RT2880_SPICTL1_REG) |= SPICTL_SPIENA_ASSERT;
	else
		RT2880_REG(RT2880_SPICTL_REG) |= SPICTL_SPIENA_ASSERT;
		
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
		RT2880_REG(RT2880_SPICTL1_REG) &= SPICTL_SPIENA_NEGATE;
	else
		RT2880_REG(RT2880_SPICTL_REG) &= SPICTL_SPIENA_NEGATE;

	mdelay(200);

	return;
}

/* spi_cs pin is not always shared with gpio on all Ralink SoC */
void pcm_reset_slic_gpio(void)
{
	unsigned long data;
	unsigned long gpio_dir_reg,gpio_data_reg,gpio_bit;
	
	MSG("Reset by GPIO\n");
#if defined(CONFIG_RALINK_MULTISPI)
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
#if defined(CONFIG_RALINK_MULTISPI)	
	data = pcm_inw(0xB0000600+0x74);
	data |= 0x020;						/* set GPIO#45 as output pin */
	pcm_outw(0xB0000600+0x74, data);
	data = pcm_inw(0xB0000600+0x70);
	data &= ~0x020;						/* set GPIO#45 as low */
	pcm_outw(0xB0000600+0x70, data);
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
#if defined(CONFIG_RALINK_MULTISPI)
	data = pcm_inw(0xB0000600+0x74);
	data |= 0x020;						/* set GPIO#45 as output pin */
	pcm_outw(0xB0000600+0x74, data);
	data = pcm_inw(0xB0000600+0x70);
	data |= 0x020;						/* set GPIO#45 as high */
	pcm_outw(0xB0000600+0x70, data);	
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
#if defined(CONFIG_RALINK_MULTISPI)
	//data = pcm_inw(0xB0000600+0x70);
	//data &= ~0x020;					/* set GPIO#45 as low */
	//pcm_outw(0xB0000600+0x70, data);	
#else
	data = pcm_inw(RALINK_REG_PIODATA);
	data &= ~RALINK_GPIO(3); 
	pcm_outw(RALINK_REG_PIODATA, data);
#endif
	mdelay(200);
	
#if defined(CONFIG_RALINK_MULTISPI)	
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
 
void pcm_reset_slic (void)
{
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
	pcm_reset_slic_gpio();
#else	
	pcm_reset_slic_cs();
#endif	

	return;
}

void pcm_dump_reg (void)
{

	MSG("RALINK_REG_GPIOMODE=0x%08X\n", pcm_inw(RALINK_REG_GPIOMODE));
	MSG("PCM_GLBCFG=0x%08X\n", pcm_inw(PCM_GLBCFG));
	MSG("PCM_PCMCFG=0x%08X\n", pcm_inw(PCM_PCMCFG));
	MSG("PCM_INT_STATUS=0x%08X\n", pcm_inw(PCM_INT_STATUS));
	MSG("PCM_INT_EN=0x%08X\n", pcm_inw(PCM_INT_EN));
	MSG("PCM_FF_STATUS=0x%08X\n", pcm_inw(PCM_FF_STATUS));
	MSG("PCM_CH0_CFG=0x%08X\n", pcm_inw(PCM_CH0_CFG));
	MSG("PCM_CH1_CFG=0x%08X\n", pcm_inw(PCM_CH1_CFG));
#if defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)
	MSG("PCM_FSYNC_CFG=0x%08X\n", pcm_inw(PCM_FSYNC_CFG));
	MSG("PCM_CH_CFG2=0x%08X\n", pcm_inw(PCM_CH_CFG2));
	MSG("PCM_DIVCOMP_CFG=0x%08X\n", pcm_inw(PCM_DIVCOMP_CFG));
	MSG("PCM_DIVINT_CFG=0x%08X\n", pcm_inw(PCM_DIVINT_CFG));
#endif	
	MSG("PCM_CH0_FIFO=0x%08X\n", pcm_inw(PCM_CH0_FIFO));
	MSG("PCM_CH1_FIFO=0x%08X\n", pcm_inw(PCM_CH1_FIFO));
}	

module_init(pcm_init);
module_exit(pcm_exit);
module_param_named(slic, slic_type, int, S_IRUGO);
MODULE_DESCRIPTION("Ralink SoC PCM Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_LICENSE("GPL");
MODULE_VERSION(PCM_MOD_VERSION);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (s_pcmdrv_major, "i");
#else
module_param (s_pcmdrv_major, int, 0);
#endif
