/*
    Module Name:
    ac_ioctl.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-02-15      Initial version
*/

#ifndef	__AC_IOCTL_H__
#define	__AC_IOCTL_H__

#define AC_ADD_MAC_UL_ENTRY		(0)
#define AC_ADD_MAC_DL_ENTRY		(1)
#define AC_DEL_MAC_UL_ENTRY		(2)
#define AC_DEL_MAC_DL_ENTRY		(3)
#define AC_ADD_IP_UL_ENTRY		(4)
#define AC_ADD_IP_DL_ENTRY		(5)
#define AC_DEL_IP_UL_ENTRY		(6)
#define AC_DEL_IP_DL_ENTRY		(7)
#define AC_ADD_VLAN_UL_ENTRY		(8)
#define AC_ADD_VLAN_DL_ENTRY		(9)
#define AC_DEL_VLAN_UL_ENTRY		(10)
#define AC_DEL_VLAN_DL_ENTRY		(11)
#define AC_GET_MAC_UL_PKT_CNT		(12)
#define AC_GET_MAC_DL_PKT_CNT		(13)
#define AC_GET_MAC_UL_BYTE_CNT		(14)
#define AC_GET_MAC_DL_BYTE_CNT		(15)
#define AC_GET_IP_UL_PKT_CNT		(16)
#define AC_GET_IP_DL_PKT_CNT		(17)
#define AC_GET_IP_UL_BYTE_CNT		(18)
#define AC_GET_IP_DL_BYTE_CNT		(19)
#define AC_GET_VLAN_UL_PKT_CNT		(20)
#define AC_GET_VLAN_DL_PKT_CNT		(21)
#define AC_GET_VLAN_UL_BYTE_CNT		(22)
#define AC_GET_VLAN_DL_BYTE_CNT		(23)
#define AC_CLEAN_TBL			(24)

#define AC_DEVNAME                     "ac0"
#define AC_MAJOR                       (240)

enum AcResult {
	AC_SUCCESS = 0,
	AC_FAIL = 1,
	AC_TBL_FULL = 2
};

struct ac_args {
	unsigned char mac[6];
	unsigned short vid:12;	/* VID */
	unsigned long ip_s;	/* Start Ip Address */
	unsigned long ip_e;	/* End Ip Address */
	unsigned int ag_idx;	/* account group index */
	long long cnt;		/* pkt_cnt or byt_cnt */
	enum AcResult result;
};

int AcRegIoctlHandler(void);
void AcUnRegIoctlHandler(void);

#endif
