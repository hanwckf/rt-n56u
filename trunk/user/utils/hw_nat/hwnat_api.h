#ifndef __HW_NAT_API
#define __HW_NAT_API

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#define NIPHALF(addr) \
        ((unsigned short *)&addr)[1], \
        ((unsigned short *)&addr)[0]

int HwNatDumpEntry(unsigned int entry_num);
int HwNatBindEntry(unsigned int entry_num);
int HwNatUnBindEntry(unsigned int entry_num);
int HwNatInvalidEntry(unsigned int entry_num);
#if !defined (CONFIG_HNAT_V2)
int HwNatSetQoS(struct hwnat_qos_args *opt, int ioctl_id);
#else
int HwNatCacheDumpEntry(void);
int HwNatGetAGCnt(struct hwnat_ac_args *opt);
#endif
int HwNatSetConfig(struct hwnat_config_args *opt, int ioctl_id);
int HwNatGetAllEntries(struct hwnat_args *opt);
int HwNatDebug(unsigned int debug);
#endif
