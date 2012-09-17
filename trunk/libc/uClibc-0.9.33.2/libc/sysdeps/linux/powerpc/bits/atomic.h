/* Atomic operations.  PowerPC Common version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <bits/wordsize.h>

#if __WORDSIZE == 64
/* Atomic operations.  PowerPC64 version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* The 32-bit exchange_bool is different on powerpc64 because the subf
   does signed 64-bit arthmatic while the lwarx is 32-bit unsigned
   (a load word and zero (high 32) form) load.
   In powerpc64 register values are 64-bit by default,  including oldval.
   The value in old val unknown sign extension, lwarx loads the 32-bit
   value as unsigned.  So we explicitly clear the high 32 bits in oldval.  */
# define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval) \
({									      \
  unsigned int __tmp, __tmp2;						      \
  __asm__ __volatile__ ("   clrldi  %1,%1,32\n"				      \
		    "1:	lwarx	%0,0,%2\n"				      \
		    "	subf.	%0,%1,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%4,0,%2\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp), "=r" (__tmp2)			      \
		    : "b" (mem), "1" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_32_rel(mem, newval, oldval) \
({									      \
  unsigned int __tmp, __tmp2;						      \
  __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		    "   clrldi  %1,%1,32\n"				      \
		    "1:	lwarx	%0,0,%2\n"				      \
		    "	subf.	%0,%1,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%4,0,%2\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	"						      \
		    : "=&r" (__tmp), "=r" (__tmp2)			      \
		    : "b" (mem), "1" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

/*
 * Only powerpc64 processors support Load doubleword and reserve index (ldarx)
 * and Store doubleword conditional indexed (stdcx) instructions.  So here
 * we define the 64-bit forms.
 */
# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
({									      \
  unsigned long	__tmp;							      \
  __asm__ __volatile__ (							      \
		    "1:	ldarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stdcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_64_rel(mem, newval, oldval) \
({									      \
  unsigned long	__tmp;							      \
  __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		    "1:	ldarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stdcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	"						      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({									      \
      __typeof (*(mem)) __tmp;						      \
      __typeof (mem)  __memp = (mem);					      \
      __asm__ __volatile__ (						      \
		        "1:	ldarx	%0,0,%1\n"			      \
		        "	cmpd	%0,%2\n"			      \
		        "	bne	2f\n"				      \
		        "	stdcx.	%3,0,%1\n"			      \
		        "	bne-	1b\n"				      \
		        "2:	" __ARCH_ACQ_INSTR			      \
		        : "=&r" (__tmp)					      \
		        : "b" (__memp), "r" (oldval), "r" (newval)	      \
		        : "cr0", "memory");				      \
      __tmp;								      \
  })

#define __arch_compare_and_exchange_val_64_rel(mem, newval, oldval) \
  ({									      \
      __typeof (*(mem)) __tmp;						      \
      __typeof (mem)  __memp = (mem);					      \
      __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		        "1:	ldarx	%0,0,%1\n"			      \
		        "	cmpd	%0,%2\n"			      \
		        "	bne	2f\n"				      \
		        "	stdcx.	%3,0,%1\n"			      \
		        "	bne-	1b\n"				      \
		        "2:	"					      \
		        : "=&r" (__tmp)					      \
		        : "b" (__memp), "r" (oldval), "r" (newval)	      \
		        : "cr0", "memory");				      \
      __tmp;								      \
  })

# define __arch_atomic_exchange_64_acq(mem, value) \
    ({									      \
      __typeof (*mem) __val;						      \
      __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
			"1:	ldarx	%0,0,%2\n"			      \
			"	stdcx.	%3,0,%2\n"			      \
			"	bne-	1b\n"				      \
		  " " __ARCH_ACQ_INSTR					      \
			: "=&r" (__val), "=m" (*mem)			      \
			: "b" (mem), "r" (value), "m" (*mem)		      \
			: "cr0", "memory");				      \
      __val;								      \
    })

# define __arch_atomic_exchange_64_rel(mem, value) \
    ({									      \
      __typeof (*mem) __val;						      \
      __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
			"1:	ldarx	%0,0,%2\n"			      \
			"	stdcx.	%3,0,%2\n"			      \
			"	bne-	1b"				      \
			: "=&r" (__val), "=m" (*mem)			      \
			: "b" (mem), "r" (value), "m" (*mem)		      \
			: "cr0", "memory");				      \
      __val;								      \
    })

# define __arch_atomic_exchange_and_add_64(mem, value) \
    ({									      \
      __typeof (*mem) __val, __tmp;					      \
      __asm__ __volatile__ ("1:	ldarx	%0,0,%3\n"			      \
			"	add	%1,%0,%4\n"			      \
			"	stdcx.	%1,0,%3\n"			      \
			"	bne-	1b"				      \
			: "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
			: "b" (mem), "r" (value), "m" (*mem)		      \
			: "cr0", "memory");				      \
      __val;								      \
    })

# define __arch_atomic_increment_val_64(mem) \
    ({									      \
      __typeof (*(mem)) __val;						      \
      __asm__ __volatile__ ("1:	ldarx	%0,0,%2\n"			      \
			"	addi	%0,%0,1\n"			      \
			"	stdcx.	%0,0,%2\n"			      \
			"	bne-	1b"				      \
			: "=&b" (__val), "=m" (*mem)			      \
			: "b" (mem), "m" (*mem)				      \
			: "cr0", "memory");				      \
      __val;								      \
    })

# define __arch_atomic_decrement_val_64(mem) \
    ({									      \
      __typeof (*(mem)) __val;						      \
      __asm__ __volatile__ ("1:	ldarx	%0,0,%2\n"			      \
			"	subi	%0,%0,1\n"			      \
			"	stdcx.	%0,0,%2\n"			      \
			"	bne-	1b"				      \
			: "=&b" (__val), "=m" (*mem)			      \
			: "b" (mem), "m" (*mem)				      \
			: "cr0", "memory");				      \
      __val;								      \
    })

# define __arch_atomic_decrement_if_positive_64(mem) \
  ({ int __val, __tmp;							      \
     __asm__ __volatile__ ("1:	ldarx	%0,0,%3\n"			      \
		       "	cmpdi	0,%0,0\n"			      \
		       "	addi	%1,%0,-1\n"			      \
		       "	ble	2f\n"				      \
		       "	stdcx.	%1,0,%3\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __ARCH_ACQ_INSTR			      \
		       : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		       : "b" (mem), "m" (*mem)				      \
		       : "cr0", "memory");				      \
     __val;								      \
  })

/*
 * All powerpc64 processors support the new "light weight"  sync (lwsync).
 */
# define atomic_read_barrier()	__asm__ ("lwsync" ::: "memory")
/*
 * "light weight" sync can also be used for the release barrier.
 */
# ifndef UP
#  define __ARCH_REL_INSTR	"lwsync"
# endif

#else
/* Atomic operations.  PowerPC32 version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*
 * The 32-bit exchange_bool is different on powerpc64 because the subf
 * does signed 64-bit arthmatic while the lwarx is 32-bit unsigned
 * (a load word and zero (high 32) form).  So powerpc64 has a slightly
 * different version in sysdeps/powerpc/powerpc64/bits/atomic.h.
 */
# define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval)         \
({									      \
  unsigned int __tmp;							      \
  __asm__ __volatile__ (							      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	" __ARCH_ACQ_INSTR				      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

# define __arch_compare_and_exchange_bool_32_rel(mem, newval, oldval)	      \
({									      \
  unsigned int __tmp;							      \
  __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		    "1:	lwarx	%0,0,%1\n"				      \
		    "	subf.	%0,%2,%0\n"				      \
		    "	bne	2f\n"					      \
		    "	stwcx.	%3,0,%1\n"				      \
		    "	bne-	1b\n"					      \
		    "2:	"						      \
		    : "=&r" (__tmp)					      \
		    : "b" (mem), "r" (oldval), "r" (newval)		      \
		    : "cr0", "memory");					      \
  __tmp != 0;								      \
})

/* Powerpc32 processors don't implement the 64-bit (doubleword) forms of
   load and reserve (ldarx) and store conditional (stdcx.) instructions.
   So for powerpc32 we stub out the 64-bit forms.  */
# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

# define __arch_compare_and_exchange_bool_64_rel(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_val_64_rel(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

# define __arch_atomic_exchange_64_acq(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_64_rel(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_and_add_64(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_increment_val_64(mem) \
    ({ abort (); (*mem)++; })

# define __arch_atomic_decrement_val_64(mem) \
    ({ abort (); (*mem)--; })

# define __arch_atomic_decrement_if_positive_64(mem) \
    ({ abort (); (*mem)--; })

#ifdef _ARCH_PWR4
/*
 * Newer powerpc64 processors support the new "light weight" sync (lwsync)
 * So if the build is using -mcpu=[power4,power5,power5+,970] we can
 * safely use lwsync.
 */
# define atomic_read_barrier()	__asm__ ("lwsync" ::: "memory")
/*
 * "light weight" sync can also be used for the release barrier.
 */
# ifndef UP
#  define __ARCH_REL_INSTR	"lwsync"
# endif
#else

/*
 * Older powerpc32 processors don't support the new "light weight"
 * sync (lwsync).  So the only safe option is to use normal sync
 * for all powerpc32 applications.
 */
# define atomic_read_barrier()	__asm__ ("sync" ::: "memory")
#endif

#endif

#include <stdint.h>

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/*
 * Powerpc does not have byte and halfword forms of load and reserve and
 * store conditional. So for powerpc we stub out the 8- and 16-bit forms.
 */
#define __arch_compare_and_exchange_bool_8_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_8_rel(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_rel(mem, newval, oldval) \
  (abort (), 0)

#ifdef UP
# define __ARCH_ACQ_INSTR	""
# define __ARCH_REL_INSTR	""
#else
# define __ARCH_ACQ_INSTR	"isync"
# ifndef __ARCH_REL_INSTR
#  define __ARCH_REL_INSTR	"sync"
# endif
#endif

#ifndef MUTEX_HINT_ACQ
# define MUTEX_HINT_ACQ
#endif
#ifndef MUTEX_HINT_REL
# define MUTEX_HINT_REL
#endif

#define atomic_full_barrier()	__asm__ ("sync" ::: "memory")
#define atomic_write_barrier()	__asm__ ("eieio" ::: "memory")

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)	      \
  ({									      \
      __typeof (*(mem)) __tmp;						      \
      __typeof (mem)  __memp = (mem);					      \
      __asm__ __volatile__ (						      \
		        "1:	lwarx	%0,0,%1\n"			      \
		        "	cmpw	%0,%2\n"			      \
		        "	bne	2f\n"				      \
		        "	stwcx.	%3,0,%1\n"			      \
		        "	bne-	1b\n"				      \
		        "2:	" __ARCH_ACQ_INSTR			      \
		        : "=&r" (__tmp)					      \
		        : "b" (__memp), "r" (oldval), "r" (newval)	      \
		        : "cr0", "memory");				      \
      __tmp;								      \
  })

#define __arch_compare_and_exchange_val_32_rel(mem, newval, oldval)	      \
  ({									      \
      __typeof (*(mem)) __tmp;						      \
      __typeof (mem)  __memp = (mem);					      \
      __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		        "1:	lwarx	%0,0,%1\n"			      \
		        "	cmpw	%0,%2\n"			      \
		        "	bne	2f\n"				      \
		        "	stwcx.	%3,0,%1\n"			      \
		        "	bne-	1b\n"				      \
		        "2:	"					      \
		        : "=&r" (__tmp)					      \
		        : "b" (__memp), "r" (oldval), "r" (newval)	      \
		        : "cr0", "memory");				      \
      __tmp;								      \
  })

#define __arch_atomic_exchange_32_acq(mem, value)			      \
  ({									      \
    __typeof (*mem) __val;						      \
    __asm__ __volatile__ (							      \
		      "1:	lwarx	%0,0,%2\n"			      \
		      "		stwcx.	%3,0,%2\n"			      \
		      "		bne-	1b\n"				      \
		      "   " __ARCH_ACQ_INSTR				      \
		      : "=&r" (__val), "=m" (*mem)			      \
		      : "b" (mem), "r" (value), "m" (*mem)		      \
		      : "cr0", "memory");				      \
    __val;								      \
  })

#define __arch_atomic_exchange_32_rel(mem, value) \
  ({									      \
    __typeof (*mem) __val;						      \
    __asm__ __volatile__ (__ARCH_REL_INSTR "\n"				      \
		      "1:	lwarx	%0,0,%2\n"			      \
		      "		stwcx.	%3,0,%2\n"			      \
		      "		bne-	1b"				      \
		      : "=&r" (__val), "=m" (*mem)			      \
		      : "b" (mem), "r" (value), "m" (*mem)		      \
		      : "cr0", "memory");				      \
    __val;								      \
  })

#define __arch_atomic_exchange_and_add_32(mem, value) \
  ({									      \
    __typeof (*mem) __val, __tmp;					      \
    __asm__ __volatile__ ("1:	lwarx	%0,0,%3\n"			      \
		      "		add	%1,%0,%4\n"			      \
		      "		stwcx.	%1,0,%3\n"			      \
		      "		bne-	1b"				      \
		      : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		      : "b" (mem), "r" (value), "m" (*mem)		      \
		      : "cr0", "memory");				      \
    __val;								      \
  })

#define __arch_atomic_increment_val_32(mem) \
  ({									      \
    __typeof (*(mem)) __val;						      \
    __asm__ __volatile__ ("1:	lwarx	%0,0,%2\n"			      \
		      "		addi	%0,%0,1\n"			      \
		      "		stwcx.	%0,0,%2\n"			      \
		      "		bne-	1b"				      \
		      : "=&b" (__val), "=m" (*mem)			      \
		      : "b" (mem), "m" (*mem)				      \
		      : "cr0", "memory");				      \
    __val;								      \
  })

#define __arch_atomic_decrement_val_32(mem) \
  ({									      \
    __typeof (*(mem)) __val;						      \
    __asm__ __volatile__ ("1:	lwarx	%0,0,%2\n"			      \
		      "		subi	%0,%0,1\n"			      \
		      "		stwcx.	%0,0,%2\n"			      \
		      "		bne-	1b"				      \
		      : "=&b" (__val), "=m" (*mem)			      \
		      : "b" (mem), "m" (*mem)				      \
		      : "cr0", "memory");				      \
    __val;								      \
  })

#define __arch_atomic_decrement_if_positive_32(mem) \
  ({ int __val, __tmp;							      \
     __asm__ __volatile__ ("1:	lwarx	%0,0,%3\n"			      \
		       "	cmpwi	0,%0,0\n"			      \
		       "	addi	%1,%0,-1\n"			      \
		       "	ble	2f\n"				      \
		       "	stwcx.	%1,0,%3\n"			      \
		       "	bne-	1b\n"				      \
		       "2:	" __ARCH_ACQ_INSTR			      \
		       : "=&b" (__val), "=&r" (__tmp), "=m" (*mem)	      \
		       : "b" (mem), "m" (*mem)				      \
		       : "cr0", "memory");				      \
     __val;								      \
  })

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_compare_and_exchange_val_32_acq(mem, newval, oldval); \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_compare_and_exchange_val_64_acq(mem, newval, oldval); \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_compare_and_exchange_val_rel(mem, newval, oldval) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_compare_and_exchange_val_32_rel(mem, newval, oldval); \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_compare_and_exchange_val_64_rel(mem, newval, oldval); \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_exchange_acq(mem, value) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_exchange_32_acq (mem, value);		      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_exchange_64_acq (mem, value);		      \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_exchange_rel(mem, value) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_exchange_32_rel (mem, value);		      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_exchange_64_rel (mem, value);		      \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_exchange_and_add(mem, value) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_exchange_and_add_32 (mem, value);	      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_exchange_and_add_64 (mem, value);	      \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_increment_val(mem) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*(mem)) == 4)						      \
      __result = __arch_atomic_increment_val_32 (mem);			      \
    else if (sizeof (*(mem)) == 8)					      \
      __result = __arch_atomic_increment_val_64 (mem);			      \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_increment(mem) ({ atomic_increment_val (mem); (void) 0; })

#define atomic_decrement_val(mem) \
  ({									      \
    __typeof (*(mem)) __result;						      \
    if (sizeof (*(mem)) == 4)						      \
      __result = __arch_atomic_decrement_val_32 (mem);			      \
    else if (sizeof (*(mem)) == 8)					      \
      __result = __arch_atomic_decrement_val_64 (mem);			      \
    else								      \
       abort ();							      \
    __result;								      \
  })

#define atomic_decrement(mem) ({ atomic_decrement_val (mem); (void) 0; })


/* Decrement *MEM if it is > 0, and return the old value.  */
#define atomic_decrement_if_positive(mem) \
  ({ __typeof (*(mem)) __result;					      \
    if (sizeof (*mem) == 4)						      \
      __result = __arch_atomic_decrement_if_positive_32 (mem);		      \
    else if (sizeof (*mem) == 8)					      \
      __result = __arch_atomic_decrement_if_positive_64 (mem);		      \
    else								      \
       abort ();							      \
    __result;								      \
  })
