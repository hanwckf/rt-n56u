#ifndef __ACL_API
#define __ACL_API

#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#define NIPHALF(addr) \
        ((unsigned short *)&addr)[0], \
        ((unsigned short *)&addr)[1]

int SetAclEntry(struct acl_args *opt, unsigned int cmd);
int AclGetAllEntries(struct acl_list_args *opt);

#endif
