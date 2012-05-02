#ifndef __ipt_time_h_included__
#define __ipt_time_h_included__


struct ipt_time_info {
	unsigned int days_match;	/* 1 bit per day (bit 0 = Sunday) */
	unsigned int time_start;	/* 0 < time_start < 24*60*60-1 = 86399 */
	unsigned int time_stop;		/* 0 < time_end < 24*60*60-1 = 86399 */
	int kerneltime;			/* ignore skb time (and use kerneltime) or not. */
};


#endif /* __ipt_time_h_included__ */
