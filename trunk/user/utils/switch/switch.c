#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include <ra_ioctl.h>

#define RT_SWITCH_HELP		1
#define RT_TABLE_MANIPULATE	1

#define REG_ESW_PFC1			0x14
#define REG_ESW_TABLE_SEARCH		0x24
#define REG_ESW_TABLE_STATUS0		0x28
#define REG_ESW_TABLE_STATUS1		0x2C
#define REG_ESW_TABLE_STATUS2		0x30
#define REG_ESW_WT_MAC_AD0		0x34
#define REG_ESW_WT_MAC_AD1		0x38
#define REG_ESW_WT_MAC_AD2		0x3C
#define REG_ESW_PVIDC_BASE		0x40
#define REG_ESW_VLAN_ID_BASE		0x50
#define REG_ESW_VLAN_MEMB_BASE		0x70
#define REG_ESW_POC2			0x98
#define REG_ESW_VLAN_UNTAG_BASE		0x100

static int esw_fd = -1;

void switch_init(void)
{
	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		exit(0);
	}
}

void switch_fini(void)
{
	close(esw_fd);
	esw_fd = -1;
}

void usage(char *cmd)
{
#if RT_SWITCH_HELP
	printf("Usage:\n");
	printf(" %s dump                                 - dump switch table\n", cmd);
	printf(" %s clear                                - clear switch table\n", cmd);
	printf(" %s add [mac] [portmap]                  - add an entry to switch table\n", cmd);
	printf(" %s add [mac] [portmap] [vlan idx]       - add an entry to switch table\n", cmd);
	printf(" %s add [mac] [portmap] [vlan idx] [age] - add an entry to switch table\n", cmd);
#ifdef CONFIG_RALINK_RT3352
	printf(" %s ingress-rate on [port] [Mbps]        - set ingress rate limit on port 0~5\n", cmd);
	printf(" %s ingress-rate off [port]              - del ingress rate limit on port 0~5\n", cmd);
	printf(" %s filt [mac]                           - add an SA filtering entry (with portmap 1111111) to switch table\n", cmd);
	printf(" %s filt [mac] [portmap]                 - add an SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan idx]      - add an SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan idx] [age]- add an SA filtering entry to switch table\n", cmd);
#elif defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	printf(" %s ingress-rate on [port] [Mbps]        - set ingress rate limit on port 0~4\n", cmd);
	printf(" %s egress-rate on [port] [Mbps]         - set egress rate limit on port 0~4\n", cmd);
	printf(" %s ingress-rate off [port]              - del ingress rate limit on port 0~4\n", cmd);
	printf(" %s egress-rate off [port]               - del egress rate limit on port 0~4\n", cmd);
	printf(" %s filt [mac]                           - add an SA filtering entry (with portmap 1111111) to switch table\n", cmd);
	printf(" %s filt [mac] [portmap]                 - add an SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan idx]      - add an SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan idx] [age]- add an SA filtering entry to switch table\n", cmd);
#endif
	printf(" %s del [mac]                            - delete an entry from switch table\n", cmd);
	printf(" %s del [mac] [vlan idx]                 - delete an entry from switch table\n", cmd);
	printf(" %s vlan dump                            - dump switch table\n", cmd);
#if defined (CONFIG_RALINK_RT3052)
	printf(" %s vlan set [idx] [vid] [portmap]       - set vlan id and associated member\n", cmd);
#else
	printf(" %s vlan set [idx] [vid] [portmap] <untag> - set vlan id and associated member\n", cmd);
#endif
	printf(" %s reg r [offset]                       - register read from offset\n", cmd);
	printf(" %s reg w [offset] [value]               - register write value to offset\n", cmd);
	printf(" %s phy [phy_addr]                       - dump phy register of specific port\n", cmd);
	printf(" %s phy                                  - dump all phy registers\n", cmd);
#endif
	switch_fini();
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
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
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
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}

int phy_dump(int phy_addr)
{
	struct ifreq ifr;
	esw_reg reg;

	reg.val = phy_addr;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_PHY_DUMP, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}

#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
int ingress_rate_set(int on_off, int port, int bw)
{
	struct ifreq ifr;
	esw_rate reg;

	reg.on_off = on_off;
	reg.port = port;
	reg.bw = bw;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_INGRESS_RATE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}
#endif

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
int egress_rate_set(int on_off, int port, int bw)
{
	struct ifreq ifr;
	esw_rate reg;

	reg.on_off = on_off;
	reg.port = port;
	reg.bw = bw;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_EGRESS_RATE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}
#endif

