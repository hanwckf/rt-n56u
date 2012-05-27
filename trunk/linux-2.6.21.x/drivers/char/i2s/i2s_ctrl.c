#include <linux/init.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include "../ralink_gdma.h"

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static	devfs_handle_t devfs_handle;
#endif

#include "i2s_ctrl.h"
#if defined(CONFIG_I2S_WM8750)
#include "wm8750.h"
#endif
#if defined(CONFIG_I2S_WM8751)
#include "wm8751.h"
#endif

static int i2sdrv_major =  234;

/* external functions declarations */
extern void audiohw_set_frequency(int fsel);
extern int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r);
extern int audiohw_set_linein_vol(int vol_l, int vol_r);

/* internal functions declarations */
irqreturn_t i2s_irq_isr(int irq, void *irqaction);
void i2s_dma_handler(u32 dma_ch);
void i2s_unmask_handler(u32 dma_ch);
int i2s_debug_cmd(unsigned int cmd, unsigned long arg);

/* forward declarations for _fops */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static int i2s_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int i2s_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int i2s_mmap(struct file *file, struct vm_area_struct *vma);
static int i2s_open(struct inode *inode, struct file *file);
static int i2s_release(struct inode *inode, struct file *file);

/* global varable definitions */
i2s_config_type* pi2s_config;
i2s_status_type* pi2s_status;

static dma_addr_t i2s_txdma_addr, i2s_rxdma_addr;
static dma_addr_t i2s_mmap_addr[MAX_I2S_PAGE*2];
						/* 8khz		11.025khz	12khz	 16khz		22.05khz	24Khz	 32khz	   44.1khz	48khz	 88.2khz  96khz*/
unsigned long i2sMaster_inclk_15p625Mhz[11] = {60<<8, 	43<<8, 		40<<8, 	 30<<8, 	21<<8, 		19<<8, 	 14<<8,    10<<8, 	9<<8, 	 7<<8, 	  4<<8};
unsigned long i2sMaster_exclk_12p288Mhz[11] = {47<<8, 	34<<8, 		31<<8,   23<<8, 	16<<8, 		15<<8, 	 11<<8,    8<<8, 	7<<8, 	 5<<8, 	  3<<8};
unsigned long i2sMaster_exclk_12Mhz[11]     = {46<<8, 	33<<8, 		30<<8,   22<<8, 	16<<8, 		15<<8, 	 11<<8,    8<<8,  	7<<8, 	 5<<8, 	  3<<8};
#if defined(CONFIG_I2S_WM8750)
unsigned long i2sSlave_exclk_12p288Mhz[11]  = {0x0C,  	0x00,   	0x10,    0x14,  	0x38,	 	0x38,  	 0x18,     0x20,  	0x00,	 0x00, 	  0x1C};
unsigned long i2sSlave_exclk_12Mhz[11]      = {0x0C,  	0x32,  		0x10,    0x14,  	0x37,  		0x38,  	 0x18,     0x22,  	0x00, 	 0x3E, 	  0x1C};
#endif
#if defined(CONFIG_I2S_WM8751)
unsigned long i2sSlave_exclk_12p288Mhz[11]  = {0x04,  	0x00,   	0x10,    0x14,  	0x38,	 	0x38,  	 0x18,     0x20,  	0x00,	 0x00, 	  0x1C};
unsigned long i2sSlave_exclk_12Mhz[11]      = {0x04,  	0x32,  		0x10,    0x14,  	0x37,  		0x38,  	 0x18,     0x22,  	0x00, 	 0x3E, 	  0x1C};
#endif

#if defined(CONFIG_RALINK_RT6855A)
unsigned long i2sMaster_inclk_int[11] = {97, 70, 65, 48, 35, 32, 24, 17, 16, 12, 8};
unsigned long i2sMaster_inclk_comp[11] = {336, 441, 53, 424, 220, 282, 212, 366, 141, 185, 70};
#else
unsigned long i2sMaster_inclk_int[11] = {78, 56, 52, 39, 28, 26, 19, 14, 13, 9, 6};
unsigned long i2sMaster_inclk_comp[11] = {64, 352, 42, 32, 176, 21, 272, 88, 10, 455, 261};
#endif

/* USB mode 22.05Khz register value in datasheet is 0x36 but will cause slow clock, 0x37 is correct value */
/* USB mode 44.1Khz register value in datasheet is 0x22 but will cause slow clock, 0x23 is correct value */

static const struct file_operations i2s_fops = {
	owner		: THIS_MODULE,
	mmap		: i2s_mmap,
	open		: i2s_open,
	release		: i2s_release,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl:     i2s_ioctl,
#else	
	ioctl		: i2s_ioctl,
#endif	
};

int __init i2s_mod_init(void)
{
	/* register device with kernel */
#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(i2sdrv_major, I2SDRV_DEVNAME , &i2s_fops)) {
		printk(KERN_WARNING " i2s: can't create device node - %s\n", I2SDRV_DEVNAME);
		return -EIO;
    }

    devfs_handle = devfs_register(NULL, I2SDRV_DEVNAME, DEVFS_FL_DEFAULT, i2sdrv_major, 0, 
	    S_IFCHR | S_IRUGO | S_IWUGO, &i2s_fops, NULL);
#else
    int result=0;
    result = register_chrdev(i2sdrv_major, I2SDRV_DEVNAME, &i2s_fops);
    if (result < 0) {
		printk(KERN_WARNING "i2s: can't get major %d\n",i2sdrv_major);
        return result;
    }

    if (i2sdrv_major == 0) {
		i2sdrv_major = result; /* dynamic */
    }
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else	
	i2smodule_class=class_create(THIS_MODULE, I2SDRV_DEVNAME);
	if (IS_ERR(i2smodule_class)) 
		return -EFAULT;
	device_create(i2smodule_class, NULL, MKDEV(i2sdrv_major, 0), I2SDRV_DEVNAME);
#endif	
	return 0;
}

void i2s_mod_exit(void)
{
		
#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(i2sdrv_major, I2SDRV_DEVNAME);
    devfs_unregister(devfs_handle);
#else
    unregister_chrdev(i2sdrv_major, I2SDRV_DEVNAME);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
	device_destroy(i2smodule_class,MKDEV(i2sdrv_major, 0));
	class_destroy(i2smodule_class); 
#endif	
	return ;
}


