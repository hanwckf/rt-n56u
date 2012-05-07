#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <strings.h>

#include "hwnat_ioctl.h"
#include "hwnat_api.h"


void show_usage(void)
{
    printf("Add Static Entry\n");
    printf("hw_nat -a -h [SMAC] -i [DMAC] -j [Sip] -k [Dip] -l [Sp] -m [Dp]\n");
    printf("	   -n [New_Sip] -o [New_Dip] -p [New_Sp] -q [New_Dp] -r [Vlan1:No/Mod/Ins/Del]\n");
    printf("	   -s [VLAN1_ID] -R [Vlan2:No/Mod/Ins/Del] -S [VLAN2_ID]\n");
    printf("	   -t [PPPoE:No/Mod/Ins/Del] -u [PPPoE_ID] -v [Tcp/Udp] -w [OutIf:CPU/GE1/GE2]\n");
    printf("Ex: hw_nat -a -h 00:11:22:33:44:55 -i 11:22:33:44:55:66 -j 10.10.10.3 -k 10.10.20.3\n");
    printf("           -l 30 -m 40 -n 10.10.20.254 -o 10.10.20.3 -p 40 -q 50 -r Ins -s 2 -t No \n");
    printf("           -u 0 -v Tcp -w GE1\n\n");

    printf("Del Static Entry\n");
    printf("hw_nat -b -j [Sip] -k [Dip] -l [Sp] -m [Dp] -v [Tcp/Udp] \n");
    printf("Ex: hw_nat -b  -j 10.10.10.3 -k 10.10.20.3 -l 30 -m 40 -v Tcp\n\n");

    printf("Show Foe Entry\n");
    printf("hw_nat -c [entry_num]\n");
    printf("Ex: hw_nat -c 1234\n\n");

    printf("Set Debug Level (0:disable) \n");
    printf("hw_nat -d [0/1]\n");
    printf("Ex: hw_nat -d \n\n");

    printf("Show All Foe Invalid Entry\n");
    printf("Ex: hw_nat -e\n\n");

    printf("Show All Foe Unbinded Entry\n");
    printf("Ex: hw_nat -f\n\n");

    printf("Show All Foe Binded Entry\n");
    printf("Ex: hw_nat -g\n\n");

    printf("Bind Entry (for semi-binding mode)\n");
    printf("Ex: hw_nat -x [entry_num]\n\n");

    printf("UnBind Entry (for semi-binding mode)\n");
    printf("Ex: hw_nat -y [entry_num]\n\n");

    printf("Invalid Entry\n");
    printf("Ex: hw_nat -z [entry_num]\n\n");

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

    printf("Set HNAT QOS Mode\n");
#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855)
    printf("Ex: hw_nat -L [0:WRR, 1:SPQ, 2:Q3>WRR(Q2,Q1,Q0), 3:Q3>Q2>WRR(Q1,Q0)]\n\n");
#else
    printf("Ex: hw_nat -L [0:WRR, 1:SPQ, 2:Q3>WRR(Q2,Q1,Q0)]\n\n");
#endif

    printf("Set the weight of GDMA Scheduler\n");
    printf("hw_nat -M Q3(1/2/4/8) Q2(1/2/4/8) Q1(1/2/4/8) Q0(1/2/4/8)\n\n");
    printf("hw_nat -M 8 4 2 1\n\n");

    printf("Set PPE Cofigurations:\n");
    printf("Set HNAT binding threshold per second (d=30)\n");
    printf("Ex: hw_nat -N [1~65535]\n\n");

    printf("Set HNAT Max entries allowed build when Free Entries>3/4, >1/2, <1/2 (d=100, 50, 25)\n");
    printf("Ex: hw_nat -O [1~16383][1~16383][1~16383]\n\n");

    printf("Set HNAT PreACL/PreMeter/PreAC/PostMeter/PostAC table size (d=383, 32, 32, 32, 32)\n");
    printf("Ex: hw_nat -P [0~511][0~64][0~64][0~64][0~64]\n");
    printf("NOTE: Total 511 rules, PreAC+PostAC<=64, PreMeter+PostMeter<=64\n\n");

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
    char options[] = "abefg?c:A:B:C:D:E:F:G:H:I:J:K:L:M:R:S:d:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:N:O:P:Q:T:U:Z:";
    int fd, method=0;
    unsigned int entry_num=0, debug=0, dir=0;
    struct hwnat_args *args;
    struct hwnat_tuple args2;
    struct hwnat_qos_args args3;
    struct hwnat_config_args args4;
    int i,result=0;

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
	case 'h':
		 str_to_mac(args2.smac, optarg);
		 break;
	case 'i':
		 str_to_mac(args2.dmac, optarg);
		 break;
	case 'j':
		 str_to_ip(&args2.sip, optarg);
		 break;
	case 'k':
		 str_to_ip(&args2.dip, optarg);
		 break;
	case 'l':
		 args2.sport = strtoll(optarg, NULL, 10);
		 break;
	case 'm':
		 args2.dport = strtoll(optarg, NULL, 10);
		 break;
	case 'n':
		 str_to_ip(&args2.new_sip, optarg);
		 break;
	case 'o':
		 str_to_ip(&args2.new_dip, optarg);
		 break;
	case 'p':
		 args2.new_sport = strtoll(optarg, NULL, 10);
		 break;
	case 'q':
		 args2.new_dport = strtoll(optarg, NULL, 10);
		 break;
	case 'r':
		 if(strcasecmp(optarg,"No")==0){
			 args2.vlan1_act=0;
		 }else if(strcasecmp(optarg,"Mod")==0){
			 args2.vlan1_act=1;
		 }else if(strcasecmp(optarg,"Ins")==0){
			 args2.vlan1_act=2;
		 }else if(strcasecmp(optarg,"Del")==0){
			 args2.vlan1_act=3;
		 }else{
			 printf("Error: -r No/Mod/Ins/Del\n");
			 return 0;
		 }
		 break;
	case 's':
		 args2.vlan1 = strtoll(optarg, NULL, 10);
                 break;
	case 'R':
		 if(strcasecmp(optarg,"No")==0){
			 args2.vlan2_act=0;
		 }else if(strcasecmp(optarg,"Mod")==0){
			 args2.vlan2_act=1;
		 }else if(strcasecmp(optarg,"Ins")==0){
			 args2.vlan2_act=2;
		 }else if(strcasecmp(optarg,"Del")==0){
			 args2.vlan2_act=3;
		 }else{
			 printf("Error: -r No/Mod/Ins/Del\n");
			 return 0;
		 }
		 break;
	case 'S':
		 args2.vlan2 = strtoll(optarg, NULL, 10);
		 break;
	case 't':
		 if(strcasecmp(optarg,"No")==0){
			 args2.pppoe_act=0;
		 }else if(strcasecmp(optarg,"Mod")==0){
			 args2.pppoe_act=1;
		 }else if(strcasecmp(optarg,"Ins")==0){
			 args2.pppoe_act=2;
		 }else if(strcasecmp(optarg,"Del")==0){
			 args2.pppoe_act=3;
		 }else{
			 printf("Error: -t No/Mod/Ins/Del\n");
			 return 0;
		 }
		 break;
	case 'u':
		 args2.pppoe_id = strtoll(optarg, NULL, 10);
		 break;
	case 'v':
		 if(strcasecmp(optarg,"Tcp")==0){
			 args2.is_udp=0;
		 }else if(strcasecmp(optarg,"Udp")==0){
			 args2.is_udp=1;
		 }else {
			 printf("Error: -v Tcp/Udp\n");
			 return 0;
		 }
		 break;
	case 'w':
		 if(strcasecmp(optarg,"CPU")==0){
			 args2.dst_port=0; 
		 }else if(strcasecmp(optarg,"GE1")==0){
			 args2.dst_port=1; 
		 }else if(strcasecmp(optarg,"GE2")==0){
			 args2.dst_port=2; 
		 }else {
			 printf("Error: -w CPU/GE1/GE2\n");
			 return 0;
		 }
		 break;
	case 'a':
		method= HW_NAT_ADD_ENTRY;
		break;
	case 'b':
		method= HW_NAT_DEL_ENTRY;
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
	case 'c':
		method = HW_NAT_DUMP_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'd':
		method = HW_NAT_DEBUG;
		debug = strtoll(optarg, NULL, 10);
		break;
	case 'x':
		method = HW_NAT_BIND_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'y':
		method = HW_NAT_UNBIND_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
	case 'z':
		method = HW_NAT_INVALID_ENTRY;
		entry_num = strtoll(optarg, NULL, 10);
		break;
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
    case HW_NAT_ADD_ENTRY:
	    HwNatAddEntry(&args2);
	    result = args2.result;
	    break;
    case HW_NAT_DEL_ENTRY:
	    HwNatDelEntry(&args2);
	    result = args2.result;
	    break;
    case HW_NAT_GET_ALL_ENTRIES:
	    HwNatGetAllEntries(args);

	    printf("Total Entry Count = %d\n",args->num_of_entries);
	    for(i=0;i<args->num_of_entries;i++){
		if(args->entries[i].fmt==0) { //IPV4_NAPT
		    printf("%d : %u.%u.%u.%u:%d->%u.%u.%u.%u:%d => %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n", \
			    args->entries[i].hash_index, \
			    NIPQUAD(args->entries[i].sip), \
			    args->entries[i].sport, \
			    NIPQUAD(args->entries[i].dip), \
			    args->entries[i].dport, \
			    NIPQUAD(args->entries[i].new_sip), \
		    args->entries[i].new_sport, \
		    NIPQUAD(args->entries[i].new_dip), \
		    args->entries[i].new_dport);
		} else if(args->entries[i].fmt==1) { //IPV4_NAT
		    printf("%d : %u.%u.%u.%u->%u.%u.%u.%u => %u.%u.%u.%u->%u.%u.%u.%u\n", \
			    args->entries[i].hash_index, \
			    NIPQUAD(args->entries[i].sip), \
			    NIPQUAD(args->entries[i].dip), \
			    NIPQUAD(args->entries[i].new_sip), \
			    NIPQUAD(args->entries[i].new_dip)); 
		} else if(args->entries[i].fmt==2) { //IPV6_ROUTING
		    printf("IPv6 Entry= %d /SIP: %x:%x:%x:%x:%x:%x:%x:%x/DIP: %x:%x:%x:%x:%x:%x:%x:%x\n", \
		    args->entries[i].hash_index, \
		    NIPHALF(args->entries[i].ipv6_dip3), \
		    NIPHALF(args->entries[i].ipv6_dip2), \
		    NIPHALF(args->entries[i].ipv6_dip1), \
		    NIPHALF(args->entries[i].ipv6_dip0));
		} else{
		    printf("Wrong entry format!\n");
		}
	    }
	    result = args->result;
	    break;
    case HW_NAT_DUMP_ENTRY:
	    result = HwNatDumpEntry(entry_num);
	    break;
    case HW_NAT_BIND_ENTRY:
	    result = HwNatBindEntry(entry_num);
	    break;
    case HW_NAT_UNBIND_ENTRY:
	    result = HwNatUnBindEntry(entry_num);
	    break;
    case HW_NAT_INVALID_ENTRY:
	    result = HwNatInvalidEntry(entry_num);
	    break;
    case HW_NAT_DEBUG:
	    result = HwNatDebug(debug);
	    break;
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
    }else {
	printf("fail\n");
    }

    free(args);
    return 0;
}
