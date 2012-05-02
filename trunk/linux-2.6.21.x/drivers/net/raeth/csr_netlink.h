#ifndef	CSR_NETLINK_H
#define CSR_NETLINK_H

#define	CSR_NETLINK	30
#define	CSR_READ	0
#define	CSR_WRITE	1
#define	CSR_TEST	2

#define RALINK_CSR_GROUP	 2882	

typedef struct rt2880_csr_msg {
  	int	enable;
  	char	reg_name[32];
  	unsigned long address;
  	unsigned long default_value;
  	unsigned long reserved_bits;	/* 1 : not reserved, 0 : reserved */
  	unsigned long write_mask;
  	unsigned long write_value;
  	int	status;
} CSR_MSG;

int csr_msg_send(CSR_MSG* msg);
int csr_msg_recv(void);

// static CSR_MSG	input_csr_msg;

#endif
