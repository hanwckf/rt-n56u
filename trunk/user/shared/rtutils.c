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
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <ralink_boards.h>
#include <ralink_priv.h>

#include "nvram_linux.h"
#include "netutils.h"
#include "rtutils.h"

static const char *wifn_list[][4] = {
	{IFNAME_2G_MAIN, IFNAME_2G_GUEST, IFNAME_2G_APCLI, IFNAME_2G_WDS0},
#if BOARD_HAS_5G_RADIO
	{IFNAME_5G_MAIN, IFNAME_5G_GUEST, IFNAME_5G_APCLI, IFNAME_5G_WDS0}
#endif
};

const char *
find_wlan_if_up(int is_aband)
{
	int i, idx = 0;

#if BOARD_HAS_5G_RADIO
	if (is_aband)
		idx = 1;
#endif

	for (i = 0; i < 4; i++) {
		const char *wifn = wifn_list[idx][i];
		if (is_interface_up(wifn))
			return wifn;
	}

	return NULL;
}

#if BOARD_HAS_5G_11AC
static int
is_phy_mode_can_vht(int i_phy_mode)
{
	switch (i_phy_mode)
	{
	case PHY_11VHT_N_ABG_MIXED:
	case PHY_11VHT_N_AG_MIXED:
	case PHY_11VHT_N_A_MIXED:
	case PHY_11VHT_N_MIXED:
		return 1;
	}

	return 0;
}
#endif

static int
is_phy_mode_can_ofdm(int i_phy_mode)
{
	switch (i_phy_mode)
	{
	case PHY_11B:
		return 0;
	}

	return 1;
}

int
calc_phy_mode(int i_val, int is_aband)
{
	int i_phy_mode;

	if (!is_aband) {
		i_phy_mode = PHY_11GN_MIXED;
		switch (i_val)
		{
		case 0:  // B
			i_phy_mode = PHY_11B;
			break;
		case 1:  // B,G
			i_phy_mode = PHY_11BG_MIXED;
			break;
		case 2:  // B,G,N
			i_phy_mode = PHY_11BGN_MIXED;
			break;
		case 3:  // N
			i_phy_mode = PHY_11N;
			break;
		case 4:  // G
			i_phy_mode = PHY_11G;
			break;
		case 5:  // G,N
			i_phy_mode = PHY_11GN_MIXED;
			break;
		}
	} else {
#if BOARD_HAS_5G_11AC
		i_phy_mode = PHY_11VHT_N_A_MIXED;
#else
		i_phy_mode = PHY_11AN_MIXED;
#endif
		switch (i_val)
		{
		case 0:  // A
			i_phy_mode = PHY_11A;
			break;
		case 1:  // N
			i_phy_mode = PHY_11N_5G;
			break;
		case 2:  // A/N
			i_phy_mode = PHY_11AN_MIXED;
			break;
#if BOARD_HAS_5G_11AC
		case 3:  // N/AC
			i_phy_mode = PHY_11VHT_N_MIXED;
			break;
		case 4:  // A/N/AC
			i_phy_mode = PHY_11VHT_N_A_MIXED;
			break;
#endif
		}
	}

	return i_phy_mode;
}

