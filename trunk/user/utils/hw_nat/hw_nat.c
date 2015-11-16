#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <strings.h>

#include "hwnat_ioctl.h"
#include "hwnat_api.h"
#include "util.h"

void show_usage(void)
{
#if defined (CONFIG_HNAT_V2)
    printf("Show Foe Cache Entries\n");
    printf("hw_nat -a\n");
    printf("Ex: hw_nat -a\n\n");
#endif

    printf("Show Foe Entry\n");
    printf("hw_nat -c [entry_num]\n");
    printf("Ex: hw_nat -c 1234\n\n");

    printf("Set Debug Level (0:disable) \n");
    printf("hw_nat -d [0~7]\n");
    printf("Ex: hw_nat -d 5\n\n");

    printf("Show All Foe Invalid Entry\n");
    printf("Ex: hw_nat -e\n\n");

    printf("Show All Foe Unbinded Entry\n");
    printf("Ex: hw_nat -f\n\n");

    printf("Show All Foe Binded Entry\n");
    printf("Ex: hw_nat -g\n\n");

    printf("Unbind Foe Entry\n");
    printf("hw_nat -x [entry_num]\n");
    printf("Ex: hw_nat -x 1234\n\n");

#if !defined (CONFIG_HNAT_V2)
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

#if defined (CONFIG_RALINK_MT7621)
    printf("Set Foe Entry to PacketDrop\n");
    printf("hw_nat -k [entry_num]\n");
    printf("Ex: hw_nat -k 1234\n\n");
#endif

    printf("Get ByteCNT and PktCnt of AG_IDX\n");
    printf("Ex: hw_nat -A [AG index]\n\n");

#if defined (CONFIG_PPE_MCAST)
    printf("Add member port in multicast entry\n");
    printf("Ex: hw_nat -B [vid] [mac] [px_en] [px_qos_en] [mc_qos_qid]\n\n");

    printf("Del member port multicast entry\n");
    printf("Ex: hw_nat -C [vid] [mac] [px_en] [px_qos_en]\n\n");

    printf("Dump all multicast entry\n");
    printf("Ex: hw_nat -D\n\n");
#endif
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

    printf("Set LAN/WAN port VLAN ID\n");
    printf("Ex: hw_nat -V [LAN_VID] [WAN_VID]\n\n");

    printf("Only Speed UP (0=Upstream, 1=Downstream, 2=Bi-Direction) flow \n");
    printf("Ex: hw_nat -Z 1\n\n");
}

