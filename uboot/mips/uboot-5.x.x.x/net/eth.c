/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
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

#include <common.h>
#include <command.h>
#include <net.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#ifdef CFG_GT_6426x
extern int gt6426x_eth_initialize(bd_t *bis);
#endif

extern int au1x00_enet_initialize(bd_t*);
extern int dc21x4x_initialize(bd_t*);
extern int e1000_initialize(bd_t*);
extern int eepro100_initialize(bd_t*);
extern int eth_3com_initialize(bd_t*);
extern int fec_initialize(bd_t*);
extern int inca_switch_initialize(bd_t*);
extern int mpc5xxx_fec_initialize(bd_t*);
extern int mpc8220_fec_initialize(bd_t*);
extern int mv6436x_eth_initialize(bd_t *);
extern int mv6446x_eth_initialize(bd_t *);
extern int natsemi_initialize(bd_t*);
extern int ns8382x_initialize(bd_t*);
extern int pcnet_initialize(bd_t*);
extern int plb2800_eth_initialize(bd_t*);
extern int ppc_4xx_eth_initialize(bd_t *);
extern int ppc_440x_eth_initialize(bd_t *);
extern int rtl8139_initialize(bd_t*);
extern int rtl8169_initialize(bd_t*);
extern int scc_initialize(bd_t*);
extern int skge_initialize(bd_t*);
extern int tsec_initialize(bd_t*, int);
extern int rt2880_eth_initialize(bd_t *bis);

static struct eth_device *eth_devices, *eth_current;

void eth_parse_enetaddr(const char *addr, uchar *enetaddr)
{
	char *end;
	int i;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

#ifdef CONFIG_NET_MULTI
struct eth_device *eth_get_dev(void)
{
	return eth_current;
}

int eth_get_dev_index (void)
{
	struct eth_device *dev;
	int num = 0;

	if (!eth_devices) {
		return (-1);
	}

	for (dev = eth_devices; dev; dev = dev->next) {
		if (dev == eth_current)
			break;
		++num;
	}

	if (dev) {
		return (num);
	}

	return (0);
}
#endif

int eth_register(struct eth_device* dev)
{
	struct eth_device *d;

	if (!eth_devices) {
		eth_current = eth_devices = dev;
#ifdef CONFIG_NET_MULTI
		/* update current ethernet name */
		{
			char *act = getenv("ethact");
			if (act == NULL || strcmp(act, eth_current->name) != 0)
				setenv("ethact", eth_current->name);
		}
#endif
	} else {
		for (d=eth_devices; d->next!=eth_devices; d=d->next);
		d->next = dev;
	}

	dev->state = ETH_STATE_INIT;
	dev->next  = eth_devices;

	return 0;
}

int eth_initialize(bd_t *bis)
{
	unsigned char rt2880_gmac1_mac[6];
	int eth_number = 0, regValue=0;

	eth_devices = NULL;
	eth_current = NULL;

#ifdef CONFIG_DB64360
	mv6436x_eth_initialize(bis);
#endif
#ifdef CONFIG_CPCI750
	mv6436x_eth_initialize(bis);
#endif
#ifdef CONFIG_DB64460
	mv6446x_eth_initialize(bis);
#endif
#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
  ( defined(CONFIG_440) && !defined(CONFIG_NET_MULTI) )
	ppc_4xx_eth_initialize(bis);
#endif
#if defined(CONFIG_440) && defined(CONFIG_NET_MULTI)
	ppc_440x_eth_initialize(bis);
#endif
#ifdef CONFIG_INCA_IP_SWITCH
	inca_switch_initialize(bis);
#endif
#ifdef CONFIG_PLB2800_ETHER

	plb2800_eth_initialize(bis);
#endif
#ifdef SCC_ENET
	scc_initialize(bis);
#endif
#if defined(FEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
	fec_initialize(bis);
#endif
#if defined(CONFIG_MPC5xxx_FEC)
	mpc5xxx_fec_initialize(bis);
#endif
#if defined(CONFIG_MPC8220)
	mpc8220_fec_initialize(bis);
#endif
#if defined(CONFIG_SK98)
	skge_initialize(bis);
#endif
#if defined(CONFIG_MPC85XX_TSEC1)
	tsec_initialize(bis, 0);
#endif
#if defined(CONFIG_MPC85XX_TSEC2)
	tsec_initialize(bis, 1);
#endif
#if defined(CONFIG_MPC85XX_FEC)
	tsec_initialize(bis, 2);
#endif
#if defined(CONFIG_AU1X00)
	au1x00_enet_initialize(bis);
#endif
#ifdef CONFIG_E1000
	e1000_initialize(bis);
#endif
#ifdef CONFIG_EEPRO100
	eepro100_initialize(bis);
#endif
#ifdef CONFIG_TULIP
	dc21x4x_initialize(bis);
#endif
#ifdef CONFIG_3COM
	eth_3com_initialize(bis);
#endif
#ifdef CONFIG_PCNET
	pcnet_initialize(bis);
#endif
#ifdef CFG_GT_6426x
	gt6426x_eth_initialize(bis);
#endif
#ifdef CONFIG_NATSEMI
	natsemi_initialize(bis);
#endif
#ifdef CONFIG_NS8382X
	ns8382x_initialize(bis);
#endif
#if defined(CONFIG_RTL8139)
	rtl8139_initialize(bis);
#endif
#if defined(CONFIG_RTL8169)
	rtl8169_initialize(bis);
#endif

#if defined(CONFIG_RT2880_ETH)
	rt2880_eth_initialize(bis);
#endif

	if (!eth_devices) {
		puts ("No ethernet found.\n");
	} else {
		struct eth_device *dev = eth_devices;
		char *ethprime = getenv ("ethprime");
		unsigned char empty_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


		do {
			if (eth_number)
				puts (", ");

			if (ethprime && strcmp (dev->name, ethprime) == 0) {
				eth_current = dev;
				puts (" [PRIME]");
			}

#if defined (MT7621_MP)
#define GMAC0_OFFSET	0xE000
#else
#define GMAC0_OFFSET	0x28
#endif

			//get Ethernet mac address from flash
#if defined (CFG_ENV_IS_IN_NAND)
			ranand_read(rt2880_gmac1_mac, 
				CFG_FACTORY_ADDR - CFG_FLASH_BASE + GMAC0_OFFSET, 6);
#elif defined (CFG_ENV_IS_IN_SPI)
			raspi_read(rt2880_gmac1_mac, 
				CFG_FACTORY_ADDR - CFG_FLASH_BASE + GMAC0_OFFSET, 6);
#else //CFG_ENV_IS_IN_FLASH
			memmove(rt2880_gmac1_mac, CFG_FACTORY_ADDR + GMAC0_OFFSET, 6);
#endif

			//if flash is empty, use default mac address
			if (memcmp(rt2880_gmac1_mac, empty_mac, 6) == 0)
				eth_parse_enetaddr(CONFIG_ETHADDR, rt2880_gmac1_mac);

			if (memcmp (rt2880_gmac1_mac, "\0\0\0\0\0\0", 6) == 0)
				eth_parse_enetaddr(CONFIG_ETHADDR, rt2880_gmac1_mac);

			memcpy(dev->enetaddr, rt2880_gmac1_mac, 6);

			eth_number++;
			dev = dev->next;
		} while(dev != eth_devices);

#ifdef CONFIG_NET_MULTI
		/* update current ethernet name */
		if (eth_current) {
			char *act = getenv("ethact");
			if (act == NULL || strcmp(act, eth_current->name) != 0)
				setenv("ethact", eth_current->name);
		} else
			setenv("ethact", NULL);
#endif
		//printf("\n eth_current->name = %s\n",eth_current->name);
		printf("\n");
	}

	return eth_number;
}

#ifdef CONFIG_NET_MULTI
void eth_set_enetaddr(int num, char *addr) {
	struct eth_device *dev;
	unsigned char enetaddr[6];
	char *end;
	int i;

	debug ("eth_set_enetaddr(num=%d, addr=%s)\n", num, addr);

	if (!eth_devices)
		return;

	for (i=0; i<6; i++) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end+1 : end;
	}

	dev = eth_devices;
	while(num-- > 0) {
		dev = dev->next;

		if (dev == eth_devices)
			return;
	}

	debug ( "Setting new HW address on %s\n"
		"New Address is             %02X:%02X:%02X:%02X:%02X:%02X\n",
		dev->name,
		dev->enetaddr[0], dev->enetaddr[1],
		dev->enetaddr[2], dev->enetaddr[3],
		dev->enetaddr[4], dev->enetaddr[5]);

	memcpy(dev->enetaddr, enetaddr, 6);
}
#endif
int eth_init(bd_t *bis)
{
	struct eth_device* old_current;

	if (!eth_current)
		return 0;

	old_current = eth_current;
	do {
		debug ("Trying %s\n", eth_current->name);

		if (eth_current->init(eth_current, bis)) {
			eth_current->state = ETH_STATE_ACTIVE;
			printf("\n ETH_STATE_ACTIVE!! \n");
			return 1;
		}
		printf  ("FAIL\n");
        //kaiker
		return (-1);

		eth_try_another(0);
	} while (old_current != eth_current);

	return 0;
}

