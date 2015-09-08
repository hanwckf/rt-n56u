#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include <ra_ioctl.h>

#define RT_QDMA_HELP		1

int qdma_fd = -1;

void qdma_init(void)
{
	qdma_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (qdma_fd < 0) {
		perror("socket");
		exit(0);
	}
}

void qdma_fini(void)
{
	if (qdma_fd >= 0) {
		close(qdma_fd);
		qdma_fd = -1;
	}
}

void usage(char *cmd)
{
#if RT_QDMA_HELP
	printf("Usage:\n");
	printf(" %s resv [queue] [hw_resv] [sw_resv]                       - set reservation for queue#\n", cmd);
	printf(" %s sch [queue] [sch]                                      - set scheduler for queue#\n", cmd);
	printf(" %s sch_rate [sch] [sch_en] [sch_rate]                     - set SCH rate control\n", cmd);
	printf(" %s weight [queue] [weighting]                             - set max rate weighting for queue#\n", cmd);
	printf(" %s rate [queue] [min_en] [min_rate] [max_en] [max_rate]   - set rate control for queue#\n", cmd);
	printf(" %s m2q [mark] [queue] {wan_lan_separate}                  - set skb->mark to queue mapping table\n", cmd);
	printf(" Where is:\n");
	printf("  queue: 0 ~ 15.\n");
	printf("  hw_resv and sw_resv in decimal.\n");
	printf("  sch: 0 for SCH1. 1 for SCH2.\n");
	printf("  weighting: 0 ~ 15.\n");
	printf("  sch_en: 1 for enable, 0 for disable. sch rate: 0 ~ 1000000 in Kbps.\n");
	printf("  min_en: 1 for enable, 0 for disable. min rate: 0 ~ 1000000 in Kbps.\n");
	printf("  max_en: 1 for enable, 0 for disable. max rate: 0 ~ 1000000 in Kbps.\n");
	printf("  mark: 0 ~ 63.\n");
	printf("  wan_lan_separate: 1 for split queues to 0 ~ 7 as LAN, 8 ~ 15 as WAN (with the same mark value).\n");
#endif
	qdma_fini();
	exit(0);
}

int reg_read(int offset, int *value)
{
	struct ifreq ifr;
	esw_reg reg;

	if (value == NULL)
		return -1;

	reg.off = offset;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(qdma_fd, RAETH_QDMA_REG_READ, &ifr)) {
		perror("ioctl");
		close(qdma_fd);
		exit(0);
	}
	*value = reg.val;
	return 0;
}

int reg_write(int offset, int value)
{
	struct ifreq ifr;
	esw_reg reg;

	reg.off = offset;
	reg.val = value;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(qdma_fd, RAETH_QDMA_REG_WRITE, &ifr)) {
		perror("ioctl");
		close(qdma_fd);
		exit(0);
	}
	return 0;
}

int queue_mapping(int mark, int queue)
{
	struct ifreq ifr;
	esw_reg reg;

	reg.off = mark;
	reg.val = queue;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(qdma_fd, RAETH_QDMA_QUEUE_MAPPING, &ifr)) {
		perror("ioctl");
		close(qdma_fd);
		exit(0);
	}
	return 0;
}

unsigned int rate_convert(unsigned int rate)
{
	unsigned int man;

	if(rate % 10)
		man = rate / 10 + 1;
	else
		man = rate / 10;

	return man;
}

