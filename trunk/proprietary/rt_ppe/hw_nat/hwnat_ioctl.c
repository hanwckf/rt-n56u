/*
    Module Name:
    hwnat_ioctl.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "foe_fdb.h"
#include "util.h"
#include "ra_nat.h"

#if defined (CONFIG_PPE_MCAST)
#include "mcast_tbl.h"
#endif

extern int udp_offload;
#if defined (CONFIG_RA_HW_NAT_IPV6)
extern int ipv6_offload;
#endif
extern uint16_t lan_vid;
extern uint16_t wan_vid;
extern uint32_t DebugLevel;

#if !defined (CONFIG_HNAT_V2)
int pre_acl_start_addr;
int pre_ac_start_addr;
int post_ac_start_addr;
int pre_mtr_start_addr;
int post_mtr_start_addr;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long HwNatIoctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
int HwNatIoctl(struct inode *inode, struct file *filp,
	   unsigned int cmd, unsigned long arg)
#endif
{
	struct hwnat_args *opt = (struct hwnat_args *)arg;
#if defined (CONFIG_HNAT_V2)
	struct hwnat_ac_args *opt3 = (struct hwnat_ac_args *)arg;
#else
	struct hwnat_qos_args *opt3 = (struct hwnat_qos_args *)arg;
#endif
	struct hwnat_config_args *opt4 = (struct hwnat_config_args *)arg;

#if defined (CONFIG_PPE_MCAST)
	struct hwnat_mcast_args *opt5 = (struct hwnat_mcast_args *)arg;
#endif

	switch (cmd) {
	case HW_NAT_GET_ALL_ENTRIES:
		opt->result = FoeGetAllEntries(opt);
		break;
	case HW_NAT_BIND_ENTRY:
		opt->result = FoeBindEntry(opt);
		break;
	case HW_NAT_UNBIND_ENTRY:
		opt->result = FoeUnBindEntry(opt);
		break;
	case HW_NAT_INVALID_ENTRY:
		opt->result = FoeDelEntry(opt);
		break;
	case HW_NAT_DUMP_ENTRY:
		FoeDumpEntry(opt->entry_num);
		break;
	case HW_NAT_DEBUG:	/* For Debug */
		DebugLevel = opt->debug;
		break;
#if defined (CONFIG_HNAT_V2)
	case HW_NAT_DROP_ENTRY:
		opt->result = FoeDropEntry(opt);
		break;
	case HW_NAT_DUMP_CACHE_ENTRY:
		FoeDumpCacheEntry();
		break;
	case HW_NAT_GET_AC_CNT:
		opt3->result = PpeGetAGCnt(opt3);
		break;
#else
	case HW_NAT_DSCP_REMARK:
		opt3->result = PpeSetDscpRemarkEbl(opt3->enable);
		break;
	case HW_NAT_VPRI_REMARK:
		opt3->result = PpeSetVpriRemarkEbl(opt3->enable);
		break;
	case HW_NAT_FOE_WEIGHT:
		opt3->result = PpeSetWeightFOE(opt3->weight);
		break;
	case HW_NAT_ACL_WEIGHT:	/*Weight for ACL to UP */
		opt3->result = PpeSetWeightACL(opt3->weight);
		break;
	case HW_NAT_DSCP_WEIGHT:
		opt3->result = PpeSetWeightDSCP(opt3->weight);
		break;
	case HW_NAT_VPRI_WEIGHT:
		opt3->result = PpeSetWeightVPRI(opt3->weight);
		break;
	case HW_NAT_DSCP_UP:
		opt3->result = PpeSetDSCP_UP(opt3->dscp_set, opt3->up);
		break;
	case HW_NAT_UP_IDSCP:
		opt3->result = PpeSetUP_IDSCP(opt3->up, opt3->dscp);
		break;
	case HW_NAT_UP_ODSCP:
		opt3->result = PpeSetUP_ODSCP(opt3->up, opt3->dscp);
		break;
	case HW_NAT_UP_VPRI:
		opt3->result = PpeSetUP_VPRI(opt3->up, opt3->vpri);
		break;
	case HW_NAT_UP_AC:
		opt3->result = PpeSetUP_AC(opt3->up, opt3->ac);
		break;
	case HW_NAT_SCH_MODE:
		opt3->result = PpeSetSchMode(opt3->mode);
		break;
	case HW_NAT_SCH_WEIGHT:
		opt3->result =
		    PpeSetSchWeight(opt3->weight0, opt3->weight1,
				    opt3->weight2, opt3->weight3);
		break;
	case HW_NAT_RULE_SIZE:
		opt4->result =
		    PpeSetRuleSize(opt4->pre_acl, opt4->pre_meter,
				   opt4->pre_ac, opt4->post_meter,
				   opt4->post_ac);
		break;
