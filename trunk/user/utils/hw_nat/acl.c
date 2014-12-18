#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>

#include "acl_ioctl.h"
#include "acl_api.h"
#include "util.h"

void show_usage(void)
{
    printf("Add SDMAC  Entry for Any Protocol\n");
    printf("acl -A -n [SDMAC] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -A -n 00:11:22:33:44:55  -u Deny \n\n");
    printf("Ex: acl -A -n 00:11:22:33:44:55  -U 3 -u FP \n\n");

    printf("Add SMAC to DIP Entry for Any Protocol\n");
    printf("acl -a -n [SMAC] -q [DipS] -r [DipE] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -a -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -u Deny \n\n");
    printf("Ex: acl -a -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -U 3 -u FP \n\n");

    printf("Add SMAC to DIP Entry for Tcp Protocol\n");
    printf("acl -b  -n [SMAC] -q [DipS] -r [DipE] -s [DpS] -t [DpE] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -b -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -s 1 -t 1024 -u Deny\n\n");

    printf("SMAC to DIP Entry for Udp Protocol\n");
    printf("acl -c  -n [SMAC] -q [DipS] -r [DipE] -s [DpS] -t [DpE] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -c -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -s 1 -t 1024 -u Deny\n\n");

    printf("Del SDMAC  Entry for Any Protocol\n");
    printf("acl -D -n [SMAC]\n");
    printf("Ex: acl -D -n 00:11:22:33:44:55 \n\n");

    printf("Del SMAC to DIP Entry for Any Protocol\n");
    printf("acl -d -n [SMAC] -q [DipS] -r [DipE]\n");
    printf("Ex: acl -d -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5\n\n");

    printf("Del SMAC to DIP Entry for Tcp Protocol\n");
    printf("acl -e  -n [SMAC] -q [DipS] -r [DipE] -s [DpS] -t [DpE]\n");
    printf("Ex: acl -e -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -s 1 -t 1024\n\n");

    printf("Del SMAC to DIP Entry for Udp Protocol\n");
    printf("acl -f  -n [SMAC] -q [DipS] -r [DipE] -s [DpS] -t [DpE]\n");
    printf("Ex: acl -f -n 00:11:22:33:44:55 -q 10.10.10.3 -r 10.10.10.5 -s 1 -t 1024\n\n");

    printf("Add SIP to DIP Entry for Any Protocol\n");
    printf("acl -H  -o [SipS] -p [SipE] -q [DipS] -r [DipE] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -H -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -u Deny\n\n");

    printf("Add SIP to DIP Entry for Tcp Protocol\n");
    printf("acl -h  -o [SipS] -p [SipE] -q [DipS] -r [DipE] -s [DpS] -t [DpE] -U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -h -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024 -u Deny\n\n");

    printf("Add SIP to DIP Entry for Udp Protocol\n");
    printf("acl -i  -o [SipS] -p [SipE] -q [DipS] -r [DipE] -s [DpS] -t [DpE]-U[UP] -u [Allow/Deny/FP]\n");
    printf("Ex: acl -i -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024 -u Deny\n\n");

    printf("Del SIP to DIP Entry for Any Protocol\n");
    printf("acl -j  -o [SipS] -p [SipE] -q [DipS] -r [DipE]\n");
    printf("Ex: acl -j -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3\n\n");

    printf("Del SIP to DIP Entry for Tcp Protocol\n");
    printf("acl -k  -o [SipS] -p [SipE] -q [DipS] -r [DipE] -s [DpS] -t [DpE]\n");
    printf("Ex: acl -k -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024\n\n");

    printf("Del SIP to DIP Entry for Udp Protocol\n");
    printf("acl -l  -o [SipS] -p [SipE] -q [DipS] -r [DipE] -s [DpS] -t [DpE]\n");
    printf("Ex: acl -l -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024\n\n");
    printf("Del All Entries\n");
    printf("acl -m\n\n");

    printf("Add S/DMAC ETYPE VID PROTOCOL SIP DIP SP DP Entry\n");
    printf("acl -E -n[SMAC] -N[DMAC] -P[ESW Port] -Z[Ethertype] -S[Protocol] -o[SipS] -p[SipE] -q[DipS] -r[DipE] -s[DpS] -t[DpE] -v[SpS] -x[SpE] -y[TosS] -z[TosE] -F[TCP/UDP/ANY] -V[VID] -u[Allow/Deny/FP]\n");
    printf("Ex: acl -E  -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024 -F UDP -u Deny\n\n");

    printf("Del S/DMAC ETYPE  VID PROTOCOL SIP DIP SP DP Entry\n");
    printf("acl -G -n[SMAC] -N[DMAC] -P[ESW Port] -Z[Ethertype] -S[Protocol] -o[SipS] -p[SipE] -q[DipS] -r[DipE] -s[DpS] -t[DpE] -v[SpS] -x[SpE] -y[TosS] -z[TosE] -F[TCP/UDP/ANY] -V[VID] \n");
    printf("Ex: acl -G  -o 10.10.10.3 -p 10.10.10.5 -q 10.10.20.3 -r 10.10.20.3 -s 1 -t 1024 -F UDP\n\n");

    printf("Show All ACL Entry\n");
    printf("Ex: acl -g\n\n");

}

