#ifndef __RAETH_IOCTL_H__
#define __RAETH_IOCTL_H__

/* ioctl commands */
#define RAETH_ESW_REG_READ		0x89F1
#define RAETH_ESW_REG_WRITE		0x89F2
#define RAETH_MII_READ			0x89F3
#define RAETH_MII_WRITE			0x89F4
#define RAETH_ESW_INGRESS_RATE		0x89F5
#define RAETH_ESW_EGRESS_RATE		0x89F6
#define RAETH_ESW_PHY_DUMP		0x89F7

#define RAETH_QDMA_REG_READ		0x89F8
#define RAETH_QDMA_REG_WRITE		0x89F9
#define RAETH_QDMA_QUEUE_MAPPING	0x89FA
#define RAETH_QDMA_READ_CPU_CLK		0x89FB

#define RAETH_MII_READ_CL45		0x89FC
#define RAETH_MII_WRITE_CL45		0x89FD

#define REG_HQOS_MAX			0x3FFF

typedef struct {
	unsigned int off;
	unsigned int val;
} esw_reg;

typedef struct {
	unsigned int phy_id;
	unsigned int reg_num;
	unsigned int val_in;
	unsigned int val_out;
	unsigned int port_num;
	unsigned int dev_addr;
	unsigned int reg_addr;
} ra_mii_ioctl_data;

typedef struct {
	unsigned int on_off;
	unsigned int port;
	unsigned int bw;/*Mbps*/
} esw_rate;

#endif
