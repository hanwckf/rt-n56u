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
#include <nvram/bcmnvram.h>

extern uint8_t _hndcrc8(uint8_t *p, uint nbytes, uint8_t crc);
extern struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name,
                                           const char *value, int is_temp);
extern void _nvram_free(struct nvram_tuple *t);
extern void _nvram_reset(void);

char * _nvram_get(const char *name);
int _nvram_set(const char *name, const char *value, int is_temp);
int _nvram_unset(const char *name);
int _nvram_getall(char *buf, int count, int include_temp);
int _nvram_generate(struct nvram_header *header, int rehash);
int _nvram_init(struct nvram_header *header);
void _nvram_uninit(void);

static struct nvram_tuple *nvram_hash[257] = {NULL};

// broadcom fake constants (not used)
#define SDRAM_INIT	0x419
#define SDRAM_CONFIG	0x0000
#define SDRAM_REFRESH	0x8040

/* Free all tuples. Should be locked. */
static void
nvram_free(void)
{
	size_t i;
	struct nvram_tuple *t, *next;

	/* Free hash table */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = next) {
			next = t->next;
			_nvram_free(t);
		}
		nvram_hash[i] = NULL;
	}

	/* Indicate that all tuples have been freed */
	_nvram_reset();
}

/* String hash */
static inline uint
hash(const char *s)
{
	uint hash = 0;

	while (*s)
		hash = 31 * hash + *s++;

	return hash;
}

/* (Re)initialize the hash table. Should be locked. */
static void
nvram_rehash(struct nvram_header *header)
{
	char *name, *value, *end, *eq;

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + NVRAM_SPACE - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		_nvram_set(name, value, 0);
		*eq = '=';
	}
}

static uint8_t
nvram_calc_crc(struct nvram_header *header)
{
	uint8_t crc;
	struct nvram_header tmp;

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htol32(header->crc_ver_init & 0xFFFFFF00);
	tmp.config_refresh = htol32(header->config_refresh);
	tmp.config_ncdl = htol32(header->config_ncdl);
	crc = _hndcrc8((uint8_t *) &tmp + 9, sizeof(struct nvram_header) - 9, CRC8_INIT_VALUE);

	/* Continue CRC8 over data bytes */
	crc = _hndcrc8((uint8_t *) &header[1], header->len - sizeof(struct nvram_header), crc);

	return crc;
}

/* Get the value of an NVRAM variable. Should be locked. */
char *
_nvram_get(const char *name)
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
_nvram_set(const char *name, const char *value, int is_temp)
{
	uint i;
	struct nvram_tuple *t, *u, **prev;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
	     prev = &t->next, t = *prev);

	/* (Re)allocate tuple */
	if (!(u = _nvram_realloc(t, name, value, is_temp)))
		return -1;

	/* Value reallocated */
	if (t && t == u)
		return 0;

	/* Store new tuple */
	*prev = u;

	/* Delete old tuple */
	if (t) {
		u->next = t->next;
		_nvram_free(t);
	}

	return 0;
}

/* Unset the value of an NVRAM variable. Should be locked. */
int
_nvram_unset(const char *name)
{
	uint i;
	struct nvram_tuple *t, **prev;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
	     prev = &t->next, t = *prev);

	/* Delete old tuple */
	if (t) {
		*prev = t->next;
		_nvram_free(t);
	}

	return 0;
}

/* Get all NVRAM variables. Should be locked. */
int
_nvram_getall(char *buf, int count, int include_temp)
{
	uint i;
	struct nvram_tuple *t;
	int len = 0;

	/* Write name=value\0 ... \0\0 */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if (!include_temp && t->val_tmp)
				continue;
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
_nvram_generate(struct nvram_header *header, int rehash)
{
	int i;
	char *ptr, *end;
	struct nvram_tuple *t;
	uint8_t crc;

	/* Generate header */
	header->magic = NVRAM_MAGIC;
	header->crc_ver_init = (NVRAM_VERSION << 8);
	header->crc_ver_init |= (SDRAM_INIT << 16);
	header->config_refresh = SDRAM_CONFIG;
	header->config_refresh |= (SDRAM_REFRESH << 16);
	header->config_ncdl = 0;

	/* Pointer to data area */
	ptr = (char *)header + sizeof(struct nvram_header);

	/* Leave space for a double NUL at the end */
	end = (char *)header + NVRAM_SPACE - 2;

	/* Write out all tuples */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if (!rehash && t->val_tmp)
				continue;
			if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
				break;
			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
		}
	}

	/* End with a double NULL */
	ptr += 1;

	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);

	/* Fill residual by 0xFF */
	if (header->len < NVRAM_SPACE)
		memset((char *)header + header->len, 0xFF, NVRAM_SPACE - header->len);

	crc = nvram_calc_crc(header);

	/* Set new CRC8 */
	header->crc_ver_init |= crc;

	/* Reinitialize hash table */
	if (rehash) {
		nvram_free();
		nvram_rehash(header);
	}

	return 0;
}

/* Initialize hash table. Should be locked. */
int
_nvram_init(struct nvram_header *header)
{
	uint8_t crc;

	if (header->magic == 0xFFFFFFFF || header->magic == 0)
		return 1; // NVRAM MTD is clear

	if (header->magic != NVRAM_MAGIC)
		return -1; // NVRAM is garbage

	if (header->len < sizeof(struct nvram_header))
		return -2; // NVRAM size underflow

	if (header->len > NVRAM_SPACE)
		return -3; // NVRAM size overflow

	crc = nvram_calc_crc(header);
	if ((header->crc_ver_init & 0x000000FF) != (uint32_t)crc)
		return -4; // NVRAM content is corrupted

	nvram_rehash(header);

	return 0;
}

/* Free hash table. Should be locked. */
void
_nvram_uninit(void)
{
	nvram_free();
}

