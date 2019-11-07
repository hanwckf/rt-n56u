/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* We don't use most of these symbols; they are here for documentation. */
#define JB_R30  0
#define JB_R31  1
#define JB_R32  2
#define JB_R33  3
#define JB_R34  4
#define JB_R35  5
#define JB_R36  6
#define JB_R37  7
#define JB_R38  8
#define JB_R39  9
#define JB_R40  10
#define JB_R41  11
#define JB_R42  12
#define JB_R43  13
#define JB_R44  14
#define JB_R45  15
#define JB_R46  16
#define JB_R47  17
#define JB_R48  18
#define JB_R49  19
#define JB_R50  20
#define JB_R51  21
#define JB_FP   22  /* r52 */
#define JB_TP   23  /* r53 */
#define JB_SP   24  /* r54 */
#define JB_PC   25  /* normally LR, r55 */
#define JB_ICS  26  /* interrupt critical section bit */

/* We save space for some extra state to accommodate future changes.  */
#define JB_LEN  32  /* number of words */

#define JB_SIZE (JB_LEN * REGSIZE)

/* Helper macro used by all the setjmp/longjmp assembly code. */
#define FOR_EACH_CALLEE_SAVED_REG(f)                              \
  .no_require_canonical_reg_names;                f(r30); f(r31); \
  f(r32); f(r33); f(r34); f(r35); f(r36); f(r37); f(r38); f(r39); \
  f(r40); f(r41); f(r42); f(r43); f(r44); f(r45); f(r46); f(r47); \
  f(r48); f(r49); f(r50); f(r51); f(r52); f(r53); f(r54); f(r55)

/* Helper for generic ____longjmp_chk(). */
#define JB_FRAME_ADDRESS(buf) \
  ((void *) (unsigned long) (buf[JB_SP]))
