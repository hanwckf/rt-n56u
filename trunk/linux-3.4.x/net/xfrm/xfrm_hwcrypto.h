#ifndef __XFRM_HWCRYPTO__
#define __XFRM_HWCRYPTO__

#define HWCRYPTO_OK		1
#define HWCRYPTO_NOMEM		0x80

extern int ipsec_esp_input(struct xfrm_state *x, struct sk_buff *skb);
extern int ipsec_esp_output(struct xfrm_state *x, struct sk_buff *skb);

extern int ipsec_esp6_input(struct xfrm_state *x, struct sk_buff *skb);
extern int ipsec_esp6_output(struct xfrm_state *x, struct sk_buff *skb);

#endif
