#ifndef _RTL8370_TYPES_H_
#define _RTL8370_TYPES_H_

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>

typedef unsigned long long    uint64;
typedef long long             int64;
typedef unsigned int          uint32;
typedef int                   int32;
typedef unsigned short        uint16;
typedef short                 int16;
typedef unsigned char         uint8;
typedef char                  int8;

#ifndef CONST_T
#define CONST_T               const
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN        6
#endif

typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;


/* type abstraction */
typedef uint32                  ipaddr_t;
typedef uint32                  memaddr;

typedef int32                   rtk_api_ret_t;
typedef int32                   ret_t;
typedef uint64                  rtk_u_long_t;


#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT rtlglue_printf


#endif /* _RTL8370_TYPES_H_ */