int main(int argc, char *argv[])
{
	unsigned int off, val, hw_resv, sw_resv;
	unsigned int sch, sch_en, sch_rate, weight;
	unsigned int min_en, min_rate, max_en, max_rate, exp, man;

	qdma_init();

	if (argc < 2)
		usage(argv[0]);
	else if (!strncmp(argv[1], "resv", 5)) {
		printf("resv\n");
		
		if (argc < 5)
			usage(argv[0]);
		
		off = strtoul(argv[2], NULL, 10)* 0x10;
		hw_resv = strtoul(argv[3], NULL, 10);
		sw_resv = strtoul(argv[4], NULL, 10);
		val = (hw_resv << 8) | sw_resv;
		if (off > 0xf0 || val > 0xffff)
			usage(argv[0]);
		//val = (hw_resv << 8) | sw_resv;
		reg_write(off, val);
		printf("set offset %x as %x for reservation.\n", off, val);
	}
	else if (!strncmp(argv[1], "sch", 4)) {
		printf("sch\n");
		if (argc < 4)
			usage(argv[0]);
		off = strtoul(argv[2], NULL, 10)* 0x10 + 0x04;
		sch = strtoul(argv[3], NULL, 10);
		if (off > 0xf4 || sch > 1)
			usage(argv[0]);
		reg_read(off, &val);
		val =(val & 0x7fffffff) | (sch << 31);
		reg_write(off, val);
		printf("set offset %x as %x for sch selection.\n", off, val);
	}
	else if (!strncmp(argv[1], "weight", 7)) {
		printf("weight\n");
		if (argc < 4)
			usage(argv[0]);
		off = strtoul(argv[2], NULL, 10)* 0x10 + 0x04;
		weight = strtoul(argv[3], NULL, 10);
		if (off > 0xf4 || weight > 0xf)
			usage(argv[0]);
		reg_read(off, &val);
		val = (val & 0xffff0fff) | (weight << 12);
		reg_write(off, val);
		printf("set offset %x as %x for max rate weight.\n", off, val);
	}
	else if (!strncmp(argv[1], "rate", 5)) {
		printf("rate\n");
		if (argc < 7)
			usage(argv[0]);
		off = strtoul(argv[2], NULL, 10)* 0x10 + 0x04;
		min_en = strtoul(argv[3], NULL, 10);
		min_rate = strtoul(argv[4], NULL, 10);
		max_rate = strtoul(argv[6], NULL, 10);
		max_en = strtoul(argv[5], NULL, 10);
		if (off > 0xf4 || (min_en > 1) || (max_en > 1)|| min_rate > 1000000 || max_rate > 1000000)
			usage(argv[0]);
		if (min_rate > 127000){
			exp = 0x04;
			man = rate_convert(min_rate / 1000 );
		}else if (min_rate > 12700){
			exp = 0x03;
			man = rate_convert(min_rate / 100);
		}else if (min_rate > 1270){
			exp = 0x02;
			man = rate_convert(min_rate / 10);
		}else if (min_rate > 127){
			exp = 0x01;
			man = rate_convert(min_rate);
		}else{
			exp = 0x00;
			man = min_rate;
		}
		min_rate = (man << 4) | exp;
		
		if (max_rate > 127000){
			exp = 0x04;
			man = rate_convert(max_rate / 1000);
		}else if (max_rate > 12700){
			exp = 0x03;
			man = rate_convert(max_rate / 100);
		}else if (max_rate > 1270){
			exp = 0x02;
			man = rate_convert(max_rate / 10);
		}else if (max_rate > 127){
			exp = 0x01;
			man = rate_convert(max_rate);
		}else{
			exp = 0x00;
			man = max_rate;
		}
		max_rate = (man << 4) | exp;
		
		reg_read(off, &val);
		val = (val&0xf000f000) | (min_en <<27) | (min_rate<<16) | (max_en<<11) | (max_rate);
		reg_write(off, val);
		printf("set offset %x as %x for rate control.\n", off, val);
	}
	else if (!strncmp(argv[1], "sch_rate", 9)) {
		if (argc < 5)
			usage(argv[0]);
		off = 0x214;
		sch = strtoul(argv[2], NULL, 10);
		sch_en = strtoul(argv[3], NULL, 10);
		sch_rate = strtoul(argv[4], NULL, 10);
		if ((sch > 1 ) || (sch_en > 1) || (sch_rate > 1000000))
			usage(argv[0]);
		if (sch_rate > 127000){
			exp = 0x04;
			man = rate_convert(sch_rate / 1000 );
		}else if (sch_rate > 12700){
			exp = 0x03;
			man = rate_convert(sch_rate / 100);
		}else if (sch_rate > 1270){
			exp = 0x02;
			man = rate_convert(sch_rate / 10);
		}else if (sch_rate > 127){
			exp = 0x01;
			man = rate_convert(sch_rate);
		}else{
			exp = 0x00;
			man = sch_rate;
		}
		sch_rate = (man << 4) | exp;
		
		reg_read(off, &val);
		if (sch == 1)
			val = (val&0xffff) | (sch_en << 27) | (sch_rate <<16);
		else
			val = (val&0xffff0000) | (sch_en << 11) | (sch_rate);
		reg_write(off, val);
		printf("set offset %x as %x for sch rate control.\n", off, val);
	}
	else if (!strncmp(argv[1], "m2q", 4)) {
		int mark, queue;
		mark = strtoul(argv[2], NULL, 10);
		queue = strtoul(argv[3], NULL, 10);
		if (mark > 63 || queue > 15)
			usage(argv[0]);
		/* Separate LAN/WAN packet with the same mark value */
		if (argc > 4 && strtoul(argv[4], NULL, 10) != 0)
			mark |= 0x100;
		queue_mapping(mark, queue);
		printf("set queue mapping: skb with mark %x to queue %d.\n",mark, queue);
	}
	else
		usage(argv[0]);

	qdma_fini();
	return 0;
}

