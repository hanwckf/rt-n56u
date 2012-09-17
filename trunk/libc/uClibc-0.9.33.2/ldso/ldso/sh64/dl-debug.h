/* vi: set sw=8 ts=8: */
/*
 * ldso/ldso/sh64/elfinterp.c
 *
 * SuperH (sh64) ELF shared library loader suppport
 *
 * Copyright (C) 2003, 2004, 2005  Paul Mundt <lethal@linux-sh.org>
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

static const char * const _dl_reltypes_tab[] = {
	/* SHcompact relocs */
	  [0] =	"R_SH_NONE",		"R_SH_DIR32",
		"R_SH_REL32",		"R_SH_DIR8WPN",
	  [4] = "R_SH_IND12W",		"R_SH_DIR8WPL",
		"R_SH_DIR8WPZ",		"R_SH_DIR8BP",
	  [8] = "R_SH_DIR8W",		"R_SH_DIR8L",
	 [25] = "R_SH_SWITCH16",	"R_SH_SWITCH32",
		"R_SH_USES",		"R_SH_COUNT",
	 [29] = "R_SH_ALIGN",		"R_SH_CODE",
		"R_SH_DATA",		"R_SH_LABEL",
	 [33] = "R_SH_SWITCH8",		"R_SH_GNU_VTINHERIT",
		"R_SH_GNU_VTENTRY",
	[160] = "R_SH_GOT32",		"R_SH_PLT32",
		"R_SH_COPY",		"R_SH_GLOB_DAT",
	[164] = "R_SH_JMP_SLOT",	"R_SH_RELATIVE",
		"R_SH_GOTOFF",		"R_SH_GOTPC",

	/* SHmedia relocs */
	 [45] = "R_SH_DIR5U",		"R_SH_DIR6U",
		"R_SH_DIR6S",		"R_SH_DIR10S",
	 [49] = "R_SH_DIR10SW",		"R_SH_DIR10SL",
		"R_SH_DIR10SQ",
	[169] = "R_SH_GOT_LOW16",	"R_SH_GOT_MEDLOW16",
		"R_SH_GOT_MEDHI16",	"R_SH_GOT_HI16",
	[173] = "R_SH_GOTPLT_LOW16",	"R_SH_GOTPLT_MEDLOW16",
		"R_SH_GOTPLT_MEDHI16",	"R_SH_GOTPLT_HI16",
	[177] = "R_SH_PLT_LOW16",	"R_SH_PLT_MEDLOW16",
		"R_SH_PLT_MEDHI16",	"R_SH_PLT_HI16",
	[181] = "R_SH_GOTOFF_LOW16",	"R_SH_GOTOFF_MEDLOW16",
		"R_SH_GOTOFF_MEDHI16",	"R_SH_GOTOFF_HI16",
	[185] = "R_SH_GOTPC_LOW16",	"R_SH_GOTPC_MEDLOW16",
		"R_SH_GOTPC_MEDHI16",	"R_SH_GOTPC_HI16",
	[189] = "R_SH_GOT10BY4",	"R_SH_GOTPLT10BY4",
		"R_SH_GOT10BY8",	"R_SH_GOTPLT10BY8",
	[193] = "R_SH_COPY64",		"R_SH_GLOB_DAT64",
		"R_SH_JMP_SLOT64",	"R_SH_RELATIVE64",
	[197] = "R_SH_RELATIVE_LOW16",	"R_SH_RELATIVE_MEDLOW16",
		"R_SH_RELATIVE_MEDHI16","R_SH_RELATIVE_HI16",
	[242] = "R_SH_SHMEDIA_CODE",	"R_SH_PT_16",
		"R_SH_IMMS16",		"R_SH_IMMU16",
	[246] = "R_SH_IMM_LOW16",	"R_SH_IMM_LOW16_PCREL",
		"R_SH_IMM_MEDLOW16",	"R_SH_IMM_MEDLOW16_PCREL",
	[250] = "R_SH_IMM_MEDHI16",	"R_SH_IMM_MEDHI16_PCREL",
		"R_SH_IMM_HI16",	"R_SH_IMM_HI16_PCREL",
	[254] = "R_SH_64",		"R_SH_64_PCREL",
};
