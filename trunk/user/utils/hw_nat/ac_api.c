#include <stdlib.h>             
#include <stdio.h>             
#include <string.h>           
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include "ac_ioctl.h"

int SetAcEntry(struct ac_args *opt, unsigned int cmd)
{
    int fd;

    fd = open("/dev/"AC_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
        printf("Open %s pseudo device failed\n","/dev/"AC_DEVNAME);
        return AC_FAIL;
    }

    if(ioctl(fd, cmd, opt)<0) {
        printf("AC_API: ioctl error\n");
        close(fd);
        return AC_FAIL;
    }

    close(fd);
    return AC_SUCCESS;
}


int GetAcEntry(struct ac_args *opt, unsigned int cmd)
{
    int fd;

    fd = open("/dev/"AC_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"AC_DEVNAME);
	return AC_FAIL;
    }

    if(ioctl(fd, cmd, opt)<0) {
	printf("AC_API: ioctl error\n");
	close(fd);
	return AC_FAIL;
    }

    close(fd);
    return AC_SUCCESS;
}
