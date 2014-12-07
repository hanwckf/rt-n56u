#ifndef _UTILS_H_
#define _UTILS_H_

#include <malloc.h>
#include <linux/types.h>
#include "msg.h"

#define BUG_ON(x) \
    do { \
        if (x) { \
            printf("[BUG] %s LINE:%d FILE:%s\n", #x, __LINE__, __FILE__); \
            while(1); \
        } \
    }while(0)
#define WARN_ON(x) \
    do { \
        if (x) { \
            MSG(WRN, "[WARN] %s LINE:%d FILE:%s\n", #x, __LINE__, __FILE__); \
        } \
    }while(0)

#define ERR_EXIT(expr, ret, expected_ret) \
    do { \
        (ret) = (expr);\
        if ((ret) != (expected_ret)) { \
            printf("[ERR] LINE:%d: %s != %d (%d)\n", __LINE__, #expr, expected_ret, ret); \
            goto exit; \
        } \
    } while(0)

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

static uint32 uffs(uint32 x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}



#ifndef min
#define min(x, y)   (x < y ? x : y)
#endif
#ifndef max
#define max(x, y)   (x > y ? x : y)
#endif

#if 1
#define udelay(us)  \
    do { \
        volatile int count = us * 5000; \
        while (count--); \
    }while(0)

#define mdelay(ms) \
    do { \
        unsigned long i; \
        for (i = 0; i < ms; i++) \
        udelay(1000); \
    }while(0)
#else
#define udelay(us)  do{GPT_Delay_us(us);}while(0)
#define mdelay(ms)  do{GPT_Delay_ms(ms);}while(0)

#endif
#define WAIT_COND(cond,tmo,left) \
    do { \
        volatile u32 t = tmo; \
        while (1) { \
            if ((cond) || (t == 0)) break; \
            if (t > 0) { mdelay(1); t--; } \
        } \
        left = t; \
        WARN_ON(left == 0); \
    }while(0)

#endif /* _UTILS_H_ */

