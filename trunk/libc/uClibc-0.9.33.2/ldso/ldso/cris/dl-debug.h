/*
 * CRIS ELF shared library loader support.
 *
 * Program to load an elf binary on a linux system, and run it.
 * References to symbols in sharable libraries can be resolved
 * by either an ELF sharable library or a linux style of shared
 * library.
 *
 * Copyright (C) 2002-2004, Axis Communications AB
 * All rights reserved
 *
 * Author: Tobias Anderberg, <tobiasa@axis.com>
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
	[0]  "R_CRIS_NONE", "R_CRIS_8", "R_CRIS_16", "R_CRIS_32",
	[4]  "R_CRIS_8_PCREL", "R_CRIS_16_PCREL", "R_CRIS_32_PCREL", "R_CRIS_GNU_VTINHERIT",
	[8]  "R_CRIS_GNU_VTENTRY", "R_CRIS_COPY", "R_CRIS_GLOB_DAT", "R_CRIS_JUMP_SLOT",
	[16] "R_CRIS_RELATIVE", "R_CRIS_16_GOT", "R_CRIS_32_GOT", "R_CRIS_16_GOTPLT",
	[32] "R_CRIS_32_GOTPLT", "R_CRIS_32_GOTREL", "R_CRIS_32_PLT_GOTREL", "R_CRIS_32_PLT_PCREL",
};
