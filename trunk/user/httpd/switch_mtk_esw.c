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
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "httpd.h"

#include <ra_esw_ioctl.h>

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

#if defined (USE_MTK_GSW)
typedef struct esw_mib_counters_s
{
	uint64_t TxGoodOctets;
	uint32_t TxUcastFrames;
	uint32_t TxMcastFrames;
	uint32_t TxBcastFrames;
	uint32_t TxDropFrames;
	uint32_t TxCollisions;
	uint32_t TxCRCError;
	uint64_t RxGoodOctets;
	uint32_t RxUcastFrames;
	uint32_t RxMcastFrames;
	uint32_t RxBcastFrames;
	uint32_t RxDropFrames;
	uint32_t RxFilterFrames;
	uint32_t RxCRCError;
	uint32_t RxAligmentError;
} esw_mib_counters_t;
#else
typedef struct esw_mib_counters_s
{
	uint64_t TxGoodOctets;
	uint32_t TxGoodFrames;
	uint32_t TxBadOctets;
	uint32_t TxBadFrames;
	uint32_t TxDropFrames;
	uint64_t RxGoodOctets;
	uint32_t RxGoodFrames;
	uint32_t RxBadOctets;
	uint32_t RxBadFrames;
	uint32_t RxDropFramesFilter;
	uint32_t RxDropFramesErr;
} esw_mib_counters_t;
#endif

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

int mtk_esw_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(MTK_ESW_DEVPATH, O_RDONLY);
	if (fd < 0)
	{
		perror(MTK_ESW_DEVPATH);
		return errno;
	}

	cmd &= ((1L << MTK_ESW_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << MTK_ESW_IOCTL_CMD_LENGTH_BITS);

	if(ioctl(fd, cmd, value) < 0)
	{
		perror("ioctl");
		retVal = errno;
	}

	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

int fill_eth_port_status(int port_id, char linkstate[32])
{
	unsigned int cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN;
	int link_value = -1, has_link = 0;
	char *link_duplex;

	switch (port_id)
	{
	case 0:
		cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN;
		break;
	case 1:
		cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1;
		break;
	case 2:
		cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2;
		break;
	case 3:
		cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3;
		break;
	case 4:
		cmd = MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4;
		break;
	}

	if (mtk_esw_ioctl(cmd, 0, &link_value) < 0)
		link_value = -1;

	if (link_value >= 0)
	{
		if ((link_value >> 16) & 0x01)
		{
			has_link = 1;
			
			if ((link_value >> 8) & 0x01)
				link_duplex = "Full Duplex";
			else
				link_duplex = "Half Duplex";
			
			switch (link_value & 0x03)
			{
			case 3:
			case 2:
				link_value = 1000;
				break;
			case 1:
				link_value = 100;
				break;
			default:
				link_value = 10;
				break;
			}
			
			sprintf(linkstate, "%d Mbps, %s", link_value, link_duplex);
		}
		else
		{
			sprintf(linkstate, "No Link");
		}
	}
	else
	{
		sprintf(linkstate, "I/O Error");
	}

	return has_link;
}

static int fill_eth_status(int port_id, webs_t wp)
{
	int ret = 0;
	unsigned int cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN;
	esw_mib_counters_t mibc;
	char etherlink[32] = {0};

	switch (port_id)
	{
	case 0:
		cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN;
		break;
	case 1:
		cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1;
		break;
	case 2:
		cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2;
		break;
	case 3:
		cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3;
		break;
	case 4:
		cmd = MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4;
		break;
	}

	fill_eth_port_status(port_id, etherlink);

	ret += websWrite(wp, "Port Link			: %s\n", etherlink);

	memset(&mibc, 0, sizeof(esw_mib_counters_t));
	if (mtk_esw_ioctl(cmd, 0, (unsigned int *)&mibc) == 0)
	{
		ret += websWrite(wp, "\nMIB Counters\n");
		ret += websWrite(wp, "----------------------------------------\n");
#if defined (USE_MTK_GSW)
		ret += websWrite(wp, "TxGoodOctets			: %llu\n", mibc.TxGoodOctets);
		ret += websWrite(wp, "TxUcastFrames			: %u\n", mibc.TxUcastFrames);
		ret += websWrite(wp, "TxMcastFrames			: %u\n", mibc.TxMcastFrames);
		ret += websWrite(wp, "TxBcastFrames			: %u\n", mibc.TxBcastFrames);
		ret += websWrite(wp, "TxDropFrames			: %u\n", mibc.TxDropFrames);
		ret += websWrite(wp, "TxCollisions			: %u\n", mibc.TxCollisions);
		ret += websWrite(wp, "TxCRCError			: %u\n", mibc.TxCRCError);
		ret += websWrite(wp, "RxGoodOctets			: %llu\n", mibc.RxGoodOctets);
		ret += websWrite(wp, "RxUcastFrames			: %u\n", mibc.RxUcastFrames);
		ret += websWrite(wp, "RxMcastFrames			: %u\n", mibc.RxMcastFrames);
		ret += websWrite(wp, "RxBcastFrames			: %u\n", mibc.RxBcastFrames);
		ret += websWrite(wp, "RxDropFrames			: %u\n", mibc.RxDropFrames);
		ret += websWrite(wp, "RxFilterFrames			: %u\n", mibc.RxFilterFrames);
		ret += websWrite(wp, "RxCRCError			: %u\n", mibc.RxCRCError);
		ret += websWrite(wp, "RxAligmentError			: %u", mibc.RxAligmentError);
#else
		ret += websWrite(wp, "TxGoodOctets			: %llu\n", mibc.TxGoodOctets);
		ret += websWrite(wp, "TxGoodFrames			: %u\n", mibc.TxGoodFrames);
		ret += websWrite(wp, "TxBadOctets			: %u\n", mibc.TxBadOctets);
		ret += websWrite(wp, "TxBadFrames			: %u\n", mibc.TxBadFrames);
		ret += websWrite(wp, "TxDropFrames			: %u\n", mibc.TxDropFrames);
		ret += websWrite(wp, "RxGoodOctets			: %llu\n", mibc.RxGoodOctets);
		ret += websWrite(wp, "RxGoodFrames			: %u\n", mibc.RxGoodFrames);
		ret += websWrite(wp, "RxBadOctets			: %u\n", mibc.RxBadOctets);
		ret += websWrite(wp, "RxBadFrames			: %u\n", mibc.RxBadFrames);
		ret += websWrite(wp, "RxDropFramesFilter		: %u\n", mibc.RxDropFramesFilter);
		ret += websWrite(wp, "RxDropFramesErr			: %u", mibc.RxDropFramesErr);
#endif
	}

	return ret;
}

int ej_eth_status_wan(int eid, webs_t wp, int argc, char **argv)
{
	return fill_eth_status(0, wp);
}

int ej_eth_status_lan1(int eid, webs_t wp, int argc, char **argv)
{
	return fill_eth_status(1, wp);
}

int ej_eth_status_lan2(int eid, webs_t wp, int argc, char **argv)
{
	return fill_eth_status(2, wp);
}

int ej_eth_status_lan3(int eid, webs_t wp, int argc, char **argv)
{
	return fill_eth_status(3, wp);
}

int ej_eth_status_lan4(int eid, webs_t wp, int argc, char **argv)
{
	return fill_eth_status(4, wp);
}

