#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#include "mtr_ioctl.h"
#include "mtr_api.h"
#include "util.h"

void show_usage(void)
{

    printf("Add Mac Upload Meter Rule\n");
    printf("mtr -a -m [Mac] -t [KB/s] -s [BucketSize:4K/8K/16K/32K or 0~127]\n");
    printf("    -u [MtrIntval:1ms/10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms] -v [Base:Byte/Pkt]\n");
    printf("ByteBase:mtr -a -m 00:11:22:33:44:55 -t 10 -s 8K -v Byte\n\n");
    printf("PktBase:mtr -a -m 00:11:22:33:44:55 -s 100 -u 1ms -v Pkt\n\n");

    printf("Add Mac Download Meter Rule\n");
    printf("mtr -b -m [Mac] -t [KB/s] -s [BucketSize:4K/8K/16K/32K or 0~127]\n");
    printf("    -u [MtrIntval:1ms/10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms] -v [Base:Byte/Pkt]\n");
    printf("ByteBase: mtr -b -m 00:11:22:33:44:55 -t 10 -s 8K -v Byte\n\n");
    printf("PktBase: mtr -b -m 00:11:22:33:44:55 -s 100 -u 1ms -v Pkt\n\n");
    printf("Del Mac Upload Meter Rule\n");
    printf("mtr -c -m [Mac]\n");
    printf("Ex: mtr -c -m 00:11:22:33:44:55\n\n");

    printf("Del Mac download Meter Rule\n");
    printf("mtr -d -m [Mac]\n");
    printf("Ex: mtr -d -m 00:11:22:33:44:55\n\n");

    printf("Add IP Upload Meter Rule\n");
    printf("mtr -e -i [IpS] -j [IpE] -t [KB/s] -s [BucketSize:4K/8K/16K/32K or 0~127]\n");
    printf("    -u [MtrIntval:1ms/10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms] -v [Base:Byte/Pkt]\n");
    printf("ByteBase: mtr -e -i 10.10.10.3 -j 10.10.10.3 -t 10 -s 8K -v Byte\n\n");
    printf("PktBase: mtr -e -i 10.10.10.3 -j 10.10.10.3 -s 100 -u 1ms -v Pkt\n\n");

    printf("Add IP Download Meter Rule\n");
    printf("mtr -f -i [IpS] -j [IpE] -t [KB/s] -s [BucketSize:4K/8K/16K/32K or 0~127] \n");
    printf("    -u [MtrIntval:1ms/10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms] -v [Base:Byte/Pkt]\n");
    printf("ByteBase: mtr -f -i 10.10.10.3 -t 10 -s 8K -v Byte\n\n");
    printf("PktBase: mtr -f -i 10.10.10.3 -j 10.10.10.3 -s 100 -u 50ms -v Pkt\n\n");

    printf("Del IP Upload Meter Rule\n");
    printf("mtr -g -i [IpS] -j [IpE]\n");
    printf("mtr -g -i 10.10.10.3 -j 10.10.10.3\n\n");

    printf("Del IP Download Meter Rule\n");
    printf("mtr -h -i [IpS] -j [IpE]\n");
    printf("mtr -h -i 10.10.10.3 -j 10.10.10.3\n\n");

    printf("Clear Meter Table\n");
    printf("mtr -z\n\n");

    printf("Add SYN  Meter Rule: mtr -k\n");
    printf("Add FIN  Meter Rule: mtr -l\n");
    printf("Add UDP  Meter Rule: mtr -n\n");
    printf("Add ICMP Meter Rule: -o\n");
    printf(" -t [KB/s] -s [BucketSize:4K/8K/16K/32K or 0~127]\n");
    printf(" -u [MtrIntval:1ms/10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms] -v [Base:Byte/Pkt]\n\n");
    printf("Del SYN  Meter Rule:mtr -p\n");
    printf("Del FIN  Meter Rule:mtr -q\n");
    printf("Del UDP  Meter Rule:mtr -r\n");
    printf("Del ICMP Meter Rule:mtr -y\n\n");

    printf("Get All Mtr Entries: mtr -w\n");
}

