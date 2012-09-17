/* vi: set sw=4 ts=4: */
/*
 * This file contains the helper routines to run init and fini functions.
 *
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@codepoet.org>
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *				David Engel, Hongjiu Lu and Mitch D'Souza
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


#include "ldso.h"

static void _dl_run_array_forward(unsigned long array, unsigned long size,
                                  DL_LOADADDR_TYPE loadaddr)
{
	if (array != 0) {
		unsigned int j;
		unsigned int jm;
		ElfW(Addr) *addrs;
		jm = size / sizeof (ElfW(Addr));
		addrs = (ElfW(Addr) *) DL_RELOC_ADDR(loadaddr, array);
		for (j = 0; j < jm; ++j) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) addrs[j];
			DL_CALL_FUNC_AT_ADDR (dl_elf_func, loadaddr, (void (*)(void)));
		}
	}
}

void _dl_run_init_array(struct elf_resolve *tpnt);
void _dl_run_init_array(struct elf_resolve *tpnt)
{
	_dl_run_array_forward(tpnt->dynamic_info[DT_INIT_ARRAY],
			      tpnt->dynamic_info[DT_INIT_ARRAYSZ],
			      tpnt->loadaddr);
}

void _dl_app_init_array(void);
void _dl_app_init_array(void)
{
	_dl_run_init_array(_dl_loaded_modules);
}

void _dl_run_fini_array(struct elf_resolve *tpnt);
void _dl_run_fini_array(struct elf_resolve *tpnt)
{
	if (tpnt->dynamic_info[DT_FINI_ARRAY]) {
		ElfW(Addr) *array = (ElfW(Addr) *) DL_RELOC_ADDR(tpnt->loadaddr, tpnt->dynamic_info[DT_FINI_ARRAY]);
		unsigned int i = (tpnt->dynamic_info[DT_FINI_ARRAYSZ] / sizeof(ElfW(Addr)));
		while (i-- > 0) {
			void (*dl_elf_func) (void);
			dl_elf_func = (void (*)(void)) (intptr_t) array[i];
			DL_CALL_FUNC_AT_ADDR (dl_elf_func, tpnt->loadaddr, (void (*)(void)));
		}
	}
}

void _dl_app_fini_array(void);
void _dl_app_fini_array(void)
{
	_dl_run_fini_array(_dl_loaded_modules);
}