#if RT_TABLE_MANIPULATE
void table_dump(void)
{
	int i, j, value, mac;
	int vid[16];

	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
		vid[2 * i] = value & 0xfff;
		vid[2 * i + 1] = (value & 0xfff000) >> 12;
	}

	reg_write(REG_ESW_TABLE_SEARCH, 0x1);
	printf("hash  port(0:6)  vidx   vid  age   mac-address\n");
	for (i = 0; i < 0x400; i++) {
		while(1) {
			reg_read(REG_ESW_TABLE_STATUS0, &value);
			if (value & 0x1) { //search_rdy
				if ((value & 0x70) == 0) {
					printf("found an unused entry (age = 3'b000), please check!\n");
					return;
				}
				printf("%03x:   ", (value >> 22) & 0x3ff); //hash_addr_lu
				j = (value >> 12) & 0x7f; //r_port_map
				printf("%c", (j & 0x01)? '1':'-');
				printf("%c", (j & 0x02)? '1':'-');
				printf("%c", (j & 0x04)? '1':'-');
				printf("%c", (j & 0x08)? '1':'-');
				printf("%c ", (j & 0x10)? '1':'-');
				printf("%c", (j & 0x20)? '1':'-');
				printf("%c", (j & 0x40)? '1':'-');
				printf("   %2d", (value >> 7) & 0xf); //r_vid
				printf("  %4d", vid[(value >> 7) & 0xf]);
				printf("    %1d", (value >> 4) & 0x7); //r_age_field
				reg_read(REG_ESW_TABLE_STATUS2, &mac);
				printf("  %08x", mac);
				reg_read(REG_ESW_TABLE_STATUS1, &mac);
				printf("%04x", (mac & 0xffff));
				printf("     %c\n", (value & 0x8)? 'y':'-');
				if (value & 0x2) {
					printf("end of table %d\n", i);
					return;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				printf("found the last entry %d (not ready)\n", i);
				return;
			}
			usleep(5000);
		}
		reg_write(REG_ESW_TABLE_SEARCH, 0x2); //search for next address
	}
}

