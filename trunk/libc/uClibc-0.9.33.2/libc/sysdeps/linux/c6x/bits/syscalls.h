#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#undef __SYSCALL_STRING
# define __SYSCALL_STRING			\
		"swe\n\t" \
		"nop\n\t"

# define __SYSCALL_RES_CHECK (__res < -255 || __res >= 0)

#define __SYSCALL_CLOBBERS "cc", "memory"

#define __SYSCALL_RETURN(type) \
	if (__SYSCALL_RES_CHECK) \
		return (type) __res; \
	__set_errno (-__res); \
	return (type) -1;

#ifndef NOT_IN_libc
#define DEBUG_SYSCALL(name) { \
      char d[64];\
      write( 2, d, snprintf( d, 64, "syscall %d error %d\n", __NR_##name, _inline_sys_result)); \
}
#else
#define DEBUG_SYSCALL(name) do{} while(0)
#endif

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)				\
  ({ unsigned int _inline_sys_result = INTERNAL_SYSCALL (name, , nr, args);	\
     if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_inline_sys_result, ), 0))	\
       {								\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (_inline_sys_result, ));		\
	 _inline_sys_result = (unsigned int) -1;				\
       }								\
     (int) _inline_sys_result; })

#undef INLINE_SYSCALL_NOERR
#define INLINE_SYSCALL_NOERR(name, nr, args...)				\
  ({ unsigned int _inline_sys_result = INTERNAL_SYSCALL (name, , nr, args);	\
     (int) _inline_sys_result; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)


#define INTERNAL_SYSCALL( name, err, nr, args...) \
    INTERNAL_SYSCALL_NCS( __NR_##name, err, nr, args )


#define INTERNAL_SYSCALL_NCS(sys_num, err, nr, args...) \
(__extension__ \
    ({                                                      \
	register long __A4 __asm__("A4");		    \
	register long __b0 __asm__("B0") = sys_num;	    \
	LOAD_ARGS_##nr(args)				    \
	__asm__ __volatile__(__SYSCALL_STRING		    \
			     : "=a" (__A4)				\
			     : "b" (__b0) ASM_ARGS_##nr			\
			     : __SYSCALL_CLOBBERS );			\
	(int)__A4;							\
    }) \
)
#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#if 0
# define CALL_ERRNO_LOCATION "call   __errno_location;"
#define __CLONE_SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" sub	%%o1, 1, %%o1;"						\
	"and	%%A4, %%o1, %%A4;"					\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	CALL_ERRNO_LOCATION						\
	" nop;"								\
	"st	%%i0, [%%A4];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%A4;"					\
	".previous;"

#define INLINE_CLONE_SYSCALL(arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __A4 __asm__ ("A4") = (long)(arg1);		\
	register long __B4 __asm__ ("B4") = (long)(arg2);		\
	register long __A6 __asm__ ("A6") = (long)(arg3);		\
	register long __B6 __asm__ ("B6") = (long)(arg4);		\
	register long __A8 __asm__ ("A8") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_clone;			\
	__asm __volatile (__CLONE_SYSCALL_STRING :			\
			  "=r" (__g1), "=r" (__A4), "=r" (__B4)	:	\
			  "0" (__g1), "1" (__A4), "2" (__B4),		\
			  "r" (__A6), "r" (__B6), "r" (__A8) :		\
			  __SYSCALL_CLOBBERS);				\
	__A4;								\
})
#endif

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(A4) \
    __A4 = (int)A4;     \
    LOAD_ARGS_0()
#define ASM_ARGS_1 ASM_ARGS_0, "a" (__A4)
#define LOAD_ARGS_2(A4, B4)			\
  register int __B4 __asm__ ("B4") = (int) (B4);	\
  LOAD_ARGS_1 (A4)
#define ASM_ARGS_2	ASM_ARGS_1, "b" (__B4)
#define LOAD_ARGS_3(A4, B4, A6)			\
  register int __A6 __asm__ ("A6") = (int) (A6);	\
  LOAD_ARGS_2 (A4, B4)
#define ASM_ARGS_3	ASM_ARGS_2, "a" (__A6)
#define LOAD_ARGS_4(A4, B4, A6, B6)		\
  register int __B6 __asm__ ("B6") = (int) (B6);	\
  LOAD_ARGS_3 (A4, B4, A6)
#define ASM_ARGS_4	ASM_ARGS_3, "b" (__B6)
#define LOAD_ARGS_5(A4, B4, A6, B6, A8)		\
  register int __A8 __asm__ ("A8") = (int) (A8);	\
  LOAD_ARGS_4 (A4, B4, A6, B6)
#define ASM_ARGS_5	ASM_ARGS_4, "a" (__A8)
#define LOAD_ARGS_6(A4, B4, A6, B6, A8, B8)	\
  register int __B8 __asm__ ("B8") = (int) (B8);	\
  LOAD_ARGS_5 (A4, B4, A6, B6, A8)
#define ASM_ARGS_6	ASM_ARGS_5, "b" (__B8)

#ifndef _syscall0

#define C_DECL_ARGS_0()			void
#define C_DECL_ARGS_1(t, v)		t v
#define C_DECL_ARGS_2(t, v, args...)	t v, C_DECL_ARGS_1(args)
#define C_DECL_ARGS_3(t, v, args...)	t v, C_DECL_ARGS_2(args)
#define C_DECL_ARGS_4(t, v, args...)	t v, C_DECL_ARGS_3(args)
#define C_DECL_ARGS_5(t, v, args...)	t v, C_DECL_ARGS_4(args)
#define C_DECL_ARGS_6(t, v, args...)	t v, C_DECL_ARGS_5(args)

#define C_ARGS_0()
#define C_ARGS_1(t, v)			v
#define C_ARGS_2(t, v, args...)		v, C_ARGS_1(args)
#define C_ARGS_3(t, v, args...)		v, C_ARGS_2(args)
#define C_ARGS_4(t, v, args...)		v, C_ARGS_3(args)
#define C_ARGS_5(t, v, args...)		v, C_ARGS_4(args)
#define C_ARGS_6(t, v, args...)		v, C_ARGS_5(args)

#define SYSCALL_FUNC(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL(name, nargs, C_ARGS_##nargs(args));	\
}

#define SYSCALL_NOERR_FUNC(nargs, type, name, args...)			\
type name(C_DECL_ARGS_##nargs(args)) {					\
	return (type)INLINE_SYSCALL_NOERR(name, nargs, C_ARGS_##nargs(args));	\
}

#define _syscall0(args...)		SYSCALL_FUNC(0, args)
#define _syscall_noerr0(args...)	SYSCALL_NOERR_FUNC(0, args)
#define _syscall1(args...)		SYSCALL_FUNC(1, args)
#define _syscall_noerr1(args...)	SYSCALL_NOERR_FUNC(1, args)
#define _syscall2(args...)		SYSCALL_FUNC(2, args)
#define _syscall3(args...)		SYSCALL_FUNC(3, args)
#define _syscall4(args...)		SYSCALL_FUNC(4, args)
#define _syscall5(args...)		SYSCALL_FUNC(5, args)
#define _syscall6(args...)		SYSCALL_FUNC(6, args)

#endif /* _syscall0 */

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */

