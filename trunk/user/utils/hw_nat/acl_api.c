#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

#include "acl_ioctl.h"

int SetAclEntry(struct acl_args *opt, unsigned int cmd)
{
    int fd;

    fd = open("/dev/"ACL_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"ACL_DEVNAME);
	return ACL_FAIL;
    }

    if(ioctl(fd, cmd, opt)<0) {
	printf("ACL_API: ioctl error\n");
	close(fd);
	return ACL_FAIL;
    }

    close(fd);
    return ACL_SUCCESS;
}

int AclGetAllEntries(struct acl_list_args *opt)
{
    int fd=0;

    fd = open("/dev/"ACL_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"ACL_DEVNAME);
	return ACL_FAIL;
    }

    if(ioctl(fd, ACL_GET_ALL_ENTRIES, opt)<0) {
	printf("ACL_API: ioctl error\n");
	close(fd);
	return ACL_FAIL;
    }

    close(fd);

    return ACL_SUCCESS;

}