void table_add(int argc, char *argv[])
{
	int i, j, value, is_filter;
	char tmpstr[9];

	is_filter = (argv[1][0] == 'f')? 1 : 0;
	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[2], 8);
	tmpstr[8] = '\0';
	value = strtoul(tmpstr, NULL, 16);
	reg_write(REG_ESW_WT_MAC_AD2, value);

	strncpy(tmpstr, argv[2]+8, 4);
	tmpstr[4] = '\0';
	value = strtoul(tmpstr, NULL, 16);
	reg_write(REG_ESW_WT_MAC_AD1, value);

	if (!argv[3] || strlen(argv[3]) != 7) {
		if (is_filter)
			argv[3] = "1111111";
		else {
			printf("portmap format error, should be of length 7\n");
			return;
		}
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[3][i] != '0' && argv[3][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[3][i] - '0') * (1 << i);
	}
	value = j << 12; //w_port_map

	if (argc > 4) {
		j = strtoul(argv[4], NULL, 0);
		if (j < 0 || 15 < j) {
			printf("wrong member index range, should be within 0~15\n");
			return;
		}
		value += (j << 7); //w_index
	}

	if (argc > 5) {
		j = strtoul(argv[5], NULL, 0);
		if (j < 1 || 7 < j) {
			printf("wrong age range, should be within 1~7\n");
			return;
		}
		value += (j << 4); //w_age_field
	}
	else
		value += (7 << 4); //w_age_field

	if (is_filter)
		value |= (1 << 3); //sa_filter

	value += 1; //w_mac_cmd
	reg_write(REG_ESW_WT_MAC_AD0, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ESW_WT_MAC_AD0, &value);
		if (value & 0x2) { //w_mac_done
			printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_del(int argc, char *argv[])
{
	int i, j, value;
	char tmpstr[9];

	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[2], 8);
	tmpstr[8] = '\0';
	value = strtoul(tmpstr, NULL, 16);
	reg_write(REG_ESW_WT_MAC_AD2, value);

	strncpy(tmpstr, argv[2]+8, 4);
	tmpstr[4] = '\0';
	value = strtoul(tmpstr, NULL, 16);
	reg_write(REG_ESW_WT_MAC_AD1, value);

	value = 0;
	if (argc > 3) {
		j = strtoul(argv[3], NULL, 0);
		if (j < 0 || 15 < j) {
			printf("wrong member index range, should be within 0~15\n");
			return;
		}
		value += (j << 7); //w_index
	}

	value += 1; //w_mac_cmd
	reg_write(REG_ESW_WT_MAC_AD0, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ESW_WT_MAC_AD0, &value);
		if (value & 0x2) { //w_mac_done
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_clear(void)
{
	int i, value, mac;
	char v[2][13];
	char *argv[4];

	memset(argv, 0, sizeof(v));
	memset(argv, 0, sizeof(argv));

	reg_write(REG_ESW_TABLE_SEARCH, 0x1);
	for (i = 0; i < 0x400; i++) {
		while(1) {
			reg_read(REG_ESW_TABLE_STATUS0, &value);
			if (value & 0x1) { //search_rdy
				if ((value & 0x70) == 0) {
					return;
				}
				sprintf(v[1], "%d", (value >> 7) & 0xf);
				reg_read(REG_ESW_TABLE_STATUS2, &mac);
				sprintf(v[0], "%08x", mac);
				reg_read(REG_ESW_TABLE_STATUS1, &mac);
				sprintf(v[0]+8, "%04x", (mac & 0xffff));
				argv[2] = v[0];
				argv[3] = v[1];
				table_del(4, argv);
				if (value & 0x2) {
					return;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				return;
			}
			usleep(5000);
		}
		reg_write(REG_ESW_TABLE_SEARCH, 0x2); //search for next address
	}
}

void vlan_dump(void)
{
	int i, j, k, vid, shift, shift2, per_vlan, value, value2, mask, mask2;

	reg_read(REG_ESW_POC2, &per_vlan);
	per_vlan &= (1u << 15);

	value2 = 0;

	printf("idx  vid  portmap  untag\n");
//                0    1  -------  -------

	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &vid);
		reg_read(REG_ESW_VLAN_MEMB_BASE + 4*(i/2), &value);
		if (per_vlan)
			reg_read(REG_ESW_VLAN_UNTAG_BASE + 4*(i/2), &value2);
		
		for (k = 0; k < 2; k++) {
			if (k == 1)
				vid >>= 12;
			printf(" %2d %4d  ", 2*i+k, vid & 0xfff);
			
			shift = 8*k;
			shift2 = 7*k;
			if (i % 2) {
				shift += 16;
				shift2 += 14;
			}
			
			for (j = 0; j < 7; j++) {
				mask = (1u << j) << shift;
				printf("%c", (value & mask)? '1':'-');
			}
			
			if (per_vlan) {
				printf("%s", "  ");
				for (j = 0; j < 7; j++) {
					mask = (1u << j) << shift;
					mask2 = (1u << j) << shift2;
					if (!(value & mask))
						printf("%c", '-');
					else
						printf("%c", (value2 & mask2)? '1':'0');
				}
				printf("%s", "\n");
			} else {
				printf("%s", "  -------\n");
			}
		}
	}

	printf("\nPVID:\n");
	printf("port  pvid  prio\n");
//                 0     1     7

	reg_read(REG_ESW_PFC1, &value);
	reg_read(REG_ESW_PVIDC_BASE + 0xC, &value2);

	for (i = 0; i < 7; i++) {
		reg_read(REG_ESW_PVIDC_BASE + 4*(i/2), &vid);
		if ((i % 2) != 0)
			vid >>= 12;
		
		vid &= 0xfff;
		mask = (value >> (i*2)) & 0x3;
		mask2 = ((value2 >> 16) >> (mask * 4)) & 0x7;
		
		printf("%4d  %4d  %4d\n", i, vid, mask2);
	}
}

void vlan_set(int argc, char *argv[])
{
	int i, mask_member, vid, value, mask_untag;
	int idx;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	idx = strtoul(argv[3], NULL, 0);
	if (idx < 0 || 15 < idx) {
		printf("wrong member index range, should be within 0~15\n");
		return;
	}

	vid = strtoul(argv[4], NULL, 0);
	if (vid > 0xfff) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}

	if (strlen(argv[5]) != 7) {
		printf("portmap format error, should be of length 7 (e.g. 1111001)\n");
		return;
	}

	mask_member = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		mask_member += (argv[5][i] - '0') * (1u << i);
	}

#if !defined (CONFIG_RALINK_RT3052)
	mask_untag = 0;
	if (argc > 6) {
		if (strlen(argv[6]) != 7) {
			printf("untag per port format error, should be of length 7 (e.g. 11111-0)\n");
			return;
		}
		
		for (i = 0; i < 7; i++) {
			if (argv[6][i] == '1')
				mask_untag |= (1u << i);
			else if (argv[6][i] == '0' || argv[6][i] == '-')
				;
			else {
				printf("untag format error, should be of '1' (untag) or '0' (tag)\n");
				return;
			}
		}
	}
