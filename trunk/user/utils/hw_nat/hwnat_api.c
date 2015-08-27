#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

#include "hwnat_ioctl.h"
#include "hwnat_api.h"

static int
HwNatIoCtlArgs(unsigned long request, void *opt)
{
    int fd, result = HWNAT_SUCCESS;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0) {
	printf("Open /dev/%s pseudo device failed!\n", HW_NAT_DEVNAME);
	return HWNAT_FAIL;
    }

    if (ioctl(fd, request, opt) < 0) {
	printf("HW_NAT_API: ioctl error!\n");
	result = HWNAT_FAIL;
    }

    close(fd);
    return result;
}

int HwNatDumpEntry(unsigned int entry_num)
{
    struct hwnat_args opt;

    opt.entry_num = entry_num;
    return HwNatIoCtlArgs(HW_NAT_DUMP_ENTRY, &opt);
}

int HwNatBindEntry(unsigned int entry_num)
{
    struct hwnat_args opt;

    opt.entry_num = entry_num;
    return HwNatIoCtlArgs(HW_NAT_BIND_ENTRY, &opt);
}

int HwNatUnBindEntry(unsigned int entry_num)
{
    struct hwnat_args opt;

    opt.entry_num = entry_num;
    return HwNatIoCtlArgs(HW_NAT_UNBIND_ENTRY, &opt);
}

int HwNatInvalidEntry(unsigned int entry_num)
{
    struct hwnat_args opt;

    opt.entry_num = entry_num;
    return HwNatIoCtlArgs(HW_NAT_INVALID_ENTRY, &opt);
}

#if !defined (CONFIG_HNAT_V2)
/*hnat qos*/
int HwNatSetQoS(struct hwnat_qos_args *opt, int ioctl_id)
{
    if (HwNatIoCtlArgs(ioctl_id, opt) != HWNAT_SUCCESS)
	return HWNAT_FAIL;

    return opt->result;
}
#else
int HwNatDropEntry(unsigned int entry_num)
{
    struct hwnat_args opt;

    opt.entry_num = entry_num;
    return HwNatIoCtlArgs(HW_NAT_DROP_ENTRY, &opt);
}

int HwNatCacheDumpEntry(void)
{
    struct hwnat_args opt;

    return HwNatIoCtlArgs(HW_NAT_DUMP_CACHE_ENTRY, &opt);
}

int HwNatGetAGCnt(struct hwnat_ac_args *opt)
{
    if (HwNatIoCtlArgs(HW_NAT_GET_AC_CNT, opt) != HWNAT_SUCCESS)
	return HWNAT_FAIL;

    printf("AC %u Bytes: %llu\n", opt->ag_index, opt->ag_byte_cnt);
    printf("AC %u Pkts:  %u\n", opt->ag_index, opt->ag_pkt_cnt);

    return opt->result;
}
#endif

int HwNatSetConfig(struct hwnat_config_args *opt, int ioctl_id)
{
    if (HwNatIoCtlArgs(ioctl_id, opt) != HWNAT_SUCCESS)
	return HWNAT_FAIL;

    return opt->result;
}

