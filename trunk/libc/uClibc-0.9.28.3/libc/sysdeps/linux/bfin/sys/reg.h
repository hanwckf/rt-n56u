/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _SYS_REG_H
#define _SYS_REG_H	1

/* Index into an array of 4 byte integers returned from ptrace for
   location of the users' stored general purpose registers. */

enum
{
  PT_IPEND = 0,
#define PT_IPEND PT_IPEND
  PT_SYSCFG = 4,
#define PT_SYSCFG PT_SYSCFG
  PT_SR = 8,
#define PT_SR PT_SR
  PT_RETE = 12,
#define PT_RETE PT_RETE
  PT_RETN = 16,
#define PT_RETN PT_RETN
  PT_RETX = 20,
#define PT_RETX PT_RETX
  PT_PC = 24,
#define PT_PC PT_PC
  PT_RETS = 28,
#define PT_RETS PT_RETS
  PT_ASTAT = 32,
#define PT_ASTAT PT_ASTAT
  PT_LB1 = 40,
#define PT_LB1 PT_LB1
  PT_LB0 = 44,
#define PT_LB0 PT_LB0
  PT_LT1 = 48,
#define PT_LT1 PT_LT1
  PT_LT0 = 52,
#define PT_LT0 PT_LT0
  PT_LC1 = 56,
#define PT_LC1 PT_LC1
  PT_LC0 = 60,
#define PT_LC0 PT_LC0
  PT_A1W = 64,
#define PT_A1W PT_A1W
  PT_A1X = 68,
#define PT_A1X PT_A1X
  PT_A0W = 72,
#define PT_A0W PT_A0W
  PT_A0X = 76,
#define PT_A0X PT_A0X
  PT_B3 = 80,
#define PT_B# PT_B3
  PT_B2 = 84,
#define PT_B2 PT_B2
  PT_B1 = 88,
#define PT_B1 PT_B1
  PT_B0 = 92,
#define PT_B0 PT_B0
  PT_L3 = 96,
#define PT_L3 PT_L3
  PT_L2 = 100,
#define PT_L2 PT_L2
  PT_L1 = 104,
#define PT_L1 PT_L1
  PT_L0 = 108,
#define PT_L0 PT_L0
  PT_M3 = 112,
#define PT_M3 PT_M3
  PT_M2 = 116,
#define PT_M2 PT_M2
  PT_M1 = 120,
#define PT_M1 PT_M1
  PT_M0 = 124,
#define PT_M0 PT_M0
  PT_I3 = 128,
#define PT_I3 PT_I3
  PT_I2 = 132,
#define PT_I2 PT_I2
  PT_I1 = 136,
#define PT_I1 PT_I1
  PT_I0 = 140,
#define PT_I0 PT_I0
  PT_USP = 144,
#define PT_USP PT_USP
  PT_FP = 148,
#define PT_FP PT_FP
  PT_P5 = 152,
#define PT_P5 PT_P5
  PT_P4 = 156,
#define PT_P4 PT_P4
  PT_P3 = 160,
#define PT_P3 PT_P3
  PT_P2 = 164,
#define PT_P2 PT_P2
  PT_P1 = 168,
#define PT_P1 PT_P1
  PT_P0 = 172,
#define PT_P0 PT_P0
  PT_R7 = 176,
#define PT_R7 PT_R7
  PT_R6 = 180,
#define PT_R6 PT_R6
  PT_R5 = 184,
#define PT_R5 PT_R5
  PT_R4 = 188,
#define PT_R4 PT_R4
  PT_R3 = 192,
#define PT_R3 PT_R3
  PT_R2 = 196,
#define PT_R2 PT_R2
  PT_R1 = 200,
#define PT_R1 PT_R1
  PT_R0 = 204,
#define PT_R0 PT_R0
  PT_ORIG_R0 = 208,
#define PT_ORIG_R0 PT_ORIG_R0
};

#endif	/* _SYS_REG_H */