int
calc_fixed_tx_mode(int i_val, int is_aband, int i_phy_mode, int *p_mcs)
{
	int i_phy = 0;		// FixedTxMode=OFF
	int i_mcs = 33;		// HT_MCS=Auto

	switch (i_val)
	{
	case 1:
		i_mcs = 2;
		i_phy = FIXED_TXMODE_HT;
		break;
	case 2:
		i_mcs = 1;
		i_phy = FIXED_TXMODE_HT;
		break;
	case 3:
		i_mcs = 0;
		i_phy = FIXED_TXMODE_HT;
		break;
	case 4:
		i_mcs = 2;
		if (is_phy_mode_can_ofdm(i_phy_mode))
			i_phy = FIXED_TXMODE_OFDM;
		else if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
		break;
	case 5:
		i_mcs = 1;
		if (is_phy_mode_can_ofdm(i_phy_mode))
			i_phy = FIXED_TXMODE_OFDM;
		else if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
		break;
	case 6:
		i_mcs = 0;
		if (is_phy_mode_can_ofdm(i_phy_mode))
			i_phy = FIXED_TXMODE_OFDM;
		else if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
		break;
	case 7:
		i_mcs = 2;
		if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
#if BOARD_HAS_5G_11AC
		else if (is_phy_mode_can_vht(i_phy_mode))
			i_phy = FIXED_TXMODE_VHT;
#endif
		else
			i_phy = FIXED_TXMODE_HT;
		break;
	case 8:
		i_mcs = 1;
		if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
#if BOARD_HAS_5G_11AC
		else if (is_phy_mode_can_vht(i_phy_mode))
			i_phy = FIXED_TXMODE_VHT;
#endif
		else
			i_phy = FIXED_TXMODE_HT;
		break;
	case 9:
		i_mcs = 0;
		if (!is_aband)
			i_phy = FIXED_TXMODE_CCK;
#if BOARD_HAS_5G_11AC
		else if (is_phy_mode_can_vht(i_phy_mode))
			i_phy = FIXED_TXMODE_VHT;
#endif
		else
			i_phy = FIXED_TXMODE_HT;
		break;
	}

	*p_mcs = i_mcs;

	return i_phy;
}

int
calc_mcast_tx_mode(int i_val, int is_aband, int *p_mmcs)
{
	int i_mphy = 2; // OFDM
	int i_mmcs = 0; // 6 Mbps

	/*	McastPhyMode, PHY mode for Multicast frames
	 *	McastMcs, MCS for Multicast frames, ranges from 0 to 15
	 *
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *
	 *	MODE=3, MCS=0: HTMIX 6.5/15Mbps
	 *	MODE=3, MCS=1: HTMIX 15/30Mbps
	 *	MODE=3, MCS=2: HTMIX 19.5/45Mbps
	 *	MODE=3, MCS=8: HTMIX 13/30Mbps
	 *	MODE=3, MCS=9: HTMIX 26/60Mbps
	 */

	switch (i_val)
	{
	case 0: // Auto
		i_mphy = 0;
		i_mmcs = 0;
		break;
	case 1: // CCK 1 Mbps
		if (!is_aband) {
			i_mphy = 1;
			i_mmcs = 0;
		}
		break;
	case 2: // CCK 2 Mbps
		if (!is_aband) {
			i_mphy = 1;
			i_mmcs = 1;
		}
		break;
	case 3: // OFDM 6 Mbps
		i_mphy = 2;
		i_mmcs = 0;
		break;
	case 4: // OFDM 9 Mbps
		i_mphy = 2;
		i_mmcs = 1;
		break;
	case 5: // OFDM 12 Mbps
		i_mphy = 2;
		i_mmcs = 2;
		break;
	case 6: // HTMIX (1S) 6.5-15 Mbps
		i_mphy = 3;
		i_mmcs = 0;
		break;
	case 7: // HTMIX (1S) 15-30 Mbps
		i_mphy = 3;
		i_mmcs = 1;
		break;
	}

	*p_mmcs = i_mmcs;

	return i_mphy;
}

void
nvram_wlan_set(int is_aband, const char *param, char *value)
{
	char wlan_param[64];
	const char *prefix = (is_aband) ? "wl" : "rt";

	snprintf(wlan_param, sizeof(wlan_param), "%s_%s", prefix, param);
	nvram_set(wlan_param, value);
}

void
nvram_wlan_set_int(int is_aband, const char *param, int value)
{
	char wlan_param[64];
	const char *prefix = (is_aband) ? "wl" : "rt";

	snprintf(wlan_param, sizeof(wlan_param), "%s_%s", prefix, param);
	nvram_set_int(wlan_param, value);
}

char*
nvram_wlan_get(int is_aband, const char *param)
{
	char wlan_param[64];
	const char *prefix = (is_aband) ? "wl" : "rt";

	snprintf(wlan_param, sizeof(wlan_param), "%s_%s", prefix, param);
	return nvram_safe_get(wlan_param);
}

int
nvram_wlan_get_int(int is_aband, const char *param)
{
	char *value = nvram_wlan_get(is_aband, param);
	if (value)
		return atoi(value);
	return 0;
}
