#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

#include "hwnat_ioctl.h"

int HwNatDumpEntry(unsigned int entry_num)
{
    struct hwnat_args opt;
    int fd;

    opt.entry_num=entry_num;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DUMP_ENTRY, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

int HwNatBindEntry(unsigned int entry_num)
{
    struct hwnat_args opt;
    int fd;

    opt.entry_num=entry_num;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_BIND_ENTRY, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

int HwNatUnBindEntry(unsigned int entry_num)
{
    struct hwnat_args opt;
    int fd;

    opt.entry_num=entry_num;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UNBIND_ENTRY, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return  HWNAT_FAIL;
    }

    close(fd);
    return  HWNAT_SUCCESS;
}

int HwNatInvalidEntry(unsigned int entry_num)
{
    struct hwnat_args opt;
    int fd;

    opt.entry_num=entry_num;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_INVALID_ENTRY, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

#if !defined (CONFIG_HNAT_V2)
/*hnat qos*/
int HwNatSetQoS(struct hwnat_qos_args *opt, int ioctl_id)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, ioctl_id, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}
#else

int HwNatCacheDumpEntry(void)
{
    struct hwnat_args opt;
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DUMP_CACHE_ENTRY, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

int HwNatGetAGCnt(struct hwnat_ac_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_GET_AC_CNT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}
#endif

int HwNatSetConfig(struct hwnat_config_args *opt, int ioctl_id)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, ioctl_id, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

int HwNatGetAllEntries(struct hwnat_args *opt)
{
    int fd=0;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_GET_ALL_ENTRIES, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    

    return HWNAT_SUCCESS;

}

int HwNatDebug(unsigned int debug)
{
    struct hwnat_args opt;
    int fd;

    opt.debug=debug;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DEBUG, &opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;
}

