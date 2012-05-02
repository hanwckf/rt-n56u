/*
################################################################################
# 
# Copyright(c) Realtek Semiconductor Corp. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# The full GNU General Public License is included in this distribution in the
# file called LICENSE.
# 
################################################################################
*/

/*
 *  This product is covered by one or more of the following patents:
 *  US5,307,459, US5,434,872, US5,732,094, US6,570,884, US6,115,776, and US6,327,625.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>

#include "r8168.h"
#include "r8168_asf.h"

extern int rtl8168_eri_read(void __iomem *ioaddr, int addr, int len, int type);
extern int rtl8168_eri_write(void __iomem *ioaddr, int addr, int len, int value, int type);

int rtl8168_asf_ioctl(struct net_device *dev,
		      struct ifreq *ifr)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	void *user_data = ifr->ifr_data;
	struct asf_ioctl_struct asf_usrdata;

	if (tp->mcfg != CFG_METHOD_7 && tp->mcfg != CFG_METHOD_8)
		return -EOPNOTSUPP;

	if (copy_from_user(&asf_usrdata, user_data, sizeof(struct asf_ioctl_struct)))
		return -EFAULT;

	switch (asf_usrdata.offset) {
	case HBPeriod:
		rtl8168_asf_hbperiod(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case WD8Timer:
		break;
	case WD16Rst:
		rtl8168_asf_wd16rst(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case WD8Rst:
		rtl8168_asf_time_period(ioaddr, asf_usrdata.arg, WD8Rst, asf_usrdata.u.data);
		break;
	case LSnsrPollCycle:
		rtl8168_asf_time_period(ioaddr, asf_usrdata.arg, LSnsrPollCycle, asf_usrdata.u.data);
		break;
	case ASFSnsrPollPrd:
		rtl8168_asf_time_period(ioaddr, asf_usrdata.arg, ASFSnsrPollPrd, asf_usrdata.u.data);
		break;
	case AlertReSendItvl:
		rtl8168_asf_time_period(ioaddr, asf_usrdata.arg, AlertReSendItvl, asf_usrdata.u.data);
		break;
	case SMBAddr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, SMBAddr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case ASFConfigR0:
		rtl8168_asf_config_regs(ioaddr, asf_usrdata.arg, ASFConfigR0, asf_usrdata.u.data);
		break;
	case ASFConfigR1:
		rtl8168_asf_config_regs(ioaddr, asf_usrdata.arg, ASFConfigR1, asf_usrdata.u.data);
		break;
	case ConsoleMA:
		rtl8168_asf_console_mac(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case ConsoleIP:
		rtl8168_asf_ip_address(ioaddr, asf_usrdata.arg, ConsoleIP, asf_usrdata.u.data);
		break;
	case IPAddr:
		rtl8168_asf_ip_address(ioaddr, asf_usrdata.arg, IPAddr, asf_usrdata.u.data);
		break;
	case UUID:
		rtl8168_asf_rw_uuid(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case IANA:
		rtl8168_asf_rw_iana(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case SysID:
		rtl8168_asf_rw_systemid(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case Community:
		rtl8168_asf_community_string(ioaddr, asf_usrdata.arg, asf_usrdata.u.string);
		break;
	case StringLength:
		rtl8168_asf_community_string_len(ioaddr, asf_usrdata.arg, asf_usrdata.u.data);
		break;
	case FmCapMsk:
		rtl8168_asf_capability_masks(ioaddr, asf_usrdata.arg, FmCapMsk, asf_usrdata.u.data);
		break;
	case SpCMDMsk:
		rtl8168_asf_capability_masks(ioaddr, asf_usrdata.arg, SpCMDMsk, asf_usrdata.u.data);
		break;
	case SysCapMsk:
		rtl8168_asf_capability_masks(ioaddr, asf_usrdata.arg, SysCapMsk, asf_usrdata.u.data);
		break;
	case RmtRstAddr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtRstAddr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtRstCmd:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtRstCmd, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtRstData:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtRstData, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOffAddr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOffAddr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOffCmd:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOffCmd, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOffData:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOffData, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOnAddr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOnAddr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOnCmd:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOnCmd, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPwrOnData:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPwrOnData, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPCRAddr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPCRAddr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPCRCmd:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPCRCmd, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case RmtPCRData:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, RmtPCRData, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case ASFSnsr0Addr:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, ASFSnsr0Addr, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case LSnsrAddr0:
		rtl8168_asf_rw_hexadecimal(ioaddr, asf_usrdata.arg, LSnsrAddr0, RW_ONE_BYTE, asf_usrdata.u.data);
		break;
	case KO:
		/* Get/Set Key Operation */
		rtl8168_asf_key_access(ioaddr, asf_usrdata.arg, KO, asf_usrdata.u.data);
		break;
	case KA:
		/* Get/Set Key Administrator */
		rtl8168_asf_key_access(ioaddr, asf_usrdata.arg, KA, asf_usrdata.u.data);
		break;
	case KG:
		/* Get/Set Key Generation */
		rtl8168_asf_key_access(ioaddr, asf_usrdata.arg, KG, asf_usrdata.u.data);
		break;
	case KR:
		/* Get/Set Key Random */
		rtl8168_asf_key_access(ioaddr, asf_usrdata.arg, KR, asf_usrdata.u.data);
		break;
	default:
		return -EOPNOTSUPP;		
	}

	if (copy_to_user(user_data, &asf_usrdata, sizeof(struct asf_ioctl_struct)))
		return -EFAULT;

	return 0;
}

