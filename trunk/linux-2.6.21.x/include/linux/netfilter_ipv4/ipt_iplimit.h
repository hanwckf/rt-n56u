#ifndef _IPT_IPLIMIT_H
#define _IPT_IPLIMIT_H

struct ipt_iplimit_data;

struct ipt_iplimit_info {
	int limit;
	int inverse;
	u_int32_t mask;
	struct ipt_iplimit_data *data;
};
#endif /* _IPT_IPLIMIT_H */
