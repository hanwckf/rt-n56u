/*
 * This string-include defines all string functions as inline
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *		NO Copyright (C) 1991, 1992 Linus Torvalds,
 *		consider these trivial functions to be PD.
 *
 * Modified for uClibc by Erik Andersen <andersen@codepoet.org>
 * These make no attempt to use nifty things like mmx/3dnow/etc.
 * These are not inline, and will therefore not be as fast as
 * modifying the headers to use inlines (and cannot therefore
 * do tricky things when dealing with const memory).  But they
 * should (I hope!) be faster than their generic equivalents....
 *
 * More importantly, these should provide a good example for
 * others to follow when adding arch specific optimizations.
 *  -Erik
 */

#define _STDIO_UTILITY
#define _GNU_SOURCE
#include <string.h>
#include <locale.h> /* for __LOCALE_C_ONLY */

#ifdef L_strcpy
char * strcpy(char * dest, const char * src)
{
    int d0, d1, d2;
    __asm__ __volatile__(
	    "1:\tlodsb\n\t"
	    "stosb\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b"
	    : "=&S" (d0), "=&D" (d1), "=&a" (d2)
	    :"0" (src),"1" (dest) : "memory");
    return dest;
}
#endif


#ifdef L_strncpy
char * strncpy(char * dest, const char * src, size_t count)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
	    "incl %2\n"
	    "1:\n"
	    "decl %2\n"
	    "jz 2f\n"
	    "lodsb\n\t"
	    "stosb\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b\n\t"
	    "decl %2\n"
	    "rep\n\t"
	    "stosb\n"
	    "2:"
	    : "=&S" (d0), "=&D" (d1), "=&c" (d2), "=&a" (d3)
	    :"0" (src),"1" (dest),"2" (count) : "memory");
    return dest;
}
#endif


#ifdef L_strcat
char *strcat(char * dest, const char * src)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
	    "repne\n\t"
	    "scasb\n\t"
	    "decl %1\n"
	    "1:\tlodsb\n\t"
	    "stosb\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b"
	    : "=&S" (d0), "=&D" (d1), "=&a" (d2), "=&c" (d3)
	    : "0" (src), "1" (dest), "2" (0), "3" (0xffffffff):"memory");
    return dest;
}
#endif


#ifdef L_strncat
char *strncat(char * dest, 
	const char * src, size_t count)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
	    "repne\n\t"
	    "scasb\n\t"
	    "decl %1\n\t"
	    "movl %8,%3\n"
	    "incl %3\n"
	    "1:\tdecl %3\n\t"
	    "jz 2f\n"
	    "lodsb\n\t"
	    "stosb\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b\n"
	    "jmp 3f\n"
	    "2:\txorl %2,%2\n\t"
	    "stosb\n"
	    "3:"
	    : "=&S" (d0), "=&D" (d1), "=&a" (d2), "=&c" (d3)
	    : "0" (src),"1" (dest),"2" (0),"3" (0xffffffff), "g" (count)
	    : "memory");
    return dest;
}
#endif


#ifdef L_strcmp
int strcmp(const char *cs, const char *ct)
{
    int d0, d1;
    register int __res;
    __asm__ __volatile__(
	    "1:\tlodsb\n\t"
	    "scasb\n\t"
	    "jne 2f\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b\n\t"
	    "xorl %%eax,%%eax\n\t"
	    "jmp 3f\n"
	    "2:\tsbbl %%eax,%%eax\n\t"
	    "orb $1,%%al\n"
	    "3:"
	    :"=a" (__res), "=&S" (d0), "=&D" (d1)
	    :"1" (cs),"2" (ct));
    return __res;
}
#ifdef __LOCALE_C_ONLY
weak_alias(strcmp,strcoll);
#endif /* __LOCALE_C_ONLY */
#endif


#ifdef L_strncmp
int strncmp(const char *cs, const char *ct, size_t count)
{
    register int __res;
    int d0, d1, d2;
    __asm__ __volatile__(
	    "incl %3\n"
	    "1:\tdecl %3\n\t"
	    "jz 2f\n"
	    "lodsb\n\t"
	    "scasb\n\t"
	    "jne 3f\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b\n"
	    "2:\txorl %%eax,%%eax\n\t"
	    "jmp 4f\n"
	    "3:\tsbbl %%eax,%%eax\n\t"
	    "orb $1,%%al\n"
	    "4:"
	    :"=a" (__res), "=&S" (d0), "=&D" (d1), "=&c" (d2)
	    :"1" (cs),"2" (ct),"3" (count));
    return __res;
}
#endif


