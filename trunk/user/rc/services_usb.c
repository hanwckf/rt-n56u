/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#include <dev_info.h>
#include <usb_info.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "rc.h"
#include "switch.h"
#include "gpio_pins.h"

void
safe_remove_usb_device(int port, const char *dev_name)
{
	int modem_devnum = 0;

	if (dev_name && strncmp(dev_name, "sd", 2) != 0) {
		modem_devnum = atoi(dev_name);
		if (modem_devnum < 0)
			modem_devnum = 0;
	}

	if (port >= 1 && port <= BOARD_NUM_USB_PORTS) {
		if (modem_devnum) {
			usb_info_t *usb_info, *follow_usb;
			int has_modem_port = 0;
			
			usb_info = get_usb_info();
			for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
				if ((follow_usb->port_root == port) &&
				    (follow_usb->id_devnum == modem_devnum) &&
				    (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
				     follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH))
					has_modem_port |= 1;
			}
			free_usb_info(usb_info);
			
			if (has_modem_port) {
				if ( get_usb_modem_dev_wan(0, modem_devnum) ) {
					safe_remove_usb_modem();
					try_wan_reconnect(0, 0);
				}
			}
		} else {
			safe_remove_stor_device(port, port, dev_name, 1);
		}
		
	} else if (port == 0) {
		safe_remove_stor_device(1, BOARD_NUM_USB_PORTS, NULL, 1);
	}
}

#if defined (BOARD_GPIO_PWR_USB) || defined (BOARD_GPIO_PWR_USB2)
void
power_control_usb_port(int port, int power_on)
{
	unsigned int pin_power_1 = (BOARD_GPIO_PWR_USB_ON != 0) ? 1 : 0;
	unsigned int pin_power_0 = (BOARD_GPIO_PWR_USB_ON != 0) ? 0 : 1;
	unsigned int pin_power = (power_on) ? pin_power_1 : pin_power_0;

#if defined (BOARD_GPIO_PWR_USB)
	if (port == 0 || port == 1)
		cpu_gpio_set_pin(BOARD_GPIO_PWR_USB, pin_power);
#endif
#if defined (BOARD_GPIO_PWR_USB2)
	if (port == 0 || port == 2)
		cpu_gpio_set_pin(BOARD_GPIO_PWR_USB2, pin_power);
#endif
}
#endif

#if defined(SRV_U2EC)
void
start_u2ec(void)
{
	if (nvram_match("u2ec_enable", "0")) 
		return;

	unlink("/var/run/u2ec.pid");
	system("/usr/sbin/u2ec &");
	nvram_set("apps_u2ec_ex", "1");
}

void
stop_u2ec(void)
{
	char* svcs[] = { "u2ec",  NULL };
	kill_services(svcs, 3, 1);
}
#endif

#if defined(SRV_LPRD)
void
start_lpd(void)
{
	if (nvram_match("lprd_enable", "0")) 
		return;

	unlink("/var/run/lpdparent.pid");
	system("/usr/sbin/lpd &");
}

void
stop_lpd(void)
{
	char* svcs[] = { "lpd", NULL };
	kill_services(svcs, 3, 1);

	unlink("/var/run/lpdparent.pid");
}
#endif

void
start_p910nd(char *devlp)
{
	if (nvram_match("rawd_enable", "0")) 
		return;

	if (nvram_match("rawd_enable", "2"))
		eval("/usr/sbin/p910nd", "-b", "-f", devlp, "0");
	else
		eval("/usr/sbin/p910nd", "-f", devlp, "0");
}

void
stop_p910nd(void)
{
	char* svcs[] = { "p910nd", NULL };
	kill_services(svcs, 3, 1);
}

static void
exec_printer_daemons(int call_fw)
{
	int i, has_printer = 0;
	char *opt_printer_script = "/opt/bin/on_hotplug_printer.sh";
	char dev_lp[16];

	for (i = 0; i < 10; i++) {
		sprintf(dev_lp, "/dev/usb/lp%d", i);
		if (check_if_dev_exist(dev_lp)) {
			has_printer = 1;
			if (call_fw) {
				if (check_if_file_exist(opt_printer_script))
					doSystem("%s %s", opt_printer_script, dev_lp);
			}
			start_p910nd(dev_lp);
		}
	}
	
	if (has_printer) {
#if defined(SRV_U2EC)
		start_u2ec();
#endif
#if defined(SRV_LPRD)
		start_lpd();
#endif
	}
}

void
stop_usb_printer_spoolers(void)
{
#if defined(SRV_U2EC)
	stop_u2ec();
#endif
#if defined(SRV_LPRD)
	stop_lpd();
#endif
	stop_p910nd();
}

void
restart_usb_printer_spoolers(void)
{
	stop_usb_printer_spoolers();
	exec_printer_daemons(0);
}

void
try_start_usb_printer_spoolers(void)
{
	stop_usb_printer_spoolers();
	exec_printer_daemons(1);
}

void
try_start_usb_modem_to_wan(void)
{
	int modem_prio, has_link;

	if (get_ap_mode())
		return;

	/* check modem prio mode  */
	modem_prio = nvram_get_int("modem_prio");
	if (modem_prio < 1)
		return;

	/* check modem already selected to WAN */
	if (get_usb_modem_wan(0))
		return;

	/* check modem enabled and ready */
	if (!get_modem_devnum())
		return;

	if (modem_prio == 2) {
		if (get_apcli_wisp_ifname())
			return;
		
		has_link = get_wan_ether_link_direct(0);
		if (has_link < 0)
			has_link = 0;
		
		if (has_link)
			return;
	}

	logmessage("USB hotplug", "try start USB Modem as WAN connection...");

	try_wan_reconnect(1, 0);
}

