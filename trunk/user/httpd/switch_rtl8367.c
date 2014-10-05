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

#include <rtl8367_ioctl.h>

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

#if defined(USE_RTL8367_API_8367B)
typedef struct mib_counters_s
{
    uint64_t ifInOctets;
    uint32_t dot3StatsFCSErrors;
    uint32_t dot3StatsSymbolErrors;
    uint32_t dot3InPauseFrames;
    uint32_t dot3ControlInUnknownOpcodes;
    uint32_t etherStatsFragments;
    uint32_t etherStatsJabbers;
    uint32_t ifInUcastPkts;
    uint32_t etherStatsDropEvents;
    uint64_t etherStatsOctets;
    uint32_t etherStatsUndersizePkts;
    uint32_t etherStatsOversizePkts;
    uint32_t etherStatsPkts64Octets;
    uint32_t etherStatsPkts65to127Octets;
    uint32_t etherStatsPkts128to255Octets;
    uint32_t etherStatsPkts256to511Octets;
    uint32_t etherStatsPkts512to1023Octets;
    uint32_t etherStatsPkts1024toMaxOctets;
    uint32_t etherStatsMcastPkts;
    uint32_t etherStatsBcastPkts;
    uint64_t ifOutOctets;
    uint32_t dot3StatsSingleCollisionFrames;
    uint32_t dot3StatsMultipleCollisionFrames;
    uint32_t dot3StatsDeferredTransmissions;
    uint32_t dot3StatsLateCollisions;
    uint32_t etherStatsCollisions;
    uint32_t dot3StatsExcessiveCollisions;
    uint32_t dot3OutPauseFrames;
    uint32_t dot1dBasePortDelayExceededDiscards;
    uint32_t dot1dTpPortInDiscards;
    uint32_t ifOutUcastPkts;
    uint32_t ifOutMulticastPkts;
    uint32_t ifOutBrocastPkts;
    uint32_t outOampduPkts;
    uint32_t inOampduPkts;
    uint32_t pktgenPkts;
    uint32_t inMldChecksumError;
    uint32_t inIgmpChecksumError;
    uint32_t inMldSpecificQuery;
    uint32_t inMldGeneralQuery;
    uint32_t inIgmpSpecificQuery;
    uint32_t inIgmpGeneralQuery;
    uint32_t inMldLeaves;
    uint32_t inIgmpLeaves;
    uint32_t inIgmpJoinsSuccess;
    uint32_t inIgmpJoinsFail;
    uint32_t inMldJoinsSuccess;
    uint32_t inMldJoinsFail;
    uint32_t inReportSuppressionDrop;
    uint32_t inLeaveSuppressionDrop;
    uint32_t outIgmpReports;
    uint32_t outIgmpLeaves;
    uint32_t outIgmpGeneralQuery;
    uint32_t outIgmpSpecificQuery;
    uint32_t outMldReports;
    uint32_t outMldLeaves;
    uint32_t outMldGeneralQuery;
    uint32_t outMldSpecificQuery;
    uint32_t inKnownMulticastPkts;
    uint32_t ifInMulticastPkts;
    uint32_t ifInBroadcastPkts;
} mib_counters_t;
#else
typedef struct mib_counters_s
{
    uint64_t ifInOctets;
    uint32_t dot3StatsFCSErrors;
    uint32_t dot3StatsSymbolErrors;
    uint32_t dot3InPauseFrames;
    uint32_t dot3ControlInUnknownOpcodes;
    uint32_t etherStatsFragments;
    uint32_t etherStatsJabbers;
    uint32_t ifInUcastPkts;
    uint32_t etherStatsDropEvents;
    uint64_t etherStatsOctets;
    uint32_t etherStatsUndersizePkts;
    uint32_t etherStatsOversizePkts;
    uint32_t etherStatsPkts64Octets;
    uint32_t etherStatsPkts65to127Octets;
    uint32_t etherStatsPkts128to255Octets;
    uint32_t etherStatsPkts256to511Octets;
    uint32_t etherStatsPkts512to1023Octets;
    uint32_t etherStatsPkts1024toMaxOctets;
    uint32_t etherStatsMcastPkts;
    uint32_t etherStatsBcastPkts;
    uint64_t ifOutOctets;
    uint32_t dot3StatsSingleCollisionFrames;
    uint32_t dot3StatsMultipleCollisionFrames;
    uint32_t dot3StatsDeferredTransmissions;
    uint32_t dot3StatsLateCollisions;
    uint32_t etherStatsCollisions;
    uint32_t dot3StatsExcessiveCollisions;
    uint32_t dot3OutPauseFrames;
    uint32_t dot1dBasePortDelayExceededDiscards;
    uint32_t dot1dTpPortInDiscards;
    uint32_t ifOutUcastPkts;
    uint32_t ifOutMulticastPkts;
    uint32_t ifOutBrocastPkts;
    uint32_t outOampduPkts;
    uint32_t inOampduPkts;
    uint32_t pktgenPkts;
} mib_counters_t;
#endif

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

