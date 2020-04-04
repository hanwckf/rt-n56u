/*
 * Copyright (C) 2008 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball
 */

#if !defined _STRING_H
#error "Never use <libc-string_i386.h> directly; include <string.h> instead"
#endif

#ifndef _LIBC_STRING_i386_H
#define _LIBC_STRING_i386_H 1

static __always_inline
void *inlined_memset_const_c_count4(void *s, unsigned eax, unsigned count)
{
	int ecx, edi;

	if (count == 0)
		return s;

	/* Very small (2 stores or less) are best done with direct
	 * mov <const>,<mem> instructions (they do not clobber registers) */
	if (count == 1) {
		*(char *)(s + 0) = eax;
		return s;
	}

	/* You wonder why & 0xff is needed? Try memset(p, '\xff', size).
	 * If char is signed, '\xff' == -1! */
	eax = (eax & 0xff) * 0x01010101; /* done at compile time */

	if (count == 2) {
		*(short *)(s + 0) = eax;
		return s;
	}
	if (count == 3) {
		*(short *)(s + 0) = eax;
		*(char *) (s + 2) = eax;
		return s;
	}
	if (count == 1*4 + 0) {
		*(int *)(s + 0) = eax;
		return s;
	}
	if (count == 1*4 + 1) {
		*(int *) (s + 0) = eax;
		*(char *)(s + 4) = eax;
		return s;
	}
	if (count == 1*4 + 2) {
		*(int *)  (s + 0) = eax;
		*(short *)(s + 4) = eax;
		return s;
	}

	/* Small string stores: don't clobber ecx
	 * (clobbers only eax and edi) */
#define small_store(arg) { \
	__asm__ __volatile__( \
		arg \
		: "=&D" (edi) \
		: "a" (eax), "0" (s) \
		: "memory" \
	); \
	return s; \
}
	if (count == 1*4 + 3) small_store("stosl; stosw; stosb");
	if (count == 2*4 + 0) {
		((int *)s)[0] = eax;
		((int *)s)[1] = eax;
		return s;
	}
	if (count == 2*4 + 1) small_store("stosl; stosl; stosb");
	if (count == 2*4 + 2) small_store("stosl; stosl; stosw");
	if (count == 2*4 + 3) small_store("stosl; stosl; stosw; stosb");
	if (count == 3*4 + 0) small_store("stosl; stosl; stosl");
	if (count == 3*4 + 1) small_store("stosl; stosl; stosl; stosb");
	if (count == 3*4 + 2) small_store("stosl; stosl; stosl; stosw");
	if (count == 3*4 + 3) small_store("stosl; stosl; stosl; stosw; stosb");
	if (count == 4*4 + 0) small_store("stosl; stosl; stosl; stosl");
	if (count == 4*4 + 1) small_store("stosl; stosl; stosl; stosl; stosb");
	/* going over 7 bytes is suboptimal */
	/* stosw is 2-byte insn, so this one takes 6 bytes: */
	if (count == 4*4 + 2) small_store("stosl; stosl; stosl; stosl; stosw");
	/* 7 bytes */
	if (count == 4*4 + 3) small_store("stosl; stosl; stosl; stosl; stosw; stosb");
	/* 5 bytes */
	if (count == 5*4 + 0) small_store("stosl; stosl; stosl; stosl; stosl");
	/* 6 bytes */
	if (count == 5*4 + 1) small_store("stosl; stosl; stosl; stosl; stosl; stosb");
	/* 7 bytes */
	if (count == 5*4 + 2) small_store("stosl; stosl; stosl; stosl; stosl; stosw");
	/* 8 bytes, but oh well... */
	if (count == 5*4 + 3) small_store("stosl; stosl; stosl; stosl; stosl; stosw; stosb");
	/* 6 bytes */
	if (count == 6*4 + 0) small_store("stosl; stosl; stosl; stosl; stosl; stosl");
	/* the rest would be 7+ bytes and is handled below instead */