#endif
	case HW_NAT_BIND_THRESHOLD:
		opt4->result = PpeSetBindThreshold(opt4->bind_threshold);
		break;
	case HW_NAT_MAX_ENTRY_LMT:
		opt4->result =
		    PpeSetMaxEntryLimit(opt4->foe_full_lmt,
					opt4->foe_half_lmt, opt4->foe_qut_lmt);
		break;
	case HW_NAT_KA_INTERVAL:
		opt4->result =
		    PpeSetKaInterval(opt4->foe_tcp_ka, opt4->foe_udp_ka);
		break;
	case HW_NAT_UB_LIFETIME:
		opt4->result = PpeSetUnbindLifeTime(opt4->foe_unb_dlta);
		break;
	case HW_NAT_BIND_LIFETIME:
		opt4->result =
		    PpeSetBindLifetime(opt4->foe_tcp_dlta,
				       opt4->foe_udp_dlta, opt4->foe_fin_dlta);
		break;
	case HW_NAT_BIND_DIRECTION:
		opt4->result = HWNAT_FAIL;
		break;
	case HW_NAT_VLAN_ID:
		wan_vid = opt4->wan_vid;
		lan_vid = opt4->lan_vid;
		opt4->result = HWNAT_SUCCESS;
		break;
#if defined (CONFIG_PPE_MCAST)
	case HW_NAT_MCAST_INS:
		foe_mcast_entry_ins(opt5->mc_vid, opt5->dst_mac, opt5->mc_px_en, opt5->mc_px_qos_en, opt5->mc_qos_qid);
		break;
	case HW_NAT_MCAST_DEL:
		foe_mcast_entry_del(opt5->mc_vid, opt5->dst_mac, opt5->mc_px_en, opt5->mc_px_qos_en);
		break;
	case HW_NAT_MCAST_DUMP:
		foe_mcast_entry_dump();
		break;
#endif
	default:
		break;
	}
	return 0;
}

struct file_operations hw_nat_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl: HwNatIoctl,
#else
	ioctl: HwNatIoctl,
#endif
};

int PpeRegIoctlHandler(void)
{
	int result = 0;
	result = register_chrdev(HW_NAT_MAJOR, HW_NAT_DEVNAME, &hw_nat_fops);
	if (result < 0) {
		printk(KERN_WARNING "hw_nat: can't get major %d\n", HW_NAT_MAJOR);
		return result;
	}

	if (HW_NAT_MAJOR == 0) {
		printk("HNAT Major num=%d\n", result);
	}

	return 0;
}

void PpeUnRegIoctlHandler(void)
{
	unregister_chrdev(HW_NAT_MAJOR, HW_NAT_DEVNAME);
}

