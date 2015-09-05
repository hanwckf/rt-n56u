/*
    Module Name:
    acl_ioctl.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-02-15      Initial version
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "frame_engine.h"
#include "acl_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "util.h"


/* Rule Boundary Check */
uint32_t AclBndryCheck(AclPlcyNode * NewNode)
{

	uint32_t CurAclEnd = PpeGetPreAclEnd();
	uint32_t MaxAclEnd = PpeGetPreMtrStr();
	uint32_t RuleSize = 0;

	if (AclExistNode(NewNode) != NULL) {
		return ACL_SUCCESS;
	}

	switch (NewNode->RuleType) {

	case ACL_ADD_SDMAC_ANY:
	case ACL_ADD_ETYPE_ANY:
		RuleSize = 1;
		break;
	case ACL_ADD_SMAC_DIP_ANY:
	case ACL_ADD_SIP_DIP_ANY:
		RuleSize = 2;	/* SMAC + DIP */
		break;
	case ACL_ADD_SMAC_DIP_TCP:
	case ACL_ADD_SMAC_DIP_UDP:
	case ACL_ADD_SIP_DIP_TCP:
	case ACL_ADD_SIP_DIP_UDP:
		RuleSize = 3;	/* SMAC/SIP + DIP +DP */
		break;
	case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
		RuleSize = 7;	/* SMAC|ETYPE + VID|SIP|SP| DIP|DP|TOS */
		break;
	}

	if (CurAclEnd + RuleSize >= MaxAclEnd) {
		return ACL_TBL_FULL;
	} else {
		return ACL_SUCCESS;
	}

}

uint32_t RunIoctlAddHandler(AclPlcyNode * NewNode, enum AclProtoType Proto)
{
	uint32_t Result;

	NewNode->Proto = Proto;
	Result = AclBndryCheck(NewNode);
	if (Result != ACL_TBL_FULL) {
		Result = AclAddNode(NewNode);
	}
	return Result;
}

uint32_t RunIoctlDelHandler(AclPlcyNode * DelNode, enum AclProtoType Proto)
{
	uint32_t Result;

	DelNode->Proto = Proto;
	Result = AclDelNode(DelNode);

	return Result;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long AclIoctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
int
AclIoctl(struct inode *inode, struct file *filp,
	 unsigned int cmd, unsigned long arg)
#endif
{
	struct acl_args *opt = (struct acl_args *)arg;
	struct acl_list_args *opt2 = (struct acl_list_args *)arg;
	AclPlcyNode node;

	memcpy(node.Mac, opt->mac, ETH_ALEN);
	memcpy(node.DMac, opt->dmac, ETH_ALEN);
	node.Method = opt->method;
	node.RuleType = cmd;
	node.SipS = opt->sip_s;
	node.SipE = opt->sip_e;
	node.DipS = opt->dip_s;
	node.DipE = opt->dip_e;
	node.SpS = opt->sp_s;
	node.SpE = opt->sp_e;
	node.DpS = opt->dp_s;
	node.DpE = opt->dp_e;
	node.up = opt->up;
	node.pn = opt->pn;
	node.TosS = opt->tos_s;
	node.TosE = opt->tos_e;
	node.Ethertype = opt->ethertype;
	node.Vid = opt->vid;
	node.Proto = opt->L4;
	node.Protocol = opt->protocol;
	node.SpecialTag = 0;	/* use for esw port > rt6855A */
	switch (cmd) {
	case ACL_ADD_SDMAC_ANY:
	case ACL_ADD_ETYPE_ANY:
		opt->result = RunIoctlAddHandler(&node, ACL_PROTO_ANY);
		break;
	case ACL_DEL_SDMAC_ANY:
	case ACL_DEL_ETYPE_ANY:
		opt->result = RunIoctlDelHandler(&node, ACL_PROTO_ANY);
		break;
	case ACL_ADD_SMAC_DIP_ANY:
	case ACL_ADD_SIP_DIP_ANY:
		opt->result = RunIoctlAddHandler(&node, ACL_PROTO_ANY);
		break;
	case ACL_DEL_SMAC_DIP_ANY:
	case ACL_DEL_SIP_DIP_ANY:
		opt->result = RunIoctlDelHandler(&node, ACL_PROTO_ANY);
		break;
	case ACL_ADD_SMAC_DIP_TCP:
	case ACL_ADD_SIP_DIP_TCP:
		opt->result = RunIoctlAddHandler(&node, ACL_PROTO_TCP);
		break;
	case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
		opt->result = RunIoctlAddHandler(&node, node.Proto);
		break;
	case ACL_DEL_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
		opt->result = RunIoctlDelHandler(&node, node.Proto);
		break;
	case ACL_DEL_SMAC_DIP_TCP:
	case ACL_DEL_SIP_DIP_TCP:
		opt->result = RunIoctlDelHandler(&node, ACL_PROTO_TCP);
		break;
	case ACL_ADD_SMAC_DIP_UDP:
	case ACL_ADD_SIP_DIP_UDP:
		opt->result = RunIoctlAddHandler(&node, ACL_PROTO_UDP);
		break;
	case ACL_DEL_SMAC_DIP_UDP:
	case ACL_DEL_SIP_DIP_UDP:
		opt->result = RunIoctlDelHandler(&node, ACL_PROTO_UDP);
		break;
	case ACL_CLEAN_TBL:
		AclCleanTbl();
		break;
	case ACL_GET_ALL_ENTRIES:
		opt2->result = AclGetAllEntries(opt2);
		break;
	default:
		break;
	}

	return 0;
}

struct file_operations acl_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl: AclIoctl,
#else
	ioctl: AclIoctl,
#endif
};

int AclRegIoctlHandler(void)
{
	int result = 0;
	result = register_chrdev(ACL_MAJOR, ACL_DEVNAME, &acl_fops);
	if (result < 0) {
		printk(KERN_WARNING "acl: can't get major %d\n", ACL_MAJOR);
		return result;
	}

	return 0;
}

void AclUnRegIoctlHandler(void)
{
	unregister_chrdev(ACL_MAJOR, ACL_DEVNAME);
}
