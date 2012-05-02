#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <linux/ralink_gpio.h>

#define RDM_DEVNAME	    "ledctrl"

int led_ioctrl_major =  23;
	
//+++Eric add CLIENT_LED function 
int clk=1;
void CLIENT_LED_WL_B(){
	GPIO_write_bit(PORT14, 1);
	for(clk = 1; clk<= 7 ; clk++){
		GPIO_write_bit(PORT4, 0);
		GPIO_write_bit(PORT4, 1);
	}
	
	GPIO_write_bit(PORT14, 0);
	for(clk = 1; clk<= 1 ; clk++){
		GPIO_write_bit(PORT4, 0);
		GPIO_write_bit(PORT4, 1);
	}
}
void CLIENT_LED_WL_A(){
		GPIO_write_bit(PORT14, 1);

		for(clk = 1; clk<= 5 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		GPIO_write_bit(PORT14, 0);
		for(clk = 1; clk<= 2 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		GPIO_write_bit(PORT14, 1);
		for(clk = 1; clk<= 1 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		
	}
void CLIENT_LED_LAN_B(){
		GPIO_write_bit(PORT14, 1);
		for(clk = 1; clk<= 2 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}

		GPIO_write_bit(PORT14, 0);
		for(clk = 1; clk<= 2 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		GPIO_write_bit(PORT14, 1);
		for(clk = 1; clk<= 3 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
	}
void CLIENT_LED_LAN_A(){
				GPIO_write_bit(PORT14, 0);

				for(clk = 1; clk<= 7 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}
	}
void CLIENT_LED_WL_B_LAN_B(){
				GPIO_write_bit(PORT14, 0);
		
				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

					GPIO_write_bit(PORT14, 1);

				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

					GPIO_write_bit(PORT14, 0);

				for(clk = 1; clk<= 1 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}
	}
void CLIENT_LED_WL_B_LAN_A(){
					GPIO_write_bit(PORT14, 0);
				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}
				
				GPIO_write_bit(PORT14, 1);
				for(clk = 1; clk<= 4 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

				GPIO_write_bit(PORT14, 0);
				for(clk = 1; clk<= 1 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}
	}
void CLIENT_LED_WL_A_LAN_B(){
		GPIO_write_bit(PORT14, 1);
		for(clk = 1; clk<= 3 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		
		GPIO_write_bit(PORT14, 0);
		for(clk = 1; clk<= 4 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
		GPIO_write_bit(PORT14, 1);
		for(clk = 1; clk<= 1 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}
	}
void CLIENT_LED_WL_A_LAN_A(){
				GPIO_write_bit(PORT14, 0);
				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

				GPIO_write_bit(PORT14, 1);
				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

				GPIO_write_bit(PORT14, 0);
				for(clk = 1; clk<= 2 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}

				GPIO_write_bit(PORT14, 1);
				for(clk = 1; clk<= 1 ; clk++){
					GPIO_write_bit(PORT4, 0);
					GPIO_write_bit(PORT4, 1);
				}
	}
void CLIENT_LED_NO(){
		GPIO_write_bit(PORT14, 1);

		for(clk = 1; clk<= 7 ; clk++){
			GPIO_write_bit(PORT4, 0);
			GPIO_write_bit(PORT4, 1);
		}	
	}

//---Eric add

int led_ioctrl_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int led_ioctrl_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int led_ioctrl_ioctl(struct inode *inode, struct file *filp, int cmd,
		unsigned long arg)
{
	int rc = 0;
	int i ;
	int reg_addr, reg_value = 0;
	
	switch (cmd) {
		case GMTK_CLOSE_SPEED_METER:
			//printk(KERN_EMERG "Do 'echo 0 > /proc/SPEED;' in ixp4xx_gpio.h   \n");
			break;

		case GMTK_SYSTEM_READY:
			GPIO_write_bit(PORT9, 0);
			break;

		case GMTK_SYSTEM_BOOT:
			GPIO_write_bit(PORT9, 1);
			break;

		case GMTK_SPEED_METER_BOOT_UP_READY:
			//printk(KERN_EMERG "Do 'echo 0 > /proc/SPEED_BOOT_UP;' in ixp4xx_gpio.h   \n");
			break;
			
		case GMTK_SPEED_METER_BOOT_UP:
			//printk(KERN_EMERG "Do 'echo 6 > /proc/SPEED_BOOT_UP;' in ixp4xx_gpio.h   \n");
			break;

		case GMTK_WL_SEC_DISABLED:
			GPIO_write_bit(PORT13, 1);
			GPIO_write_bit(PORT12, 1);
			break;

		case GMTK_WL_SEC_ENABLED:
			GPIO_write_bit(PORT13, 0);
			GPIO_write_bit(PORT12, 1);
			break;

		case GMTK_LAN_LINK_DOWN:	//=======================Not Used....Become GMTK_NO_WIRE_WIRELESS_COMPUTER ==================/
			//printk(KERN_EMERG "Setting LED!!  GMTK_LAN_LINK_DOWN\n");
			break;

		case GMTK_LAN_LINK_UP:		
			CLIENT_LED_LAN_B();
			break;

		case GMTK_LAN_LINK_ERROR:	
			CLIENT_LED_LAN_A();
			break;

		case GMTK_WAN_MODEM_DOWN:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_MODEM_DOWN\n");
			modem_led_set(4);
			break;

		case GMTK_WAN_MODEM_UP:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_MODEM_UP\n");
			modem_led_set(0);
			break;

		case GMTK_WAN_MODEM_ERROR:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_MODEM_ERROR\n");
			modem_led_set(3);
			break;
		case GMTK_WAN_INTERNET_DOWN:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_INTERNET_DOWN\n");
			net_led_set(4);
			break;

		case GMTK_WAN_INTERNET_CONNECTING:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_INTERNET_CONNECTING\n");
			net_led_set(1);
			break;

		case GMTK_WAN_INTERNET_CONNECTED:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_INTERNET_CONNECTED\n");
			net_led_set(0);
			break;	

		case GMTK_WAN_INTERNET_ERROR:
			//printk(KERN_EMERG "Setting LED!!  GMTK_WAN_INTERNET_ERROR\n");
			net_led_set(3);
			break;

		case GMTK_WL_CLI_NOT_ASSOC:		//=======================Not Used....Become GMTK_NO_WIRE_WIRELESS_COMPUTER ==================/
			//printk(KERN_EMERG "Setting LED!!  GMTK_WL_CLI_NOT_ASSOC\n");
			break;
			
		case GMTK_WL_CLI_ASSOC:	
			CLIENT_LED_WL_B();
			break;
		case GMTK_WL_CLI_ASSOC_ERROR:
				client_led_set(0);
/*
				CLIENT_LED_WL_A();
*/
			break;
		case GMTK_USB_DOWN:
			printk(KERN_EMERG "Do 'echo 0 > /proc/USB_BLINK;echo 1 > /proc/GPIO23;echo 1 > /proc/GPIO22;' in ixp4xx_gpio.h   \n");
			usb_blink_led(0);
			GPIO_write_bit(PORT23, 1);
			GPIO_write_bit(PORT22, 1);
			break;
			
		case GMTK_USB_READY:
			printk(KERN_EMERG "Do 'echo 0 > /proc/USB_BLINK;echo 0 > /proc/GPIO23;echo 1 > /proc/GPIO22;' in ixp4xx_gpio.h   \n");
			usb_blink_led(0);
			GPIO_write_bit(PORT23, 0);
			GPIO_write_bit(PORT22, 1);
			break;

		case GMTK_USB_ERROR:
			printk(KERN_EMERG "echo 1 > /proc/GPIO23;echo 1 > /proc/USB_BLINK;' in ixp4xx_gpio.h   \n");
			usb_blink_led(1);
			GPIO_write_bit(PORT23, 1);
			break;

		case GMTK_NO_WIRE_WIRELESS_COMPUTER:
			CLIENT_LED_NO();
				break;

		case GMTK_LAN_WL_LINK_UP:
			CLIENT_LED_WL_B_LAN_B();
				break;

		case GMTK_LAN_UP_WL_ERROR:
				client_led_set(1);
/*
			CLIENT_LED_WL_A_LAN_B();
*/				
				break;

		case GMTK_LAN_ERROR_WL_UP:
				client_led_set(2);
/*				
			CLIENT_LED_WL_B_LAN_A();
*/
				break;
			
		case GMTK_LAN_ERROR_WL_ERROR:
				client_led_set(3);		
//			CLIENT_LED_WL_A_LAN_A();
				break;
	}
	return rc;
}

static const struct file_operations led_ioctrl_fops = {
	.owner = THIS_MODULE,
	.ioctl = led_ioctrl_ioctl,
	.open = led_ioctrl_open,
	.release = led_ioctrl_release,
};

static int led_ioctrl_init(void)
{
	int result=0;
	result = register_chrdev(led_ioctrl_major, RDM_DEVNAME, &led_ioctrl_fops);
	if (result < 0) {
		return result;
	}

	if (led_ioctrl_major == 0) {
		led_ioctrl_major = result; /* dynamic */
	}
	return 0;
}

static void led_ioctrl_exit(void)
{
	//printk(KERN_EMERG "gemtek_ledctrl_exit\n");
	unregister_chrdev(led_ioctrl_major, RDM_DEVNAME);
}

module_init(led_ioctrl_init);
module_exit(led_ioctrl_exit);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (led_ioctrl_major, "i");
#else
module_param (led_ioctrl_major, int, 0);
#endif

MODULE_LICENSE("GPL");