int HwNatGetAllEntries(unsigned int entry_state)
{
    int i, result;
    struct hwnat_args *args;

    /* Max table size is 16K */
    args = malloc(sizeof(struct hwnat_args)+sizeof(struct hwnat_tuple)*1024*16);
    if (!args) {
	printf("Allocate memory for hwnat_args and hwnat_tuple failed!\n");
	return HWNAT_FAIL;
    }

    args->entry_state = entry_state;

    if (HwNatIoCtlArgs(HW_NAT_GET_ALL_ENTRIES, args) != HWNAT_SUCCESS) {
	free(args);
	return HWNAT_FAIL;
    }

    printf("Total Entry Count = %d\n",args->num_of_entries);

    for(i=0;i<args->num_of_entries;i++) {
	if(args->entries[i].pkt_type==0) { //IPV4_NAPT
	    printf("IPv4_NAPT=%d : %u.%u.%u.%u:%d->%u.%u.%u.%u:%d => %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n", \
			args->entries[i].hash_index, \
			NIPQUAD(args->entries[i].ing_sipv4), \
			args->entries[i].ing_sp, \
			NIPQUAD(args->entries[i].ing_dipv4), \
			args->entries[i].ing_dp, \
			NIPQUAD(args->entries[i].eg_sipv4), \
			args->entries[i].eg_sp, \
			NIPQUAD(args->entries[i].eg_dipv4), \
			args->entries[i].eg_dp);
	} else if(args->entries[i].pkt_type==1) { //IPV4_NAT
	    printf("IPv4_NAT=%d : %u.%u.%u.%u->%u.%u.%u.%u => %u.%u.%u.%u->%u.%u.%u.%u\n", \
			args->entries[i].hash_index, \
			NIPQUAD(args->entries[i].ing_sipv4), \
			NIPQUAD(args->entries[i].ing_dipv4), \
			NIPQUAD(args->entries[i].eg_sipv4), \
			NIPQUAD(args->entries[i].eg_dipv4)); 
	} else if(args->entries[i].pkt_type==2) { //IPV6_ROUTING
	    printf("IPv6_1T=%d /DIP: %x:%x:%x:%x:%x:%x:%x:%x\n", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3));
	} else if(args->entries[i].pkt_type==3) { //IPV4_DSLITE
	    printf("DS-Lite=%d : %u.%u.%u.%u:%d->%u.%u.%u.%u:%d (%x:%x:%x:%x:%x:%x:%x:%x -> %x:%x:%x:%x:%x:%x:%x:%x)\n", \
			args->entries[i].hash_index, \
			NIPQUAD(args->entries[i].ing_sipv4),  \
			args->entries[i].ing_sp,     \
			NIPQUAD(args->entries[i].ing_dipv4),  \
			args->entries[i].ing_dp, \
			NIPHALF(args->entries[i].eg_sipv6_0), \
			NIPHALF(args->entries[i].eg_sipv6_1), \
			NIPHALF(args->entries[i].eg_sipv6_2), \
			NIPHALF(args->entries[i].eg_sipv6_3), \
			NIPHALF(args->entries[i].eg_dipv6_0), \
			NIPHALF(args->entries[i].eg_dipv6_1), \
			NIPHALF(args->entries[i].eg_dipv6_2), \
			NIPHALF(args->entries[i].eg_dipv6_3));
	} else if(args->entries[i].pkt_type==4) { //IPV6_3T_ROUTE
	    printf("IPv6_3T=%d SIP: %x:%x:%x:%x:%x:%x:%x:%x DIP: %x:%x:%x:%x:%x:%x:%x:%x\n", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_sipv6_0), \
			NIPHALF(args->entries[i].ing_sipv6_1), \
			NIPHALF(args->entries[i].ing_sipv6_2), \
			NIPHALF(args->entries[i].ing_sipv6_3), \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3));
	} else if(args->entries[i].pkt_type==5) { //IPV6_5T_ROUTE
	    if(args->entries[i].ipv6_flowlabel==1) {
		printf("IPv6_5T=%d SIP: %x:%x:%x:%x:%x:%x:%x:%x DIP: %x:%x:%x:%x:%x:%x:%x:%x (Flow Label=%x)\n", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_sipv6_0), \
			NIPHALF(args->entries[i].ing_sipv6_1), \
			NIPHALF(args->entries[i].ing_sipv6_2), \
			NIPHALF(args->entries[i].ing_sipv6_3), \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3), \
			((args->entries[i].ing_sp << 16) | (args->entries[i].ing_dp))&0xFFFFF);
	    }else {
		printf("IPv6_5T=%d SIP: %x:%x:%x:%x:%x:%x:%x:%x (SP:%d) DIP: %x:%x:%x:%x:%x:%x:%x:%x (DP=%d)\n", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_sipv6_0), \
			NIPHALF(args->entries[i].ing_sipv6_1), \
			NIPHALF(args->entries[i].ing_sipv6_2), \
			NIPHALF(args->entries[i].ing_sipv6_3), \
			args->entries[i].ing_sp, \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3), \
			args->entries[i].ing_dp);
	    }
	} else if(args->entries[i].pkt_type==7) { //IPV6_6RD
	    if(args->entries[i].ipv6_flowlabel==1) {
		printf("6RD=%d %x:%x:%x:%x:%x:%x:%x:%x->%x:%x:%x:%x:%x:%x:%x:%x [Flow Label=%x]\n", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_sipv6_0), \
			NIPHALF(args->entries[i].ing_sipv6_1), \
			NIPHALF(args->entries[i].ing_sipv6_2), \
			NIPHALF(args->entries[i].ing_sipv6_3), \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3), \
			((args->entries[i].ing_sp << 16) | (args->entries[i].ing_dp))&0xFFFFF);
			printf("(%u.%u.%u.%u->%u.%u.%u.%u)\n", NIPQUAD(args->entries[i].eg_sipv4), NIPQUAD(args->entries[i].eg_dipv4));
	    }else {
		printf("6RD=%d /SIP: %x:%x:%x:%x:%x:%x:%x:%x [SP:%d] /DIP: %x:%x:%x:%x:%x:%x:%x:%x [DP=%d]", \
			args->entries[i].hash_index, \
			NIPHALF(args->entries[i].ing_sipv6_0), \
			NIPHALF(args->entries[i].ing_sipv6_1), \
			NIPHALF(args->entries[i].ing_sipv6_2), \
			NIPHALF(args->entries[i].ing_sipv6_3), \
			args->entries[i].ing_sp, \
			NIPHALF(args->entries[i].ing_dipv6_0), \
			NIPHALF(args->entries[i].ing_dipv6_1), \
			NIPHALF(args->entries[i].ing_dipv6_2), \
			NIPHALF(args->entries[i].ing_dipv6_3), \
			args->entries[i].ing_dp); 
		printf("(%u.%u.%u.%u->%u.%u.%u.%u)\n", NIPQUAD(args->entries[i].eg_sipv4), NIPQUAD(args->entries[i].eg_dipv4));
	    }
	} else{
	    printf("unknown packet type! (pkt_type=%d) \n", args->entries[i].pkt_type);
	}
    }

    result = args->result;
    free(args);

    return result;
}

int HwNatDebug(unsigned int debug)
{
    struct hwnat_args opt;

    opt.debug = debug;
    return HwNatIoCtlArgs(HW_NAT_DEBUG, &opt);
}

#if defined (CONFIG_PPE_MCAST)
int HwNatMcastIns(struct hwnat_mcast_args *opt)
{
    return HwNatIoCtlArgs(HW_NAT_MCAST_INS, opt);
}

int HwNatMcastDel(struct hwnat_mcast_args *opt)
{
    return HwNatIoCtlArgs(HW_NAT_MCAST_DEL, opt);
}

int HwNatMcastDump(void)
{
    return HwNatIoCtlArgs(HW_NAT_MCAST_DUMP, NULL);
}
#endif
