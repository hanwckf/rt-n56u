#ifndef __MCRYPTO_AES_API_H__
#define __MCRYPTO_AES_API_H__

#include <linux/crypto.h>
#include <crypto/aes.h>

//#define DBG

#ifdef DBG
extern int aes_dbg_print;
#define DBG_LOW		3
#define DBG_MID		2
#define DBG_HIGH	1
#define DBGPRINT(level, fmt, args...)	do{if (aes_dbg_print>=level) printk(fmt, ## args);}while(0)
#else
#define DBGPRINT(level, fmt, args...)	do{}while(0)
#endif

#define MCRYPTO_MODE_ENC	BIT(0)
#define MCRYPTO_MODE_CBC	BIT(1)

struct mcrypto_ctx {
	/* common */
	u8 key[AES_MAX_KEY_SIZE];
	u32 keylen;

	/* cipher */
	u8* iv;
};

extern struct crypto_alg mcrypto_aes_cbc_alg;
extern struct crypto_alg mcrypto_aes_ecb_alg;

#endif /* !_MCRYPTO_AES_API_H_ */

