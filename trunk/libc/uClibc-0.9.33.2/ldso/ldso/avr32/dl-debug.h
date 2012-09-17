/*
 * AVR32 ELF shared libary loader support
 *
 * Copyright (C) 2005-2007 Atmel Corporation
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
    "R_AVR32_NONE",
    "R_AVR32_32", "R_AVR32_16", "R_AVR32_8",
    "R_AVR32_32_PCREL", "R_AVR32_16_PCREL", "R_AVR32_8_PCREL",
    "R_AVR32_DIFF32", "R_AVR32_DIFF16", "R_AVR32_DIFF8",
    "R_AVR32_GOT32", "R_AVR32_GOT16", "R_AVR32_GOT8",
    "R_AVR32_21S", "R_AVR32_16U", "R_AVR32_16S", "R_AVR32_8S", "R_AVR32_8S_EXT",
    "R_AVR32_22H_PCREL", "R_AVR32_18W_PCREL", "R_AVR32_16B_PCREL",
    "R_AVR32_16N_PCREL", "R_AVR32_14UW_PCREL", "R_AVR32_11H_PCREL",
    "R_AVR32_10UW_PCREL", "R_AVR32_9H_PCREL", "R_AVR32_9UW_PCREL",
    "R_AVR32_HI16", "R_AVR32_LO16",
    "R_AVR32_GOTPC", "R_AVR32_GOTCALL", "R_AVR32_LDA_GOT",
    "R_AVR32_GOT21S", "R_AVR32_GOT18SW", "R_AVR32_GOT16S", "R_AVR32_GOT7UW",
    "R_AVR32_32_CPENT", "R_AVR32_CPCALL", "R_AVR32_16_CP", "R_AVR32_9W_CP",
    "R_AVR32_RELATIVE", "R_AVR32_GLOB_DAT", "R_AVR32_JMP_SLOT",
    "R_AVR32_ALIGN",
};
