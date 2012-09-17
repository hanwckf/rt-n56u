/* vi: set sw=4 ts=4: */
/* m68k ELF shared library loader suppport
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *                         David Engel, Hongjiu Lu and Mitch D'Souza
 * Adapted to ELF/68k by Andreas Schwab.
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

static const char *_dl_reltypes_tab[] = {
	 [0] "R_68K_NONE",
	 [1] "R_68K_32",     "R_68K_16",       "R_68K_8",
	 [4] "R_68K_PC32",   "R_68K_PC16",     "R_68K_PC8",
	 [7] "R_68K_GOT32",  "R_68K_GOT16",    "R_68K_GOT8",
	[10] "R_68K_GOT32O", "R_68K_GOT16O",   "R_68K_GOT8O",
	[13] "R_68K_PLT32",  "R_68K_PLT16",    "R_68K_PLT8",
	[16] "R_68K_PLT32O", "R_68K_PLT16O",   "R_68K_PLT8O",
	[19] "R_68K_COPY",   "R_68K_GLOB_DAT", "R_68K_JMP_SLOT", "R_68K_RELATIVE",
	[23] "R_68K_NUM"
};
