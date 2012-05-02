/*
 * NVRAM variable manipulation (common)
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram.c,v 1.1 2007/06/08 07:38:05 arthur Exp $
 */

#include <linux/string.h>
#include <nvram/typedefs.h>
#include <nvram/bcmdefs.h>
#include <nvram/bcmutils.h>
#include <nvram/bcmendian.h>
#include <nvram/bcmnvram.h>

extern struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name,
                                           const char *value);
extern void _nvram_free(struct nvram_tuple *t);
extern int _nvram_read_mtd(unsigned char *buf);

char * _nvram_get(const char *name);
int _nvram_set(const char *name, const char *value);
int _nvram_unset(const char *name);
int _nvram_getall(char *buf, int count);
int _nvram_commit(struct nvram_header *header);
int _nvram_init(void);
void _nvram_uninit(void);

static struct nvram_tuple * BCMINITDATA(nvram_hash)[257] = {NULL};
static struct nvram_tuple * nvram_dead = NULL;

// Copy from sbsdram.h
/* SDRAM configuration (config) register bits */
#define SDRAM_BURSTFULL 0x0000  /* Use full page bursts */
#define SDRAM_BURST8    0x0001  /* Use burst of 8 */
#define SDRAM_BURST4    0x0002  /* Use burst of 4 */
#define SDRAM_BURST2    0x0003  /* Use burst of 2 */
#define SDRAM_CAS3  0x0000  /* Use CAS latency of 3 */
#define SDRAM_CAS2  0x0004  /* Use CAS latency of 2 */
/* SDRAM refresh control (refresh) register bits */
#define SDRAM_REF(p)    (((p)&0xff) | SDRAM_REF_EN) /* Refresh period */
#define SDRAM_REF_EN    0x8000      /* Writing 1 enables periodic refresh */
/* SDRAM Core default Init values (OCP ID 0x803) */
#define MEM1MX16    0x009   /* 2 MB */
#define MEM1MX16X2  0x409   /* 4 MB */
#define MEM2MX8X2   0x809   /* 4 MB */
#define MEM2MX8X4   0xc09   /* 8 MB */
#define MEM2MX32    0x439   /* 8 MB */
#define MEM4MX16    0x019   /* 8 MB */
#define MEM4MX16X2  0x419   /* 16 MB */
#define MEM8MX8X2   0x819   /* 16 MB */
#define MEM8MX16    0x829   /* 16 MB */
#define MEM4MX32    0x429   /* 16 MB */
#define MEM8MX8X4   0xc19   /* 32 MB */
#define MEM8MX16X2  0xc29   /* 32 MB */

#define SDRAM_INIT  MEM4MX16X2
#define SDRAM_CONFIG    SDRAM_BURSTFULL
#define SDRAM_REFRESH   SDRAM_REF(0x40)


/* Free all tuples. Should be locked. */
static void
BCMINITFN(nvram_free)(void)
{
	uint i;
	struct nvram_tuple *t, *next;

	/* Free hash table */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = next) {
			next = t->next;
			_nvram_free(t);
		}
		nvram_hash[i] = NULL;
	}

	/* Free dead table */
	for (t = nvram_dead; t; t = next) {
		next = t->next;
		_nvram_free(t);
	}
	nvram_dead = NULL;

	/* Indicate to per-port code that all tuples have been freed */
	_nvram_free(NULL);
}

/* String hash */
static INLINE uint
hash(const char *s)
{
	uint hash = 0;

	while (*s)
		hash = 31 * hash + *s++;

	return hash;
}

/* (Re)initialize the hash table. Should be locked. */
static int
BCMINITFN(nvram_rehash)(struct nvram_header *header)
{
	char /*eric--buf[] = "0xXXXXXXXX",*/ *name, *value, *end, *eq;

	/* (Re)initialize hash table */
	nvram_free();

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + NVRAM_SPACE - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		_nvram_set(name, value);
		*eq = '=';
	}

	return 0;
}

/* Get the value of an NVRAM variable. Should be locked. */
char *
BCMINITFN(_nvram_get)(const char *name)
{
	uint i;
	struct nvram_tuple *t;
	char *value;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);

	/* Find the associated tuple in the hash table */
	for (t = nvram_hash[i]; t && strcmp(t->name, name); t = t->next);

	value = t ? t->value : NULL;

	return value;
}