static int i2s_open(struct inode *inode, struct file *filp)
{
	int Ret;
	int minor = iminor(inode);

	if (minor >= I2S_MAX_DEV)
		return -ENODEV;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif

	if (filp->f_flags & O_NONBLOCK) {
		MSG("filep->f_flags O_NONBLOCK set\n");
		return -EAGAIN;
	}

	/* set i2s_config */
	pi2s_config = (i2s_config_type*)kmalloc(sizeof(i2s_config_type), GFP_KERNEL);
	if(pi2s_config==NULL)
		return -1;

	filp->private_data = pi2s_config;
	memset(pi2s_config, 0, sizeof(i2s_config_type));
	
#ifdef I2S_STATISTIC
	pi2s_status = (i2s_status_type*)kmalloc(sizeof(i2s_status_type), GFP_KERNEL);
	if(pi2s_status==NULL)
		return -1;
	memset(pi2s_status, 0, sizeof(i2s_status_type));	
#endif

	pi2s_config->flag = 0;
	pi2s_config->dmach = GDMA_I2S_TX0;
	pi2s_config->tx_ff_thres = CONFIG_I2S_TFF_THRES;
	pi2s_config->tx_ch_swap = CONFIG_I2S_CH_SWAP;
	pi2s_config->rx_ff_thres = CONFIG_I2S_TFF_THRES;
	pi2s_config->rx_ch_swap = CONFIG_I2S_CH_SWAP;
	pi2s_config->slave_en = CONFIG_I2S_SLAVE_EN;
	
	pi2s_config->srate = 44100;
	pi2s_config->txvol = 0;
	pi2s_config->rxvol = 0;
	pi2s_config->lbk = CONFIG_I2S_INLBK;
	pi2s_config->extlbk = CONFIG_I2S_EXLBK;
	pi2s_config->fmt = CONFIG_I2S_FMT;
	
#if 0	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	Ret = request_irq(SURFBOARDINT_I2S, i2s_irq_isr, IRQF_DISABLED, "Ralink_I2S", NULL);
#else
	Ret = request_irq(SURFBOARDINT_I2S, i2s_irq_isr, SA_INTERRUPT, "Ralink_I2S", NULL);
#endif
	
	if(Ret){
		MSG("IRQ %d is not free.\n", SURFBOARDINT_I2S);
		i2s_release(inode, filp);
		return -1;
	}
#endif	
	pi2s_config->dmach = GDMA_I2S_TX0;
 
    init_waitqueue_head(&(pi2s_config->i2s_tx_qh));
    init_waitqueue_head(&(pi2s_config->i2s_rx_qh));
/*
#if defined(CONFIG_RALINK_RT6855A)	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x834);
	data |=1<<17;
	i2s_outw(RALINK_SYSCTL_BASE+0x834, data);
	data = i2s_inw(RALINK_SYSCTL_BASE+0x834);
	data &=~(1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x834, data);
	audiohw_preinit();
#endif	
*/
	return 0;
}


static int i2s_release(struct inode *inode, struct file *filp)
{
	int i;
	i2s_config_type* ptri2s_config;
	
	/* decrement usage count */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif

	//free_irq(SURFBOARDINT_I2S, NULL);
	
	ptri2s_config = filp->private_data;
	if(ptri2s_config==NULL)
		goto EXIT;
#ifdef CONFIG_MMAP	
	for(i = 0 ; i < MAX_I2S_PAGE*2 ; i ++)
		if(ptri2s_config->pMMAPBufPtr[i])
		{ 
			printk("ummap MMAP[%d]=0x%08X\n",i,ptri2s_config->pMMAPBufPtr[i]);
			dma_unmap_single(NULL, i2s_mmap_addr[i], I2S_PAGE_SIZE, DMA_BIDIRECTIONAL);
			kfree(ptri2s_config->pMMAPBufPtr[i]);	
		}
	ptri2s_config->mmap_index = 0;
#else
	for(i = 0 ; i < MAX_I2S_PAGE ; i ++) {
		if(ptri2s_config->pMMAPRxBufPtr[i])
			kfree(ptri2s_config->pMMAPRxBufPtr[i]);
		if(ptri2s_config->pMMAPTxBufPtr[i])
			kfree(ptri2s_config->pMMAPTxBufPtr[i]);
	}
#endif	
	/* free buffer */
	if(ptri2s_config->pPage0TxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0TxBuf8ptr, i2s_txdma_addr);
		ptri2s_config->pPage0TxBuf8ptr = NULL;
	}
	
	if(ptri2s_config->pPage0RxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0RxBuf8ptr, i2s_rxdma_addr);
		ptri2s_config->pPage0RxBuf8ptr = NULL;
	}
	
	kfree(ptri2s_config);
	
EXIT:			
#ifdef I2S_STATISTIC
	kfree(pi2s_status);
#endif

	MSG("i2s_release succeeds\n");
	return 0;
}

static int i2s_mmap(struct file *filp, struct vm_area_struct *vma)
{
   	int nRet;
	unsigned long size = vma->vm_end-vma->vm_start;
	
	if((pi2s_config->pMMAPBufPtr[0]==NULL)&&(pi2s_config->mmap_index!=0))
		pi2s_config->mmap_index = 0;
		
	if(pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index]!=NULL)
		goto EXIT;

	pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index] = kmalloc(size, GFP_DMA);
	i2s_mmap_addr[pi2s_config->mmap_index] = (dma_addr_t)dma_map_single(NULL, pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], size, DMA_BIDIRECTIONAL);
	MSG("MMAP[%d]=0x%08X, vm_start=%08X,vm_end=%08X\n",pi2s_config->mmap_index, (u32)pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], (u32)vma->vm_start, (u32)vma->vm_end);	
	if( pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index] == NULL ) 
	{
		MSG("i2s_mmap failed\n");
		return -1;
	}
EXIT:	 	
	memset(pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], 0, size);
		
	nRet = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index]) >> PAGE_SHIFT,  size, vma->vm_page_prot);
	
	if( nRet != 0 )
	{
		MSG("i2s_mmap->remap_pfn_range failed\n");
		return -EIO;
	}

	pi2s_config->mmap_index++;
	
#if defined(CONFIG_I2S_TXRX)
	if(pi2s_config->mmap_index>MAX_I2S_PAGE*2)
#else	
	if(pi2s_config->mmap_index>MAX_I2S_PAGE)
#endif	
		pi2s_config->mmap_index = 0;
	return 0;
}

int i2s_reset_tx_config(i2s_config_type* ptri2s_config)
{
	ptri2s_config->bTxDMAEnable = 0;
	ptri2s_config->tx_isr_cnt = 0;
	ptri2s_config->tx_w_idx = 0;
	ptri2s_config->tx_r_idx = 0;	
	pi2s_status->txbuffer_unrun = 0;
	pi2s_status->txbuffer_ovrun = 0;
	pi2s_status->txdmafault = 0;
	pi2s_status->txovrun = 0;
	pi2s_status->txunrun = 0;
	pi2s_status->txthres = 0;
	pi2s_status->txbuffer_len = 0;
	
	return 0;
}

