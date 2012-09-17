#ifndef RA_QOS_H
#define	RA_QOS_H

#include "ra2882ethreg.h"
#define RING0	0
#define RING1	1
#define RING2	2
#define RING3	3
void get_tx_desc_and_dtx_idx(END_DEVICE* ei_local, int ring_no, unsigned long *tx_dtx_idx, struct PDMA_txdesc **tx_desc);
int get_tx_ctx_idx(unsigned int ring_no, unsigned long *idx);
int fe_tx_desc_init(struct net_device *dev, unsigned int ring_no, unsigned int qn, unsigned int pn);
int fe_qos_packet_send(struct net_device *dev, struct sk_buff* skb, unsigned int ring_no, unsigned int qn, unsigned int pn);

int  pkt_classifier(struct sk_buff *skb,int gmac_no, int *ring_no, int *queue_no, int *port_no);
void set_schedule_pause_condition(void);
void set_scheduler_weight(void);
void set_output_shaper(void);
#endif
