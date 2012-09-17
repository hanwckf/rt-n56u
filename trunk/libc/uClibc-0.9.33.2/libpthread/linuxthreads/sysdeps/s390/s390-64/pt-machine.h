/* Machine-dependent pthreads configuration and inline functions.
   64 bit S/390 version.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _PT_MACHINE_H
#define _PT_MACHINE_H   1

#ifndef PT_EI
# define PT_EI __extern_always_inline
#endif

extern long int testandset (int *spinlock);
extern int __compare_and_swap (long int *p, long int oldval, long int newval);

/* For multiprocessor systems, we want to ensure all memory accesses
   are completed before we reset a lock.  On other systems, we still
   need to make sure that the compiler has flushed everything to memory.  */
#define MEMORY_BARRIER() __asm__ __volatile__ ("bcr 15,0" : : : "memory")

/* Spinlock implementation; required.  */
PT_EI long int
testandset (int *spinlock)
{
  int ret;

  __asm__ __volatile__(
       "    la    1,%1\n"
       "    lhi   0,1\n"
       "    l     %0,%1\n"
       "0:  cs    %0,0,0(1)\n"
       "    jl    0b"
       : "=&d" (ret), "+m" (*spinlock)
       : : "0", "1", "cc");

  return ret;
}


/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#define CURRENT_STACK_FRAME  stack_pointer
register char * stack_pointer __asm__ ("15");

#ifdef USE_TLS
/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF ((pthread_descr) __builtin_thread_pointer ())

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr) __builtin_set_thread_pointer (descr)
#else
/* Return the thread descriptor for the current thread.
   64 bit S/390 uses access register 0 and 1 as "thread register".  */
#define THREAD_SELF  ({                                                       \
  register pthread_descr __self;                                              \
  __asm__ ("   ear  %0,%%a0\n"                                                \
           "   sllg %0,%0,32\n"                                               \
           "   ear  %0,%%a1\n"                                                \
           : "=d" (__self) );                                                 \
  __self;                                                                     \
})

/* Initialize the thread-unique value.  */
#define INIT_THREAD_SELF(descr, nr)  ({                                       \
  __asm__ ("   sar  %%a1,%0\n"                                                \
           "   srlg 0,%0,32\n"                                                \
           "   sar  %%a0,0\n"                                                 \
           : : "d" (descr) : "0" );                                           \
})
#endif

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_GETMEM_NC(descr, member) \
  ((void) sizeof (descr), THREAD_SELF->member)
#define THREAD_SETMEM(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))
#define THREAD_SETMEM_NC(descr, member, value) \
  ((void) sizeof (descr), THREAD_SELF->member = (value))

/* We want the OS to assign stack addresses.  */
#define FLOATING_STACKS 1

/* Maximum size of the stack if the rlimit is unlimited.  */
#define ARCH_STACK_MAX_SIZE     8*1024*1024

/* Compare-and-swap for semaphores. */

#define HAS_COMPARE_AND_SWAP

PT_EI int
__compare_and_swap(long int *p, long int oldval, long int newval)
{
        int retval;

        __asm__ __volatile__(
                "  lgr  0,%2\n"
                "  csg  0,%3,%1\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "+m" (*p)
                : "d" (oldval) , "d" (newval)
                : "cc", "0");
        return retval == 0;
}

#endif /* pt-machine.h */
