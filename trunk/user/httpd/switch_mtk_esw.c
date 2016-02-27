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

#include <mtk_esw/ioctl.h>

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
	uint32_t TxPauseFrames;
	uint32_t TxCollisions;
	uint32_t TxCRCError;
	uint64_t RxGoodOctets;
	uint32_t RxUcastFrames;
	uint32_t RxMcastFrames;
	uint32_t RxBcastFrames;
	uint32_t RxDropFrames;
	uint32_t RxPauseFrames;
	uint32_t RxFilterFrames;
	uint32_t RxCRCError;
	uint32_t RxAligmentError;
} esw_mib_counters_t;
#elif defined (CONFIG_RALINK_MT7620)
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
#else
#define ESW_RT3X5X
typedef struct esw_mib_counters_s
{
	uint32_t TxGoodFrames;
	uint32_t TxBadFrames;
	uint32_t RxGoodFrames;
	uint32_t RxBadFrames;
} esw_mib_counters_t;
#endif

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

int mtk_esw_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, retVal = 0;

	fd = open(MTK_ESW_DEVPATH, O_RDONLY);
	if (fd < 0) {
		perror(MTK_ESW_DEVPATH);
		return errno;
	}

	cmd &= ((1L << MTK_ESW_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << MTK_ESW_IOCTL_CMD_LENGTH_BITS);

	if (ioctl(fd, cmd, value) < 0) {
		perror("ioctl");
		retVal = errno;
	}

	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

int get_eth_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx)
{
	port_bytes_t pb;
	int ioctl_result;

	ioctl_result = mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_BYTES_PORT, port_id_uapi, (unsigned int *)&pb);
	if (ioctl_result < 0)
		return ioctl_result;

	*rx = pb.RX;
	*tx = pb.TX;

	return ioctl_result;
}

int fill_eth_port_status(int port_id_uapi, char linkstate[40])
{
	unsigned int link_value = 0;
	int has_link = 0;

	if (mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_SPEED_PORT, port_id_uapi, &link_value) < 0) {
		strcpy(linkstate, "I/O Error");
		return 0;
	}

	has_link = (link_value >> 16) & 0x01;

	if (has_link) {
		int lspeed;
		const char *text_fc = "";
		const char *text_dup = "Half Duplex";
		const char *text_eee = "";
		
		switch (link_value & 0x03)
		{
		case 3:
		case 2:
			lspeed = 1000;
			break;
		case 1:
			lspeed = 100;
			break;
		default:
			lspeed = 10;
			break;
		}
		
		if ((link_value >> 8) & 0x01) {
			unsigned int link_fc = (link_value >> 9) & 0x03;
			if (link_fc == 0x03)
				text_fc = "TX/RX";
			else if (link_fc == 0x01)
				text_fc = "TX Asy";
			else if (link_fc == 0x02)
				text_fc = "RX Asy";
			else
				text_fc = "OFF";
			text_dup = "Full Duplex, FC ";
		}
		
		if ((link_value >> 11) & 0x03)
			text_eee = ", EEE";
		
		/* 1000 Mbps, Full Duplex, FC TX/RX, EEE */
		sprintf(linkstate, "%d Mbps, %s%s%s", lspeed, text_dup, text_fc, text_eee);
	} else
		strcpy(linkstate, "No Link");

	return has_link;
}

int fill_eth_status(int port_id_uapi, webs_t wp)
{
	int ret = 0;
	esw_mib_counters_t mibc;
	char etherlink[40] = {0};

	fill_eth_port_status(port_id_uapi, etherlink);

	ret += websWrite(wp, "Port Link			: %s\n", etherlink);

	memset(&mibc, 0, sizeof(esw_mib_counters_t));
	if (mtk_esw_ioctl(MTK_ESW_IOCTL_STATUS_MIB_PORT, port_id_uapi, (unsigned int *)&mibc) == 0) {
		ret += websWrite(wp, "\nMIB Counters\n");
		ret += websWrite(wp, "----------------------------------------\n");
#if defined (USE_MTK_GSW)
		ret += websWrite(wp, "TxGoodOctets			: %llu\n", mibc.TxGoodOctets);
		ret += websWrite(wp, "TxUcastFrames			: %u\n", mibc.TxUcastFrames);
		ret += websWrite(wp, "TxMcastFrames			: %u\n", mibc.TxMcastFrames);
		ret += websWrite(wp, "TxBcastFrames			: %u\n", mibc.TxBcastFrames);
		ret += websWrite(wp, "TxDropFrames			: %u\n", mibc.TxDropFrames);
		ret += websWrite(wp, "TxPauseFrames			: %u\n", mibc.TxPauseFrames);
		ret += websWrite(wp, "TxCollisions			: %u\n", mibc.TxCollisions);
		ret += websWrite(wp, "TxCRCError			: %u\n", mibc.TxCRCError);
		ret += websWrite(wp, "RxGoodOctets			: %llu\n", mibc.RxGoodOctets);
		ret += websWrite(wp, "RxUcastFrames			: %u\n", mibc.RxUcastFrames);
		ret += websWrite(wp, "RxMcastFrames			: %u\n", mibc.RxMcastFrames);
		ret += websWrite(wp, "RxBcastFrames			: %u\n", mibc.RxBcastFrames);
		ret += websWrite(wp, "RxDropFrames			: %u\n", mibc.RxDropFrames);
		ret += websWrite(wp, "RxPauseFrames			: %u\n", mibc.RxPauseFrames);
		ret += websWrite(wp, "RxFilterFrames			: %u\n", mibc.RxFilterFrames);
		ret += websWrite(wp, "RxCRCError			: %u\n", mibc.RxCRCError);
		ret += websWrite(wp, "RxAligmentError			: %u", mibc.RxAligmentError);
#elif !defined (ESW_RT3X5X)
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
#else
		ret += websWrite(wp, "TxGoodFrames			: %u\n", mibc.TxGoodFrames);
		ret += websWrite(wp, "TxBadFrames			: %u\n", mibc.TxBadFrames);
		ret += websWrite(wp, "RxGoodFrames			: %u\n", mibc.RxGoodFrames);
		ret += websWrite(wp, "RxBadFrames			: %u\n", mibc.RxBadFrames);
#endif
	}

	return ret;
}
