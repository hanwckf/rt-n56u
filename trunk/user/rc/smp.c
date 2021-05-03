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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "rc.h"

static void
irq_affinity_set(int irq_num, int cpu)
{
	char proc_path[40];

	snprintf(proc_path, sizeof(proc_path), "/proc/irq/%d/smp_affinity", irq_num);
	fput_int(proc_path, cpu);
}

static void
rps_queue_set(const char *ifname, int cpu_mask)
{
	char proc_path[64], cmx[4];

	snprintf(proc_path, sizeof(proc_path), "/sys/class/net/%s/queues/rx-%d/rps_cpus", ifname, 0);
	snprintf(cmx, sizeof(cmx), "%x", cpu_mask);
	fput_string(proc_path, cmx);
}

static void
xps_queue_set(const char *ifname, int cpu_mask)
{
	char proc_path[64], cmx[4];

	snprintf(proc_path, sizeof(proc_path), "/sys/class/net/%s/queues/tx-%d/xps_cpus", ifname, 0);
	snprintf(cmx, sizeof(cmx), "%x", cpu_mask);
	fput_string(proc_path, cmx);
}

struct smp_irq_layout_t {
	unsigned int irq;
	unsigned int cpu_mask;
};

static const char *rps_iflist[] = {
	IFNAME_MAC,
#if !defined (USE_SINGLE_MAC)
	IFNAME_MAC2,
#endif
	IFNAME_2G_MAIN,
	IFNAME_2G_GUEST,
	IFNAME_2G_APCLI,
	IFNAME_2G_WDS0,
	IFNAME_2G_WDS1,
	IFNAME_2G_WDS2,
	IFNAME_2G_WDS3,
#if BOARD_HAS_5G_RADIO
	IFNAME_5G_MAIN,
	IFNAME_5G_GUEST,
	IFNAME_5G_APCLI,
	IFNAME_5G_WDS0,
	IFNAME_5G_WDS1,
	IFNAME_5G_WDS2,
	IFNAME_5G_WDS3,
#endif
};

#define SMP_MASK_CPU0	(1U << 0)
#define SMP_MASK_CPU1	(1U << 1)
#define SMP_MASK_CPU2	(1U << 2)
#define SMP_MASK_CPU3	(1U << 3)

#if defined (CONFIG_RALINK_MT7621)

#define GIC_IRQ_FE	(GIC_OFFSET+3)
#define GIC_IRQ_PCIE0	(GIC_OFFSET+4)
#define GIC_IRQ_PCIE1	(GIC_OFFSET+24)
#define GIC_IRQ_PCIE2	(GIC_OFFSET+25)
#define GIC_IRQ_SDXC	(GIC_OFFSET+20)
#define GIC_IRQ_XHCI	(GIC_OFFSET+22)
#define GIC_IRQ_EIP93	(GIC_OFFSET+19)

#define LAN_RPS_MAP_4	SMP_MASK_CPU3
#define VPNC_RPS_MAP_4	SMP_MASK_CPU2
#define VPNS_RPS_MAP_4	SMP_MASK_CPU0

#define LAN_RPS_MAP_2	SMP_MASK_CPU1
#define VPNC_RPS_MAP_2	SMP_MASK_CPU1
#define VPNS_RPS_MAP_2	SMP_MASK_CPU0

static const struct smp_irq_layout_t mt7621a_irq[] = {
	{ GIC_IRQ_FE,    SMP_MASK_CPU1 },	/* GMAC  -> CPU:0, VPE:1 */
	{ GIC_IRQ_EIP93, SMP_MASK_CPU1 },	/* EIP93 -> CPU:0, VPE:1 */
	{ GIC_IRQ_PCIE0, SMP_MASK_CPU2 },	/* PCIe0 -> CPU:1, VPE:0 (usually rai0) */
#if defined (BOARD_MT7915_DBDC)
	{ GIC_IRQ_PCIE1, SMP_MASK_CPU2 },
#else
	{ GIC_IRQ_PCIE1, SMP_MASK_CPU3 },	/* PCIe1 -> CPU:1, VPE:1 (usually ra0) */
#endif
	{ GIC_IRQ_PCIE2, SMP_MASK_CPU0 },	/* PCIe2 -> CPU:0, VPE:0 (usually ahci) */
	{ GIC_IRQ_SDXC,  SMP_MASK_CPU2 },	/* SDXC  -> CPU:1, VPE:0 */
	{ GIC_IRQ_XHCI,  SMP_MASK_CPU3 },	/* xHCI  -> CPU:1, VPE:1 */
};