int main(int argc, char *argv[])
{
    int opt;
#if !defined (CONFIG_HNAT_V2)
    char options[] = "efg?c:x:d:A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:T:U:V:Z:";
#else
    char options[] = "aefg?c:x:k:d:A:B:C:DN:O:P:Q:T:U:V:Z:";
#endif
    int method = -1;
    unsigned int entry_state = 0;
    unsigned int entry_num = 0;
    unsigned int debug = 0;
#if !defined (CONFIG_HNAT_V2)
    struct hwnat_qos_args args3;
#else
    struct hwnat_ac_args args3;
#endif
    struct hwnat_config_args args4;
    int result = 0;

#if defined (CONFIG_PPE_MCAST)
    struct hwnat_mcast_args args5;
    unsigned char mac[6];
#endif

    if(argc < 2) {
	show_usage();
	return 1;
    }

    while ((opt = getopt (argc, argv, options)) != -1) {
	switch (opt) {
	case 'c':
		method = HW_NAT_DUMP_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'x':
		method = HW_NAT_UNBIND_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'd':
		method = HW_NAT_DEBUG;
		debug = strtoll(optarg, NULL, 10);
		break;
	case 'e':
		method = HW_NAT_GET_ALL_ENTRIES;
		entry_state=0; /* invalid entry */
		break;
	case 'f':
		method = HW_NAT_GET_ALL_ENTRIES;
		entry_state=1; /* unbinded entry */
		break;
	case 'g':
		method = HW_NAT_GET_ALL_ENTRIES;
		entry_state=2; /* binded entry */
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
	case 'k':
		method = HW_NAT_DROP_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'a':
		method = HW_NAT_DUMP_CACHE_ENTRY;
		break;
	case 'A':
		method = HW_NAT_GET_AC_CNT;
		args3.ag_index = strtoll(optarg, NULL, 10);
		break;
#if defined (CONFIG_PPE_MCAST)
	case 'B':
		method = HW_NAT_MCAST_INS;
		args5.mc_vid = strtoll(argv[2], NULL, 10);
		str_to_mac(mac, argv[3]);
		memcpy(args5.dst_mac, mac, sizeof(mac));
		args5.mc_px_en = strtoll(argv[4], NULL, 10);
		args5.mc_px_qos_en = strtoll(argv[5], NULL, 10);
		args5.mc_qos_qid = strtoll(argv[6], NULL, 10);
		break;
	case 'C':
		method = HW_NAT_MCAST_DEL;
		args5.mc_vid = strtoll(argv[2], NULL, 10);
		str_to_mac(mac, argv[3]);
		memcpy(args5.dst_mac, mac, sizeof(mac));
		memcpy(args5.dst_mac, mac, sizeof(mac));
		args5.mc_px_en = strtoll(argv[4], NULL, 10);
		args5.mc_px_qos_en = strtoll(argv[5], NULL, 10);
		break;
	case 'D':
		method = HW_NAT_MCAST_DUMP;
		break;
#endif
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
	case 'V':
		method = HW_NAT_VLAN_ID;
		args4.lan_vid = strtoll(argv[2], NULL, 10);
		args4.wan_vid = strtoll(argv[3], NULL, 10);
		break;
	case 'Z':
		method = HW_NAT_BIND_DIRECTION;
		args4.bind_dir = strtoll(optarg, NULL, 10);
		break;
	case '?':
		show_usage();
	}
    }

    switch(method){
    case HW_NAT_GET_ALL_ENTRIES:
	    result = HwNatGetAllEntries(entry_state);
	    break;
    case HW_NAT_DUMP_ENTRY:
	    result = HwNatDumpEntry(entry_num);
	    break;
    case HW_NAT_UNBIND_ENTRY:
	    result = HwNatUnBindEntry(entry_num);
	    break;
    case HW_NAT_DEBUG:
	    result = HwNatDebug(debug);
	    break;
#if !defined (CONFIG_HNAT_V2)
    case HW_NAT_DSCP_REMARK:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_VPRI_REMARK:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_FOE_WEIGHT:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_ACL_WEIGHT:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_DSCP_WEIGHT:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_VPRI_WEIGHT:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_DSCP_UP:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_UP_IDSCP:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_UP_ODSCP:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_UP_VPRI:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_UP_AC:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_SCH_MODE:
	    result = HwNatSetQoS(&args3, method);
	    break;
    case HW_NAT_SCH_WEIGHT:
	    result = HwNatSetQoS(&args3, method);
	    break;
#else
    case HW_NAT_DROP_ENTRY:
	    result = HwNatDropEntry(entry_num);
	    break;
    case HW_NAT_DUMP_CACHE_ENTRY:
	    result = HwNatCacheDumpEntry();
	    break;
    case HW_NAT_GET_AC_CNT:
	    result = HwNatGetAGCnt(&args3);
	    break;
#endif
    case HW_NAT_BIND_THRESHOLD:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_MAX_ENTRY_LMT:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_RULE_SIZE:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_KA_INTERVAL:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_UB_LIFETIME:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_BIND_LIFETIME:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_VLAN_ID:
	    result = HwNatSetConfig(&args4, method);
	    break;
    case HW_NAT_BIND_DIRECTION:
	    result = HwNatSetConfig(&args4, method);
	    break;
#if defined (CONFIG_PPE_MCAST)
    case HW_NAT_MCAST_INS:
	    result = HwNatMcastIns(&args5);
	    break;
    case HW_NAT_MCAST_DEL:
	    result = HwNatMcastDel(&args5);
	    break;
    case HW_NAT_MCAST_DUMP:
	    result = HwNatMcastDump();
	    break;
#endif
    default:
	    result = HWNAT_FAIL;
	    break;
    }

    if (result == HWNAT_SUCCESS){
	printf("done\n");
    } else if(result == HWNAT_ENTRY_NOT_FOUND) {
	printf("entry not found\n");
    } else {
	return 1;
    }

    return 0;
}