int i2s_reset_rx_config(i2s_config_type* ptri2s_config)
{
	ptri2s_config->bRxDMAEnable = 0;
	ptri2s_config->rx_isr_cnt = 0;
	ptri2s_config->rx_w_idx = 0;
	ptri2s_config->rx_r_idx = 0;	
	pi2s_status->rxbuffer_unrun = 0;
	pi2s_status->rxbuffer_ovrun = 0;
	pi2s_status->rxdmafault = 0;
	pi2s_status->rxovrun = 0;
	pi2s_status->rxunrun = 0;
	pi2s_status->rxthres = 0;
	pi2s_status->rxbuffer_len = 0;
	
	return 0;
}	
	


/*
 *  Ralink Audio System Clock Enable
 *	
 *  I2S_WS : signal direction opposite to/same as I2S_CLK 
 *
 *  I2S_CLK : Integer division or fractional division
 *			  REFCLK from Internal or External (external REFCLK not support for fractional division)
 *			  Suppose external REFCLK always be the same as external MCLK
 * 		
 *  MCLK : External OSC or internal generation
 *
 */
int i2s_clock_enable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	unsigned long index;
	unsigned long* pTable;
	/* audio sampling rate decision */
	switch(ptri2s_config->srate)
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
		/* enable internal MCLK */
#if defined(CONFIG_I2S_IN_MCLK)
#if defined(CONFIG_RALINK_RT6855A)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
#else
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
#endif
#if defined(CONFIG_I2S_MCLK_12MHZ)
	MSG("Enable SoC MCLK 12Mhz\n");
#if defined(CONFIG_RALINK_RT3350)
	data |= (0x1<<8);
#elif defined(CONFIG_RALINK_RT3883)
	data &= ~(0x03<<13);
	data |= (0x1<<13);	
#elif defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) 
	data &= ~(0x0F<<8);
	data |= (0x3<<8);	
#elif defined(CONFIG_RALINK_RT6855A)
	data |= (0x1<<17);
	data &= ~(0x7<<18);
	data |= (0x1<<18);
#else	
	#error "This SoC do not provide MCLK to audio codec\n");	
#endif
#endif /* MCLK_12MHZ */
#if defined(CONFIG_I2S_MCLK_12P288MHZ)
	MSG("Enable SoC MCLK 12.288Mhz\n");
#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855)
	data &= ~(0x01F<<18);
	data |= 31<<18;
	data &= ~(0x01F<<12);
	data |= 1<<12;
	data |= (0xF<<8);
#elif defined(CONFIG_RALINK_RT6855A)
	//FIXME:need config frac div for 12.288Mhz
#else
	#error "This SoC do not provide MCLK 12.288Mhz audio codec\n");	
#endif
#endif /* MCLK_12P288MHZ */
#if defined(CONFIG_RALINK_RT6855A)
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#else
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#endif
#else	
	MSG("Disable SoC MCLK, use external OSC\n");
#if defined(CONFIG_RALINK_RT6855A)
    data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
#else	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
#endif
#if defined(CONFIG_RALINK_RT3350)
	data &= ~(0x1<<8);
#elif defined(CONFIG_RALINK_RT3883)
#elif defined(CONFIG_RALINK_RT3352)
	data &= ~(0x0F<<8);
#elif defined(CONFIG_RALINK_RT5350)		
	data |= (0x0F<<8);
#elif defined (CONFIG_RALINK_RT6855)
	data &= ~(0x0F<<8);
#elif defined (CONFIG_RALINK_RT6855A)
	data &= ~(1<<17);
#elif defined (CONFIG_RALINK_RT6352)
	//FIXME:turn off REFCLK output	
#endif
#if defined(CONFIG_RALINK_RT6855A)
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#else	
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);	
#endif
#endif	
	
	/* set share pins to i2s/gpio mode and i2c mode */
#if defined(CONFIG_RALINK_RT6855A)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
	data |= 0x00008080;
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#else	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60); 
	data &= 0xFFFFFFE2;
	data |= 0x00000018;
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#endif		
	if(ptri2s_config->slave_en==0)
	{
		/* Setup I2S_WS and I2S_CLK */
		
		#if defined(CONFIG_I2S_IN_CLK)
			/* REFCLK is 15.625Mhz or 40Mhz(fractional division) */
			#if defined(CONFIG_I2S_FRAC_DIV)
				MSG("Internal REFCLK with fractional division\n");
				pTable = i2sMaster_inclk_int;
				data = (unsigned long)(pTable[index]);
				i2s_outw(I2S_DIVINT_CFG, data);
				pTable = i2sMaster_inclk_comp;
				data = (unsigned long)(pTable[index]);
				data |= REGBIT(1, I2S_CLKDIV_EN); 
				i2s_outw(I2S_DIVCOMP_CFG, data);
			#else
				MSG("Internal REFCLK 15.625Mhz \n");
				pTable = i2sMaster_inclk_15p625Mhz;
				data = i2s_inw(RALINK_SYSCTL_BASE+0x30); 
				data &= 0xFFFF00FF;
				data |= (unsigned long)(pTable[index]);
				data |= 0x00008000;
				i2s_outw(RALINK_SYSCTL_BASE+0x30, data);  
			#endif
		#else
			#if defined(CONFIG_I2S_MCLK_12MHZ)
				/* REFCLK = MCLK = 12Mhz */
				MSG("External REFCLK 12Mhz \n");
				pTable = i2sMaster_exclk_12Mhz;
				data = i2s_inw(RALINK_SYSCTL_BASE+0x30);
				data &= 0xFFFF00FF;
				data |= (unsigned long)(pTable[index]); 
				data |= 0x0000C000;
				i2s_outw(RALINK_SYSCTL_BASE+0x30, data);  	
			#else
				/* REFCLK = MCLK = 12.288Mhz */
				pTable = i2sMaster_exclk_12p288Mhz;
				MSG("External REFCLK 12.288Mhz \n");
				data = i2s_inw(RALINK_SYSCTL_BASE+0x30);
				data &= 0xFFFF00FF;
				data |= (unsigned long)(pTable[index]); 
				data |= 0x0000C000;
				i2s_outw(RALINK_SYSCTL_BASE+0x30, data); 					 
			#endif
		
		#endif /* Not CONFIG_I2S_IN_CLK */
	
		#if defined(CONFIG_I2S_WS_EDGE)
			data = i2s_inw(I2S_I2SCFG);
			data |= REGBIT(0x1, I2S_WS_INV);
			i2s_outw(I2S_I2SCFG, data);
		#endif
		
		#if defined(CONFIG_RALINK_RT3052)
			data = i2s_inw(I2S_I2SCFG);
			data &= ~REGBIT(0x1, I2S_SLAVE_EN);
			data &= ~REGBIT(0x1, I2S_CLK_OUT_DIS);
			i2s_outw(I2S_I2SCFG, data);
		#elif defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)||\
			defined(CONFIG_RALINK_RT6855)||defined(CONFIG_RALINK_RT6352)||defined(CONFIG_RALINK_RT6855A)
			data = i2s_inw(I2S_I2SCFG);
			data &= ~REGBIT(0x1, I2S_SLAVE_MODE);
			i2s_outw(I2S_I2SCFG, data);
		#else
		#error "a strange clock mode"	
		#endif
		i2s_codec_enable(ptri2s_config);
	#if defined(CONFIG_I2S_MCLK_12MHZ)
	        pTable = i2sSlave_exclk_12Mhz;
	        data = pTable[index];
	        audiohw_set_frequency(data|0x01);
	#else
	        pTable = i2sSlave_exclk_12p288Mhz;
	        data = pTable[index];
	        audiohw_set_frequency(data);
	#endif

	}
	else
	{
		#if defined(CONFIG_RALINK_RT3052)
			data = i2s_inw(I2S_I2SCFG);
			data |= REGBIT(0x1, I2S_SLAVE_EN);
			data |= REGBIT(0x1, I2S_CLK_OUT_DIS);
			i2s_outw(I2S_I2SCFG, data);
		#elif defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)||\
			defined(CONFIG_RALINK_RT6855)||defined(CONFIG_RALINK_RT6352)||defined(CONFIG_RALINK_RT6855A)
			data = i2s_inw(I2S_I2SCFG);
			data |= REGBIT(0x1, I2S_SLAVE_MODE);
			i2s_outw(I2S_I2SCFG, data);
		#else
		#error "a strange clock mode "	
		#endif
		
		i2s_codec_enable(ptri2s_config);
		
		#if defined(CONFIG_I2S_MCLK_12MHZ)
			pTable = i2sSlave_exclk_12Mhz;
			data = pTable[index];
			audiohw_set_frequency(data|0x1); /* for MCLK=12Mhz only. */
		#else
			pTable = i2sSlave_exclk_12p288Mhz;
			data = pTable[index];
			audiohw_set_frequency(data);
		#endif
		
	}

	return 0;
		
}	

