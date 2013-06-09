/*
    Module Name:
    acl_ioctl.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-02-15      Initial version
*/

#ifndef	__ACL_IOCTL_H__
#define	__ACL_IOCTL_H__

#define ACL_ADD_SMAC_DIP_ANY   		(0)
#define ACL_ADD_SMAC_DIP_TCP   		(1)
#define ACL_ADD_SMAC_DIP_UDP   		(2)
#define ACL_DEL_SMAC_DIP_ANY   		(3)
#define ACL_DEL_SMAC_DIP_TCP   		(4)
#define ACL_DEL_SMAC_DIP_UDP   		(5)

#define ACL_ADD_SIP_DIP_ANY    		(6)
#define ACL_ADD_SIP_DIP_TCP    		(7)
#define ACL_ADD_SIP_DIP_UDP    		(8)
#define ACL_DEL_SIP_DIP_ANY    		(9)
#define ACL_DEL_SIP_DIP_TCP    		(10)
#define ACL_DEL_SIP_DIP_UDP    		(11)
#define ACL_CLEAN_TBL    		(12)

#define ACL_ADD_SDMAC_ANY   		(13)
#define ACL_DEL_SDMAC_ANY   		(14)
#define ACL_ADD_ETYPE_ANY   		(15)
#define ACL_DEL_ETYPE_ANY   		(16)

#define ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT   	(17)
#define ACL_DEL_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT   	(18)
#define ACL_GET_ALL_ENTRIES				(19)

#define ACL_DEVNAME			"acl0"
#define ACL_MAJOR			(230)

enum AclRuleMethod {
	ACL_ALLOW_RULE = 0,
	ACL_DENY_RULE = 1,
	ACL_PRIORITY_RULE = 2	/*set user priority only */
};

enum AclRuleOpt {
	ACL_RULE_ADD = 0,
	ACL_RULE_DEL = 1
};

enum AclProtoType {
	ACL_PROTO_ANY = 0,
	ACL_PROTO_TCP = 1,
	ACL_PROTO_UDP = 2
};

enum AclResult {
	ACL_SUCCESS = 0,
	ACL_FAIL = 1,
	ACL_TBL_FULL = 2
};

struct acl_args {
	unsigned char mac[6];
	unsigned char dmac[6];
	enum AclResult result;	/* ioctl result */
	enum AclRuleMethod method;	/* Deny, Allow */
	enum AclProtoType L4;
	unsigned long sip_s;	/* start of sip */
	unsigned long sip_e;	/* end of sip */
	unsigned long dip_s;	/* start of dip */
	unsigned long dip_e;	/* end of dip */
	unsigned short dp_s;	/* start of dp */
	unsigned short dp_e;	/* end of dp */
	unsigned short sp_s;	/* start of sp */
	unsigned short sp_e;	/* end of sp */
	unsigned char tos_s;	/* start of tos */
	unsigned char tos_e;	/* end of tos */
	unsigned short ethertype;	/* end of tos */
	unsigned short protocol;	/* protocol of ip header */
	unsigned int vid:12;
	 /*VID*/ unsigned int up:3;	/*acl=>up */
	unsigned int pn:3;	/*physical port */
};

struct acl_list_args {
	enum AclResult result;
	unsigned int num_of_entries:16;
	struct acl_args entries[0];
};
int AclRegIoctlHandler(void);
void AclUnRegIoctlHandler(void);

#endif
