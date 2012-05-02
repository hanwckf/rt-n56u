/* vi: set sw=4 ts=4: */
/*
 * Program to load an ELF binary on a linux system, and run it
 * after resolving ELF shared library symbols
 *
 * Copyright (C) 2005 by Joakim Tjernlund
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
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

/*
 * The main trick with this program is that initially, we ourselves are not
 * dynamicly linked.  This means that we cannot access any global variables or
 * call any functions.  No globals initially, since the Global Offset Table
 * (GOT) is initialized by the linker assuming a virtual address of 0, and no
 * function calls initially since the Procedure Linkage Table (PLT) is not yet
 * initialized.
 *
 * There are additional initial restrictions - we cannot use large switch
 * statements, since the compiler generates tables of addresses and jumps
 * through them.  We cannot use normal syscall stubs, because these all
 * reference the errno global variable which is not yet initialized.  We _can_
 * use all of the local stack variables that we want.  We _can_ use inline
 * functions, because these do not transfer control to a new address, but they
 * must be static so that they are not exported from the modules.
 *
 * Life is further complicated by the fact that initially we do not want to do
 * a complete dynamic linking.  We want to allow the user to supply new
 * functions to override symbols (i.e. weak symbols and/or LD_PRELOAD).  So
 * initially, we only perform relocations for variables that start with "_dl_"
 * since ANSI specifies that the user is not supposed to redefine any of these
 * variables.
 *
 * Fortunately, the linker itself leaves a few clues lying around, and when the
 * kernel starts the image, there are a few further clues.  First of all, there
 * is Auxiliary Vector Table information sitting on which is provided to us by
 * the kernel, and which includes information about the load address that the
 * program interpreter was loaded at, the number of sections, the address the
 * application was loaded at and so forth.  Here this information is stored in
 * the array auxvt.  For details see linux/fs/binfmt_elf.c where it calls
 * NEW_AUX_ENT() a bunch of time....
 *
 * Next, we need to find the GOT.  On most arches there is a register pointing
 * to the GOT, but just in case (and for new ports) I've added some (slow) C
 * code to locate the GOT for you.
 *
 * This code was originally written for SVr4, and there the kernel would load
 * all text pages R/O, so they needed to call mprotect a zillion times to mark
 * all text pages as writable so dynamic linking would succeed.  Then when they
 * were done, they would change the protections for all the pages back again.
 * Well, under Linux everything is loaded writable (since Linux does copy on
 * write anyways) so all the mprotect stuff has been disabled.
 *
 * Initially, we do not have access to _dl_malloc since we can't yet make
 * function calls, so we mmap one page to use as scratch space.  Later on, when
 * we can call _dl_malloc we reuse this this memory.  This is also beneficial,
 * since we do not want to use the same memory pool as malloc anyway - esp if
 * the user redefines malloc to do something funky.
 *
 * Our first task is to perform a minimal linking so that we can call other
 * portions of the dynamic linker.  Once we have done this, we then build the
 * list of modules that the application requires, using LD_LIBRARY_PATH if this
 * is not a suid program (/usr/lib otherwise).  Once this is done, we can do
 * the dynamic linking as required, and we must omit the things we did to get
 * the dynamic linker up and running in the first place.  After we have done
 * this, we just have a few housekeeping chores and we can transfer control to
 * the user's application.
 */

#include "ldso.h"

/* Pull in all the arch specific stuff */
#include "dl-startup.h"

/* Static declarations */
int (*_dl_elf_main) (int, char **, char **);

static void* __rtld_stack_end; /* Points to argc on stack, e.g *((long *)__rtld_stackend) == argc */
strong_alias(__rtld_stack_end, __libc_stack_end) /* Exported version of __rtld_stack_end */

