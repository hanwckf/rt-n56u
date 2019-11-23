/* elfrw.h: The elfrw library's internally shared functions.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _elfrw_int_h_
#define _elfrw_int_h_

#include <stdio.h>
#include <elf.h>
#include "elfrw.h"

/* Internal shared variables.
 */
extern unsigned char _elfrw_native_data;
extern unsigned char _elfrw_current_class;
extern unsigned char _elfrw_current_data;
extern unsigned char _elfrw_current_version;

/* Macros that encapsulate the commonly needed tests.
 */
#define native_form() (_elfrw_native_data == _elfrw_current_data)
#define is64bit_form() (_elfrw_current_class == ELFCLASS64)

/*
 * Endianness-swapping functions.
 */

static inline unsigned short rev2(unsigned short in)
{
    return ((in & 0x00FF) << 8) | ((in & 0xFF00) >> 8);
}

static inline unsigned int rev4(unsigned int in)
{
#if defined __i386__ || defined __x86_64__
    __asm__("bswap %0" : "=r"(in) : "0"(in));
    return in;
#else
    return ((in & 0x000000FFU) << 24)
	 | ((in & 0x0000FF00U) << 8)
	 | ((in & 0x00FF0000U) >> 8)
	 | ((in & 0xFF000000U) >> 24);
#endif
}

static inline unsigned long long rev8(unsigned long long in)
{
#if defined __x86_64__
    __asm__("bswap %0" : "=r"(in) : "0"(in));
    return in;
#else
    return ((in & 0x00000000000000FFULL) << 56)
	 | ((in & 0x000000000000FF00ULL) << 40)
	 | ((in & 0x0000000000FF0000ULL) << 24)
	 | ((in & 0x00000000FF000000ULL) << 8)
	 | ((in & 0x000000FF00000000ULL) >> 8)
	 | ((in & 0x0000FF0000000000ULL) >> 24)
	 | ((in & 0x00FF000000000000ULL) >> 40)
	 | ((in & 0xFF00000000000000ULL) >> 56);
#endif
}

#define rev_32half(in)  ((Elf32_Half)rev2((Elf32_Half)(in)))
#define rev_32word(in)  ((Elf32_Word)rev4((Elf32_Word)(in)))
#define rev_32xword(in) ((Elf32_Xword)rev8((Elf32_Xword)(in)))
#define rev_32addr(in)  ((Elf32_Addr)rev4((Elf32_Addr)(in)))
#define rev_32off(in)   ((Elf32_Off)rev4((Elf32_Off)(in)))
#define rev_64half(in)  ((Elf64_Half)rev2((Elf64_Half)(in)))
#define rev_64word(in)  ((Elf64_Word)rev4((Elf64_Word)(in)))
#define rev_64xword(in) ((Elf64_Xword)rev8((Elf64_Xword)(in)))
#define rev_64addr(in)  ((Elf64_Addr)rev8((Elf64_Addr)(in)))
#define rev_64off(in)   ((Elf64_Off)rev8((Elf64_Off)(in)))

static inline void revinplc2(void *in)
{
    unsigned char tmp;

    tmp = ((unsigned char*)in)[0];
    ((unsigned char*)in)[0] = ((unsigned char*)in)[1];
    ((unsigned char*)in)[1] = tmp;
}

static inline void revinplc4(void *in)
{
#if defined __i386__ || defined __x86_64__
    __asm__("bswap %0" : "=r"(*(unsigned int*)in) : "0"(*(unsigned int*)in));
#else
    unsigned char tmp;

    tmp = ((unsigned char*)in)[0];
    ((unsigned char*)in)[0] = ((unsigned char*)in)[3];
    ((unsigned char*)in)[3] = tmp;
    tmp = ((unsigned char*)in)[1];
    ((unsigned char*)in)[1] = ((unsigned char*)in)[2];
    ((unsigned char*)in)[2] = tmp;
#endif
}

static inline void revinplc8(void *in)
{
#if defined __x86_64__
    __asm__("bswap %0" : "=r"(*(unsigned long*)in) : "0"(*(unsigned long*)in));
#else
    unsigned char tmp;

    tmp = ((unsigned char*)in)[0];
    ((unsigned char*)in)[0] = ((unsigned char*)in)[7];
    ((unsigned char*)in)[7] = tmp;
    tmp = ((unsigned char*)in)[1];
    ((unsigned char*)in)[1] = ((unsigned char*)in)[6];
    ((unsigned char*)in)[6] = tmp;
    tmp = ((unsigned char*)in)[2];
    ((unsigned char*)in)[2] = ((unsigned char*)in)[5];
    ((unsigned char*)in)[5] = tmp;
    tmp = ((unsigned char*)in)[3];
    ((unsigned char*)in)[3] = ((unsigned char*)in)[4];
    ((unsigned char*)in)[4] = tmp;
#endif
}

#define revinplc_32half(in)  (revinplc2(in))
#define revinplc_32word(in)  (revinplc4(in))
#define revinplc_32xword(in) (revinplc8(in))
#define revinplc_32addr(in)  (revinplc4(in))
#define revinplc_32off(in)   (revinplc4(in))
#define revinplc_64half(in)  (revinplc2(in))
#define revinplc_64word(in)  (revinplc4(in))
#define revinplc_64xword(in) (revinplc8(in))
#define revinplc_64addr(in)  (revinplc8(in))
#define revinplc_64off(in)   (revinplc8(in))

#endif
