/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 * For nds32 ISA, the syscall number(SWID) shall be determined at compile time.
 * (ex: asm("syscall SWID"); )
 * If the value of syscall number is determined at run time, we shall issue
 * this syscall through sys_syscall.
 * (ex:
 *     asm("move $r0, SYSCALL_number"
 *         "syscall 0x5071");
 *     where 0x5071 is syscall number for sys_syscall
 * )
 *
 * The following two macros are implemented according that syscall number
 * is determined in compiler time or run time,
 *
 * 1. INTERNAL_SYSCALL_NCS: the syscall number is determined at run time
 * 2. INTERNAL_SYSCALL: the syscall number is determined at compile time
 *
 */


#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__
#include <errno.h>
#include <common/sysdep.h>

#define X(x) #x
#define Y(x) X(x)
#define        LIB_SYSCALL    __NR_syscall

#define __issue_syscall(syscall_name)                   		\
"       syscall  "  Y(syscall_name) ";	\n"

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) ((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                        	\
  ({                                                             	\
     INTERNAL_SYSCALL_DECL (err);                                	\
     long result_var = INTERNAL_SYSCALL (name, err, nr, args);   	\
     if (INTERNAL_SYSCALL_ERROR_P (result_var, err))             	\
       {                                                         	\
         __set_errno (INTERNAL_SYSCALL_ERRNO (result_var, err)); 	\
			result_var = -1 ;			 	\
       }                                                         	\
     result_var;                                                 	\
  })


#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)


#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...) internal_syscall##nr(__NR_##name, err, args)

/*
   The _NCS variant allows non-constant syscall numbers
 */
#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) internal_syscall_ncs##nr(name, err, args)


#define internal_syscall0(name, err, dummy...)                   	\
  ({                                                             	\
       register long ___res  __asm__("$r0");               		\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       :							 	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
       ___res;							 	\
  })

#define internal_syscall1(name, err, arg1)                       	\
  ({                                                             	\
       register long ___res  __asm__("$r0");                         	\
       register long __arg1 __asm__("$r0") = (long) (arg1);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall2(name, err, arg1, arg2)                 	\
  ({                                                             	\
       register long ___res  __asm__("$r0");                         	\
       register long __arg1 __asm__("$r0") = (long) (arg1);         	\
       register long __arg2 __asm__("$r1") = (long) (arg2);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall3(name, err, arg1, arg2, arg3)           	\
  ({                                                             	\
       register long ___res  __asm__("$r0");                         	\
       register long __arg1 __asm__("$r0") = (long) (arg1);         	\
       register long __arg2 __asm__("$r1") = (long) (arg2);         	\
       register long __arg3 __asm__("$r2") = (long) (arg3);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__arg3)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall4(name, err, arg1, arg2, arg3, arg4)     	\
  ({                                                             	\
       register long ___res  __asm__("$r0");                         	\
       register long __arg1 __asm__("$r0") = (long) (arg1);         	\
       register long __arg2 __asm__("$r1") = (long) (arg2);         	\
       register long __arg3 __asm__("$r2") = (long) (arg3);         	\
       register long __arg4 __asm__("$r3") = (long) (arg4);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__arg3)         /* input operands  */              	\
       , "r" (__arg4)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall5(name, err, arg1, arg2, arg3, arg4, arg5) 	\
  ({                                                             	\
       register long ___res  __asm__("$r0");                         	\
       register long __arg1 __asm__("$r0") = (long) (arg1);         	\
       register long __arg2 __asm__("$r1") = (long) (arg2);         	\
       register long __arg3 __asm__("$r2") = (long) (arg3);         	\
       register long __arg4 __asm__("$r3") = (long) (arg4);         	\
       register long __arg5 __asm__("$r4") = (long) (arg5);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (name)                                    	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__arg3)         /* input operands  */              	\
       , "r" (__arg4)         /* input operands  */              	\
       , "r" (__arg5)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall6(name, err, arg1, arg2, arg3, arg4, arg5, arg6) 	\
  ({                                                             		\
       register long ___res  __asm__("$r0");                         		\
       register long __arg1 __asm__("$r0") = (long) (arg1);         		\
       register long __arg2 __asm__("$r1") = (long) (arg2);         		\
       register long __arg3 __asm__("$r2") = (long) (arg3);         		\
       register long __arg4 __asm__("$r3") = (long) (arg4);         		\
       register long __arg5 __asm__("$r4") = (long) (arg5);         		\
       register long __arg6 __asm__("$r5") = (long) (arg6);         		\
       __asm__ volatile (                                        		\
       __issue_syscall (name)                                    		\
       : "=r" (___res)         /* output operands  */             		\
       : "r" (__arg1)         /* input operands  */              		\
       , "r" (__arg2)         /* input operands  */              		\
       , "r" (__arg3)         /* input operands  */              		\
       , "r" (__arg4)         /* input operands  */              		\
       , "r" (__arg5)         /* input operands  */              		\
       , "r" (__arg6)         /* input operands  */              		\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 		\
        ___res;                                                   		\
  })
#define internal_syscall7(name, err, arg1, arg2, arg3, arg4, arg5, arg6, arg7) 	\
  ({                                                             		\
       register long ___res  __asm__("$r0");                         		\
       register long __arg1 __asm__("$r0") = (long) (arg1);         		\
       register long __arg2 __asm__("$r1") = (long) (arg2);         		\
       register long __arg3 __asm__("$r2") = (long) (arg3);         		\
       register long __arg4 __asm__("$r3") = (long) (arg4);         		\
       register long __arg5 __asm__("$r4") = (long) (arg5);         		\
       register long __arg6 __asm__("$r5") = (long) (arg6);         		\
       __asm__ volatile (                                        		\
	"addi10.sp\t  #-4\n\t"							\
	CFI_ADJUST_CFA_OFFSET(4)"\n\t"						\
        "push\t %7\n\t"								\
	CFI_ADJUST_CFA_OFFSET(4)"\n\t"						\
       __issue_syscall (name)                                    		\
	"addi10.sp\t  #4\n\t"							\
	CFI_ADJUST_CFA_OFFSET(-4)"\n\t"						\
        "pop\t %7\n\t"								\
	CFI_ADJUST_CFA_OFFSET(-4)"\n\t"						\
       : "=r" (___res)         /* output operands  */             		\
       : "r" (__arg1)         /* input operands  */              		\
       , "r" (__arg2)         /* input operands  */              		\
       , "r" (__arg3)         /* input operands  */              		\
       , "r" (__arg4)         /* input operands  */              		\
       , "r" (__arg5)         /* input operands  */              		\
       , "r" (__arg6)         /* input operands  */              		\
       , "r" (arg7)         /* input operands  */              		\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 		\
        ___res;                                                   		\
  })

#define internal_syscall_ncs0(name, err, dummy...)               	\
  ({                                                             	\
       register long __res  __asm__("$r0");                         	\
       register long __no   __asm__("$r0") = (long) (name);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (LIB_SYSCALL)                             	\
       : "=r" (__res)         /* output operands  */             	\
       : "r" (__no)           /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
       __res;							 	\
  })

#define internal_syscall_ncs1(name, err, arg1)                   	\
  ({                                                             	\
       register long __res  __asm__("$r0");                         	\
       register long __no   __asm__("$r0") = (long) (name);         	\
       register long __arg1 __asm__("$r1") = (long) (arg1);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (LIB_SYSCALL)                             	\
       : "=r" (__res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__no)           /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        __res;                                                   	\
  })

#define internal_syscall_ncs2(name, err, arg1, arg2)             	\
  ({                                                             	\
       register long __res  __asm__("$r0");                         	\
       register long __no   __asm__("$r0") = (long) (name);         	\
       register long __arg1 __asm__("$r1") = (long) (arg1);         	\
       register long __arg2 __asm__("$r2") = (long) (arg2);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (LIB_SYSCALL)                             	\
       : "=r" (__res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__no)           /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        __res;                                                   	\
  })

#define internal_syscall_ncs3(name, err, arg1, arg2, arg3)      	\
  ({                                                            	\
       register long __res  __asm__("$r0");                     	\
       register long __no   __asm__("$r0") = (long) (name);     	\
       register long __arg1 __asm__("$r1") = (long) (arg1);     	\
       register long __arg2 __asm__("$r2") = (long) (arg2);     	\
       register long __arg3 __asm__("$r3") = (long) (arg3);     	\
       __asm__ volatile (                                       	\
       __issue_syscall (LIB_SYSCALL)                            	\
       : "=r" (__res)         /* output operands  */            	\
       : "r" (__arg1)         /* input operands  */             	\
       , "r" (__arg2)         /* input operands  */             	\
       , "r" (__arg3)         /* input operands  */             	\
       , "r" (__no)           /* input operands  */             	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */	\
        __res;                                                  	\
  })

#define internal_syscall_ncs4(name, err, arg1, arg2, arg3, arg4) 	\
  ({                                                             	\
       register long __res  __asm__("$r0");                      	\
       register long __no   __asm__("$r0") = (long) (name);      	\
       register long __arg1 __asm__("$r1") = (long) (arg1);      	\
       register long __arg2 __asm__("$r2") = (long) (arg2);      	\
       register long __arg3 __asm__("$r3") = (long) (arg3);      	\
       register long __arg4 __asm__("$r4") = (long) (arg4);      	\
       __asm__ volatile (                                        	\
       __issue_syscall (LIB_SYSCALL)                             	\
       : "=r" (__res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__arg3)         /* input operands  */              	\
       , "r" (__arg4)         /* input operands  */              	\
       , "r" (__no)           /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        __res;                                                   	\
  })

#define internal_syscall_ncs5(name, err, arg1, arg2, arg3, arg4, arg5) 	\
  ({                                                             	\
       register long ___res __asm__("$r0");                         	\
       register long __no   __asm__("$r0") = (long) (name);         	\
       register long __arg1 __asm__("$r1") = (long) (arg1);         	\
       register long __arg2 __asm__("$r2") = (long) (arg2);         	\
       register long __arg3 __asm__("$r3") = (long) (arg3);         	\
       register long __arg4 __asm__("$r4") = (long) (arg4);         	\
       register long __arg5 __asm__("$r5") = (long) (arg5);         	\
       __asm__ volatile (                                        	\
       __issue_syscall (LIB_SYSCALL)                                   	\
       : "=r" (___res)         /* output operands  */             	\
       : "r" (__arg1)         /* input operands  */              	\
       , "r" (__arg2)         /* input operands  */              	\
       , "r" (__arg3)         /* input operands  */              	\
       , "r" (__arg4)         /* input operands  */              	\
       , "r" (__arg5)         /* input operands  */              	\
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */ 	\
        ___res;                                                   	\
  })

#define internal_syscall_ncs6(name, err, arg1, arg2, arg3, arg4, arg5, arg6)	\
  ({                                                                            \
       register long __res  __asm__("$r0");                                     \
       register long __no   __asm__("$r0") = (long) (name);                     \
       register long __arg1 __asm__("$r1") = (long) (arg1);                     \
       register long __arg2 __asm__("$r2") = (long) (arg2);                     \
       register long __arg3 __asm__("$r3") = (long) (arg3);                     \
       register long __arg4 __asm__("$r4") = (long) (arg4);                     \
       register long __arg5 __asm__("$r5") = (long) (arg5);                     \
       __asm__ volatile (                                                       \
        "addi10.sp\t  #-4\n\t"                                                  \
        CFI_ADJUST_CFA_OFFSET(4)"\n\t"                                          \
        "push\t %7\n\t"                                                         \
        CFI_ADJUST_CFA_OFFSET(4)"\n\t"                                          \
       __issue_syscall (LIB_SYSCALL)                                            \
        "pop\t %7\n\t"                                                          \
        CFI_ADJUST_CFA_OFFSET(-4)"\n\t"                                         \
        "addi10.sp\t  #4\n\t"                                                   \
        CFI_ADJUST_CFA_OFFSET(-4)"\n\t"                                         \
       : "=r" (__res)         /* output operands  */                            \
       : "r" (__no)           /* input operands  */                             \
       , "r" (__arg1)         /* input operands  */                             \
       , "r" (__arg2)         /* input operands  */                             \
       , "r" (__arg3)         /* input operands  */                             \
       , "r" (__arg4)         /* input operands  */                             \
       , "r" (__arg5)         /* input operands  */                             \
       , "r" (arg6)         /* input operands  */                               \
       : __SYSCALL_CLOBBERS); /* list of clobbered registers  */                \
        __res;                                                                  \
  })

#define __SYSCALL_CLOBBERS "$lp", "memory"
#endif /* ! __ASSEMBLER__  */
#endif /* _BITS_SYSCALLS_H */
