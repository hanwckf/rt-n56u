#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/page.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define jiffiespersec		HZ

#define surf_standby		2
#define surf_active		4
#define surf_boot		5
#define surf_halt		6
#define surf_restore_settings	7
#define surf_changing_state	8

#define surf_ns_unknown		0
#define surf_ns_must_halt	1
#define surf_ns_must_not_halt	2

//these times are in m-sec
#define surf_halt_blink_rate			1000
#define surf_boot_blink_rate			500
#define surf_restore_settings_blink_rate	250

//these times are in sec
#define surf_halt_enable_time			2
#define surf_boot_enable_time			10
#define surf_restore_settings_enable_time	15
#define surf_do_nothing_time			20
#ifdef CONFIG_RALINK_FLASH_API
#define FLASH_WRITE_LOCATION	0x10000
#else
#define FLASH_WRITE_LOCATION	0xbfc00000+0x10000
#endif
#define FLASH_ERASE_SECTOR	1
#define SURF_BUTTON_INTERRUPT	4
#define SURF_TIMER_INTERRUPT 	1

#define SURF_TIMER_BASE		0x300100
#define SURF_TMR_STAT		0x0
#define SURF_TMR1LOAD		0x20
#define SURF_TMR1CTL		0x28
#define SURF_POWER_BUTTON_MASK	0x8
#define TIMER_LOAD_VALUE	0xff

#define SURF_PIO_BASE		0x300600
#define SURF_BTNINT		0x40
#define SURF_BTNRMASK		0x48
#define SURF_BTNFMASK		0x4C
#define SURF_PIODATA2		0x60
#define SURF_PIOPOL2		0x64
#define SURF_LEDCFG		0x68
#define SURF_LED0_MASK		0x00010000
#define SURF_BTN0_MASK		SURF_POWER_BUTTON_MASK
#define SURF_LED0_USE_DATA_REGISTER	0x00000001

#define INIT_CTL		"/dev/initctl"
#define FIFO_FILE		"tmp/system"
#define PROC_ENTRY		"surf_status"

#define SURF_IN(base, reg, val) do { * ((unsigned int *)KSEG1ADDR( base + reg )) = val; } while(0)
#define SURF_OUT(base, reg) *((unsigned int *)KSEG1ADDR( base + reg ))

#define EnableTimer SURF_IN( SURF_TIMER_BASE, SURF_TMR1CTL, 0x9f )
#define DisableTimer SURF_IN( SURF_TIMER_BASE, SURF_TMR1CTL, 0x1f ) 
#define ActiveBoard  { 		unsigned int temp=SURF_OUT(SURF_PIO_BASE, SURF_PIODATA2) | SURF_LED0_MASK; \
				SURF_IN( SURF_PIO_BASE, SURF_PIODATA2, temp ); \
				surf_button_state=surf_board_state=surf_active; \
				DisableTimer; }

// static volatile unsigned int pio_btn_edge;
static struct proc_dir_entry *button_dir, *status_file;
static const char MODULE_NAME[]={"p-button"};
static volatile int surf_board_state=surf_standby;
static volatile int surf_button_state=surf_boot;
static volatile int surf_btn_push_jiffies=-1;
static void surf_events_bh(void *dummy);


// Data structure written to flash memory 
struct flashData 
{
	unsigned int boot_to_standby_prefrence;
	unsigned int need_standby;
} flash_data;


int WriteToFlash(char *buff, int size) //return 0 for success
{
	if( FlashErase(FLASH_ERASE_SECTOR, FLASH_ERASE_SECTOR) != 0 )
		return 0;
	FlashWrite(buff, FLASH_WRITE_LOCATION, size);
}

void ReadFromFlash(char *buff, int size)
{
	memcpy(buff, FLASH_WRITE_LOCATION, size);
}

static struct tq_struct surf_events_task  = {
        routine:        surf_events_bh,
};

static unsigned int should_boot_to_standby()
{
	ReadFromFlash(&flash_data, sizeof(flash_data));
	return flash_data.boot_to_standby_prefrence;
}

struct init_request {
	int	magic;			/* Magic number                 */
	int	cmd;			/* What kind of request         */
	int	runlevel;		/* Runlevel to change to        */
	int	sleeptime;		/* Time between TERM and KILL   */
	char 	data[368];
};


static int proc_read_status(char *page, char **start,
                            off_t off, int count, 
                            int *eof, void *data)
{
	return 0;
}


