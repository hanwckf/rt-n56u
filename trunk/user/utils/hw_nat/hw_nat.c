#include <stdlib.h>             
#include <stdio.h>             
#include <string.h>           
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <strings.h>
#include <linux/autoconf.h>

#include "hwnat_ioctl.h"
#include "hwnat_api.h"

void show_usage(void)
{
    printf("Show Foe Entry\n");
    printf("hw_nat -c [entry_num]\n");
    printf("Ex: hw_nat -c 1234\n\n");
    
    printf("Set Debug Level (0:disable) \n");
    printf("hw_nat -d [0~7]\n");
    printf("Ex: hw_nat -d \n\n");
    
    printf("Show All Foe Invalid Entry\n");
    printf("Ex: hw_nat -e\n\n");
    
    printf("Show All Foe Unbinded Entry\n");
    printf("Ex: hw_nat -f\n\n");
    
    printf("Show All Foe Binded Entry\n");
    printf("Ex: hw_nat -g\n\n");

#if! defined (CONFIG_HNAT_V2)
    printf("Enable DSCP Remark\n");
    printf("Ex: hw_nat -A [0/1]\n\n");

    printf("Enable VLAN Priority Remark\n");
    printf("Ex: hw_nat -B [0/1]\n\n");

    printf("Set weight of FOE priority decision in resolution\n");
    printf("Ex: hw_nat -C [0~7]\n\n");

    printf("Set weight of ACL priority decision in resolution\n");
    printf("Ex: hw_nat -D [0~7]\n\n");

    printf("Set weight of DSCP priority decision in resolution\n");
    printf("Ex: hw_nat -E [0~7]\n\n");

    printf("Set weight of VLAN priority decision in resolution\n");
    printf("Ex: hw_nat -F [0~7]\n\n");

    printf("Set mapping of DSCP set to UP\n");
    printf("Ex: hw_nat -G [DSCP_SET:0~7][UP:0~7]\n\n");

    printf("Set mapping of UP to Inprofile DSCP\n");
    printf("Ex: hw_nat -H [UP:0~7][DSCP:0~63]\n\n");

    printf("Set mapping of UP to Outprofile DSCP\n");
    printf("Ex: hw_nat -I [UP:0~7][DSCP:0~63]\n\n");

    printf("Set mapping of UP to VLAN Priority\n");
    printf("Ex: hw_nat -J [UP:0~7][VPRI:0~7]\n\n");

    printf("Set mapping of UP to Access Category\n");
    printf("Ex: hw_nat -K [UP:0~7][AC:0~3]\n\n");

    printf("Set HNAT PreACL/PreMeter/PreAC/PostMeter/PostAC table size (d=383, 32, 32, 32, 32)\n");
    printf("Ex: hw_nat -P [0~511][0~64][0~64][0~64][0~64]\n");
    printf("NOTE: Total 511 rules, PreAC+PostAC<=64, PreMeter+PostMeter<=64\n\n");

    printf("Set HNAT QOS Mode\n");
#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855)
    printf("Ex: hw_nat -L [0:WRR, 1:SPQ, 2:Q3>WRR(Q2,Q1,Q0), 3:Q3>Q2>WRR(Q1,Q0)]\n\n");
#else
    printf("Ex: hw_nat -L [0:WRR, 1:SPQ, 2:Q3>WRR(Q2,Q1,Q0)]\n\n");
#endif

    printf("Set the weight of GDMA Scheduler\n");
    printf("hw_nat -M Q3(1/2/4/8) Q2(1/2/4/8) Q1(1/2/4/8) Q0(1/2/4/8)\n\n");
    printf("hw_nat -M 8 4 2 1\n\n");
#else
    printf("Get ByteCNT and PktCnt of AG_IDX\n");
    printf("Ex: hw_nat -A [AG index]\n\n");
#endif

    printf("Set PPE Cofigurations:\n");
    printf("Set HNAT binding threshold per second (d=30)\n");
    printf("Ex: hw_nat -N [1~65535]\n\n");

    printf("Set HNAT Max entries allowed build when Free Entries>3/4, >1/2, <1/2 (d=100, 50, 25)\n");
    printf("Ex: hw_nat -O [1~16383][1~16383][1~16383]\n\n");


    printf("Set HNAT TCP/UDP keepalive interval (d=1, 1)(unit:4sec)\n");
    printf("Ex: hw_nat -Q [1~255][1~255]\n\n");

    printf("Set HNAT Life time of unbind entry (d=3)(unit:1Sec)\n");
    printf("Ex: hw_nat -T [1~255]\n\n");

    printf("Set HNAT Life time of Binded TCP/UDP/FIN entry(d=5, 5, 5)(unit:1Sec) \n");
    printf("Ex: hw_nat -U [1~65535][1~65535][1~65535]\n\n");

    printf("Only Speed UP (0=Upstream, 1=Downstream, 2=Bi-Direction) flow \n");
    printf("Ex: hw_nat -Z 1\n\n");
   
}