#if defined (CONFIG_HNAT_V2)
int PpeGetAGCnt(struct hwnat_ac_args * opt3)
{
#if defined (CONFIG_RALINK_MT7620)
	opt3->ag_byte_cnt = RegRead(AC_BASE + opt3->ag_index * 8);     /* Low bytes */
	opt3->ag_pkt_cnt = RegRead(AC_BASE + opt3->ag_index * 8 + 4);  /* High bytes */
#elif defined (CONFIG_RALINK_MT7621)
	opt3->ag_byte_cnt = RegRead(AC_BASE + opt3->ag_index * 16);     /* 64bit bytes cnt */
	opt3->ag_byte_cnt |= ((unsigned long long)(RegRead(AC_BASE + opt3->ag_index * 16 + 4)) << 32);
	opt3->ag_pkt_cnt = RegRead(AC_BASE + opt3->ag_index * 16 + 8);  /* 32bit packet cnt */
#endif
	return HWNAT_SUCCESS;
}
#else
int PpeSetDscpRemarkEbl(uint32_t enable)
{
	RegModifyBits(PPE_GLO_CFG, enable, 11, 1);
	return HWNAT_SUCCESS;
}

int PpeSetVpriRemarkEbl(uint32_t enable)
{
	/* Re-generate VLAN Priority */
	RegModifyBits(PPE_GLO_CFG, enable, 10, 1);
	return HWNAT_SUCCESS;
}

int PpeSetWeightFOE(uint32_t weight)
{
	/* Set weight of decision in resolution */
	RegModifyBits(UP_RES, weight, FUP_WT_OFFSET, 3);
	return HWNAT_SUCCESS;
}

int PpeSetWeightACL(uint32_t weight)
{
	/* Set weight of decision in resolution */
	RegModifyBits(UP_RES, weight, AUP_WT_OFFSET, 3);
	return HWNAT_SUCCESS;
}

int PpeSetWeightDSCP(uint32_t weight)
{
	RegModifyBits(UP_RES, weight, DUP_WT_OFFSET, 3);
	return HWNAT_SUCCESS;
}

int PpeSetWeightVPRI(uint32_t weight)
{
	/* Set weight of decision in resolution */
	RegModifyBits(UP_RES, weight, VUP_WT_OFFSET, 3);
	return HWNAT_SUCCESS;
}

int PpeSetDSCP_UP(uint32_t DSCP_SET, unsigned char UP)
{
	int DSCP_UP;

	DSCP_UP = ((UP << 0) | (UP << 4) | (UP << 8) | (UP << 12)
		   | (UP << 16) | (UP << 20) | (UP << 24) | (UP << 28));
	/* Set DSCP to User priority mapping table */
	switch (DSCP_SET) {
	case 0:
		RegWrite(DSCP0_7_MAP_UP, DSCP_UP);
		break;
	case 1:
		RegWrite(DSCP8_15_MAP_UP, DSCP_UP);
		break;
	case 2:
		RegWrite(DSCP16_23_MAP_UP, DSCP_UP);
		break;
	case 3:
		RegWrite(DSCP24_31_MAP_UP, DSCP_UP);
		break;
	case 4:
		RegWrite(DSCP32_39_MAP_UP, DSCP_UP);
		break;
	case 5:
		RegWrite(DSCP40_47_MAP_UP, DSCP_UP);
		break;
	case 6:
		RegWrite(DSCP48_55_MAP_UP, DSCP_UP);
		break;
	case 7:
		RegWrite(DSCP56_63_MAP_UP, DSCP_UP);
		break;
	default:

		break;
	}
	return HWNAT_SUCCESS;
}

int PpeSetUP_IDSCP(uint32_t UP, uint32_t IDSCP)
{
	/* Set mapping table of user priority to in-profile DSCP */
	switch (UP) {
	case 0:
		RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 0, 6);
		break;
	case 1:
		RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 8, 6);
		break;
	case 2:
		RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 16, 6);
		break;
	case 3:
		RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 24, 6);
		break;
	case 4:
		RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 0, 6);
		break;
	case 5:
		RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 8, 6);
		break;
	case 6:
		RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 16, 6);
		break;
	case 7:
		RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 24, 6);
		break;
	default:
		break;
	}
	return HWNAT_SUCCESS;
}

