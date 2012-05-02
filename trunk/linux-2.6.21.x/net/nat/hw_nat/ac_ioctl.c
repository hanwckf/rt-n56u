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
    ac_ioctl.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-02-15      Initial version
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
#include "ac_ioctl.h"
#include "ac_policy.h"
#include "mtr_policy.h"
#include "util.h"

int	ac_major =  AC_MAJOR;

struct _AcParam {
	enum AcType Type;
	enum AcRuleType RuleType;

} AcParam[]= {

	{PRE_AC, AC_MAC_GROUP}, /*AC_ADD_MAC_UL_ENTRY*/
	{POST_AC, AC_MAC_GROUP}, /*AC_ADD_MAC_DL_ENTRY*/
	{PRE_AC, AC_MAC_GROUP}, /*AC_DEL_MAC_UL_ENTRY*/
	{POST_AC, AC_MAC_GROUP}, /*AC_DEL_MAC_DL_ENTRY*/

	{PRE_AC, AC_IP_GROUP}, /*AC_ADD_IP_UL_ENTRY*/
	{POST_AC, AC_IP_GROUP}, /*AC_ADD_IP_DL_ENTRY*/
	{PRE_AC, AC_IP_GROUP}, /*AC_DEL_IP_UL_ENTRY*/
	{POST_AC, AC_IP_GROUP}, /*AC_DEL_IP_DL_ENTRY*/

	{PRE_AC, AC_VLAN_GROUP}, /*AC_ADD_VLAN_UL_ENTRY*/
	{POST_AC, AC_VLAN_GROUP}, /*AC_ADD_VLAN_DL_ENTRY*/
	{PRE_AC, AC_VLAN_GROUP}, /*AC_DEL_VLAN_UL_ENTRY*/
	{POST_AC, AC_VLAN_GROUP}, /*AC_DEL_VLAN_DL_ENTRY*/


	{PRE_AC, AC_MAC_GROUP}, /*AC_GET_MAC_UL_PKT_CNT*/
	{POST_AC, AC_MAC_GROUP}, /*AC_GET_MAC_DL_PKT_CNT*/
	{PRE_AC, AC_MAC_GROUP}, /*AC_GET_MAC_UL_BYTE_CNT*/
	{POST_AC, AC_MAC_GROUP}, /*AC_GET_MAC_DL_BYTE_CNT*/

	{PRE_AC, AC_IP_GROUP}, /*AC_GET_IP_UL_PKT_CNT*/
	{POST_AC, AC_IP_GROUP}, /*AC_GET_IP_DL_PKT_CNT*/
	{PRE_AC, AC_IP_GROUP}, /*AC_GET_IP_UL_BYTE_CNT*/
	{POST_AC, AC_IP_GROUP}, /*AC_GET_IP_DL_BYTE_CNT*/

	{PRE_AC, AC_VLAN_GROUP}, /*AC_GET_VLAN_UL_PKT_CNT*/
	{POST_AC, AC_VLAN_GROUP}, /*AC_GET_VLAN_DL_PKT_CNT*/
	{PRE_AC, AC_VLAN_GROUP}, /*AC_GET_VLAN_UL_BYTE_CNT*/
	{POST_AC, AC_VLAN_GROUP}, /*AC_GET_VLAN_DL_BYTE_CNT*/


	{PRE_AC+POST_AC, AC_MAC_GROUP + AC_IP_GROUP} /* AC_CLEAN_TBL */

};

/* Rule Boundary Check */
uint32_t  AcBndryCheck(AcPlcyNode *NewNode)
{
        uint32_t CurAcEnd=0;
        uint32_t MaxAcEnd=0;

	//exist node
	if(AcExistNode(NewNode)!=NULL) {
		return AC_SUCCESS;
	}
	 
        switch(NewNode->Type)
        {
	case PRE_AC:
		CurAcEnd=PpeGetPreAcEnd();
		MaxAcEnd=PpeGetPostMtrStr();
		break;
	case POST_AC:
		CurAcEnd=PpeGetPostAcEnd();
		MaxAcEnd=511;
		break;
        }

	//AC ip/mac rule needs 1 entry
        if(++CurAcEnd >= MaxAcEnd){
                return AC_TBL_FULL;
        }else {
                return AC_SUCCESS;
        }

}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long AcIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg)
#else
int AcIoctl(struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg)
#endif
{
    struct ac_args *opt=(struct ac_args *)arg;
    AcPlcyNode node;

    MacReverse(opt->mac);
    memcpy(node.Mac,opt->mac,ETH_ALEN);
    node.IpS=opt->ip_s;
    node.IpE=opt->ip_e;
    node.VLAN=opt->vid;
    node.Type=AcParam[cmd].Type;	
    node.RuleType=AcParam[cmd].RuleType;

    switch(cmd) 
    {
    case AC_ADD_MAC_UL_ENTRY:
    case AC_ADD_MAC_DL_ENTRY:
    case AC_ADD_IP_UL_ENTRY:
    case AC_ADD_IP_DL_ENTRY:
    case AC_ADD_VLAN_UL_ENTRY:
    case AC_ADD_VLAN_DL_ENTRY:
        opt->result=AcBndryCheck(&node);
	if(opt->result!=AC_TBL_FULL) {
		opt->result=AcAddNode(&node);
	}
	break;
    case AC_DEL_MAC_UL_ENTRY:
    case AC_DEL_MAC_DL_ENTRY:
    case AC_DEL_IP_UL_ENTRY:
    case AC_DEL_IP_DL_ENTRY:
    case AC_DEL_VLAN_UL_ENTRY:
    case AC_DEL_VLAN_DL_ENTRY:
	opt->result=AcDelNode(&node);
	break;
    case AC_GET_MAC_UL_PKT_CNT:
    case AC_GET_IP_UL_PKT_CNT:
    case AC_GET_VLAN_UL_PKT_CNT:
    case AC_GET_MAC_DL_PKT_CNT:
    case AC_GET_IP_DL_PKT_CNT:
    case AC_GET_VLAN_DL_PKT_CNT:
	opt->cnt=AcGetCnt(&node, AC_PKT_CNT);
	break;
    case AC_GET_MAC_UL_BYTE_CNT:
    case AC_GET_IP_UL_BYTE_CNT:
    case AC_GET_VLAN_UL_BYTE_CNT:
    case AC_GET_MAC_DL_BYTE_CNT:
    case AC_GET_IP_DL_BYTE_CNT:
    case AC_GET_VLAN_DL_BYTE_CNT:
	opt->cnt=AcGetCnt(&node, AC_BYTE_CNT);
	break;
    case AC_CLEAN_TBL:
	AcCleanTbl();
	break;
    default:
	break;
    }

    return 0;
}

struct file_operations ac_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    unlocked_ioctl:     AcIoctl,
#else
    ioctl:		AcIoctl,
#endif
};


int AcRegIoctlHandler(void)
{
    int result=0;
    result = register_chrdev(ac_major, AC_DEVNAME, &ac_fops);
    if (result < 0) {
	NAT_PRINT("ac: can't get major %d\n",ac_major);
        return result;
    }

    if (ac_major == 0) {
	ac_major = result; /* dynamic */
    }

    return 0;
}

void AcUnRegIoctlHandler(void)
{
    unregister_chrdev(ac_major, AC_DEVNAME);
}

EXPORT_SYMBOL(AcBndryCheck);
EXPORT_SYMBOL(AcAddNode);