void eth_halt(void)
{
	if (!eth_current)
		return;

	eth_current->halt(eth_current);

	eth_current->state = ETH_STATE_PASSIVE;
}

int eth_send(volatile void *packet, int length)
{
	if (!eth_current)
		return -1;

	return eth_current->send(eth_current, packet, length);
}

int eth_rx(void)
{
	if (!eth_current)
		return -1;

	return eth_current->recv(eth_current);
}

void eth_try_another(int first_restart)
{
	static struct eth_device *first_failed = NULL;

	if (!eth_current)
		return;

	if (first_restart) {
		first_failed = eth_current;
	}

	eth_current = eth_current->next;

#ifdef CONFIG_NET_MULTI
	/* update current ethernet name */
	{
		char *act = getenv("ethact");
		if (act == NULL || strcmp(act, eth_current->name) != 0)
			setenv("ethact", eth_current->name);
	}

	if (first_failed == eth_current) {
		NetRestartWrap = 1;
	}
#endif
}

#ifdef CONFIG_NET_MULTI
void eth_set_current(void)
{
	char *act;
	struct eth_device* old_current;

	if (!eth_current)	/* XXX no current */
		return;

	act = getenv("ethact");
	if (act != NULL) {
		old_current = eth_current;
		do {
			if (strcmp(eth_current->name, act) == 0)
				return;
			eth_current = eth_current->next;
		} while (old_current != eth_current);
	}

	setenv("ethact", eth_current->name);
}
#endif

#if defined(CONFIG_NET_MULTI)
char *eth_get_name (void)
{
	return (eth_current ? eth_current->name : "unknown");
}
#endif
#endif