int main(int argc, char *argv[])
{
    int opt;
    char options[] = "abcdefghklnopqryz?m:i:j:t:s:u:v:w";

    int method=-1;
    struct mtr_args args;
    struct mtr_list_args *args2;
    int result = 0;
    int i;

    if(argc < 2) {
	show_usage();
	return 0;
    }

    while ((opt = getopt (argc, argv, options)) != -1) {
	switch (opt) {
	case 'a':  
	    method=MTR_ADD_MAC_UL_ENTRY;
	    break;
	case 'b':  
	    method=MTR_ADD_MAC_DL_ENTRY;
	    break;
	case 'c': 
	    method=MTR_DEL_MAC_UL_ENTRY;
	    break;
	case 'd': 
	    method=MTR_DEL_MAC_DL_ENTRY;
	    break;
	case 'e': 
	    method=MTR_ADD_IP_UL_ENTRY;
	    break;
	case 'f': 
	    method=MTR_ADD_IP_DL_ENTRY;
	    break;
	case 'g': 
	    method=MTR_DEL_IP_UL_ENTRY;
	    break;
	case 'h': 
	    method=MTR_DEL_IP_DL_ENTRY;
	    break;
	case 'k': 
	    method=MTR_ADD_SYN_ENTRY;
	    break;
	case 'l': 
	    method=MTR_ADD_FIN_ENTRY;
	    break;
	case 'n': 
	    method=MTR_ADD_UDP_ENTRY;
	    break;
 	case 'o': 
	    method=MTR_ADD_ICMP_ENTRY;
	    break;
	case 'p': 
	    method=MTR_DEL_SYN_ENTRY;
	    break;
	case 'q': 
	    method=MTR_DEL_FIN_ENTRY;
	    break;
	case 'r': 
	    method=MTR_DEL_UDP_ENTRY;
	    break;
	case 'y': 
	    method=MTR_DEL_ICMP_ENTRY;
	    break;
	case 'z': /* CleanTbl */
	    method=MTR_CLEAN_TBL;
	    break;
	case 'm': /* Mac */
	    str_to_mac(args.mac, optarg);
	    break;
	case 'i': /* IpS */
	    str_to_ip(&args.ip_s, optarg);
	    break;
	case 'j': /* IpE */
	    str_to_ip(&args.ip_e, optarg);
	    break;
	case 't': /* TokenRate */
	    args.token_rate=strtoll(optarg, NULL, 10);
	    break;
	case 's': /* Bucket Size */
	    if(strcasecmp(optarg,"4K")==0){
		args.bk_size=0;
	    }else if(strcasecmp(optarg,"8K")==0){
		args.bk_size=1;
	    }else if(strcasecmp(optarg,"16K")==0){
		args.bk_size=2;
	    }else if(strcasecmp(optarg,"32K")==0){
		args.bk_size=3;
	    }else {
		args.bk_size=strtoll(optarg, NULL, 10);
	    }
	    break;
	case 'u':
	    if(strcasecmp(optarg,"1ms")==0){
		args.mtr_intval=_1MS;
	    }else if(strcasecmp(optarg,"10ms")==0){
		args.mtr_intval=_10MS;
	    }else if(strcasecmp(optarg,"50ms")==0){
		args.mtr_intval=_50MS;
	    }else if(strcasecmp(optarg,"100ms")==0){
		args.mtr_intval=_100MS;
	    }else if(strcasecmp(optarg,"500ms")==0){
		args.mtr_intval=_500MS;
	    }else if(strcasecmp(optarg,"1000ms")==0){
		args.mtr_intval=_1000MS;
	    }else if(strcasecmp(optarg,"5000ms")==0){
		args.mtr_intval=_5000MS;
	    }else if(strcasecmp(optarg,"10000ms")==0){
		args.mtr_intval=_10000MS;
	    }else {
		printf("Error: -u 10ms/50ms/100ms/500ms/1000ms/5000ms/10000ms\n");
		return 0;
	    }
	    break;
	case 'v':
	    if(strcasecmp(optarg,"Byte")==0){
		args.mtr_mode=0;
	    }else if(strcasecmp(optarg,"Pkt")==0){
		args.mtr_mode=1;
	    }else {
		printf("Error: -v Byte/Pkt\n");
		return 0;
	    }
	    break;
	case 'w':
	    method=MTR_GET_ALL_ENTRIES;
	    break;
	case '?': /* Help */
	    show_usage();
	    break;
	}
    } 

    switch(method) 
    {
    case MTR_ADD_MAC_UL_ENTRY:
    case MTR_ADD_MAC_DL_ENTRY:
    case MTR_DEL_MAC_UL_ENTRY:
    case MTR_DEL_MAC_DL_ENTRY:
    case MTR_ADD_IP_UL_ENTRY:
    case MTR_ADD_IP_DL_ENTRY:
    case MTR_DEL_IP_UL_ENTRY:
    case MTR_DEL_IP_DL_ENTRY:
    case MTR_CLEAN_TBL:
    case MTR_ADD_SYN_ENTRY:
    case MTR_ADD_FIN_ENTRY:
    case MTR_ADD_UDP_ENTRY:
    case MTR_ADD_ICMP_ENTRY:
    case MTR_DEL_SYN_ENTRY:
    case MTR_DEL_FIN_ENTRY:
    case MTR_DEL_UDP_ENTRY:
    case MTR_DEL_ICMP_ENTRY:
	    SetMtrEntry(&args, method);
	    result = args.result;
	    break;
    case MTR_GET_ALL_ENTRIES:
	    args2 = malloc(sizeof(struct mtr_list_args) + sizeof(struct mtr_args)*511);
	    if (!args2) {
		printf("Allocate memory for mtr_list_args failed!\n");
		return 1;
	    }
	    MtrGetAllEntries(args2);
	    result = args2->result;
	    printf("Total Entry Count = %d\n", args2->num_of_entries);
	    for(i=0;i<args2->num_of_entries;i++){
		printf("#%d :MAC=%02X:%02X:%02X:%02X:%02X:%02X\n", \
			i, args2->entries[i].mac[0], args2->entries[i].mac[1], args2->entries[i].mac[2], \
			args2->entries[i].mac[3], args2->entries[i].mac[4], args2->entries[i].mac[5]);
		printf("   :SIP %u.%u.%u.%u->%u.%u.%u.%u\n\r", NIPQUAD(args2->entries[i].ip_s), NIPQUAD(args2->entries[i].ip_e));
		printf("   :BucketSize=%d Token_Rate:%d MtrInterval=%d\n", args2->entries[i].bk_size, args2->entries[i].token_rate, args2->entries[i].mtr_intval);
	    }
	    free(args2);
	    break;
    default:
	    result = MTR_FAIL;
	    break;
    }

    if(result == MTR_TBL_FULL) {
	printf("table full\n");
    }else if(result == MTR_FAIL){
	printf("fail\n");
    }else{
	printf("done\n");
    }

    return 0;
}
