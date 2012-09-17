/* vi: set sw=4 ts=4: */
/* ARM ELF shared library loader suppport
 *
 * Copyright (C) 2001-2004 Erik Andersen
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const char * const _dl_reltypes_tab[] =
{
  [0]	"R_ARM_NONE",	    "R_ARM_PC24",	"R_ARM_ABS32",		"R_ARM_REL32",
  [4]	"R_ARM_PC13",	    "R_ARM_ABS16",	"R_ARM_ABS12",		"R_ARM_THM_ABS5",
  [8]	"R_ARM_ABS8",		"R_ARM_SBREL32","R_ARM_THM_PC22",	"R_ARM_THM_PC8",
  [12]	"R_ARM_AMP_VCALL9",	"R_ARM_SWI24",	"R_ARM_THM_SWI8",	"R_ARM_XPC25",
  [16]	"R_ARM_THM_XPC22",  "R_ARM_TLS_DTPMOD32",   "R_ARM_TLS_DTPOFF32",   "R_ARM_TLS_TPOFF32",
  [20]	"R_ARM_COPY",		"R_ARM_GLOB_DAT","R_ARM_JUMP_SLOT",	"R_ARM_RELATIVE",
  [24]	"R_ARM_GOTOFF",		"R_ARM_GOTPC",	 "R_ARM_GOT32",		"R_ARM_PLT32",
  [32]	"R_ARM_ALU_PCREL_7_0","R_ARM_ALU_PCREL_15_8","R_ARM_ALU_PCREL_23_15","R_ARM_LDR_SBREL_11_0",
  [36]	"R_ARM_ALU_SBREL_19_12","R_ARM_ALU_SBREL_27_20",
  [100]	"R_ARM_GNU_VTENTRY","R_ARM_GNU_VTINHERIT","R_ARM_THM_PC11","R_ARM_THM_PC9",
  [104] "R_ARM_TLS_GD32","R_ARM_TLS_LDM32","R_ARM_TLS_LDO32","R_ARM_TLS_IE32",
  [108] "R_ARM_TLS_LE32","R_ARM_TLS_LDO12","R_ARM_TLS_LE12","R_ARM_TLS_IE12GP",
  [249] "R_ARM_RXPC25", "R_ARM_RSBREL32", "R_ARM_THM_RPC22", "R_ARM_RREL32",
  [253] "R_ARM_RABS22", "R_ARM_RPC24", "R_ARM_RBASE",
};