int PpeSetUP_ODSCP(uint32_t UP, uint32_t ODSCP)
{
	/* Set mapping table of user priority to out-profile DSCP */
	switch (UP) {
	case 0:
		RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 0, 6);
		break;
	case 1:
		RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 8, 6);
		break;
	case 2:
		RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 16, 6);
		break;
	case 3:
		RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 24, 6);
		break;
	case 4:
		RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 0, 6);
		break;
	case 5:
		RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 8, 6);
		break;
	case 6:
		RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 16, 6);
		break;
	case 7:
		RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 24, 6);
		break;
	default:
		break;
	}
	return HWNAT_SUCCESS;
}

int PpeSetUP_VPRI(uint32_t UP, uint32_t VPRI)
{
	/* Set mapping table of user priority to vlan priority */
	switch (UP) {
	case 0:
		RegModifyBits(UP_MAP_VPRI, VPRI, 0, 3);
		break;
	case 1:
		RegModifyBits(UP_MAP_VPRI, VPRI, 4, 3);
		break;
	case 2:
		RegModifyBits(UP_MAP_VPRI, VPRI, 8, 3);
		break;
	case 3:
		RegModifyBits(UP_MAP_VPRI, VPRI, 12, 3);
		break;
	case 4:
		RegModifyBits(UP_MAP_VPRI, VPRI, 16, 3);
		break;
	case 5:
		RegModifyBits(UP_MAP_VPRI, VPRI, 20, 3);
		break;
	case 6:
		RegModifyBits(UP_MAP_VPRI, VPRI, 24, 3);
		break;
	case 7:
		RegModifyBits(UP_MAP_VPRI, VPRI, 28, 3);
		break;
	default:
		break;
	}
	return HWNAT_SUCCESS;
}

int PpeSetUP_AC(uint32_t UP, uint32_t AC)
{
	/* Set mapping table of user priority to access category */
	switch (UP) {
	case 0:
		RegModifyBits(UP_MAP_AC, AC, 0, 2);
		break;
	case 1:
		RegModifyBits(UP_MAP_AC, AC, 2, 2);
		break;
	case 2:
		RegModifyBits(UP_MAP_AC, AC, 4, 2);
		break;
	case 3:
		RegModifyBits(UP_MAP_AC, AC, 6, 2);
		break;
	case 4:
		RegModifyBits(UP_MAP_AC, AC, 8, 2);
		break;
	case 5:
		RegModifyBits(UP_MAP_AC, AC, 10, 2);
		break;
	case 6:
		RegModifyBits(UP_MAP_AC, AC, 12, 2);
		break;
	case 7:
		RegModifyBits(UP_MAP_AC, AC, 14, 2);
		break;
	default:
		break;
	}
	return HWNAT_SUCCESS;
}

int PpeSetSchMode(uint32_t policy)
{
	/* Set GDMA1&2 Schduling Mode */
	RegModifyBits(FE_GDMA1_SCH_CFG, policy, 24, 2);
	RegModifyBits(FE_GDMA2_SCH_CFG, policy, 24, 2);

	return HWNAT_SUCCESS;
}

/* In general case, we only need 1/2/4/8 weight */
int PpeWeightRemap(uint8_t W)
{
	switch (W) {
	case 8:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
		return 3;
#else
		return 7;
#endif
	case 4:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
		return 2;
#else
		return 3;
#endif
	case 2:
		return 1;
	case 1:
		return 0;
	default:
		/* invalid value */
		return -1;
	}
}