int i2s_clock_disable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	i2s_codec_disable(ptri2s_config);
	
	/* disable internal MCLK */
#if defined(CONFIG_I2S_IN_MCLK)	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
#if defined(CONFIG_RALINK_RT3350)	
	data &= ~(0x1<<8);
#elif defined(CONFIG_RALINK_RT3883)
	data &= ~(0x03<<13);
#elif defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855)
	data &= ~(0x03<<8);
#elif defined(CONFIG_RALINK_RT6352)
	//FIXME	
#elif defined(CONFIG_RALINK_RT6855A)
	//FIXME:REFCLK should be disable
#endif	
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);	
#endif
	return 0;
}	

int i2s_codec_enable(i2s_config_type* ptri2s_config)
{
	
	int AIn = 0, AOut = 0;
	/* Codec initialization */
	audiohw_preinit();

#if defined(CONFIG_I2S_TXRX)	
	
	if(ptri2s_config->bTxDMAEnable)
		AOut = 1;
	if(ptri2s_config->bRxDMAEnable)
		AIn = 1;
	audiohw_postinit(!(ptri2s_config->slave_en), AIn, AOut);
	
#else
#if defined(CONFIG_I2S_WM8750)
	audiohw_postinit(!(ptri2s_config->slave_en), 0, 1);
#else	
	if(ptri2s_config->slave_en==0)
		audiohw_postinit(1,1);
	else
		audiohw_postinit(0,1);
#endif		
#endif

	return 0;	
}


int i2s_codec_disable(i2s_config_type* ptri2s_config)
{
	audiohw_close();
	return 0;
}	
	
int i2s_tx_config(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	/* set I2S_I2SCFG */
	data = i2s_inw(I2S_I2SCFG);
	data &= 0xFFFFFF81;
	data |= REGBIT(ptri2s_config->tx_ff_thres, I2S_TX_FF_THRES);
	data |= REGBIT(ptri2s_config->tx_ch_swap, I2S_TX_CH_SWAP);
#if defined(CONFIG_RALINK_RT6855A)	
	data |= REGBIT(1, I2S_BYTE_SWAP);
#endif	
	data &= ~REGBIT(1, I2S_TX_CH0_OFF);
	data &= ~REGBIT(1, I2S_TX_CH1_OFF);
	i2s_outw(I2S_I2SCFG, data);
	
#if defined(CONFIG_I2S_EXTENDCFG)	
	/* set I2S_I2SCFG1 */
	data = i2s_inw(I2S_I2SCFG1);
	data |= REGBIT(ptri2s_config->lbk, I2S_LBK_EN);
	data |= REGBIT(ptri2s_config->extlbk, I2S_EXT_LBK_EN);
	data &= 0xFFFFFFFC;
	data |= REGBIT(ptri2s_config->fmt, I2S_DATA_FMT);
	i2s_outw(I2S_I2SCFG1, data);
#endif
	
	return 0;
}	

int i2s_rx_config(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	/* set I2S_I2SCFG */
	data = i2s_inw(I2S_I2SCFG);
	data &= 0xFFFF81FF;
	data |= REGBIT(ptri2s_config->rx_ff_thres, I2S_RX_FF_THRES);
	data |= REGBIT(ptri2s_config->rx_ch_swap, I2S_RX_CH_SWAP);
	data &= ~REGBIT(1, I2S_RX_CH0_OFF);
	data &= ~REGBIT(1, I2S_RX_CH1_OFF);
	i2s_outw(I2S_I2SCFG, data);
	
#if defined(CONFIG_I2S_EXTENDCFG)	
	/* set I2S_I2SCFG1 */
	data = i2s_inw(I2S_I2SCFG1);
	data |= REGBIT(ptri2s_config->lbk, I2S_LBK_EN);
	data |= REGBIT(ptri2s_config->extlbk, I2S_EXT_LBK_EN);
	data &= 0xFFFFFFFC;
	data |= REGBIT(ptri2s_config->fmt, I2S_DATA_FMT);
	i2s_outw(I2S_I2SCFG1, data);
#endif
	return 0;	
}

/* Turn On Tx DMA and INT */
int i2s_tx_enable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	
	data = i2s_inw(I2S_INT_EN);
#if defined(I2S_FIFO_MODE)
	data = 0;
#else
	data |= REGBIT(0x1, I2S_TX_INT3_EN);
	data |= REGBIT(0x1, I2S_TX_INT2_EN);
	data |= REGBIT(0x1, I2S_TX_INT1_EN);
	//data |= REGBIT(0x1, I2S_TX_INT0_EN);
#endif
	i2s_outw(I2S_INT_EN, data);
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data |= REGBIT(0x1, I2S_TX_EN);
#else
	data |= REGBIT(0x1, I2S_EN);
#endif	
#if defined(I2S_FIFO_MODE)
	data &= ~REGBIT(0x1, I2S_DMA_EN);
#else
	ptri2s_config->bTxDMAEnable = 1;
	data |= REGBIT(0x1, I2S_DMA_EN);
