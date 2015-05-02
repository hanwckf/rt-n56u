#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/types.h>

#include <ra_ioctl.h>

#define MII_MGR_HELP	1

void show_usage(void)
{
#if MII_MGR_HELP
	printf("Get: mii_mgr_cl45 -g -p [port number] -d [dev number] -r [register number]\n");
	printf("Example: mii_mgr_cl45 -g -p 3 -d 0x5 -r 0x4\n\n");
	printf("Set: mii_mgr_cl45 -s -p [port number] -d [dev number] -r [register number] -v [value]\n");
	printf("Example: mii_mgr_cl45 -s -p 4 -d 0x6 -r 0x1 -v 0xff11\n\n");
#endif
}

int main(int argc, char *argv[])
{
	int sk, opt, ret = 0;
	char options[] = "gsp:d:r:v:?t";
	int method = 0;
	struct ifreq ifr;
	ra_mii_ioctl_data mii;

	if (argc < 8) {
		show_usage();
		return 0;
	}

	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0) {
		printf("Open socket failed\n");
		return -1;
	}

	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &mii;

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
			case 'g':
				method = RAETH_MII_READ_CL45;
				break;
			case 's':
				method = RAETH_MII_WRITE_CL45;
				break;
			case 'p':
				mii.port_num = strtoul(optarg, NULL, 16);
				break;
			case 'd':
				mii.dev_addr = strtoul(optarg, NULL, 16);
			case 'r':
				mii.reg_addr = strtol(optarg, NULL, 16);
				break;
			case 'v':
				mii.val_in = strtol(optarg, NULL, 16);
				break;
			case '?':
				show_usage();
				break;
		}
	}

	if ((method == RAETH_MII_READ_CL45) || (method == RAETH_MII_WRITE_CL45)) {
		ret = ioctl(sk, method, &ifr);
		if (ret < 0) {
			printf("mii_mgr_cl45: ioctl error\n");
		}
		else {
			switch (method) {
			case RAETH_MII_READ_CL45:
				printf("Get: port%d dev%Xh_reg%Xh = 0x%04X\n",
						mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_out);
				break;
			case RAETH_MII_WRITE_CL45:
				printf("Set: port%d dev%Xh_reg%Xh = 0x%04X\n",
						mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_in);
				break;
			}
		}
	}

	close(sk);
	return ret;
}