#undef small_store

	/* Not small, but multiple-of-4 store.
	 * "mov <const>,%ecx; rep; stosl" sequence is 7 bytes */
	__asm__ __volatile__(
		"	rep; stosl\n"
		: "=&c" (ecx), "=&D" (edi)
		: "a" (eax), "0" (count / 4), "1" (s)
		: "memory"
	);
	return s;
}
#if 1 /* -51 bytes on shared i386 build with gcc 4.3.0 */
#define memset(s, c, count) ( \
	( !(__builtin_constant_p(c) && __builtin_constant_p(count)) \
	  || ((count) > (6*4 + 0) && ((count) % 4) != 0) \
	) \
	? memset((s), (c), (count)) \
	: inlined_memset_const_c_count4((s), (c), (count)) \
	)
#endif


static __always_inline
void *inlined_mempcpy_const_count4(void *d, const void *s, unsigned count)
{
	int ecx;
	char *esi, *edi;

	if (count == 0)
		return d;

	if (count == 1) {
		*(char *)d = *(char *)s;
		return d + 1;
	}
	if (count == 2) {
		*(short *)d = *(short *)s;
		return d + 2;
	}
	/* Small string moves: don't clobber ecx
	 * (clobbers only esi and edi) */
#define small_move(arg) { \
	__asm__ __volatile__( \
		arg \
		: "=&S" (esi), "=&D" (edi) \
		: "0" (s), "1" (d) \
		: "memory" \
	); \
	return edi; \
}
	if (count == 3) small_move("movsw; movsb");
	if (count == 1*4 + 0) {
		*(int *)d = *(int *)s;
		return d + 4;
	}
	if (count == 1*4 + 1) small_move("movsl; movsb");
	if (count == 1*4 + 2) small_move("movsl; movsw");
	if (count == 1*4 + 3) small_move("movsl; movsw; movsb");
	if (count == 2*4 + 0) small_move("movsl; movsl");
	if (count == 2*4 + 1) small_move("movsl; movsl; movsb");
	if (count == 2*4 + 2) small_move("movsl; movsl; movsw");
	if (count == 2*4 + 3) small_move("movsl; movsl; movsw; movsb");
	if (count == 3*4 + 0) small_move("movsl; movsl; movsl");
	if (count == 3*4 + 1) small_move("movsl; movsl; movsl; movsb");
	if (count == 3*4 + 2) small_move("movsl; movsl; movsl; movsw");
	if (count == 3*4 + 3) small_move("movsl; movsl; movsl; movsw; movsb");
	if (count == 4*4 + 0) small_move("movsl; movsl; movsl; movsl");
	if (count == 4*4 + 1) small_move("movsl; movsl; movsl; movsl; movsb");
	/* going over 7 bytes is suboptimal */
	/* movsw is 2-byte insn, so this one takes 6 bytes: */
	if (count == 4*4 + 2) small_move("movsl; movsl; movsl; movsl; movsw");
	/* 7 bytes */
	if (count == 4*4 + 3) small_move("movsl; movsl; movsl; movsl; movsw; movsb");
	/* 5 bytes */
	if (count == 5*4 + 0) small_move("movsl; movsl; movsl; movsl; movsl");
	/* 6 bytes */
	if (count == 5*4 + 1) small_move("movsl; movsl; movsl; movsl; movsl; movsb");
	/* 7 bytes */
	if (count == 5*4 + 2) small_move("movsl; movsl; movsl; movsl; movsl; movsw");
	/* 8 bytes, but oh well... */
	if (count == 5*4 + 3) small_move("movsl; movsl; movsl; movsl; movsl; movsw; movsb");
	/* 6 bytes */
	if (count == 6*4 + 0) small_move("movsl; movsl; movsl; movsl; movsl; movsl");
	/* the rest would be 7+ bytes and is handled below instead */
#undef small_move

	/* Not small, but multiple-of-4 move.
	 * "mov <const>,%ecx; rep; movsl" sequence is 7 bytes */
	__asm__ __volatile__(
		"	rep; movsl\n"
		: "=&c" (ecx), "=&S" (esi), "=&D" (edi)
		: "0" (count / 4), "1" (s), "2" (d)
		: "memory"
	);
	return edi;
}
static __always_inline
void *inlined_memcpy_const_count4(void *d, const void *s, unsigned count)
{
	inlined_mempcpy_const_count4(d, s, count);
	return d;
}
#if 1 /* +34 bytes on shared i386 build with gcc 4.3.0 */
#define mempcpy(d, s, count) ( \
	( !(__builtin_constant_p(count)) \
	  || ((count) > (6*4 + 0) && ((count) % 4) != 0) \
	) \
	? mempcpy((d), (s), (count)) \
	: inlined_mempcpy_const_count4((d), (s), (count)) \
	)