#ifdef L_strchr
char * strchr(const char *s, int c)
{
    int d0;
    register char * __res;
    __asm__ __volatile__(
	    "movb %%al,%%ah\n"
	    "1:\tlodsb\n\t"
	    "cmpb %%ah,%%al\n\t"
	    "je 2f\n\t"
	    "testb %%al,%%al\n\t"
	    "jne 1b\n\t"
	    "movl $1,%1\n"
	    "2:\tmovl %1,%0\n\t"
	    "decl %0"
	    :"=a" (__res), "=&S" (d0) : "1" (s),"0" (c));
    return __res;
}
weak_alias(strchr,index);
#endif


#ifdef L_strrchr
char *strrchr(const char *s, int c)
{
    int d0, d1;
    register char * __res;
    __asm__ __volatile__(
	    "movb %%al,%%ah\n"
	    "1:\tlodsb\n\t"
	    "cmpb %%ah,%%al\n\t"
	    "jne 2f\n\t"
	    "leal -1(%%esi),%0\n"
	    "2:\ttestb %%al,%%al\n\t"
	    "jne 1b"
	    :"=g" (__res), "=&S" (d0), "=&a" (d1) :"0" (0),"1" (s),"2" (c));
    return __res;
}
weak_alias(strrchr,rindex);
#endif



#ifdef L_strlen
size_t strlen(const char *s)
{
    int d0;
    register int __res;
    __asm__ __volatile__(
	    "repne\n\t"
	    "scasb\n\t"
	    "notl %0\n\t"
	    "decl %0"
	    :"=c" (__res), "=&D" (d0) :"1" (s),"a" (0), "0" (0xffffffff));
    return __res;
}
#endif


#ifdef L_strnlen
size_t strnlen(const char *s, size_t count)
{
    int d0;
    register int __res;
    __asm__ __volatile__(
	    "movl %2,%0\n\t"
	    "incl %1\n"
	    "jmp 2f\n"
	    "1:\tcmpb $0,(%0)\n\t"
	    "je 3f\n\t"
	    "incl %0\n"
	    "2:\tdecl %1\n\t"
	    "jne 1b\n"
	    "3:\tsubl %2,%0"
	    :"=a" (__res), "=&d" (d0)
	    :"c" (s),"1" (count));
    return __res;
}
#endif


#ifdef L_memcpy
void *memcpy(void * to, const void * from, size_t n)
{
    int d0, d1, d2;
    __asm__ __volatile__(
	    "rep ; movsl\n\t"
	    "testb $2,%b4\n\t"
	    "je 1f\n\t"
	    "movsw\n"
	    "1:\ttestb $1,%b4\n\t"
	    "je 2f\n\t"
	    "movsb\n"
	    "2:"
	    : "=&c" (d0), "=&D" (d1), "=&S" (d2)
	    :"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
	    : "memory");
    return (to);
}
#endif


#ifdef L_memmove
void *memmove(void *dest, const void *src, size_t n)
{
    int d0, d1, d2;
    if (dest<src)
	__asm__ __volatile__(
		"rep\n\t"
		"movsb"
		: "=&c" (d0), "=&S" (d1), "=&D" (d2)
		:"0" (n),"1" (src),"2" (dest)
		: "memory");
    else
	__asm__ __volatile__(
		"std\n\t"
		"rep\n\t"
		"movsb\n\t"
		"cld"
		: "=&c" (d0), "=&S" (d1), "=&D" (d2)
		:"0" (n),
		"1" (n-1+(const char *)src),
		"2" (n-1+(char *)dest)
		:"memory");
    return dest;
}
#endif

#ifdef L_memchr
void *memchr(const void *cs, int c, size_t count)
{
    int d0;
    register void * __res;
    if (!count)
	return NULL;
    __asm__ __volatile__(
	    "repne\n\t"
	    "scasb\n\t"
	    "je 1f\n\t"
	    "movl $1,%0\n"
	    "1:\tdecl %0"
	    :"=D" (__res), "=&c" (d0) : "a" (c),"0" (cs),"1" (count));
    return __res;
}
#endif

#ifdef L_memset
void *memset(void *s, int c, size_t count)
{
    int d0, d1;
    __asm__ __volatile__(
	    "rep\n\t"
	    "stosb"
	    : "=&c" (d0), "=&D" (d1)
	    :"a" (c),"1" (s),"0" (count)
	    :"memory");
    return s;
}
#endif

