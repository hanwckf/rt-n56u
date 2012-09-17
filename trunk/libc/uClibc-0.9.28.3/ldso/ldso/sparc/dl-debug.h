/* vi: set sw=4 ts=4: */
/* sparc ELF shared library loader suppport
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *				David Engel, Hongjiu Lu and Mitch D'Souza
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

static const char * _dl_reltypes_tab[] = {
	"R_SPARC_NONE", "R_SPARC_8",
	"R_SPARC_16", "R_SPARC_32", "R_SPARC_DISP8", "R_SPARC_DISP16",
	"R_SPARC_DISP32", "R_SPARC_WDISP30", "R_SPARC_WDISP22",
	"R_SPARC_HI22", "R_SPARC_22", "R_SPARC_13", "R_SPARC_LO10",
	"R_SPARC_GOT10", "R_SPARC_GOT13", "R_SPARC_GOT22", "R_SPARC_PC10",
	"R_SPARC_PC22", "R_SPARC_WPLT30", "R_SPARC_COPY",
	"R_SPARC_GLOB_DAT", "R_SPARC_JMP_SLOT", "R_SPARC_RELATIVE",
	"R_SPARC_UA32"
};