static int rtl8367_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(RTL8367_DEVPATH, O_RDONLY);
	if (fd < 0) {
		perror(RTL8367_DEVPATH);
		return errno;
	}

	cmd &= ((1L << RTL8367_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << RTL8367_IOCTL_CMD_LENGTH_BITS);

	if(ioctl(fd, cmd, value) < 0)
	{
		perror("ioctl");
		retVal = errno;
	}
	
	close(fd);

	return retVal;
}

int fill_eth_port_status(int port_id, char linkstate[32])
{
	unsigned int cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_WAN;
	int link_value = -1, has_link = 0;
	char *link_duplex;

	switch (port_id)
	{
	case 0:
		cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_WAN;
		break;
	case 1:
		cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN1;
		break;
	case 2:
		cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN2;
		break;
	case 3:
		cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN3;
		break;
	case 4:
		cmd = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN4;
		break;
	}

	if (rtl8367_ioctl(cmd, 0, &link_value) < 0)
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
	unsigned int cmd = RTL8367_IOCTL_STATUS_CNT_PORT_WAN;
	char etherlink[32] = {0};
	mib_counters_t mibc;

	switch (port_id)
	{
	case 0:
		cmd = RTL8367_IOCTL_STATUS_CNT_PORT_WAN;
		break;
	case 1:
		cmd = RTL8367_IOCTL_STATUS_CNT_PORT_LAN1;
		break;
	case 2:
		cmd = RTL8367_IOCTL_STATUS_CNT_PORT_LAN2;
		break;
	case 3:
		cmd = RTL8367_IOCTL_STATUS_CNT_PORT_LAN3;
		break;
	case 4:
		cmd = RTL8367_IOCTL_STATUS_CNT_PORT_LAN4;
		break;
	}

	fill_eth_port_status(port_id, etherlink);

	ret += websWrite(wp, "Port Link			: %s\n", etherlink);

	memset(&mibc, 0, sizeof(mib_counters_t));
	if (rtl8367_ioctl(cmd, 0, (unsigned int *)&mibc) == 0)
	{
		ret += websWrite(wp, "\nMIB Counters\n");
		ret += websWrite(wp, "----------------------------------------\n");
		ret += websWrite(wp, "ifInOctets			: %llu\n", mibc.ifInOctets);
		ret += websWrite(wp, "ifOutOctets			: %llu\n", mibc.ifOutOctets);
		ret += websWrite(wp, "etherStatsMcastPkts		: %u\n", mibc.etherStatsMcastPkts);
		ret += websWrite(wp, "etherStatsBcastPkts		: %u\n", mibc.etherStatsBcastPkts);
		ret += websWrite(wp, "etherStatsFragments		: %u\n", mibc.etherStatsFragments);
		ret += websWrite(wp, "etherStatsDropEvents		: %u\n", mibc.etherStatsDropEvents);
		ret += websWrite(wp, "etherStatsUndersizePkts		: %u\n", mibc.etherStatsUndersizePkts);
		ret += websWrite(wp, "etherStatsOversizePkts		: %u\n", mibc.etherStatsOversizePkts);
		ret += websWrite(wp, "etherStatsCollisions		: %u\n", mibc.etherStatsCollisions);
		ret += websWrite(wp, "dot3StatsFCSErrors		: %u\n", mibc.dot3StatsFCSErrors);
		ret += websWrite(wp, "dot3StatsSymbolErrors		: %u\n", mibc.dot3StatsSymbolErrors);
		ret += websWrite(wp, "dot3ControlInUnknownOpcodes	: %u\n", mibc.dot3ControlInUnknownOpcodes);
		ret += websWrite(wp, "dot3InPauseFrames		: %u\n", mibc.dot3InPauseFrames);
		ret += websWrite(wp, "dot3OutPauseFrames		: %u\n", mibc.dot3OutPauseFrames);
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
