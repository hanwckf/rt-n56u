/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

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
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <linux/wireless.h>
#include <asm/uaccess.h>

#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "foe_fdb.h"
#include "util.h"
#include "ra_nat.h"

int	hw_nat_major =  HW_NAT_MAJOR;
unsigned char bind_dir = BIDIRECTION;
#ifdef HWNAT_DEBUG
extern int DebugLevel;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long HwNatIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg)
#else
int HwNatIoctl (struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg)
#endif
{
    struct hwnat_args *opt=(struct hwnat_args *)arg;
    struct hwnat_tuple *opt2=(struct hwnat_tuple *)arg;
    struct hwnat_qos_args *opt3=(struct hwnat_qos_args *)arg;
    struct hwnat_config_args *opt4=(struct hwnat_config_args *)arg;
 
    switch(cmd) 
    {
    case HW_NAT_ADD_ENTRY:
	opt2->result = FoeAddEntry(opt2);
	break;
    case HW_NAT_DEL_ENTRY:
	opt2->result = FoeDelEntry(opt2);
	break;
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
	opt->result = FoeDelEntryByNum(opt->entry_num);
	break;
    case HW_NAT_DUMP_ENTRY: 
	FoeDumpEntry(opt->entry_num);
	break;
#ifdef HWNAT_DEBUG
    case HW_NAT_DEBUG: /* For Debug */
	DebugLevel=opt->debug;
	break;
#endif
    case HW_NAT_DSCP_REMARK:
	opt3->result = PpeSetDscpRemarkEbl(opt3->enable);
	break;
    case HW_NAT_VPRI_REMARK:
	opt3->result = PpeSetVpriRemarkEbl(opt3->enable);
	break;
    case HW_NAT_FOE_WEIGHT:
	opt3->result = PpeSetWeightFOE(opt3->weight);
	break;
    case HW_NAT_ACL_WEIGHT:/*Weight for ACL to UP */
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
	opt3->result = PpeSetSchWeight(opt3->weight0, opt3->weight1, opt3->weight2, opt3->weight3);
	break;
    case HW_NAT_BIND_THRESHOLD:
	opt4->result = PpeSetBindThreshold(opt4->bind_threshold);
	break;
    case HW_NAT_MAX_ENTRY_LMT:
	opt4->result = PpeSetMaxEntryLimit(opt4->foe_full_lmt, opt4->foe_half_lmt, opt4->foe_qut_lmt);
	break;
    case HW_NAT_RULE_SIZE:
	opt4->result = PpeSetRuleSize(opt4->pre_acl, opt4->pre_meter, opt4->pre_ac, opt4->post_meter, opt4->post_ac);
	break;
    case HW_NAT_KA_INTERVAL: 
	opt4->result = PpeSetKaInterval(opt4->foe_tcp_ka, opt4->foe_udp_ka);
	break;
    case HW_NAT_UB_LIFETIME: 
	opt4->result = PpeSetUnbindLifeTime(opt4->foe_unb_dlta);
	break;
    case HW_NAT_BIND_LIFETIME: 
	opt4->result = PpeSetBindLifetime(opt4->foe_tcp_dlta, opt4->foe_udp_dlta, opt4->foe_fin_dlta);
	break;
    case HW_NAT_BIND_DIRECTION:
	bind_dir=opt->bind_dir;
	break;
    default:
	break;
    }
    return 0;
}

struct file_operations hw_nat_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    unlocked_ioctl:      HwNatIoctl,
#else
    ioctl:		 HwNatIoctl,
#endif
};


int PpeRegIoctlHandler(void)
{
    int result=0;
    result = register_chrdev(hw_nat_major, HW_NAT_DEVNAME, &hw_nat_fops);
    if (result < 0) {
	NAT_PRINT("hw_nat: can't get major %d\n",hw_nat_major);
        return result;
    }

    if (hw_nat_major == 0) {
	hw_nat_major = result; /* dynamic */
    }
    return 0;
}



void PpeUnRegIoctlHandler(void)
{
    unregister_chrdev(hw_nat_major, HW_NAT_DEVNAME);
}