int main(int argc, char *argv[])
{
    int opt;
    char options[] = "AabcDdEefGgHhijklm?F:n:N:o:P:p:q:r:s:S:t:u:U:v:x:y:V:z:Z:";
    struct acl_args args;
    struct acl_list_args *args2;
    int method=-1;
    int result = 0;
    int i;

    memset(&args, 0, sizeof(struct acl_args));
    args.pn = 7; /* Default do not care*/

    if (argc < 2) {
	show_usage();
	return 0;
    }

    while ((opt = getopt (argc, argv, options)) != -1) {
	switch (opt) {
	case 'A': 
		method=ACL_ADD_SDMAC_ANY;
		break;
	case 'a': 
		method=ACL_ADD_SMAC_DIP_ANY;
		break;
	case 'b':
		method=ACL_ADD_SMAC_DIP_TCP;
		break;
	case 'c':
		method=ACL_ADD_SMAC_DIP_UDP;
		break;
	case 'D': 
		method=ACL_DEL_SDMAC_ANY;
		break;
	case 'd': 
		method=ACL_DEL_SMAC_DIP_ANY;
		break;
	case 'e':
		method=ACL_DEL_SMAC_DIP_TCP;
		break;
	case 'E':
		method=ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT;
		break;
	case 'F':
                if(strcasecmp(optarg,"TCP")==0){
                            args.L4=ACL_PROTO_TCP;
	                }else if(strcasecmp(optarg,"UDP")==0){
	                        args.L4=ACL_PROTO_UDP;
	                    }else if(strcasecmp(optarg,"ANY")==0){
	                        args.L4=ACL_PROTO_ANY;
	                }else{
                            printf("Error: -t TCP or UDP or ANY\n");
                        return 0;
                }

		break;
	case 'G':
		method=ACL_DEL_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT;
		break;
	case 'f':
		method=ACL_DEL_SMAC_DIP_UDP;
		break;
	case 'H':
		method=ACL_ADD_SIP_DIP_ANY;
		break;
        case 'g':
                method = ACL_GET_ALL_ENTRIES;
		break;
	case 'h':
		method=ACL_ADD_SIP_DIP_TCP;
		break;
	case 'i':
		method=ACL_ADD_SIP_DIP_UDP;
		break;
	case 'j':
		method=ACL_DEL_SIP_DIP_ANY;
		break;
	case 'k':
		method=ACL_DEL_SIP_DIP_TCP;
		break;
	case 'l':
		method=ACL_DEL_SIP_DIP_UDP;
		break;
	case 'm':
		method=ACL_CLEAN_TBL;
		break;
	case 'n': /* source mac address */
		str_to_mac(args.mac, optarg);
		break;
	case 'N': /* destination mac address */
		str_to_mac(args.dmac, optarg);
		break;
	case 'o': /* start of sip */
		str_to_ip(&args.sip_s, optarg);
		break;
	case 'p': /* end of sip */
		str_to_ip(&args.sip_e, optarg);
		break;
	case 'P': /* Port Number */
		args.pn=strtoll(optarg, NULL, 10);
		break;
	case 'q': /* start of dip */
		str_to_ip(&args.dip_s, optarg);
		break;
	case 'r': /* end of dip */
		str_to_ip(&args.dip_e, optarg);
		break;
	case 's': /* start of dp */
		args.dp_s=strtoll(optarg, NULL, 10);
		break;
	case 't': /* end of dp */
		args.dp_e=strtoll(optarg, NULL, 10);
		break;
	case 'S': /* Protocol */
		args.protocol=strtoll(optarg, NULL, 10);
		break;

	case 'v': /* start of sp */
		args.sp_s=strtoll(optarg, NULL, 10);
		break;
	case 'x': /* end of sp */
		args.sp_e=strtoll(optarg, NULL, 10);
		break;
	case 'y': /* start of tos */
		args.tos_s=strtoll(optarg, NULL, 10);
		break;
	case 'z': /* end of tos */
		args.tos_e=strtoll(optarg, NULL, 10);
		break;
	case 'Z': /* ethertype */
		args.ethertype=strtoll(optarg, NULL, 16);
		break;
	case 'V': /* VID */
		args.vid=strtoll(optarg, NULL, 10);
		break;
	case 'u': /* Deny/Allow */
		if(strcasecmp(optarg,"Deny")==0){
			args.method=ACL_DENY_RULE;
		}else if(strcasecmp(optarg,"Allow")==0){
			args.method=ACL_ALLOW_RULE;
		}else if(strcasecmp(optarg,"FP")==0){
			args.method=ACL_PRIORITY_RULE;
		}else{
			printf("Error: -t Deny or Allow\n");
			return 0;
		}
		break;
	case 'U': /* User Priority */
		args.up=strtoll(optarg, NULL, 10);
		break;
	case '?':
	default:
	    show_usage();
            return 0;
	}
    }

    switch(method) {
    case ACL_ADD_SDMAC_ANY:
    case ACL_ADD_ETYPE_ANY:	
    case ACL_ADD_SMAC_DIP_ANY:
    case ACL_ADD_SMAC_DIP_TCP:
    case ACL_ADD_SMAC_DIP_UDP:
    case ACL_DEL_SDMAC_ANY:
    case ACL_DEL_ETYPE_ANY:
    case ACL_DEL_SMAC_DIP_ANY:
    case ACL_DEL_SMAC_DIP_TCP:
    case ACL_DEL_SMAC_DIP_UDP:
    case ACL_ADD_SIP_DIP_ANY:
    case ACL_ADD_SIP_DIP_TCP:
    case ACL_ADD_SIP_DIP_UDP:
    case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:	
    case ACL_DEL_SIP_DIP_ANY:
    case ACL_DEL_SIP_DIP_TCP:
    case ACL_DEL_SIP_DIP_UDP:
    case ACL_DEL_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
    case ACL_CLEAN_TBL:
	      SetAclEntry(&args, method);
	      result = args.result;
	      break;
    case ACL_GET_ALL_ENTRIES:
	      /* Max 511 acl entries */
	      args2=malloc(sizeof(struct acl_list_args) + sizeof(struct acl_args)*511);
	      if (!args2) {
		printf("Allocate memory for acl_list_args failed!\n");
		return 1;
	      }
	      AclGetAllEntries(args2);
	      result = args2->result;
	      printf("Total Entry Count = %d\n",args2->num_of_entries);
	      for(i=0;i<args2->num_of_entries;i++){
		  printf("#%d :SMAC=%02X:%02X:%02X:%02X:%02X:%02X => DMAC=%02X:%02X:%02X:%02X:%02X:%02X PROTOCOL=0x%2x\n", \
			  i, args2->entries[i].mac[0], args2->entries[i].mac[1], args2->entries[i].mac[2], \
			  args2->entries[i].mac[3], args2->entries[i].mac[4], args2->entries[i].mac[5], \
			  args2->entries[i].dmac[0], args2->entries[i].dmac[1],args2->entries[i].dmac[2], \
			  args2->entries[i].dmac[3], args2->entries[i].dmac[4],args2->entries[i].dmac[5], \
			  args2->entries[i].protocol);
		  printf("   :SIP %u.%u.%u.%u->%u.%u.%u.%u=>DIP %u.%u.%u.%u->%u.%u.%u.%u  SP %d->%d=>DP %d->%d TOS:0x%2x->0x%2x VID:%d ETYPE=0x%4x TCP_UDP=0/TCP=1/UDP=2:%d PN:%d\n\r", \
			  NIPQUAD(args2->entries[i].sip_s), \
			  NIPQUAD(args2->entries[i].sip_e), \
			  NIPQUAD(args2->entries[i].dip_s), \
			  NIPQUAD(args2->entries[i].dip_e), \
			  args2->entries[i].sp_s, \
			  args2->entries[i].sp_e, \
			  args2->entries[i].dp_s, \
			  args2->entries[i].dp_e, \
			  args2->entries[i].tos_s, \
		    args2->entries[i].tos_e, \
		    args2->entries[i].vid, \
		    args2->entries[i].ethertype, \
		    args2->entries[i].L4, \
		    args2->entries[i].pn);
	      }
	      free(args2);
	      break;
    default:
	    result = ACL_FAIL;
	    break;
    }

    if(result == ACL_SUCCESS) {
	printf("done\n");
    }else if (result ==  ACL_TBL_FULL) {
	printf("table full\n");
    } else {
	printf("fail\n");
    }

    return 0;
}
