/* Atomic instructions for userspace.
 *
 * The actual implementations can be found in the kernel.
 *
 * Copyright (c) 2008 Analog Devices, Inc.
 *
 * Licensed under the LGPL v2.1.
 */

#ifndef __BFIN_FIXED_CODE_H__
#define __BFIN_FIXED_CODE_H__

#include <stdint.h>

#include <asm/fixed_code.h>

#ifndef __ASSEMBLY__

static inline
uint32_t bfin_atomic_xchg32(uint32_t *__bfin_ptr, uint32_t __bfin_newval)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R1: value to store
	 * Output:   R0: old contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_XCHG32), "q1" (__bfin_newval),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "RETS", "memory"
	);
	return __bfin_ret;
}

static inline
uint32_t bfin_atomic_cas32(uint32_t *__bfin_ptr, uint32_t __bfin_exp, uint32_t __bfin_newval)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R1: compare value
	 *           R2: new value to store
	 * Output:   R0: old contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_CAS32), "q1" (__bfin_exp), "q2" (__bfin_newval),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "RETS", "memory"
	);
	return __bfin_ret;
}

static inline
uint32_t bfin_atomic_add32(uint32_t *__bfin_ptr, uint32_t __bfin_inc)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R0: value to add
	 * Output:   R0: new contents of the memory address
	 *           R1: previous contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_ADD32), "q0" (__bfin_inc),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "R1", "RETS", "memory"
	);
	return __bfin_ret;
}
#define bfin_atomic_inc32(ptr) bfin_atomic_add32(ptr, 1)

static inline
uint32_t bfin_atomic_sub32(uint32_t *__bfin_ptr, uint32_t __bfin_dec)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R0: value to subtract
	 * Output:   R0: new contents of the memory address
	 *           R1: previous contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_SUB32), "q0" (__bfin_dec),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "R1", "RETS", "memory"
	);
	return __bfin_ret;
}
#define bfin_atomic_dec32(ptr)        bfin_atomic_sub32(ptr, 1)

static inline
uint32_t bfin_atomic_ior32(uint32_t *__bfin_ptr, uint32_t __bfin_ior)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R0: value to ior
	 * Output:   R0: new contents of the memory address
	 *           R1: previous contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_IOR32), "q0" (__bfin_ior),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "R1", "RETS", "memory"
	);
	return __bfin_ret;
}

static inline
uint32_t bfin_atomic_and32(uint32_t *__bfin_ptr, uint32_t __bfin_and)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R0: value to and
	 * Output:   R0: new contents of the memory address
	 *           R1: previous contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_AND32), "q0" (__bfin_and),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "R1", "RETS", "memory"
	);
	return __bfin_ret;
}

static inline
uint32_t bfin_atomic_xor32(uint32_t *__bfin_ptr, uint32_t __bfin_xor)
{
	uint32_t __bfin_ret;
	/* Input:    P0: memory address to use
	 *           R0: value to xor
	 * Output:   R0: new contents of the memory address
	 *           R1: previous contents of the memory address
	 */
	__asm__ __volatile__(
		"CALL (%[__bfin_func])"
		: "=q0" (__bfin_ret), "=m" (*__bfin_ptr)
		: [__bfin_func] "a" (ATOMIC_XOR32), "q0" (__bfin_xor),
		  "qA" (__bfin_ptr), "m" (*__bfin_ptr)
		: "R1", "RETS", "memory"
	);
	return __bfin_ret;
}

#endif

#endif
