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

#if defined (CONFIG_RALINK_MT7621)
void
set_cpu_affinity(int is_ap_mode)
{
	/* set initial IRQ affinity and RPS/XPS balancing */
	int ncpu = sysconf(_SC_NPROCESSORS_ONLN);

#define GIC_IRQ_FE	(GIC_OFFSET+3)
#define GIC_IRQ_PCIE0	(GIC_OFFSET+4)
#define GIC_IRQ_PCIE1	(GIC_OFFSET+24)
#define GIC_IRQ_PCIE2	(GIC_OFFSET+25)
#define GIC_IRQ_SDXC	(GIC_OFFSET+20)
#define GIC_IRQ_XHCI	(GIC_OFFSET+22)
#define GIC_IRQ_EIP93	(GIC_OFFSET+19)

	if (ncpu == 4) {
		irq_affinity_set(GIC_IRQ_FE,    2);	/* GMAC  -> CPU:0, VPE:1 */
		irq_affinity_set(GIC_IRQ_PCIE0, 4);	/* PCIe0 -> CPU:1, VPE:0 (usually rai0) */
		irq_affinity_set(GIC_IRQ_PCIE1, 8);	/* PCIe1 -> CPU:1, VPE:1 (usually ra0) */
		irq_affinity_set(GIC_IRQ_PCIE2, 1);	/* PCIe2 -> CPU:0, VPE:0 (usually ahci) */
		irq_affinity_set(GIC_IRQ_SDXC,  4);	/* SDXC  -> CPU:1, VPE:0 */
		irq_affinity_set(GIC_IRQ_XHCI,  8);	/* xHCI  -> CPU:1, VPE:1 */
		irq_affinity_set(GIC_IRQ_EIP93, 8);	/* EIP93 -> CPU:1, VPE:1 */
		
		rps_queue_set(IFNAME_2G_MAIN,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_MAIN,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_GUEST, 0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_GUEST, 0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_APCLI, 0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_APCLI, 0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS0,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS0,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS1,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS1,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS2,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS2,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS3,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS3,  0x8);	/* CPU:1, VPE:1 (PCIe1) */
#if BOARD_HAS_5G_RADIO
		rps_queue_set(IFNAME_5G_MAIN,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_MAIN,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_GUEST, 0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_GUEST, 0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_APCLI, 0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_APCLI, 0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS0,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS0,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS1,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS1,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS2,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS2,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS3,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS3,  0x4);	/* CPU:1, VPE:0 (PCIe0) */
#endif
		if (is_ap_mode) {
			rps_queue_set(IFNAME_MAC, 0x3);	/* CPU:0, VPE:0+1 */
			xps_queue_set(IFNAME_MAC, 0x3);	/* CPU:0, VPE:0+1 */
		} else {
			rps_queue_set(IFNAME_LAN, 0x2);	/* CPU:0, VPE:1 */
			xps_queue_set(IFNAME_LAN, 0x2);	/* CPU:0, VPE:1 */
			rps_queue_set(IFNAME_WAN, 0x2);	/* CPU:0, VPE:1 */
			xps_queue_set(IFNAME_WAN, 0x2);	/* CPU:0, VPE:1 */
#if defined (USE_SINGLE_MAC)
			rps_queue_set(IFNAME_MAC, 0x2);	/* CPU:0, VPE:1 */
			xps_queue_set(IFNAME_MAC, 0x2);	/* CPU:0, VPE:1 */
#endif
		}
		
	} else if (ncpu == 2) {
		irq_affinity_set(GIC_IRQ_FE,    1);	/* GMAC  -> CPU:0, VPE:0 */
		irq_affinity_set(GIC_IRQ_PCIE0, 2);	/* PCIe0 -> CPU:0, VPE:1 (usually rai0) */
		irq_affinity_set(GIC_IRQ_PCIE1, 1);	/* PCIe1 -> CPU:0, VPE:0 (usually ra0) */
		irq_affinity_set(GIC_IRQ_PCIE2, 1);	/* PCIe2 -> CPU:0, VPE:0 (usually ahci) */
		irq_affinity_set(GIC_IRQ_SDXC,  2);	/* SDXC  -> CPU:0, VPE:1 */
		irq_affinity_set(GIC_IRQ_XHCI,  2);	/* xHCI  -> CPU:0, VPE:1 */
		irq_affinity_set(GIC_IRQ_EIP93, 2);	/* EIP93 -> CPU:0, VPE:1 */
		
		rps_queue_set(IFNAME_2G_MAIN,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_MAIN,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_GUEST, 0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_GUEST, 0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_APCLI, 0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_APCLI, 0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS0,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS0,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS1,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS1,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS2,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS2,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		rps_queue_set(IFNAME_2G_WDS3,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
		xps_queue_set(IFNAME_2G_WDS3,  0x1);	/* CPU:0, VPE:0 (PCIe1) */
#if BOARD_HAS_5G_RADIO
		rps_queue_set(IFNAME_5G_MAIN,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_MAIN,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_GUEST, 0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_GUEST, 0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_APCLI, 0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_APCLI, 0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS0,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS0,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS1,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS1,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS2,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS2,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		rps_queue_set(IFNAME_5G_WDS3,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
		xps_queue_set(IFNAME_5G_WDS3,  0x2);	/* CPU:0, VPE:1 (PCIe0) */
#endif
		if (is_ap_mode) {
			rps_queue_set(IFNAME_MAC, 0x3);	/* CPU:0, VPE:0+1 */
			xps_queue_set(IFNAME_MAC, 0x3);	/* CPU:0, VPE:0+1 */
		} else {
			rps_queue_set(IFNAME_LAN, 0x1);	/* CPU:0, VPE:0 (GMAC) */
			xps_queue_set(IFNAME_LAN, 0x1);	/* CPU:0, VPE:0 (GMAC) */
			rps_queue_set(IFNAME_WAN, 0x1);	/* CPU:0, VPE:0 (GMAC) */
			xps_queue_set(IFNAME_WAN, 0x1);	/* CPU:0, VPE:0 (GMAC) */
#if defined (USE_SINGLE_MAC)
			rps_queue_set(IFNAME_MAC, 0x1);	/* CPU:0, VPE:0 (GMAC) */
			xps_queue_set(IFNAME_MAC, 0x1);	/* CPU:0, VPE:0 (GMAC) */
#endif
		}
	}
}

void
set_vpn_balancing(const char *vpn_ifname)
{
	/* set RPS/XPS balancing for PPP/SIT/TUN/TAP interfaces */
	int ncpu = sysconf(_SC_NPROCESSORS_ONLN);

	if (ncpu == 4) {
		rps_queue_set(vpn_ifname, 0x8);	/* CPU:1, VPE:1 */
		xps_queue_set(vpn_ifname, 0x8);	/* CPU:1, VPE:1 */
	} else if (ncpu == 2) {
		rps_queue_set(vpn_ifname, 0x2);	/* CPU:0, VPE:1 */
		xps_queue_set(vpn_ifname, 0x2);	/* CPU:0, VPE:1 */
	}
}

#else
#error "undefined SoC with SMP!"
#endif