void rtl8168_asf_hbperiod(void __iomem *ioaddr, int arg, unsigned int *data)
{
	if (arg == ASF_GET)
		data[ASFHBPERIOD] = rtl8168_eri_read(ioaddr, HBPeriod, RW_TWO_BYTES, ERIAR_ASF);
	else if (arg == ASF_SET) {
		rtl8168_eri_write(ioaddr, HBPeriod, RW_TWO_BYTES, data[ASFHBPERIOD], ERIAR_ASF);
		rtl8168_eri_write(ioaddr, 0x1EC, RW_ONE_BYTE, 0x07, ERIAR_ASF);
	}
}

void rtl8168_asf_wd16rst(void __iomem *ioaddr, int arg, unsigned int *data)
{
	data[ASFWD16RST] = rtl8168_eri_read(ioaddr, WD16Rst, RW_TWO_BYTES, ERIAR_ASF);
}

void rtl8168_asf_console_mac(void __iomem *ioaddr, int arg, unsigned int *data)
{
	int i;

	if (arg == ASF_GET) {
		for (i = 0; i < 6; i++)
			data[i] = rtl8168_eri_read(ioaddr, ConsoleMA + i, RW_ONE_BYTE, ERIAR_ASF);
	} else if (arg == ASF_SET) {
		for (i = 0; i < 6; i++)
			rtl8168_eri_write(ioaddr, ConsoleMA + i, RW_ONE_BYTE, data[i], ERIAR_ASF);
	}
}

void rtl8168_asf_ip_address(void __iomem *ioaddr, int arg, int offset, unsigned int *data)
{
	int i;

	if (arg == ASF_GET) {
		for (i = 0; i < 4; i++)
			data[i] = rtl8168_eri_read(ioaddr, offset + i, RW_ONE_BYTE, ERIAR_ASF);
	} else if (arg == ASF_SET) {
		for (i = 0; i < 4; i++)
			rtl8168_eri_write(ioaddr, offset + i, RW_ONE_BYTE, data[i], ERIAR_ASF);
	}
}

void rtl8168_asf_config_regs(void __iomem *ioaddr, int arg, int offset, unsigned int *data)
{
	unsigned int value;

	if (arg == ASF_GET) {
		data[ASFCAPABILITY] = (rtl8168_eri_read(ioaddr, offset, RW_ONE_BYTE, ERIAR_ASF) & data[ASFCONFIG]) ? FUNCTION_ENABLE : FUNCTION_DISABLE;
	} else if (arg == ASF_SET) {
		value = rtl8168_eri_read(ioaddr, offset, RW_ONE_BYTE, ERIAR_ASF);

		if (data[ASFCAPABILITY] == FUNCTION_ENABLE)
			value |= data[ASFCONFIG];
		else if (data[ASFCAPABILITY] == FUNCTION_DISABLE)
			value &= ~data[ASFCONFIG];

		rtl8168_eri_write(ioaddr, offset, RW_ONE_BYTE, value, ERIAR_ASF);
	}
}

void rtl8168_asf_capability_masks(void __iomem *ioaddr, int arg, int offset, unsigned int *data)
{
	unsigned int len, bit_mask;

	bit_mask = DISABLE_MASK;

	if (offset == FmCapMsk) {
		/* System firmware capabilities */
		len = RW_FOUR_BYTES;
		if (data[ASFCAPMASK] == FUNCTION_ENABLE)
			bit_mask = FMW_CAP_MASK;
	} else if (offset == SpCMDMsk) {
		/* Special commands */
		len = RW_TWO_BYTES;
		if (data[ASFCAPMASK] == FUNCTION_ENABLE)
			bit_mask = SPC_CMD_MASK;
	} else {
		/* System capability (offset == SysCapMsk)*/
		len = RW_ONE_BYTE;
		if (data[ASFCAPMASK] == FUNCTION_ENABLE)
			bit_mask = SYS_CAP_MASK;
	}

	if (arg == ASF_GET)
		data[ASFCAPMASK] = rtl8168_eri_read(ioaddr, offset, len, ERIAR_ASF) ? FUNCTION_ENABLE : FUNCTION_DISABLE;
	else /* arg == ASF_SET */
		rtl8168_eri_write(ioaddr, offset, len, bit_mask, ERIAR_ASF);
}

