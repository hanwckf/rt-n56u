/*
 * NVRAM variable manipulation
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmnvram.h,v 1.1 2007/06/08 07:40:19 arthur Exp $
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_CLEAR_MAGIC	0x0
#define NVRAM_INVALID_MAGIC	0xFFFFFFFF
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20
#define NVRAM_MTD_SIZE		0x10000		/* mtdblock1 64K */
#define NVRAM_MTD_OFFSET	0x01000		/* uboot env max space 4K */
#define NVRAM_SPACE		(NVRAM_MTD_SIZE-NVRAM_MTD_OFFSET)

#define NVRAM_MAX_PARAM_LEN	64
#define NVRAM_MAX_VALUE_LEN	4096

#define NVRAM_MAJOR		228

#define NVRAM_IOCTL_COMMIT	10
#define NVRAM_IOCTL_CLEAR	20
#define NVRAM_IOCTL_SET		30
#define NVRAM_IOCTL_GET		40

#ifdef __KERNEL__

#include <linux/types.h>

#define CRC8_INIT_VALUE		(0xff)
#define htol32(i)		(i)
#define ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

struct nvram_tuple {
	char *name;
	char *value;
	uint32_t val_len:31,
	         val_tmp:1;
	struct nvram_tuple *next;
};

#else

#include <stdint.h>

struct nvram_pair {
	char *name;
	char *value;
};

extern char *nvram_get(const char *name);
extern char *nvram_safe_get(const char *name);
extern int nvram_get_int(const char *name);
extern int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max);
extern int nvram_getall(char *buf, int count, int include_temp);

extern int nvram_set(const char *name, const char *value);
extern int nvram_set_int(const char *name, int value);
extern int nvram_unset(const char *name);

extern int nvram_set_temp(const char *name, const char *value);
extern int nvram_set_int_temp(const char *name, int value);

extern int nvram_match(const char *name, char *match);
extern int nvram_invmatch(const char *name, char *invmatch);

extern int nvram_commit(void);
extern int nvram_clear(void);

#endif

struct nvram_header {
	uint32_t magic;
	uint32_t len;
	uint32_t crc_ver_init;		/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32_t config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32_t config_ncdl;		/* ncdl values for memc */
};

typedef struct anvram_ioctl_s {
	int size;
	int is_temp;
	int len_param;
	int len_value;
	char *param;
	char *value;
} anvram_ioctl_t;

#endif /* _bcmnvram_h_ */

