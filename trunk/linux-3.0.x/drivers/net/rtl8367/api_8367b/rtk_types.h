#ifndef _RTL8367B_TYPES_H_
#define _RTL8367B_TYPES_H_

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>

typedef unsigned long long    rtk_uint64;
typedef long long             rtk_int64;
typedef unsigned int          rtk_uint32;
typedef int                   rtk_int32;
typedef unsigned short        rtk_uint16;
typedef short                 rtk_int16;
typedef unsigned char         rtk_uint8;
typedef char                  rtk_int8;

#ifndef CONST
#define CONST                 const
#endif

#ifndef CONST_T
#define CONST_T               const
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN        6
#endif

typedef struct ether_addr_s {
	rtk_uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;


/* type abstraction */
typedef rtk_uint32                  ipaddr_t;
typedef rtk_uint32                  memaddr;

typedef rtk_int32                   rtk_api_ret_t;
typedef rtk_int32                   ret_t;
typedef rtk_uint64                  rtk_u_long_t;


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


#endif /* _RTL8367B_TYPES_H_ */