static const struct smp_irq_layout_t mt7621s_irq[] = {
	{ GIC_IRQ_FE,    SMP_MASK_CPU0 },	/* GMAC  -> CPU:0, VPE:0 */
	{ GIC_IRQ_EIP93, SMP_MASK_CPU0 },	/* EIP93 -> CPU:0, VPE:0 */
	{ GIC_IRQ_PCIE0, SMP_MASK_CPU1 },	/* PCIe0 -> CPU:0, VPE:1 (usually rai0) */
	{ GIC_IRQ_PCIE1, SMP_MASK_CPU0 },	/* PCIe1 -> CPU:0, VPE:0 (usually ra0) */
	{ GIC_IRQ_PCIE2, SMP_MASK_CPU0 },	/* PCIe2 -> CPU:0, VPE:0 (usually ahci) */
	{ GIC_IRQ_SDXC,  SMP_MASK_CPU1 },	/* SDXC  -> CPU:0, VPE:1 */
	{ GIC_IRQ_XHCI,  SMP_MASK_CPU1 },	/* xHCI  -> CPU:0, VPE:1 */
};

#else
#error "undefined SoC with SMP!"
#endif

void
set_cpu_affinity(int is_ap_mode)
{
	/* set initial IRQ affinity and RPS/XPS balancing */
	const struct smp_irq_layout_t *irq_map;
	int i, j, ncpu, irq_len, if_irq, rps_lan, last_cpu_mask;

	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#if defined (CONFIG_RALINK_MT7621)
	if (ncpu == 4) {
		irq_map = mt7621a_irq;
		irq_len = ARRAY_SIZE(mt7621a_irq);
		rps_lan = LAN_RPS_MAP_4;
	} else if (ncpu == 2) {
		irq_map = mt7621s_irq;
		irq_len = ARRAY_SIZE(mt7621s_irq);
		rps_lan = LAN_RPS_MAP_2;
	} else
#endif
		return;

	/* set CPU affinity */
	for (i = 0; i < irq_len; i++)
		irq_affinity_set(irq_map[i].irq, irq_map[i].cpu_mask);

	/* set network interfaces RPS/XPS mask */
	last_cpu_mask = -1;
	for (j = 0; j < ARRAY_SIZE(rps_iflist); j++) {
		if (!is_interface_exist(rps_iflist[j]))
			continue;

		if_irq = get_interface_irq(rps_iflist[j]);
		if (if_irq > 0) {
			for (i = 0; i < irq_len; i++) {
				if (irq_map[i].irq == if_irq) {
					last_cpu_mask = (int)irq_map[i].cpu_mask;
					break;
				}
			}
		}

		if (last_cpu_mask >= 0) {
			rps_queue_set(rps_iflist[j], last_cpu_mask);
			xps_queue_set(rps_iflist[j], last_cpu_mask);
		}
	}

#if !defined (USE_SINGLE_MAC)
	if (!is_ap_mode) {
		/* split MT7621 eth2 RPS in Router mode */
		if_irq = get_interface_irq(IFNAME_MAC2);
		if (if_irq <= 0) {
			rps_queue_set(IFNAME_MAC, rps_lan);
			xps_queue_set(IFNAME_MAC, rps_lan);
		}
	}
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
	else {
		/* split MT7621 eth3 RPS in non-Router mode */
		if_irq = get_interface_irq(IFNAME_MAC2);
		if (if_irq <= 0) {
			rps_queue_set(IFNAME_MAC2, rps_lan);
			xps_queue_set(IFNAME_MAC2, rps_lan);
		}
	}
#endif
#endif
}

void
set_vpn_balancing(const char *vpn_ifname, int is_server)
{
	/* set RPS/XPS balancing for PPP/SIT/TUN/TAP interfaces */
	int ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	int rps_vpn;

	switch (ncpu) {
	case 4:
		rps_vpn = (is_server) ? VPNS_RPS_MAP_4 : VPNC_RPS_MAP_4;
		break;
	case 2:
		rps_vpn = (is_server) ? VPNS_RPS_MAP_2 : VPNC_RPS_MAP_2;
		break;
	default:
		return;
	}

	rps_queue_set(vpn_ifname, rps_vpn);
	xps_queue_set(vpn_ifname, rps_vpn);
}