#endif	
	i2s_outw(I2S_I2SCFG, data);
	
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_EN);
#if defined(I2S_TX_BYTE_SWAP)		
	data |= REGBIT(0x1, I2S_BYTE_SWAP);
#else
	data &= ~REGBIT(0x1, I2S_BYTE_SWAP);	
#endif	
	i2s_outw(I2S_I2SCFG, data);
	
	MSG("i2s_tx_enable done\n");
	return I2S_OK;
}

/* Turn On Rx DMA and INT */
int i2s_rx_enable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	
	data = i2s_inw(I2S_INT_EN);
#if defined(I2S_FIFO_MODE)
	data = 0;
#else
	data |= REGBIT(0x1, I2S_RX_INT3_EN);
	data |= REGBIT(0x1, I2S_RX_INT2_EN);
	data |= REGBIT(0x1, I2S_RX_INT1_EN);
	//data |= REGBIT(0x1, I2S_RX_INT0_EN);
#endif
	i2s_outw(I2S_INT_EN, data);
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data |= REGBIT(0x1, I2S_RX_EN);
#else
	data |= REGBIT(0x1, I2S_EN);
#endif	
#if defined(I2S_FIFO_MODE)
	data &= ~REGBIT(0x1, I2S_DMA_EN);
#else
	ptri2s_config->bRxDMAEnable = 1;
	data |= REGBIT(0x1, I2S_DMA_EN);
#endif	
	i2s_outw(I2S_I2SCFG, data);
	
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_EN);
#if defined(I2S_RX_BYTE_SWAP)	
	data |= REGBIT(0x1, I2S_BYTE_SWAP);
#else
	data &= ~REGBIT(0x1, I2S_BYTE_SWAP);		
#endif	
	i2s_outw(I2S_I2SCFG, data);

	MSG("i2s_rx_enable done\n");
	return I2S_OK;
}
/* Turn Off Tx DMA and INT */
int i2s_tx_disable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	
	data = i2s_inw(I2S_INT_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT3_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT2_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT1_EN);
	//data &= ~REGBIT(0x1, I2S_TX_INT0_EN);
	i2s_outw(I2S_INT_EN, data);
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data &= ~REGBIT(0x1, I2S_TX_EN);
#else
	data &= ~REGBIT(0x1, I2S_EN);
#endif	
	if(ptri2s_config->bRxDMAEnable==0)
	{
		ptri2s_config->bTxDMAEnable = 0;
		data &= ~REGBIT(0x1, I2S_DMA_EN);
                data &= ~REGBIT(0x1, I2S_EN);
	}
	i2s_outw(I2S_I2SCFG, data);
	
	return I2S_OK;
}
/* Turn Off Rx DMA and INT */	
int i2s_rx_disable(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	
	data = i2s_inw(I2S_INT_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT3_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT2_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT1_EN);
	//data &= ~REGBIT(0x1, I2S_RX_INT0_EN);
	i2s_outw(I2S_INT_EN, data);
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data &= ~REGBIT(0x1, I2S_RX_EN);
#else
	data &= ~REGBIT(0x1, I2S_EN);
#endif
	if(ptri2s_config->bTxDMAEnable==0)
	{
		ptri2s_config->bRxDMAEnable = 0;
		data &= ~REGBIT(0x1, I2S_DMA_EN);
                data &= ~REGBIT(0x1, I2S_EN);
	}
	i2s_outw(I2S_I2SCFG, data);
	
	return I2S_OK;
}
	
void i2s_dma_tx_handler(u32 dma_ch)
{
	u32 i2s_status;
	
	i2s_status=i2s_inw(I2S_INT_STATUS);
	
	if(pi2s_config->bTxDMAEnable==0)
	{
		MSG("TxDMA not enable dc=%d, i=%d\n", dma_ch, pi2s_config->tx_isr_cnt);
		return;
	}
	
	pi2s_config->tx_isr_cnt++;

#ifdef 	I2S_STATISTIC
	if(pi2s_config->tx_isr_cnt%40==0)
		MSG("tisr i=%u,c=%u,o=%u,u=%d,s=%X [r=%d,w=%d]\n",pi2s_config->tx_isr_cnt,dma_ch,pi2s_status->txovrun,pi2s_status->txunrun,i2s_status,pi2s_config->tx_r_idx,pi2s_config->tx_w_idx);
#endif

	if(pi2s_config->tx_r_idx==pi2s_config->tx_w_idx)
	{
		/* Buffer Empty */
		MSG("TXBE r=%d w=%d[i=%u,c=%u]\n",pi2s_config->tx_r_idx,pi2s_config->tx_w_idx,pi2s_config->tx_isr_cnt,dma_ch);
#ifdef I2S_STATISTIC		
		pi2s_status->txbuffer_unrun++;
#endif	
		if(dma_ch==GDMA_I2S_TX0)
		{
			memset(pi2s_config->pPage0TxBuf8ptr, 0, I2S_PAGE_SIZE);
			GdmaI2sTx((u32)pi2s_config->pPage0TxBuf8ptr, I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		}
		else
		{
			memset(pi2s_config->pPage1TxBuf8ptr, 0, I2S_PAGE_SIZE);
			GdmaI2sTx((u32)pi2s_config->pPage1TxBuf8ptr, I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		}
		
		goto EXIT;	
	}
	
	if(pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx]==NULL)
	{
		MSG("mmap buf NULL [%d]\n",pi2s_config->tx_r_idx);
		if(dma_ch==GDMA_I2S_TX0)
			GdmaI2sTx((u32)pi2s_config->pPage0TxBuf8ptr, I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		else
			GdmaI2sTx((u32)pi2s_config->pPage1TxBuf8ptr, I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);

		goto EXIT;	
	}
#ifdef I2S_STATISTIC	
	pi2s_status->txbuffer_len--;
#endif
	if(dma_ch==GDMA_I2S_TX0)
	{	
#if defined(CONFIG_I2S_MMAP)
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[pi2s_config->tx_r_idx], I2S_PAGE_SIZE, DMA_TO_DEVICE);
		GdmaI2sTx((u32)(pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx]), I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
#else
		memcpy(pi2s_config->pPage0TxBuf8ptr,  pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx], I2S_PAGE_SIZE);			
		GdmaI2sTx((u32)(pi2s_config->pPage0TxBuf8ptr), I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
#endif
		pi2s_config->dmach = GDMA_I2S_TX0;
		pi2s_config->tx_r_idx = (pi2s_config->tx_r_idx+1)%MAX_I2S_PAGE;
	}
	else
	{
#if defined(CONFIG_I2S_MMAP)
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[pi2s_config->tx_r_idx], I2S_PAGE_SIZE, DMA_TO_DEVICE);
		GdmaI2sTx((u32)(pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx]), I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
#else
		memcpy(pi2s_config->pPage1TxBuf8ptr,  pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx], I2S_PAGE_SIZE);
		GdmaI2sTx((u32)(pi2s_config->pPage1TxBuf8ptr), I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
#endif
		pi2s_config->dmach = GDMA_I2S_TX1;
		pi2s_config->tx_r_idx = (pi2s_config->tx_r_idx+1)%MAX_I2S_PAGE;
		
	}
EXIT:		
	wake_up_interruptible(&(pi2s_config->i2s_tx_qh));	
	return;
}

