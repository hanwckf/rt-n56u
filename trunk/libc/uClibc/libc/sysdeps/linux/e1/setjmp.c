/*  This file is lisenced under LGPL
 *  Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
 *                              Yannis Mitsos <yannis.mitsos@gdt.gr>
 */

#include <setjmp.h>
#include <stdio.h>
#include <signal.h>

int setjmp( jmp_buf state)
{
	asm volatile(	"mov %0, G3\n\t"           
			"mov %1, G4\n\t" 
			:"=l"(state->__jmpbuf->G3), 
			 "=l"(state->__jmpbuf->G4) 
			:/*no input*/ 
			:"%G3", "%G4" );

	asm volatile(   "setadr  %0\n\t"
			"mov %1, L1\n\t"
			"mov %2, L2\n\t"
			:"=l"(state->__jmpbuf->SavedSP),
			 "=l"(state->__jmpbuf->SavedPC),
			 "=l"(state->__jmpbuf->SavedSR)
			:/*no input*/);
	return 0;
}

int sigsetjmp( sigjmp_buf state , int savesigs)
{

	if(savesigs) {
		state->__mask_was_saved = 1;
		/* how arg in <sigprocmask> is not significant */
		sigprocmask(SIG_SETMASK, NULL, &state->__saved_mask);
	} else
		state->__mask_was_saved = 0;

	asm volatile(	"mov %0, G3\n\t"           
			"mov %1, G4\n\t" 
			:"=l"(state->__jmpbuf->G3), 
			 "=l"(state->__jmpbuf->G4) 
			:/*no input*/ 
			:"%G3", "%G4" );

	asm volatile(   "setadr  %0\n\t"
			"mov %1, L2\n\t"
			"mov %2, L3\n\t"
			:"=l"(state->__jmpbuf->SavedSP),
			 "=l"(state->__jmpbuf->SavedPC),
			 "=l"(state->__jmpbuf->SavedSR)
			:/*no input*/);
	return 0;
}