/* When we enter this piece of code, the program stack looks like this:
	argc            argument counter (integer)
	argv[0]         program name (pointer)
	argv[1...N]     program args (pointers)
	argv[argc-1]    end of args (integer)
	NULL
	env[0...N]      environment variables (pointers)
	NULL
	auxvt[0...N]   Auxiliary Vector Table elements (mixed types)
*/
static void * __attribute_used__ _dl_start(unsigned long args)
{
	unsigned int argc;
	char **argv, **envp;
	unsigned long load_addr;
	ElfW(Addr) got;
	unsigned long *aux_dat;
	ElfW(Ehdr) *header;
	struct elf_resolve tpnt_tmp;
	struct elf_resolve *tpnt = &tpnt_tmp;
	ElfW(auxv_t) auxvt[AT_EGID + 1];
	ElfW(Dyn) *dpnt;

	/* WARNING! -- we cannot make _any_ funtion calls until we have
	 * taken care of fixing up our own relocations.  Making static
	 * inline calls is ok, but _no_ function calls.  Not yet
	 * anyways. */

	/* First obtain the information on the stack that tells us more about
	   what binary is loaded, where it is loaded, etc, etc */
	GET_ARGV(aux_dat, args);
	argc = *(aux_dat - 1);
	argv = (char **) aux_dat;
	aux_dat += argc;			/* Skip over the argv pointers */
	aux_dat++;					/* Skip over NULL at end of argv */
	envp = (char **) aux_dat;
#ifndef NO_EARLY_SEND_STDERR
	SEND_STDERR_DEBUG("argc=");
	SEND_NUMBER_STDERR_DEBUG(argc, 0);
	SEND_STDERR_DEBUG(" argv=");
	SEND_ADDRESS_STDERR_DEBUG(argv, 0);
	SEND_STDERR_DEBUG(" envp=");
	SEND_ADDRESS_STDERR_DEBUG(envp, 1);
#endif
	while (*aux_dat)
		aux_dat++;				/* Skip over the envp pointers */
	aux_dat++;					/* Skip over NULL at end of envp */

	/* Place -1 here as a checkpoint.  We later check if it was changed
	 * when we read in the auxvt */
	auxvt[AT_UID].a_type = -1;

	/* The junk on the stack immediately following the environment is
	 * the Auxiliary Vector Table.  Read out the elements of the auxvt,
	 * sort and store them in auxvt for later use. */
	while (*aux_dat) {
		ElfW(auxv_t) *auxv_entry = (ElfW(auxv_t) *) aux_dat;

		if (auxv_entry->a_type <= AT_EGID) {
			_dl_memcpy(&(auxvt[auxv_entry->a_type]), auxv_entry, sizeof(ElfW(auxv_t)));
		}
		aux_dat += 2;
	}

	/* locate the ELF header.   We need this done as soon as possible
	 * (esp since SEND_STDERR() needs this on some platforms... */
	if (!auxvt[AT_BASE].a_un.a_val)
		auxvt[AT_BASE].a_un.a_val = elf_machine_load_address();
	load_addr = auxvt[AT_BASE].a_un.a_val;
	header = (ElfW(Ehdr) *) auxvt[AT_BASE].a_un.a_val;

	/* Check the ELF header to make sure everything looks ok.  */
	if (!header || header->e_ident[EI_CLASS] != ELF_CLASS ||
			header->e_ident[EI_VERSION] != EV_CURRENT
			/* Do not use an inline _dl_strncmp here or some arches
			* will blow chunks, i.e. those that need to relocate all
			* string constants... */
			|| header->e_ident[EI_MAG0] != ELFMAG0
			|| header->e_ident[EI_MAG1] != ELFMAG1
			|| header->e_ident[EI_MAG2] != ELFMAG2
			|| header->e_ident[EI_MAG3] != ELFMAG3)
	{
		SEND_STDERR("Invalid ELF header\n");
		_dl_exit(0);
	}
	SEND_STDERR_DEBUG("ELF header=");
	SEND_ADDRESS_STDERR_DEBUG(load_addr, 1);

	/* Locate the global offset table.  Since this code must be PIC
	 * we can take advantage of the magic offset register, if we
	 * happen to know what that is for this architecture.  If not,
	 * we can always read stuff out of the ELF file to find it... */
	got = elf_machine_dynamic();
	dpnt = (ElfW(Dyn) *) (got + load_addr);
	SEND_STDERR_DEBUG("First Dynamic section entry=");
	SEND_ADDRESS_STDERR_DEBUG(dpnt, 1);
	_dl_memset(tpnt, 0, sizeof(struct elf_resolve));
	tpnt->loadaddr = load_addr;
	/* OK, that was easy.  Next scan the DYNAMIC section of the image.
	   We are only doing ourself right now - we will have to do the rest later */
	SEND_STDERR_DEBUG("Scanning DYNAMIC section\n");
	tpnt->dynamic_addr = dpnt;
#if defined(NO_FUNCS_BEFORE_BOOTSTRAP)
	/* Some architectures cannot call functions here, must inline */
	__dl_parse_dynamic_info(dpnt, tpnt->dynamic_info, NULL, load_addr);
#else
	_dl_parse_dynamic_info(dpnt, tpnt->dynamic_info, NULL, load_addr);
#endif

	SEND_STDERR_DEBUG("Done scanning DYNAMIC section\n");

#if defined(PERFORM_BOOTSTRAP_GOT)

	SEND_STDERR_DEBUG("About to do specific GOT bootstrap\n");
	/* some arches (like MIPS) we have to tweak the GOT before relocations */
	PERFORM_BOOTSTRAP_GOT(tpnt);

#else

	/* OK, now do the relocations.  We do not do a lazy binding here, so
	   that once we are done, we have considerably more flexibility. */
	SEND_STDERR_DEBUG("About to do library loader relocations\n");

	{
		int goof, indx;
#ifdef  ELF_MACHINE_PLTREL_OVERLAP
# define INDX_MAX 1
#else
# define INDX_MAX 2
#endif
		goof = 0;
		for (indx = 0; indx < INDX_MAX; indx++) {
			unsigned int i;
			unsigned long *reloc_addr;
			unsigned long symbol_addr __maybe_unused;
			int symtab_index;
			ElfW(Sym) *sym;
			ELF_RELOC *rpnt;
			unsigned long rel_addr, rel_size;
			ElfW(Word) relative_count = tpnt->dynamic_info[DT_RELCONT_IDX];

			rel_addr = (indx ? tpnt->dynamic_info[DT_JMPREL] :
			                   tpnt->dynamic_info[DT_RELOC_TABLE_ADDR]);
			rel_size = (indx ? tpnt->dynamic_info[DT_PLTRELSZ] :
			                   tpnt->dynamic_info[DT_RELOC_TABLE_SIZE]);

			if (!rel_addr)
				continue;

			/* Now parse the relocation information */
			/* Since ldso is linked with -Bsymbolic, all relocs will be RELATIVE(for those archs that have
			   RELATIVE relocs) which means that the for(..) loop below has nothing to do and can be deleted.
			   Possibly one should add a HAVE_RELATIVE_RELOCS directive and #ifdef away some code. */
			if (!indx && relative_count) {
				rel_size -= relative_count * sizeof(ELF_RELOC);
				elf_machine_relative(load_addr, rel_addr, relative_count);
				rel_addr += relative_count * sizeof(ELF_RELOC);;
			}

			rpnt = (ELF_RELOC *) (rel_addr + load_addr);
			for (i = 0; i < rel_size; i += sizeof(ELF_RELOC), rpnt++) {
				reloc_addr = (unsigned long *) (load_addr + (unsigned long) rpnt->r_offset);
				symtab_index = ELF_R_SYM(rpnt->r_info);
				symbol_addr = 0;
				sym = NULL;
				if (symtab_index) {
					char *strtab;
					ElfW(Sym) *symtab;

					symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
					strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
					sym = &symtab[symtab_index];
					symbol_addr = load_addr + sym->st_value;

					SEND_STDERR_DEBUG("relocating symbol: ");
					SEND_STDERR_DEBUG(strtab + sym->st_name);
					SEND_STDERR_DEBUG("\n");
				} else
					SEND_STDERR_DEBUG("relocating unknown symbol\n");
				/* Use this machine-specific macro to perform the actual relocation.  */
				PERFORM_BOOTSTRAP_RELOC(rpnt, reloc_addr, symbol_addr, load_addr, sym);
			}
		}

		if (goof) {
			_dl_exit(14);
		}
	}
#endif

	/* Wahoo!!! */
	SEND_STDERR_DEBUG("Done relocating ldso; we can now use globals and make function calls!\n");

	/* Now we have done the mandatory linking of some things.  We are now
	   free to start using global variables, since these things have all been
	   fixed up by now.  Still no function calls outside of this library,
	   since the dynamic resolver is not yet ready. */

	__rtld_stack_end = (void *)(argv - 1);

	_dl_get_ready_to_run(tpnt, load_addr, auxvt, envp, argv);


	/* Transfer control to the application.  */
	SEND_STDERR_DEBUG("transfering control to application @ ");
	_dl_elf_main = (int (*)(int, char **, char **)) auxvt[AT_ENTRY].a_un.a_val;
	SEND_ADDRESS_STDERR_DEBUG(_dl_elf_main, 1);

#ifndef START
	return _dl_elf_main;
#else
#warning You need to update your arch ldso code
	START();
#endif
}