static int proc_write_status(struct file *file,
                             const char *buffer,
                             unsigned long count, 
                             void *data)
{
	char lbuf[64];
	int len=64;
	if( count < len)
		len = count;
	
	if(copy_from_user(lbuf, buffer, len))
	{
		return -EFAULT;
	}

	if( strncmp(lbuf, "reboot_complete", 15)==0)
	{
		ActiveBoard;
	}
	else if( strncmp(lbuf, "standby_on", 10)==0)
	{
		flash_data.boot_to_standby_prefrence=1;
		WriteToFlash(&flash_data, sizeof(flash_data));	
	}
	else if( strncmp(lbuf, "standby_off", 11)==0)
	{
		flash_data.boot_to_standby_prefrence=0;
		WriteToFlash(&flash_data, sizeof(flash_data));	
	}
	else printk("unknown message: %d\n", lbuf);

        return len;
}


static int create_proc()
{
        int rv = 0;

        /* create directory */
        button_dir = proc_mkdir(MODULE_NAME, NULL);
        if(button_dir == NULL)
		return rv;
        
        button_dir->owner = THIS_MODULE;
        
        status_file = create_proc_entry(PROC_ENTRY, 0644, button_dir);
        if(status_file == NULL)
	{
		remove_proc_entry(MODULE_NAME, NULL);
		return rv;
	}

        status_file->read_proc = NULL;
        status_file->write_proc = proc_write_status;
        status_file->owner = THIS_MODULE;
        
        return rv;
}

static void surf_events_bh(void *dummy)
{
	switch( surf_button_state )
	{
		case surf_active:
			ActiveBoard;
			break;
		case surf_halt:
		case surf_boot:
			flash_data.need_standby=(surf_button_state==surf_halt)?surf_ns_must_halt:surf_ns_must_not_halt;
			WriteToFlash(&flash_data, sizeof(flash_data));
			{
				// init 6 - Reboot
				struct file *filep = NULL;
				struct init_request req;
				req.magic=0x03091969;		//this magic number is needly exactly to enter run-level
				req.cmd=0x1;			
				req.runlevel='6';		//run level 6 requested
				req.sleeptime=5;
				bzero(req.data, sizeof(req.data));

				//Now pass message to init controller
				filep = filp_open(INIT_CTL, O_WRONLY, 0); 
				if( filep != NULL )
				{
					filep->f_op->write(filep, &req, sizeof(req), &filep->f_pos);
					filp_close(filep, NULL);
				}
			}
			break;
		case surf_restore_settings:
			{
				struct file *filep = NULL;
				filep = filp_open(FIFO_FILE, O_WRONLY, 0); 
				if( filep != NULL )
				{
					filep->f_op->write(filep, "restore_settings", 16, &filep->f_pos);
					filp_close(filep, NULL);
				}				
			}
			ActiveBoard;
			break;
	}
}

static void inline SetButtonInterrupt(unsigned int reg)
{
	SURF_IN( SURF_PIO_BASE, SURF_BTNRMASK, reg);
	SURF_IN( SURF_PIO_BASE, SURF_BTNFMASK, reg);
}

static void inline button_released()
{
	surf_btn_push_jiffies=-1;
	if( surf_board_state==surf_active)
	{
		surf_board_state=surf_changing_state;
		schedule_task(&surf_events_task);
	}
}
	

static inline void update_LED()
{
	static unsigned int lastjiffies=0;
	unsigned int ledstatus = SURF_OUT(SURF_PIO_BASE, SURF_PIODATA2);
	int blink = 1;				//if this is set to 0 the blinking effect is disabled
	int diff= jiffies - lastjiffies;
	diff *= (1000 / jiffiespersec);		// diff now contains the number of milliseconds passed since last this function was called


	//surf halt should blink every 1000ms, surf_boot should blink every 500ms and surf_restore_settings should blink every 250ms
	switch( surf_button_state )
	{
		case surf_active:
			blink=0;
			break;
		case surf_halt:
			if ( diff < surf_halt_blink_rate) return; 
			break;
		case surf_boot:
			if ( diff < surf_boot_blink_rate) return;
			break;
		case surf_restore_settings:
			if ( diff < surf_restore_settings_blink_rate) return;
			break;
	}

	//for alternate behaviour (blinking), toggle the ledstatus bit of 1st led
	if( blink && (ledstatus & SURF_LED0_MASK) )
		ledstatus &= (~SURF_LED0_MASK);
	else
		ledstatus |= SURF_LED0_MASK;
	SURF_IN(SURF_PIO_BASE, SURF_PIODATA2, ledstatus);

	lastjiffies = jiffies;
}