void i2s_dma_rx_handler(u32 dma_ch)
{
	u32 i2s_status;
	static int next_p0_idx, next_p1_idx;
#if defined(CONFIG_I2S_TXRX)	
	i2s_status=i2s_inw(I2S_INT_STATUS);
	if(pi2s_config->bRxDMAEnable==0) {
		MSG("RxDMA not enable dc=%d,i=%d\n",dma_ch,pi2s_config->rx_isr_cnt);
		return;
	}
	if(pi2s_config->rx_isr_cnt==0)
	{
		next_p0_idx = 0;
		next_p1_idx = 1;
	}	
	pi2s_config->rx_isr_cnt++;

#ifdef  I2S_STATISTIC
	if(pi2s_config->rx_isr_cnt%40==0)
		MSG("risr i=%u,c=%u,o=%u,u=%d,s=%X [r=%d,w=%d]\n",pi2s_config->rx_isr_cnt,dma_ch,\
				pi2s_status->rxovrun,pi2s_status->rxunrun,i2s_status,pi2s_config->rx_r_idx,\
				pi2s_config->rx_w_idx);
#endif

	if(((pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE)==pi2s_config->rx_r_idx)
	{
		/* Buffer Full */
		MSG("RXBF r=%d w=%d[i=%u,c=%u]\n",pi2s_config->rx_r_idx,pi2s_config->rx_w_idx,
											pi2s_config->rx_isr_cnt,dma_ch);

#ifdef I2S_STATISTIC		
		pi2s_status->rxbuffer_unrun++;
#endif	
		if(dma_ch==GDMA_I2S_RX0)
		{
			GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		}
		else
		{
			GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage1RxBuf8ptr, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		}
		
		goto EXIT;	
	}
	
#ifdef I2S_STATISTIC	
	pi2s_status->rxbuffer_len++;
#endif

	if(dma_ch==GDMA_I2S_RX0)
	{
		pi2s_config->rx_w_idx = (pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE;	 	
#ifdef CONFIG_I2S_MMAP 
		dma_sync_single_for_cpu(NULL,  i2s_mmap_addr[next_p0_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
        next_p0_idx = (pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE;
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[next_p0_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pMMAPRxBufPtr[next_p0_idx]), 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#else		
		memcpy(pi2s_config->pMMAPRxBufPtr[pi2s_config->rx_w_idx], pi2s_config->pPage0RxBuf8ptr, I2S_PAGE_SIZE);	
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pPage0RxBuf8ptr), 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#endif
		pi2s_config->dmach = GDMA_I2S_RX0;
		
	}
	else
	{
		pi2s_config->rx_w_idx = (pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE; 
#ifdef CONFIG_I2S_MMAP
		dma_sync_single_for_cpu(NULL,  i2s_mmap_addr[next_p1_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
		next_p1_idx = (pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE;
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[next_p1_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pMMAPRxBufPtr[next_p1_idx]), 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#else
		memcpy(pi2s_config->pMMAPRxBufPtr[pi2s_config->rx_w_idx], pi2s_config->pPage1RxBuf8ptr, I2S_PAGE_SIZE);	
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pPage1RxBuf8ptr), 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#endif		
		pi2s_config->dmach = GDMA_I2S_RX1;
		
	}
EXIT:
	//MSG("risr i=%u,d=%d [r=%d,w=%d][p0=%d,p1=%d]\n",pi2s_config->rx_isr_cnt,dma_ch,pi2s_config->rx_r_idx,pi2s_config->rx_w_idx,next_p0_idx,next_p1_idx);

	wake_up_interruptible(&(pi2s_config->i2s_rx_qh));
#endif	
	return;
}

irqreturn_t i2s_irq_isr(int irq, void *irqaction)
{
	u32 i2s_status;
#if defined(I2S_FIFO_MODE)
	i2s_outw(I2S_INT_STATUS, 0xFFFFFFFF);
	return IRQ_HANDLED;
#endif
	//MSG("i2s_irq_isr [0x%08X]\n",i2s_inw(I2S_INT_STATUS));
	if((pi2s_config->tx_isr_cnt>0)||(pi2s_config->rx_isr_cnt>0))
	{
		i2s_status=i2s_inw(I2S_INT_STATUS);
		MSG("i2s_irq_isr [0x%08X]\n",i2s_status);
	}
	else
		return IRQ_HANDLED;
		
	if(i2s_status&REGBIT(1, I2S_TX_DMA_FAULT))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txdmafault++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_OVRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txovrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_UNRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txunrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_THRES))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txthres++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_DMA_FAULT))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxdmafault++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_OVRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxovrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_UNRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxunrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_THRES))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxthres++;
#endif
	}
	i2s_outw(I2S_INT_STATUS, 0xFFFFFFFF);
	return IRQ_HANDLED;
}


void i2s_unmask_handler(u32 dma_ch)
{
	MSG("i2s_unmask_handler ch=%d\n",dma_ch);
	if((dma_ch==GDMA_I2S_TX0)&&(pi2s_config->bTxDMAEnable))
	{
		GdmaI2sTx((u32)pi2s_config->pPage0TxBuf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_TX0);
	}
	if((dma_ch==GDMA_I2S_TX1)&&(pi2s_config->bTxDMAEnable))
	{
		GdmaI2sTx((u32)pi2s_config->pPage1TxBuf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_TX1);
	}
#if defined(CONFIG_I2S_TXRX)	
	if((dma_ch==GDMA_I2S_RX0)&&(pi2s_config->bRxDMAEnable))
	{
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_RX0);
	}
	if((dma_ch==GDMA_I2S_RX1)&&(pi2s_config->bRxDMAEnable))
	{
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage1RxBuf8ptr, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		GdmaUnMaskChannel(GDMA_I2S_RX1);
	}