void rtl8168_asf_community_string(void __iomem *ioaddr, int arg, char *string)
{
	int i;

	if (arg == ASF_GET) {
		for (i = 0; i < COMMU_STR_MAX_LEN; i++)
			string[i] = rtl8168_eri_read(ioaddr, Community + i, RW_ONE_BYTE, ERIAR_ASF);
	} else { /* arg == ASF_SET */
		for (i = 0; i < COMMU_STR_MAX_LEN; i++)
			rtl8168_eri_write(ioaddr, Community + i, RW_ONE_BYTE, string[i], ERIAR_ASF);
	}
}

void rtl8168_asf_community_string_len(void __iomem *ioaddr, int arg, unsigned int *data)
{
	if (arg == ASF_GET)
		data[ASFCOMMULEN] = rtl8168_eri_read(ioaddr, StringLength, RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		rtl8168_eri_write(ioaddr, StringLength, RW_ONE_BYTE, data[ASFCOMMULEN], ERIAR_ASF);
}

void rtl8168_asf_time_period(void __iomem *ioaddr, int arg, int offset, unsigned int *data)
{
	int pos;

	if (offset == WD8Rst)
		pos = ASFWD8RESET;
	else if (offset == LSnsrPollCycle)
		pos = ASFLSNRPOLLCYC;
	else if (offset == ASFSnsrPollPrd)
		pos = ASFSNRPOLLCYC;
	else if (offset == AlertReSendItvl)
		pos = ASFALERTRESND;

	if (arg == ASF_GET)
		data[pos] = rtl8168_eri_read(ioaddr, offset, RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		rtl8168_eri_write(ioaddr, offset, RW_ONE_BYTE, data[pos], ERIAR_ASF);

}

void rtl8168_asf_key_access(void __iomem *ioaddr, int arg, int offset, unsigned int *data)
{
	int i;

	if (arg == ASF_GET)
		for (i = 0; i < KEY_LEN; i++)
			data[i] = rtl8168_eri_read(ioaddr, offset + KEY_LEN - (i + 1), RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		for (i = 0; i < KEY_LEN; i++)
			rtl8168_eri_write(ioaddr, offset + KEY_LEN - (i + 1), RW_ONE_BYTE, data[i], ERIAR_ASF);
}

void rtl8168_asf_rw_hexadecimal(void __iomem *ioaddr, int arg, int offset, int len, unsigned int *data)
{
	if (arg == ASF_GET)
		data[ASFRWHEXNUM] = rtl8168_eri_read(ioaddr, offset, len, ERIAR_ASF);
	else /* arg == ASF_SET */
		rtl8168_eri_write(ioaddr, offset, len, data[ASFRWHEXNUM], ERIAR_ASF);
}

void rtl8168_asf_rw_systemid(void __iomem *ioaddr, int arg, unsigned int *data)
{
	int i;

	if (arg == ASF_GET)
		for (i = 0; i < SYSID_LEN ; i++)
			data[i] = rtl8168_eri_read(ioaddr, SysID + i, RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		for (i = 0; i < SYSID_LEN ; i++)
			rtl8168_eri_write(ioaddr, SysID + i, RW_ONE_BYTE, data[i], ERIAR_ASF);
}

void rtl8168_asf_rw_iana(void __iomem *ioaddr, int arg, unsigned int *data)
{
	int i;

	if (arg == ASF_GET)
		for (i = 0; i < RW_FOUR_BYTES; i++)
			data[i] = rtl8168_eri_read(ioaddr, IANA + i, RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		for (i = 0; i < RW_FOUR_BYTES; i++)
			rtl8168_eri_write(ioaddr, IANA + i, RW_ONE_BYTE, data[i], ERIAR_ASF);
}

void rtl8168_asf_rw_uuid(void __iomem *ioaddr, int arg, unsigned int *data)
{
	int i, j;

	if (arg == ASF_GET)
		for (i = UUID_LEN - 1, j = 0; i >= 0 ; i--, j++)
			data[j] = rtl8168_eri_read(ioaddr, UUID + i, RW_ONE_BYTE, ERIAR_ASF);
	else /* arg == ASF_SET */
		for (i = UUID_LEN - 1, j = 0; i >= 0 ; i--, j++)
			rtl8168_eri_write(ioaddr, UUID + i, RW_ONE_BYTE, data[j], ERIAR_ASF);
}
