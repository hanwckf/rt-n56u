/* vi: set sw=4 ts=4: */
/* mips/mipsel ELF shared library loader suppport
 *
   Copyright (C) 2002, Steven J. Hill (sjhill@realitydiluted.com)
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
	[0]		"R_MIPS_NONE",	"R_MIPS_16",	"R_MIPS_32",
	[3]		"R_MIPS_REL32",	"R_MIPS_26",	"R_MIPS_HI16",
	[6]		"R_MIPS_LO16",	"R_MIPS_GPREL16",	"R_MIPS_LITERAL",
	[9]		"R_MIPS_GOT16",	"R_MIPS_PC16",	"R_MIPS_CALL16",
	[12]	"R_MIPS_GPREL32",
	[16]	"R_MIPS_SHIFT5",	"R_MIPS_SHIFT6",	"R_MIPS_64",
	[19]	"R_MIPS_GOT_DISP",	"R_MIPS_GOT_PAGE",	"R_MIPS_GOT_OFST",
	[22]	"R_MIPS_GOT_HI16",	"R_MIPS_GOT_LO16",	"R_MIPS_SUB",
	[25]	"R_MIPS_INSERT_A",	"R_MIPS_INSERT_B",	"R_MIPS_DELETE",
	[28]	"R_MIPS_HIGHER",	"R_MIPS_HIGHEST",	"R_MIPS_CALL_HI16",
	[31]	"R_MIPS_CALL_LO16",	"R_MIPS_SCN_DISP",	"R_MIPS_REL16",
	[34]	"R_MIPS_ADD_IMMEDIATE",	"R_MIPS_PJUMP",	"R_MIPS_RELGOT",
	[37]	"R_MIPS_JALR",
};