#endif

	// set vlan identifier
	reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &value);
	if ((idx % 2) == 0) {
		value &= 0xfff000;
		value |= vid;
	} else {
		value &= 0xfff;
		value |= (vid << 12);
	}
	reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), value);

	// set vlan member
	reg_read(REG_ESW_VLAN_MEMB_BASE + 4*(idx/4), &value);
	if ((idx % 4) == 0) {
		value &= 0xffffff00;
		value |= mask_member;
	} else if ((idx % 4) == 1) {
		value &= 0xffff00ff;
		value |= (mask_member << 8);
	} else if ((idx % 4) == 2) {
		value &= 0xff00ffff;
		value |= (mask_member << 16);
	} else {
		value &= 0x00ffffff;
		value |= (mask_member << 24);
	}
	reg_write(REG_ESW_VLAN_MEMB_BASE + 4*(idx/4), value);

	// set vlan untag ports
#if !defined (CONFIG_RALINK_RT3052)
	if (argc > 6) {
		reg_read(REG_ESW_VLAN_UNTAG_BASE + 4*(idx/4), &value);
		if ((idx % 4) == 0) {
			value &= ~(0x7f);
			value |= mask_untag;
		} else if ((idx % 4) == 1) {
			value &= ~(0x7f << 7);
			value |= (mask_untag << 7);
		} else if ((idx % 4) == 2) {
			value &= ~(0x7f << 14);
			value |= (mask_untag << 14);
		} else {
			value &= ~(0x7f << 21);
			value |= (mask_untag << 21);
		}
		reg_write(REG_ESW_VLAN_UNTAG_BASE + 4*(idx/4), value);
	}
#endif
}
#endif

int main(int argc, char *argv[])
{
	switch_init();

	if (argc < 2)
		usage(argv[0]);
#if RT_TABLE_MANIPULATE
	if (argc == 2) {
		if (!strncmp(argv[1], "dump", 5))
			table_dump();
		else if (!strncmp(argv[1], "clear", 6)) {
			table_clear();
			printf("done.\n");
		}
		else if (!strncmp(argv[1], "phy", 4)) {
			phy_dump(32); //dump all phy register
		}
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "add", 4))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "filt", 5))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "del", 4))
		table_del(argc, argv);
	else if (!strncmp(argv[1], "vlan", 5)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			vlan_dump();
		else if (!strncmp(argv[2], "set", 4))
			vlan_set(argc, argv);
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "phy", 4)) {
		if (argc == 3) {
			int phy_addr = strtoul(argv[2], NULL, 10);
			phy_dump(phy_addr);
		}else {
			phy_dump(32); //dump all phy register
		}
	}
#endif
	else if (!strncmp(argv[1], "reg", 4)) {
		int off, val=0;
		if (argc < 4)
			usage(argv[0]);
		if (argv[2][0] == 'r') {
			off = strtoul(argv[3], NULL, 16);
			reg_read(off, &val);
			printf("switch reg read offset=%x, value=%x\n", off, val);
		}
		else if (argv[2][0] == 'w') {
			if (argc != 5)
				usage(argv[0]);
			off = strtoul(argv[3], NULL, 16);
			val = strtoul(argv[4], NULL, 16);
			printf("switch reg write offset=%x, value=%x\n", off, val);
			reg_write(off, val);
		}
		else
			usage(argv[0]);
	}
#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	else if (!strncmp(argv[1], "ingress-rate", 6)) {
		int port=0, bw=0;

		if (argv[2][1] == 'n') {
			port = strtoul(argv[3], NULL, 0);
			bw = strtoul(argv[4], NULL, 0);
			ingress_rate_set(1, port, bw);
			printf("switch port=%d, bw=%d\n", port, bw);
		}
		else if (argv[2][1] == 'f') {
			if (argc != 4)
				usage(argv[0]);
			port = strtoul(argv[3], NULL, 0);
			ingress_rate_set(0, port, bw);
			printf("switch port=%d ingress rate limit off\n", port);
		}
		else
			usage(argv[0]);
	}
#endif
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	else if (!strncmp(argv[1], "egress-rate", 6)) {
		int port=0, bw=0;
		
		if (argv[2][1] == 'n') {
			port = strtoul(argv[3], NULL, 0);
			bw = strtoul(argv[4], NULL, 0);
			egress_rate_set(1, port, bw);
			printf("switch port=%d, bw=%d\n", port, bw);
		}
		else if (argv[2][1] == 'f') {
			if (argc != 4)
				usage(argv[0]);
			port = strtoul(argv[3], NULL, 0);
			egress_rate_set(0, port, bw);
			printf("switch port=%d egress rate limit off\n", port);
		}
		else
			usage(argv[0]);
	}
#endif
	else
		usage(argv[0]);

	switch_fini();
	return 0;
}

