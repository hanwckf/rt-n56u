/* vi: set sw=4 ts=4: */
/* x86_64 debug code for ELF shared library loader suppport
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *                              David Engel, Hongjiu Lu and Mitch D'Souza
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

static const char * const _dl_reltypes_tab[] = {
	[ 0] "R_X86_64_NONE",     "R_X86_64_64",       "R_X86_64_PC32",     "R_X86_64_GOT32",
	[ 4] "R_X86_64_PLT32",    "R_X86_64_COPY",     "R_X86_64_GLOB_DAT", "R_X86_64_JUMP_SLOT",
	[ 8] "R_X86_64_RELATIVE", "R_X86_64_GOTPCREL", "R_X86_64_32",       "R_X86_64_32S",
	[12] "R_X86_64_16",       "R_X86_64_PC16",     "R_X86_64_8",        "R_X86_64_PC8",
	[16] "R_X86_64_DTPMOD64", "R_X86_64_DTPOFF64", "R_X86_64_TPOFF64",  "R_X86_64_TLSGD",
	[20] "R_X86_64_TLSLD",    "R_X86_64_DTPOFF32", "R_X86_64_GOTTPOFF", "R_X86_64_TPOFF32"
};