/* Set the value of an NVRAM variable. Should be locked. */
int
BCMINITFN(_nvram_set)(const char *name, const char *value)
{
	uint i;
	struct nvram_tuple *t, *u, **prev;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
	     prev = &t->next, t = *prev);

	/* (Re)allocate tuple */
	if (!(u = _nvram_realloc(t, name, value)))
		return -12; /* -ENOMEM */

	/* Value reallocated */
	if (t && t == u)
		return 0;

	/* Move old tuple to the dead table */
	if (t) {
		*prev = t->next;
		t->next = nvram_dead;
		nvram_dead = t;
	}

	/* Add new tuple to the hash table */
	u->next = nvram_hash[i];
	nvram_hash[i] = u;

	return 0;
}

/* Unset the value of an NVRAM variable. Should be locked. */
int
BCMINITFN(_nvram_unset)(const char *name)
{
	uint i;
	struct nvram_tuple *t, **prev;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
	     prev = &t->next, t = *prev);

	/* Move it to the dead table */
	if (t) {
		*prev = t->next;
		t->next = nvram_dead;
		nvram_dead = t;
	}

	return 0;
}

/* Get all NVRAM variables. Should be locked. */
int
BCMINITFN(_nvram_getall)(char *buf, int count)
{
	uint i;
	struct nvram_tuple *t;
	int len = 0;

	/* Write name=value\0 ... \0\0 */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((count - len) > (strlen(t->name) + 1 + strlen(t->value) + 1))
				len += sprintf(buf + len, "%s=%s", t->name, t->value) + 1;
			else
				break;
		}
	}

	return 0;
}

/* Regenerate NVRAM. Should be locked. */
int
BCMINITFN(_nvram_commit)(struct nvram_header *header)
{
	char *ptr, *end;
	int i;
	struct nvram_tuple *t;
	struct nvram_header tmp;
	uint8 crc;

	/* Regenerate header */
	header->magic = NVRAM_MAGIC;
	header->crc_ver_init = (NVRAM_VERSION << 8);
	header->crc_ver_init |= SDRAM_INIT << 16;
	header->config_refresh = SDRAM_CONFIG;
	header->config_refresh |= SDRAM_REFRESH << 16;
	header->config_ncdl = 0;

	/* Clear data area */
	ptr = (char *) header + sizeof(struct nvram_header);
	memset(ptr, 0, NVRAM_SPACE - sizeof(struct nvram_header));

	/* Leave space for a double NUL at the end */
	end = (char *) header + NVRAM_SPACE - 2;

	/* Write out all tuples */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
				break;
			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
		}
	}

	/* End with a double NUL */
	ptr += 2;

	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htol32(header->crc_ver_init);
	tmp.config_refresh = htol32(header->config_refresh);
	tmp.config_ncdl = htol32(header->config_ncdl);
	crc = hndcrc8((uint8 *) &tmp + 9, sizeof(struct nvram_header) - 9, CRC8_INIT_VALUE);

	/* Continue CRC8 over data bytes */
	crc = hndcrc8((uint8 *) &header[1], header->len - sizeof(struct nvram_header), crc);

	/* Set new CRC8 */
	header->crc_ver_init |= crc;

	/* Reinitialize hash table */
	return nvram_rehash(header);
}

/* Initialize hash table. Should be locked. */
int
BCMINITFN(_nvram_init)(void)
{
	unsigned char *buf;
	struct nvram_header *header;
	int ret;

	if (!(buf = kmalloc(NVRAM_SPACE, GFP_ATOMIC))) {
		printk("nvram_init: out of memory\n");
		return -12; /* -ENOMEM */
	}
	
	memset(buf, 0, NVRAM_SPACE);
	
	header = (struct nvram_header *) buf;

	if ((ret = _nvram_read_mtd(buf)) == 0 && header->magic == NVRAM_MAGIC)
		nvram_rehash(header);

	kfree(buf);

	return ret;
}

/* Free hash table. Should be locked. */
void
BCMINITFN(_nvram_uninit)(void)
{
	nvram_free();
}
