/*  This file is lisenced under LGPL
 *  Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
 *                              Yannis Mitsos <yannis.mitsos@gdt.gr>
 */

#include <syscall.h>
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>

#define __NR_e1newSP  224
static inline _syscall1(int, e1newSP, unsigned long, SavedSP )

unsigned long jmpbuf_ptr;

void longjmp(jmp_buf state, int value )
{
	if(!value)
		state->__jmpbuf->ReturnValue = 1;
	else
		state->__jmpbuf->ReturnValue = value;

	jmpbuf_ptr = (unsigned long)state; 
	e1newSP(state->__jmpbuf->SavedSP);

#define _state_ ((struct __jmp_buf_tag*)jmpbuf_ptr)
	asm volatile("mov L0, %0\n\t"
		     "mov L1, %1\n\t"
		     "mov L2, %2\n\t"
		     "mov G3, %3\n\t"
		     "mov G4, %4\n\t"
		     "ret PC, L1\n\t"
		     :/*no output*/
		     :"l"(_state_->__jmpbuf->ReturnValue),
		      "l"(_state_->__jmpbuf->SavedPC),
		      "l"(_state_->__jmpbuf->SavedSR),
		      "l"(_state_->__jmpbuf->G3),
		      "l"(_state_->__jmpbuf->G4)
		     :"%G3", "%G4", "%L0", "%L1" );
#undef _state_
}

void siglongjmp(sigjmp_buf state, int value )
{
	if( state->__mask_was_saved )
		sigprocmask(SIG_SETMASK, &state->__saved_mask, NULL);

	if(!value)
		state->__jmpbuf->ReturnValue = 1;
	else
		state->__jmpbuf->ReturnValue = value;

	jmpbuf_ptr = (unsigned long)state; 
	e1newSP(state->__jmpbuf->SavedSP);
	

#define _state_ ((struct __jmp_buf_tag*)jmpbuf_ptr)
	asm volatile("mov L0, %0\n\t"
		     "mov L1, %1\n\t"
		     "mov L2, %2\n\t"
		     "mov G3, %3\n\t"
		     "mov G4, %4\n\t"
		     "ret PC, L1\n\t"
		     :/*no output*/
		     :"l"(_state_->__jmpbuf->ReturnValue),
		      "l"(_state_->__jmpbuf->SavedPC),
		      "l"(_state_->__jmpbuf->SavedSR),
		      "l"(_state_->__jmpbuf->G3),
		      "l"(_state_->__jmpbuf->G4)
		     :"%G3", "%G4", "%L0", "%L1" );
#undef _state_
}
