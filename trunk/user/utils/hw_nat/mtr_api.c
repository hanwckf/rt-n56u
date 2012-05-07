#include <stdlib.h>             
#include <stdio.h>             
#include <string.h>           
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include "mtr_ioctl.h"

int SetMtrEntry(struct mtr_args *opt, unsigned int cmd)
{
    int fd;

    fd = open("/dev/"MTR_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"MTR_DEVNAME);
	return MTR_FAIL;
    }

    if(ioctl(fd, cmd, opt)<0) {
	printf("MTR_API: ioctl error\n");
	close(fd);
	return MTR_FAIL;
    }

    close(fd);
    return MTR_SUCCESS;
}

int MtrGetAllEntries(struct mtr_list_args *opt)
{
    int fd=0;

    fd = open("/dev/"MTR_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
        printf("Open %s pseudo device failed\n","/dev/"MTR_DEVNAME);
        return MTR_FAIL;
    }

    if(ioctl(fd, MTR_GET_ALL_ENTRIES, opt)<0) {
        printf("MTR_API: ioctl error\n");
        close(fd);
        return MTR_FAIL;
    }

    close(fd);

    return MTR_SUCCESS;

}