#endif
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
int i2s_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
int i2s_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int i ;
	unsigned long flags, data;
	i2s_config_type* ptri2s_config;
	    
	ptri2s_config = filp->private_data;
	switch (cmd) {
	case I2S_SRATE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		{
#if defined(CONFIG_RALINK_RT6855A)			
			u32 reg_addr = RALINK_SYSCTL_BASE+0x834; 
#else
			u32 reg_addr = RALINK_SYSCTL_BASE+0x34;
#endif			
			data = *(unsigned long*)(reg_addr);
			data |=(1<<17);
	    	*(unsigned long*)(reg_addr) = data;
	   		data = *(unsigned long*)(reg_addr);
			data &=~(1<<17);
    		*(unsigned long*)(reg_addr) = data;
	    		
	    		audiohw_preinit();
		}	
		if((arg>MAX_SRATE_HZ)||(arg<MIN_SRATE_HZ))
		{
			MSG("audio sampling rate %u should be %d ~ %d Hz\n", (u32)arg, MIN_SRATE_HZ, MAX_SRATE_HZ);
			break;
		}	
		ptri2s_config->srate = arg;
		MSG("set audio sampling rate to %d Hz\n", ptri2s_config->srate);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_TX_VOL:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg > 127)
		{
			ptri2s_config->txvol = 127;
		}
		else if((int)arg < 96)
		{
			ptri2s_config->txvol = 96;
		}	
		else
		ptri2s_config->txvol = arg;
		
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_VOL:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg > 63)
		{
			ptri2s_config->rxvol = 63;
		}
		else if((int)arg < 0)
		{
			ptri2s_config->rxvol = 0;
		}	
		else
		ptri2s_config->rxvol = arg;

		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;	
	case I2S_TX_ENABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_TXENABLE\n");

		/* allocate tx buffer */
		ptri2s_config->pPage0TxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*2 , &i2s_txdma_addr);
		if(ptri2s_config->pPage0TxBuf8ptr==NULL)
		{
			MSG("Allocate Tx Page Buffer Failed\n");
			return -1;
		}
		ptri2s_config->pPage1TxBuf8ptr = ptri2s_config->pPage0TxBuf8ptr + I2S_PAGE_SIZE;
		for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
		{
#if defined(CONFIG_I2S_MMAP)
			ptri2s_config->pMMAPTxBufPtr[i] = ptri2s_config->pMMAPBufPtr[i];
#else
			if(ptri2s_config->pMMAPTxBufPtr[i]==NULL)
				ptri2s_config->pMMAPTxBufPtr[i] = kmalloc(I2S_PAGE_SIZE, GFP_KERNEL);
#endif
		}
#if defined(I2S_FIFO_MODE)
#else
		GdmaI2sTx((u32)ptri2s_config->pPage0TxBuf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
		GdmaI2sTx((u32)ptri2s_config->pPage1TxBuf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_unmask_handler);
#endif	
		
		i2s_reset_tx_config(ptri2s_config);
		ptri2s_config->bTxDMAEnable = 1;
		i2s_tx_config(ptri2s_config);
		
		if(ptri2s_config->bRxDMAEnable==0)
			i2s_clock_enable(ptri2s_config);

		audiohw_set_lineout_vol(1, ptri2s_config->txvol, ptri2s_config->txvol);
		//data = i2s_inw(RALINK_REG_INTENA);
		//data |=0x0400;
		//i2s_outw(RALINK_REG_INTENA, data);
		i2s_tx_enable(ptri2s_config);
#if defined(I2S_FIFO_MODE)
#else
		GdmaUnMaskChannel(GDMA_I2S_TX0);
#endif		
		MSG("I2S_TXENABLE done\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_TX_DISABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_TXDISABLE\n");
		i2s_tx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0)) {
			//data = i2s_inw(RALINK_REG_INTDIS);
			//data |= 0x400;
			//i2s_outw(RALINK_REG_INTDIS, data);
		}
		
		data=i2s_inw(GDMA_CTRL_REG(GDMA_I2S_TX0));
        data&=~(1<<CH_EBL_OFFSET);
		i2s_outw(GDMA_CTRL_REG(GDMA_I2S_TX0), data);
		data=i2s_inw(GDMA_CTRL_REG(GDMA_I2S_TX1));
		data&=~(1<<CH_EBL_OFFSET);
		i2s_outw(GDMA_CTRL_REG(GDMA_I2S_TX1), data);
		i2s_reset_tx_config(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
		
		for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
		{
			if(ptri2s_config->pMMAPTxBufPtr[i] != NULL)
			{
#if defined(CONFIG_I2S_MMAP)
				dma_unmap_single(NULL, i2s_mmap_addr[i], I2S_PAGE_SIZE, DMA_BIDIRECTIONAL);
				ptri2s_config->pMMAPBufPtr[i] = NULL;
				
#endif
				kfree(ptri2s_config->pMMAPTxBufPtr[i]);		
				ptri2s_config->pMMAPTxBufPtr[i] = NULL;
			}
		}
		if(ptri2s_config->mmap_index <= MAX_I2S_PAGE)
			ptri2s_config->mmap_index = 0;
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0TxBuf8ptr, i2s_txdma_addr);
		ptri2s_config->pPage0TxBuf8ptr = NULL;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_ENABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_RXENABLE\n");
		
		/* allocate rx buffer */
		ptri2s_config->pPage0RxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*2 , &i2s_rxdma_addr);
		if(ptri2s_config->pPage0RxBuf8ptr==NULL)
		{
			MSG("Allocate Rx Page Buffer Failed\n");
			return -1;
		}
		ptri2s_config->pPage1RxBuf8ptr = ptri2s_config->pPage0RxBuf8ptr + I2S_PAGE_SIZE;
		
		for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
		{
#if defined(CONFIG_I2S_MMAP)
			ptri2s_config->pMMAPRxBufPtr[i] = ptri2s_config->pMMAPBufPtr[i+(ptri2s_config->mmap_index-MAX_I2S_PAGE)];
#else
			if(ptri2s_config->pMMAPRxBufPtr[i]==NULL)
				ptri2s_config->pMMAPRxBufPtr[i] = kmalloc(I2S_PAGE_SIZE, GFP_KERNEL);
#endif
		}
