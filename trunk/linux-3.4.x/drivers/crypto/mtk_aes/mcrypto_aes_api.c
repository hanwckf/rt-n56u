#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <crypto/algapi.h>

#include "mcrypto_aes_api.h"
#include "aes_engine.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
#define CRYPTO_ALG_KERN_DRIVER_ONLY	0
#endif

#ifdef DBG
int aes_dbg_print = 1;
#endif

static int
mcrypto_init_blk(struct crypto_tfm *tfm)
{
	return 0;
}

static int
mcrypto_setkey_blk(struct crypto_tfm *tfm, const u8 *key,
		unsigned int len)
{
	struct mcrypto_ctx *ctx = crypto_tfm_ctx(tfm);
	unsigned int ret = 0;

	if (len != AES_KEYSIZE_128 &&
	    len != AES_KEYSIZE_192 &&
	    len != AES_KEYSIZE_256) {
		tfm->crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	memcpy(ctx->key, key, len);
	ctx->keylen = len;

#ifdef DBG
	{
		int i;
		DBGPRINT(DBG_LOW, "key[] = [");
		for(i=0;i<len;i++)
			DBGPRINT(DBG_LOW, "%02X ", ctx->key[i]);
		DBGPRINT(DBG_LOW, "]\n");
	}
#endif

	return ret;
}

static int
mcrypto_aes_cbc_decrypt(struct blkcipher_desc *desc,
		  struct scatterlist *dst, struct scatterlist *src,
		  unsigned int nbytes)
{
	struct mcrypto_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);

	DBGPRINT(DBG_LOW, "%s nbytes=%d\n", __func__, nbytes);

	if (nbytes < AES_BLOCK_SIZE || !IS_ALIGNED(nbytes, AES_BLOCK_SIZE)) {
		desc->flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
		return -EINVAL;
	}

	ctx->iv = (u8*)desc->info;

	return mtk_aes_process_sg(src, dst, ctx, nbytes, MCRYPTO_MODE_CBC);
}

static int
mcrypto_aes_cbc_encrypt(struct blkcipher_desc *desc,
		  struct scatterlist *dst, struct scatterlist *src,
		  unsigned int nbytes)
{
	struct mcrypto_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);

	DBGPRINT(DBG_LOW, "%s keylen=%d nbytes=%d\n", __func__, ctx->keylen, nbytes);

	if (nbytes < AES_BLOCK_SIZE || !IS_ALIGNED(nbytes, AES_BLOCK_SIZE)) {
		desc->flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
		return -EINVAL;
	}

	ctx->iv = (u8*)desc->info;

	return mtk_aes_process_sg(src, dst, ctx, nbytes, MCRYPTO_MODE_CBC | MCRYPTO_MODE_ENC);
}

static int
mcrypto_aes_ecb_decrypt(struct blkcipher_desc *desc,
		  struct scatterlist *dst, struct scatterlist *src,
		  unsigned int nbytes)
{
	struct mcrypto_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);

	DBGPRINT(DBG_LOW, "%s keylen=%d nbytes=%d\n", __func__, ctx->keylen, nbytes);

	if (nbytes < AES_BLOCK_SIZE || !IS_ALIGNED(nbytes, AES_BLOCK_SIZE)) {
		desc->flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
		return -EINVAL;
	}

	return mtk_aes_process_sg(src, dst, ctx, nbytes, 0);
}

static int
mcrypto_aes_ecb_encrypt(struct blkcipher_desc *desc,
		  struct scatterlist *dst, struct scatterlist *src,
		  unsigned int nbytes)
{
	struct mcrypto_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);

	DBGPRINT(DBG_LOW, "%s keylen=%d nbytes=%d\n", __func__, ctx->keylen, nbytes);

	if (nbytes < AES_BLOCK_SIZE || !IS_ALIGNED(nbytes, AES_BLOCK_SIZE)) {
		desc->flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
		return -EINVAL;
	}

	return mtk_aes_process_sg(src, dst, ctx, nbytes, MCRYPTO_MODE_ENC);
}

struct crypto_alg mcrypto_aes_cbc_alg = {
	.cra_module		= THIS_MODULE,
	.cra_name		= "cbc(aes)",
	.cra_driver_name	= "cbc-aes-mcrypto",
	.cra_priority		= 400,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct mcrypto_ctx),
	.cra_alignmask		= 0xf,
	.cra_type		= &crypto_blkcipher_type,
	.cra_list		= LIST_HEAD_INIT(mcrypto_aes_cbc_alg.cra_list),
	.cra_init		= mcrypto_init_blk,
	.cra_u			= {
		.blkcipher	= {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.setkey		= mcrypto_setkey_blk,
			.encrypt	= mcrypto_aes_cbc_encrypt,
			.decrypt	= mcrypto_aes_cbc_decrypt,
			.ivsize		= AES_BLOCK_SIZE,
		}
	}
};

struct crypto_alg mcrypto_aes_ecb_alg = {
	.cra_module		= THIS_MODULE,
	.cra_name		= "ecb(aes)",
	.cra_driver_name	= "ecb-aes-mcrypto",
	.cra_priority		= 400,
	.cra_flags		= CRYPTO_ALG_TYPE_BLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct mcrypto_ctx),
	.cra_alignmask		= 0xf,
	.cra_type		= &crypto_blkcipher_type,
	.cra_list		= LIST_HEAD_INIT(mcrypto_aes_ecb_alg.cra_list),
	.cra_init		= mcrypto_init_blk,
	.cra_u			= {
		.blkcipher	= {
			.min_keysize	= AES_MIN_KEY_SIZE,
			.max_keysize	= AES_MAX_KEY_SIZE,
			.setkey		= mcrypto_setkey_blk,
			.encrypt	= mcrypto_aes_ecb_encrypt,
			.decrypt	= mcrypto_aes_ecb_decrypt,
		}
	}
};