#define memcpy(d, s, count) ( \
	( !(__builtin_constant_p(count)) \
	  || ((count) > (6*4 + 0) && ((count) % 4) != 0) \
	) \
	? memcpy((d), (s), (count)) \
	: inlined_memcpy_const_count4((d), (s), (count)) \
	)
#endif


static __always_inline
size_t inlined_strlen(const char *s)
{
	int edi;
	int ecx;
	__asm__ __volatile__(
		"	repne; scasb\n"
	/*	"	notl	%0\n" */
	/*	"	decl	%0\n" */
		: "=c" (ecx), "=&D" (edi)
		: "1" (s), "a" (0), "0" (0xffffffffu)
		/* : no clobbers */
	);
	return -ecx - 1;
}
#if 0 /* +1108 bytes on shared i386 build with gcc 4.3.0 */
#define strlen(s) inlined_strlen(s)
#endif


static __always_inline
char *inlined_stpcpy(char *dest, const char *src)
{
	char *esi, *edi;
	int eax;
	__asm__ __volatile__(
		"1:	lodsb\n"
		"	stosb\n"
		"	testb	%%al, %%al\n"
		"	jnz	1b\n"
		: "=&S" (esi), "=&D" (edi), "=&a" (eax)
		: "0" (src), "1" (dest)
		: "memory"
	);
	return edi - 1;
}
static __always_inline
char *inlined_strcpy(char *dest, const char *src)
{
	inlined_stpcpy(dest, src);
	return dest;
}
#if 0 /* +562 bytes on shared i386 build with gcc 4.3.0 */
#define stpcpy(dest, src) inlined_stpcpy(dest, src)
#define strcpy(dest, src) inlined_strcpy(dest, src)
#endif


static __always_inline
void *inlined_memchr(const void *s, int c, size_t count)
{
	void *edi;
	int ecx;
	/* Unfortunately, c gets loaded to %eax (wide insn), not %al */
	__asm__ __volatile__(
		"	jecxz	1f\n"
		"	repne; scasb\n"
		"	leal	-1(%%edi), %%edi\n"
		"	je	2f\n"
		"1:\n"
		"	xorl	%%edi, %%edi\n"
		"2:\n"
		: "=&D" (edi), "=&c" (ecx)
		: "a" (c), "0" (s), "1" (count)
		/* : no clobbers */
	);
	return edi;
}
static __always_inline
void *inlined_memchr_const_c(const void *s, int c, size_t count)
{
#if defined __OPTIMIZE__
	void *edi;
	int ecx, eax;
	__asm__ __volatile__(
		"	jecxz	1f\n"
		"	movb	%4, %%al\n" /* const c to %%al */
		"	repne; scasb\n"
		"	leal	-1(%%edi), %%edi\n"
		"	je	2f\n"
		"1:\n"
		"	xorl	%%edi, %%edi\n"
		"2:\n"
		: "=&D" (edi), "=&c" (ecx), "=&a" (eax)
		: "0" (s), "i" (c), "1" (count)
		/* : no clobbers */
	);
	return edi;
#else
	/* With -O0, gcc can't figure out how to encode CONST c
	 * as an immediate operand. Generating slightly bigger code
	 * (usually "movl CONST,%eax", 3 bytes bigger than needed):
	 */
	void *edi;
	int ecx, eax;
	__asm__ __volatile__(
		"	jecxz	1f\n"
		"	repne; scasb\n"
		"	leal	-1(%%edi), %%edi\n"
		"	je	2f\n"
		"1:\n"
		"	xorl	%%edi, %%edi\n"
		"2:\n"
		: "=&D" (edi), "=&c" (ecx), "=&a" (eax)
		: "0" (s), "2" (c), "1" (count)
		/* : no clobbers */
	);
	return edi;
#endif
}
#if 1 /* +2 bytes on shared i386 build with gcc 4.3.0 */
#define memchr(s, c, count) ( \
	__builtin_constant_p(c) \
	? inlined_memchr_const_c(s, (c) & 0xff, count) \
	: inlined_memchr(s, c, count) \
	)
#endif

#endif /* _LIBC_STRING_i386_H  */