#if defined(I2S_FIFO_MODE)
#else
#if defined(CONFIG_MMAP)
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pMMAPBufPtr[0+(ptri2s_config->mmap_index-MAX_I2S_PAGE]\
				, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pMMAPBufPtr[1+(ptri2s_config->mmap_index-MAX_I2S_PAGE]\
				, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#else		
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pPage1RxBuf8ptr, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_unmask_handler);
#endif
#endif
		i2s_reset_rx_config(ptri2s_config);
		ptri2s_config->bRxDMAEnable = 1;
		i2s_rx_config(ptri2s_config);
#if defined(I2S_FIFO_MODE)
#else		
		GdmaUnMaskChannel(GDMA_I2S_RX0);
#endif
		if(ptri2s_config->bTxDMAEnable==0)
			i2s_clock_enable(ptri2s_config);
#if defined(CONFIG_I2S_TXRX)
		audiohw_set_linein_vol(ptri2s_config->rxvol,  ptri2s_config->rxvol);
#endif
		i2s_rx_enable(ptri2s_config);

		//data = i2s_inw(RALINK_REG_INTENA);
		//data |=0x0400;
		//i2s_outw(RALINK_REG_INTENA, data);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		break;
	case I2S_RX_DISABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_RXDISABLE\n");
		i2s_rx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
		{
			//data = i2s_inw(RALINK_REG_INTDIS);
			//data |= 0x0400;
			//i2s_outw(RALINK_REG_INTDIS, data);
		}
    	data=i2s_inw(GDMA_CTRL_REG(GDMA_I2S_RX0));
        data&=~(1<<CH_EBL_OFFSET);
		i2s_outw(GDMA_CTRL_REG(GDMA_I2S_RX0), data);
		data=i2s_inw(GDMA_CTRL_REG(GDMA_I2S_RX1));
		data&=~(1<<CH_EBL_OFFSET);
        i2s_outw(GDMA_CTRL_REG(GDMA_I2S_RX1), data);

		
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
		
		for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
		{
			if(ptri2s_config->pMMAPRxBufPtr[i] != NULL) {
#if defined(CONFIG_I2S_MMAP)
				dma_unmap_single(NULL, i2s_mmap_addr[i+(ptri2s_config->mmap_index-MAX_I2S_PAGE)], \
								I2S_PAGE_SIZE, DMA_BIDIRECTIONAL);
				ptri2s_config->pMMAPBufPtr[i+(ptri2s_config->mmap_index-MAX_I2S_PAGE)] = NULL;
#endif
				kfree(ptri2s_config->pMMAPRxBufPtr[i]);		
				ptri2s_config->pMMAPRxBufPtr[i] = NULL;
			}
		}
		ptri2s_config->mmap_index = 0;	
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0RxBuf8ptr, i2s_rxdma_addr);
		ptri2s_config->pPage0RxBuf8ptr = NULL;
		i2s_reset_rx_config(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_PUT_AUDIO:
		//MSG("I2S_PUT_AUDIO\n");	
#if defined(I2S_FIFO_MODE)		
		{
			
			long* pData ;
			//MSG("I2S_PUT_AUDIO FIFO\n");
			copy_from_user(ptri2s_config->pMMAPTxBufPtr[0], (char*)arg, I2S_PAGE_SIZE);
			pData = ptri2s_config->pMMAPTxBufPtr[0];
			for(i = 0 ; i < I2S_PAGE_SIZE>>2 ; i++ )	
			{
				int j;
				unsigned long status = i2s_inw(I2S_FF_STATUS);
				while((status&0x0F)==0)
				{
					for(j = 0 ; j < 50 ; j++);
					status = i2s_inw(I2S_FF_STATUS);
				}
				*((volatile uint32_t *)(I2S_TX_FIFO_WREG)) = cpu_to_le32(*pData);
				if(i==16)
					MSG("I2S_PUT_AUDIO FIFO[0x%08X]\n", *pData);
				pData++;
				
					
			}
		}
		break;
#else		
		do{
			spin_lock_irqsave(&ptri2s_config->lock, flags);
			
			if(((ptri2s_config->tx_w_idx+4)%MAX_I2S_PAGE)!=ptri2s_config->tx_r_idx)
			{
				ptri2s_config->tx_w_idx = (ptri2s_config->tx_w_idx+1)%MAX_I2S_PAGE;	
				//printk("put TB[%d] for user write\n",ptri2s_config->tx_w_idx);
#if defined(CONFIG_I2S_MMAP)
				put_user(ptri2s_config->tx_w_idx, (int*)arg);
#else
				copy_from_user(ptri2s_config->pMMAPTxBufPtr[ptri2s_config->tx_w_idx], (char*)arg, I2S_PAGE_SIZE);
#endif
				pi2s_status->txbuffer_len++;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);	
				break;
			}
			else
			{
				/* Buffer Full */
				//printk("TBF tr=%d, tw=%d\n", ptri2s_config->tx_r_idx, ptri2s_config->tx_w_idx);
				pi2s_status->txbuffer_ovrun++;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);
				interruptible_sleep_on(&(ptri2s_config->i2s_tx_qh));
				
			}
		}while(1);
		break;
#endif
	case I2S_GET_AUDIO:
#if defined(I2S_FIFO_MODE)			
		{
			
			long* pData ;
			
			
			pData = ptri2s_config->pMMAPRxBufPtr[0];
			for(i = 0 ; i < I2S_PAGE_SIZE>>2 ; i++ )	
			{
				int j;
				unsigned long status = i2s_inw(I2S_FF_STATUS);
				while((status&0x0F0)==0)
				{
					for(j = 0 ; j < 50 ; j++);
					status = i2s_inw(I2S_FF_STATUS);
				}
				
				*pData = i2s_inw(I2S_RX_FIFO_RREG);
				if(i==16)
					MSG("I2S_GET_AUDIO FIFO[0x%08X]\n", *pData);
				pData++;
			}
			
			copy_to_user((char*)arg, ptri2s_config->pMMAPRxBufPtr[0], I2S_PAGE_SIZE);
		}
		break;
#else		
		do{
			spin_lock_irqsave(&ptri2s_config->lock, flags);
			//printk("GA rr=%d, rw=%d,i=%d\n", ptri2s_config->rx_r_idx, ptri2s_config->rx_w_idx,ptri2s_config->rx_isr_cnt);
			if(ptri2s_config->rx_r_idx!=ptri2s_config->rx_w_idx)
			{			
#if defined(CONFIG_I2S_MMAP)
				put_user(ptri2s_config->rx_r_idx, (int*)arg);
#else
				copy_to_user((char*)arg, ptri2s_config->pMMAPRxBufPtr[ptri2s_config->rx_r_idx], I2S_PAGE_SIZE);
#endif
				ptri2s_config->rx_r_idx = (ptri2s_config->rx_r_idx+1)%MAX_I2S_PAGE;
				pi2s_status->rxbuffer_len--;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);	
				break;
			}
			else
			{
				/* Buffer Full */
				//printk("RBF rr=%d, rw=%d\n", ptri2s_config->rx_r_idx, ptri2s_config->rx_w_idx);
				pi2s_status->rxbuffer_ovrun++;
				spin_unlock_irqrestore(&ptri2s_config->lock, flags);
				interruptible_sleep_on(&(ptri2s_config->i2s_rx_qh));
				spin_lock_irqsave(&ptri2s_config->lock, flags);
			}
		}while(1);
		break;
#endif		
	case I2S_DEBUG_CLKGEN:
	case I2S_DEBUG_INLBK:
	case I2S_DEBUG_EXLBK:
	case I2S_DEBUG_CODECBYPASS:	
	case I2S_DEBUG_FMT:
	case I2S_DEBUG_RESET:
		i2s_debug_cmd(cmd, arg);
		break;							
	default :
		MSG("i2s_ioctl: command format error\n");
	}

	return 0;
}

module_init(i2s_mod_init);
module_exit(i2s_mod_exit);

MODULE_DESCRIPTION("Ralink SoC I2S Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_SUPPORTED_DEVICE("I2S");
MODULE_VERSION(I2S_MOD_VERSION);
MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (i2sdrv_major, "i");
#else
module_param (i2sdrv_major, int, 0);
#endif
