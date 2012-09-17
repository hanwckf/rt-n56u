/* vi: set sw=4 ts=4: */
/* microblaze shared library loader suppport
 *
 * Copyright (C) 2011 Ryan Flux
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
		"R_MICROBLAZE_NONE",
		"R_MICROBLAZE_32",
		"R_MICROBLAZE_32_PCREL",
		"R_MICROBLAZE_64_PCREL",
		"R_MICROBLAZE_32_PCREL_LO",
		"R_MICROBLAZE_64",
		"R_MICROBLAZE_32_LO",
		"R_MICROBLAZE_SRO32",
		"R_MICROBLAZE_SRW32",
		"R_MICROBLAZE_64_NONE",
		"R_MICROBLAZE_32_SYM_OP_SYM",
		"R_MICROBLAZE_GNU_VTINHERIT",
		"R_MICROBLAZE_GNU_VTENTRY",
		"R_MICROBLAZE_GOTPC_64",
		"R_MICROBLAZE_GOT_64",
		"R_MICROBLAZE_PLT_64",
		"R_MICROBLAZE_REL",
		"R_MICROBLAZE_JUMP_SLOT",
		"R_MICROBLAZE_GLOB_DAT",
		"R_MICROBLAZE_GOTOFF_64",
		"R_MICROBLAZE_GOTOFF_32",
		"R_MICROBLAZE_COPY",
	};
