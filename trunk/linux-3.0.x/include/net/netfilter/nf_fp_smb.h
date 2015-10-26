#ifndef _NF_FP_SMB_H
#define _NF_FP_SMB_H

#define NF_FP_CACHE_SMB		(1UL<<7)

int nf_fp_smb_hook_in(struct sk_buff *skb);
int nf_fp_smb_hook_out(struct sk_buff *skb);

int __init netfilter_fp_smb_init(void);

#endif /* _NF_FP_SMB_H */