int main(int argc, char *argv[])
{
    int opt;
#if !defined (CONFIG_HNAT_V2)
    char options[] = "efg?c:d:A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:T:U:Z:";
#else
    char options[] = "aefg?c:d:A:N:O:P:Q:T:U:Z:";
#endif
    int fd, method;
    int i=0;
    unsigned int entry_num;
    unsigned int debug;
    unsigned int dir;
    struct hwnat_args *args;
    struct hwnat_tuple args2;
#if !defined (CONFIG_HNAT_V2)
    struct hwnat_qos_args args3;
#else
    struct hwnat_ac_args args3;
#endif
    struct hwnat_config_args args4;
    int	   result;

    fd = open("/dev/"HW_NAT_DEVNAME, O_RDONLY);
    if (fd < 0)
    {
	printf("Open %s pseudo device failed\n","/dev/"HW_NAT_DEVNAME);
	return 0;
    }

    if(argc < 2) {
	show_usage();
	return 0;
    }

    /* Max table size is 16K */
    args=malloc(sizeof(struct hwnat_args)+sizeof(struct hwnat_tuple)*1024*16);

    while ((opt = getopt (argc, argv, options)) != -1) {
	switch (opt) {
#if defined (CONFIG_HNAT_V2)
	case 'a':
		method = HW_NAT_DUMP_CACHE_ENTRY;
		break;
#endif
	case 'c':
		method = HW_NAT_DUMP_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'd':
		method = HW_NAT_DEBUG;
		debug = strtoll(optarg, NULL, 10);
		break;
	case 'e':
		method = HW_NAT_GET_ALL_ENTRIES;
		args->entry_state=0; /* invalid entry */
		break;
	case 'f':
		method = HW_NAT_GET_ALL_ENTRIES;
		args->entry_state=1; /* unbinded entry */
		break;
	case 'g':
		method = HW_NAT_GET_ALL_ENTRIES;
		args->entry_state=2; /* binded entry */
		break;
#if !defined (CONFIG_HNAT_V2)
	case 'A':
		method = HW_NAT_DSCP_REMARK;
		args3.enable = strtoll(optarg, NULL, 10);
		break;
	case 'B':
		method = HW_NAT_VPRI_REMARK;
		args3.enable = strtoll(optarg, NULL, 10);
		break;
	case 'C':
		method = HW_NAT_FOE_WEIGHT;
		args3.weight = strtoll(optarg, NULL, 10);
		break;
	case 'D':
		method = HW_NAT_ACL_WEIGHT;
		args3.weight = strtoll(optarg, NULL, 10);
		break;
	case 'E':
		method = HW_NAT_DSCP_WEIGHT;
		args3.weight = strtoll(optarg, NULL, 10);
		break;
	case 'F':
		method = HW_NAT_VPRI_WEIGHT;
		args3.weight = strtoll(optarg, NULL, 10);
		break;
	case 'G':
		method = HW_NAT_DSCP_UP;
		args3.dscp_set = strtoll(argv[2], NULL, 10);
		args3.up = strtoll(argv[3], NULL, 10);
		break;
	case 'H':
		method = HW_NAT_UP_IDSCP;

		args3.up = strtoll(argv[2], NULL, 10);
		args3.dscp = strtoll(argv[3], NULL, 10);
		break;
	case 'I':
		method = HW_NAT_UP_ODSCP;
		args3.up = strtoll(argv[2], NULL, 10);
		args3.dscp = strtoll(argv[3], NULL, 10);
		break;
	case 'J':
		method = HW_NAT_UP_VPRI;
		args3.up = strtoll(argv[2], NULL, 10);
		args3.vpri = strtoll(argv[3], NULL, 10);
		break;
	case 'K':
		method = HW_NAT_UP_AC;
		args3.up = strtoll(argv[2], NULL, 10);
		args3.ac = strtoll(argv[3], NULL, 10);
		break;
	case 'L':
		method = HW_NAT_SCH_MODE;
		args3.mode = strtoll(argv[2], NULL, 10);
		break;
	case 'M':
		method = HW_NAT_SCH_WEIGHT;
		args3.weight3 = strtoll(argv[2], NULL, 10);
		args3.weight2 = strtoll(argv[3], NULL, 10);
		args3.weight1 = strtoll(argv[4], NULL, 10);
		args3.weight0 = strtoll(argv[5], NULL, 10);
		break;
#else
	case 'A':
		method = HW_NAT_GET_AC_CNT;
		args3.ag_index = strtoll(optarg, NULL, 10);
		break;
#endif
	case 'N':
		method = HW_NAT_BIND_THRESHOLD;
		args4.bind_threshold = strtoll(argv[2], NULL, 10);
		break;
	case 'O':
		method = HW_NAT_MAX_ENTRY_LMT;
		args4.foe_qut_lmt = strtoll(argv[2], NULL, 10);
		args4.foe_half_lmt = strtoll(argv[3], NULL, 10);
		args4.foe_full_lmt  = strtoll(argv[4], NULL, 10);
		break;
	case 'P':
		method = HW_NAT_RULE_SIZE;
		args4.pre_acl    = strtoll(argv[2], NULL, 10);
		args4.pre_meter  = strtoll(argv[3], NULL, 10);
		args4.pre_ac     = strtoll(argv[4], NULL, 10);
		args4.post_meter = strtoll(argv[5], NULL, 10);
		args4.post_ac    = strtoll(argv[6], NULL, 10);
		break;
	case 'Q':
		method = HW_NAT_KA_INTERVAL;
		args4.foe_tcp_ka = strtoll(argv[2], NULL, 10);
		args4.foe_udp_ka = strtoll(argv[3], NULL, 10);
		break;
	case 'T':
		method = HW_NAT_UB_LIFETIME;
		args4.foe_unb_dlta = strtoll(argv[2], NULL, 10);
		break;
	case 'U':
		method = HW_NAT_BIND_LIFETIME;
		args4.foe_tcp_dlta = strtoll(argv[2], NULL, 10);
		args4.foe_udp_dlta = strtoll(argv[3], NULL, 10);
		args4.foe_fin_dlta = strtoll(argv[4], NULL, 10);
		break;
	case 'Z':
		method = HW_NAT_BIND_DIRECTION;
		dir = strtoll(optarg, NULL, 10);
		break;
	case '?':
		show_usage();

	}
    } 


    switch(method){
    case HW_NAT_GET_ALL_ENTRIES:
	    HwNatGetAllEntries(args);

	    printf("Total Entry Count = %d\n",args->num_of_entries);	
	    for(i=0;i<args->num_of_entries;i++){
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
		    printf("IPv6_1T= %d /SIP: %x:%x:%x:%x:%x:%x:%x:%x\n", \
		    args->entries[i].hash_index, \
		    NIPHALF(args->entries[i].ing_sipv6_0), \
		    NIPHALF(args->entries[i].ing_sipv6_1), \
		    NIPHALF(args->entries[i].ing_sipv6_2), \
		    NIPHALF(args->entries[i].ing_sipv6_3));
		} else if(args->entries[i].pkt_type==3) { //IPV4_DSLITE
		    printf("DS-Lite= %d : %u.%u.%u.%u:%d->%u.%u.%u.%u:%d (%x:%x:%x:%x:%x:%x:%x:%x -> %x:%x:%x:%x:%x:%x:%x:%x) \n", \
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
		    printf("IPv6_3T= %d SIP: %x:%x:%x:%x:%x:%x:%x:%x DIP: %x:%x:%x:%x:%x:%x:%x:%x\n", \
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
			printf("IPv6_5T= %d SIP: %x:%x:%x:%x:%x:%x:%x:%x DIP: %x:%x:%x:%x:%x:%x:%x:%x (Flow Label=%d)\n", \
				args->entries[i].hash_index, \
				NIPHALF(args->entries[i].ing_sipv6_0), \
				NIPHALF(args->entries[i].ing_sipv6_1), \
				NIPHALF(args->entries[i].ing_sipv6_2), \
				NIPHALF(args->entries[i].ing_sipv6_3), \
				NIPHALF(args->entries[i].ing_dipv6_0), \
				NIPHALF(args->entries[i].ing_dipv6_1), \
				NIPHALF(args->entries[i].ing_dipv6_2), \
				NIPHALF(args->entries[i].ing_dipv6_3), \
				(args->entries[i].ing_sp << 16) | (args->entries[i].ing_dp));
		    }else {
			printf("IPv6_5T= %d SIP: %x:%x:%x:%x:%x:%x:%x:%x (SP:%d) DIP: %x:%x:%x:%x:%x:%x:%x:%x (DP=%d)\n", \
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
			printf("6RD= %d %x:%x:%x:%x:%x:%x:%x:%x->%x:%x:%x:%x:%x:%x:%x:%x [Flow Label=%d]\n", \
				args->entries[i].hash_index, \
				NIPHALF(args->entries[i].ing_sipv6_0), \
				NIPHALF(args->entries[i].ing_sipv6_1), \
				NIPHALF(args->entries[i].ing_sipv6_2), \
				NIPHALF(args->entries[i].ing_sipv6_3), \
				NIPHALF(args->entries[i].ing_dipv6_0), \
				NIPHALF(args->entries[i].ing_dipv6_1), \
				NIPHALF(args->entries[i].ing_dipv6_2), \
				NIPHALF(args->entries[i].ing_dipv6_3), \
				(args->entries[i].ing_sp << 16) | (args->entries[i].ing_dp));
				printf("(%u.%u.%u.%u->%u.%u.%u.%u)\n", NIPQUAD(args->entries[i].eg_sipv4), NIPQUAD(args->entries[i].eg_dipv4));
		    }else {
			printf("6RD= %d /SIP: %x:%x:%x:%x:%x:%x:%x:%x [SP:%d] /DIP: %x:%x:%x:%x:%x:%x:%x:%x [DP=%d]", \
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
	    break;
#if defined (CONFIG_HNAT_V2)
    case HW_NAT_DUMP_CACHE_ENTRY:
	    result = HwNatCacheDumpEntry();
	    break;
#endif
    case HW_NAT_DUMP_ENTRY:
	    result = HwNatDumpEntry(entry_num);
	    break;
    case HW_NAT_DEBUG:
	    result = HwNatDebug(debug);
	    break;
#if !defined (CONFIG_HNAT_V2)
    case HW_NAT_DSCP_REMARK:
            HwNatDscpRemarkEbl(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_VPRI_REMARK:
            HwNatVpriRemarkEbl(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_FOE_WEIGHT:
	    HwNatSetFoeWeight(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_ACL_WEIGHT:
	    HwNatSetAclWeight(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_DSCP_WEIGHT:
	    HwNatSetDscpWeight(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_VPRI_WEIGHT:
	    HwNatSetVpriWeight(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_DSCP_UP:
	    HwNatSetDscp_Up(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_UP_IDSCP:
	    HwNatSetUp_InDscp(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_UP_ODSCP:
	    HwNatSetUp_OutDscp(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_UP_VPRI:
	    HwNatSetUp_Vpri(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_UP_AC:
	    HwNatSetUp_Ac(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_SCH_MODE:
	    HwNatSetSchMode(&args3);
	    result = args3.result;
	    break;
    case HW_NAT_SCH_WEIGHT:
	    HwNatSetSchWeight(&args3);
	    result = args3.result;
	    break;
#else
    case HW_NAT_GET_AC_CNT:
	    HwNatGetAGCnt(&args3);
	    printf("Byte cnt=%d\n", args3.ag_byte_cnt);
	    printf("Pkt cnt=%d\n", args3.ag_pkt_cnt);
	    result = args3.result;
	    break;
#endif
    case HW_NAT_BIND_THRESHOLD:
	    HwNatSetBindThreshold(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_MAX_ENTRY_LMT:
	    HwNatSetMaxEntryRateLimit(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_RULE_SIZE:
	    HwNatSetRuleSize(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_KA_INTERVAL:
	    HwNatSetKaInterval(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_UB_LIFETIME:
	    HwNatSetUnbindLifeTime(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_BIND_LIFETIME:
	    HwNatSetBindLifeTime(&args4);
	    result = args4.result;
	    break;
    case HW_NAT_BIND_DIRECTION:
	    result = HwNatSetBindDir(dir);
	    break;

    }

    if(result==HWNAT_SUCCESS){
	printf("done\n");
    }else if(result==HWNAT_ENTRY_NOT_FOUND) {
	printf("entry not found\n");
    }else {
	printf("fail\n");
    }

    free(args);
    return 0;
}