int PpeSetSchWeight(uint8_t W0, uint8_t W1, uint8_t W2, uint8_t W3)
{
	int32_t _W0, _W1, _W2, _W3;

	_W0 = PpeWeightRemap(W0);
	_W1 = PpeWeightRemap(W1);
	_W2 = PpeWeightRemap(W2);
	_W3 = PpeWeightRemap(W3);

	if ((_W0 == -1) || (_W1 == -1) || (_W2 == -1) || (_W3 == -1)) {
		return HWNAT_FAIL;
	}

	/* Set GDMA1 Schduling Weight */
	RegModifyBits(FE_GDMA1_SCH_CFG, _W0, 0, 3);
	RegModifyBits(FE_GDMA1_SCH_CFG, _W1, 4, 3);
	RegModifyBits(FE_GDMA1_SCH_CFG, _W2, 8, 3);
	RegModifyBits(FE_GDMA1_SCH_CFG, _W3, 12, 3);

	/* Set GDMA2 Schduling Weight */
	RegModifyBits(FE_GDMA2_SCH_CFG, _W0, 0, 3);
	RegModifyBits(FE_GDMA2_SCH_CFG, _W1, 4, 3);
	RegModifyBits(FE_GDMA2_SCH_CFG, _W2, 8, 3);
	RegModifyBits(FE_GDMA2_SCH_CFG, _W3, 12, 3);

	return HWNAT_SUCCESS;
}

void PpeRstPreAclPtr(void)
{
	RegModifyBits(PPE_PRE_ACL, 0, 0, 9);
	RegModifyBits(PPE_PRE_ACL, 0, 16, 9);
}

void PpeRstPreAcPtr(void)
{
	RegModifyBits(PPE_PRE_AC, pre_ac_start_addr, 0, 9);
	RegModifyBits(PPE_PRE_AC, pre_ac_start_addr, 16, 9);
}

void PpeRstPostAcPtr(void)
{
	RegModifyBits(PPE_POST_AC, post_ac_start_addr, 0, 9);
	RegModifyBits(PPE_POST_AC, post_ac_start_addr, 16, 9);
}

void PpeRstPreMtrPtr(void)
{
	RegModifyBits(PPE_PRE_MTR, pre_mtr_start_addr, 0, 9);
	RegModifyBits(PPE_PRE_MTR, pre_mtr_start_addr, 16, 9);
}

void PpeRstPostMtrPtr(void)
{
	RegModifyBits(PPE_POST_MTR, post_mtr_start_addr, 0, 9);
	RegModifyBits(PPE_POST_MTR, post_mtr_start_addr, 16, 9);
}

int
PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac,
	       uint16_t post_meter, uint16_t post_ac)
{
	pre_acl_start_addr  = 0;
	pre_mtr_start_addr  = 0 + pre_acl;
	pre_ac_start_addr   = 0 + pre_acl + pre_meter;
	post_mtr_start_addr = 0 + pre_acl + pre_meter + pre_ac;
	post_ac_start_addr  = 0 + pre_acl + pre_meter + pre_ac + post_meter;
	  
	/* Set Pre ACL Table */
	PpeRstPreAclPtr();

	/* Set Pre MTR Table */
	PpeRstPreMtrPtr();
	
	/* Set Pre AC Table */
	PpeRstPreAcPtr();

	/* Set Post MTR Table */
	PpeRstPostMtrPtr();
	
	/* Set Post AC Table */
	PpeRstPostAcPtr();

	return HWNAT_SUCCESS;
}
#endif

int PpeSetBindThreshold(uint32_t threshold)
{
	/* Set reach bind rate for unbind state */
	RegModifyBits(PPE_FOE_BNDR, threshold, 0, 16);

	return HWNAT_SUCCESS;
}

int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, qurt, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, half, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, full, 0, 14);

	return HWNAT_SUCCESS;
}

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka)
{
	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, tcp_ka, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, udp_ka, 24, 8);

	return HWNAT_SUCCESS;
}

int PpeSetUnbindLifeTime(uint8_t lifetime)
{
	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, lifetime, 0, 8);

	return HWNAT_SUCCESS;
}

int PpeSetBindLifetime(uint16_t tcp_life, uint16_t udp_life, uint16_t fin_life)
{

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE0, udp_life, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, fin_life, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, tcp_life, 0, 16);

	return HWNAT_SUCCESS;
}

