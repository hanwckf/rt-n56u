#include <stdlib.h>             
#include <stdio.h>             
#include <string.h>           
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/autoconf.h>
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
int HwNatDscpRemarkEbl(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DSCP_REMARK, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatVpriRemarkEbl(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_VPRI_REMARK, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetFoeWeight(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_FOE_WEIGHT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetAclWeight(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_ACL_WEIGHT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetDscpWeight(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DSCP_WEIGHT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetVpriWeight(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_VPRI_WEIGHT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetDscp_Up(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_DSCP_UP, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetUp_InDscp(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UP_IDSCP, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetUp_OutDscp(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UP_ODSCP, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetUp_Vpri(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UP_VPRI, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}
int HwNatSetUp_Ac(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UP_AC, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetSchMode(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_SCH_MODE, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetSchWeight(struct hwnat_qos_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_SCH_WEIGHT, opt)<0) {
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

int HwNatSetBindThreshold(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_BIND_THRESHOLD, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetMaxEntryRateLimit(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_MAX_ENTRY_LMT, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}


int HwNatSetRuleSize(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_RULE_SIZE, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetKaInterval(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_KA_INTERVAL, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}




int HwNatSetUnbindLifeTime(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_UB_LIFETIME, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetBindLifeTime(struct hwnat_config_args *opt)
{
    int fd;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_BIND_LIFETIME, opt)<0) {
	printf("HW_NAT_API: ioctl error\n");
	close(fd);
	return HWNAT_FAIL;
    }

    close(fd);
    return HWNAT_SUCCESS;

}

int HwNatSetBindDir(unsigned int dir)
{
    struct hwnat_args opt;
    int fd;

    opt.bind_dir=dir;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if(ioctl(fd, HW_NAT_BIND_DIRECTION, &opt)<0) {
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