static void surf_timer_interrupt(int irq, void *dev, struct pt_regs *regs)
{
	int total_time;

	//clear timer intstat
	SURF_IN( SURF_TIMER_BASE, SURF_TMR_STAT, 0x2);	

	DisableTimer;

	if(surf_btn_push_jiffies != -1)
	{
		total_time = jiffies - surf_btn_push_jiffies;
		total_time /= jiffiespersec;

		// change button status based on the time passed since the button was bushed
		if( total_time >= surf_do_nothing_time )
			surf_btn_push_jiffies=jiffies;

		else if( total_time >= surf_restore_settings_enable_time )
		{
			surf_button_state=surf_restore_settings;
		}

		else if( total_time >= surf_boot_enable_time )
			surf_button_state=surf_boot;

		else if( total_time >= surf_halt_enable_time )
			surf_button_state=surf_halt;

		else surf_button_state=surf_active;
	}
	
	update_LED();
	EnableTimer;
}

static inline void surf_button_interrupt(int irq, void *dev, struct pt_regs *regs)
{
	unsigned int pio_btn_data = SURF_OUT(SURF_PIO_BASE, SURF_PIODATA2); 

	//clear btn instat
	SURF_IN( SURF_PIO_BASE, SURF_BTNINT, 0xffffffff); 

	switch( surf_board_state )
	{
		case surf_active:
			if( pio_btn_data & SURF_POWER_BUTTON_MASK)
			{
				surf_btn_push_jiffies=jiffies;
				EnableTimer;
			}
			else
			{
				button_released();
			}
			break;
		case surf_standby:
			surf_board_state=surf_changing_state;
			break;
	}
}

int power_button_init_module()
{
	int status;

	//reset timer
	SURF_IN(SURF_TIMER_BASE, SURF_TMR_STAT, 0x20);

	//register interrupt
	request_irq(SURF_BUTTON_INTERRUPT, surf_button_interrupt, IRQF_DISABLED, MODULE_NAME, NULL);
	request_irq(SURF_TIMER_INTERRUPT, surf_timer_interrupt, IRQF_DISABLED, MODULE_NAME, NULL);

	//initialize timer
	SURF_IN( SURF_TIMER_BASE, SURF_TMR1LOAD, TIMER_LOAD_VALUE) ;

	// enable interrupts on 4th button

	SetButtonInterrupt(SURF_POWER_BUTTON_MASK);

	//initialze led, use 1st led
	status = SURF_OUT( SURF_PIO_BASE, SURF_BTNFMASK);
	status &= (~SURF_LED0_MASK);
	status |= SURF_LED0_USE_DATA_REGISTER;
	SURF_IN( SURF_PIO_BASE, SURF_LEDCFG, status);

	//set proper polarity values
	status = SURF_OUT( SURF_PIO_BASE, SURF_PIOPOL2);
	status |= (SURF_LED0_MASK | SURF_BTN0_MASK); 
	SURF_IN( SURF_PIO_BASE, SURF_PIOPOL2, status);

	//turn the led off
	SURF_IN( SURF_PIO_BASE, SURF_PIODATA2, SURF_OUT(SURF_PIO_BASE, SURF_PIODATA2) & (~SURF_LED0_MASK) );

	//check the flash register to ready boot to standy prefrence, in case of garbage value write default
	ReadFromFlash(&flash_data, sizeof(flash_data));
	if( flash_data.boot_to_standby_prefrence > 1)
	{
		flash_data.boot_to_standby_prefrence=1;
	}
	if( flash_data.need_standby==surf_ns_unknown && should_boot_to_standby() || flash_data.need_standby==surf_ns_must_halt)
	{
		printk("Press button to start booting...\n");
		while (surf_board_state==surf_standby);
	}
	flash_data.need_standby=surf_ns_unknown;
	WriteToFlash(&flash_data, sizeof(flash_data));

	// Enable timer to show the light blinking (board is booting status)
	EnableTimer;

	// Create proc enteries, so as user processes can change boot prefrence	
	if( create_proc() != 0)
	{
		printk("error creating proc enteries for surf_status\n");
		ActiveBoard;	
	}

	return 0;	
}

void power_button_cleanup_module()
{
	//free IRQ
	free_irq(SURF_TIMER_INTERRUPT, NULL);
	free_irq(SURF_BUTTON_INTERRUPT, NULL);

	//remove proc enteries
        remove_proc_entry("surf_status", button_dir);
        remove_proc_entry(MODULE_NAME, NULL);
}
