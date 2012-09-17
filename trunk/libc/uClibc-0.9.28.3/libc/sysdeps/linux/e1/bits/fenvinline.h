/* 
   Inline floating-point environment handling functions for Hyperstone e1-32X.
   Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
                               Yannis Mitsos <yannis.mitsos@gdt.gr>

   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#if defined __GNUC__ && !defined _SOFT_FLOAT && !defined __NO_MATH_INLINES

/********************************************************** 
 *  --- A small description of the E1-16/32X FP unit. ---
 * FP exceptions can be enabled and disabled through 
 * <feenableexcept>, <fedisableexcept>.
 *
 * - When an enabled exception takes place a SIGFPE signal
 * is sent to the process by the exception handler. User
 * can test for the exception that took place through
 * <fetestexcept>.
 * feraiseexcept works only for accrued exceptions.
 *
 * - When a disabld exception takes place it does not generate
 * a trap. The user can check if any exception took place after
 * an FP instruction by issuing an <fetestexcept> command.
 * User should first clear the G2 register by issuing an
 * <feclearexcept> function. 
 * The following program is a typical example of how the user
 * should check for exceptions that did not generate a SIGFPE
 * signal :
 * {
 *   double f;
 *   int raised;
 *   feclearexcept (FE_ALL_EXCEPT);
 *   f = compute ();
 *   raised = fetestexcept (FE_OVERFLOW | FE_INVALID);
 *   if (raised & FE_OVERFLOW) {  ...  }
 *   if (raised & FE_INVALID) {  ...  }
 *    ... 
 * }
 ***********************************************************/

/* Get FPU rounding mode  */
#define fegetround()                     \
({                                       \
	unsigned int tmp;                \
	asm volatile("mov %0, SR"        \
			:"=l"(tmp)       \
			:/*no input*/);  \
	tmp &= (3<<13);                  \
	(tmp);                           \
})

/* Set FPU rounding mode  */
#define fesetround(round)                \
({                                       \
	unsigned int tmp = (3 << 13);    \
	while(1) {                       \
	/* Clear SR.FRM field */         \
	asm volatile("andn SR, %0"       \
			:/*no output*/   \
			:"l"(tmp) );     \
	tmp &= round;                    \
                                         \
	if(tmp) {                        \
		tmp = -1;                \
		break;                   \
	}                                \
                                         \
	asm volatile("or SR, %0"         \
			:/*no input*/    \
			:"l"(round) );   \
	tmp = 0;                         \
	break;                           \
	}                                \
	(tmp);                           \
})

/* The following functions test for accrued exceptions.
 * No trap is generated on an FP exception.
 */
static inline feclearexcept(int __excepts)
{
	unsigned int enabled_excepts, disabled_excepts;

	/* Check that __excepts is correctly set */
	if( __excepts & (~0x1F00) )
		return -1;

	asm volatile("mov %0, SR"
		     :"=l"(enabled_excepts)
		     :/*no input*/ ); 

	enabled_excepts  &= 0x1F00;
	disabled_excepts = ~enabled_excepts;
	disabled_excepts &= 0x1F00;

	enabled_excepts  &= __excepts;
	disabled_excepts &= __excepts;

	/* Clear accrued exceptions */
	asm volatile("andn G2, %0\n\t"
		     "andn G2, %1\n\t"
			:/*no output*/
			:"l"(enabled_excepts),
			 "l"(disabled_excepts >> 8) );
	return 0;
}

/* fetestexcepts tests both for actual and accrued
 * excepts. You can test for an exception either after
 * an FP instruction or within a SIGFPE handler
 */ 
inline int fetestexcept(int __excepts)
{	
	unsigned int G2, G2en, G2dis;
	unsigned int enabled_excepts, disabled_excepts;

	/* Check that __excepts is correctly set */
	if( __excepts & (~0x1F00) )
		return -1;

	asm volatile("mov %0, SR"
		     :"=l"(enabled_excepts)
		     :/*no input*/ ); 

	enabled_excepts &= 0x1F00;
	disabled_excepts = ~enabled_excepts;
	disabled_excepts &= 0x1F00;

 	asm volatile("mov %0, G2"
		    :"=l"(G2)
		    :/*no input*/ );

	G2en  = G2 & 0x1F00;
	G2dis = G2 & 0x1F;
	G2en  &= enabled_excepts;
	G2dis &= (disabled_excepts >> 8);
	return ( G2en | (G2dis << 8) );
}

static inline int feraiseexcept(int __excepts)
{
	asm volatile("or G2, %0"
			:/*no output*/
			:"l"( __excepts >> 8  ) );
	return 0;
}

/* The following functions enable/disable individual exceptions.
 * If enabling an exception trap is going to occur, in case of error.
 */
#define feenableexcept(__excepts)          \
({                                         \
	int __retval, __pexcepts;          \
	int __tmpexcepts = __excepts;      \
                                           \
	while(1) {                         \
	    asm volatile("mov %0, SR"      \
		     :"=l"(__pexcepts)     \
		     :/*no input*/ );      \
	    __pexcepts &= 0x1F00;          \
	                                   \
/* Check if __except values are valid */   \
	    if( __tmpexcepts & ~0x1F00 ) { \
	        __retval = -1;             \
		fprintf(stderr,"Non valid excepts\n");\
	        break;                     \
	    }                              \
	                                   \
	    asm volatile("or SR, %0"       \
			:/*no output*/     \
			:"l"(__tmpexcepts) ); \
	    __retval = __pexcepts;         \
	    break;                         \
	}                                  \
	(__retval);                        \
})


#define fedisableexcept(__excepts)         \
({                                         \
	int __retval, __pexcepts;          \
	int __tmpexcepts = __excepts;      \
	                                   \
	while(1) {                         \
	    asm volatile("mov %0, SR"      \
		     :"=l"(__pexcepts)     \
		     :/*no input*/ );      \
	    __pexcepts &= 0x1F00;          \
	                                   \
/* Check if __except values are valid */   \
	    if( __tmpexcepts & ~0x1F00 ) { \
	        __retval = -1;             \
		fprintf(stderr,"Non valid excepts\n");\
	        break;                     \
	    }                              \
	                                   \
	    asm volatile("andn SR, %0"     \
			:/*no output*/     \
			:"l"(__tmpexcepts) ); \
	    __retval = __pexcepts;         \
	    break;                         \
	}                                  \
	(__retval);                        \
})

static inline int fegetexcept(int excepts)
{
	unsigned int tmp;
	asm volatile("mov %0, SR"
		    :"=l"(tmp)
		    :/*no input*/ );
	tmp &= 0x1F00;
	return tmp;
}

static inline int fegetenv(fenv_t *envp)
{
	asm volatile("mov %0, SR\n\t
		      mov %1, SR\n\t
		      mov %2, G2\n\t
		      mov %3, G2\n\t"
		     :"=l"(envp->round_mode),
		      "=l"(envp->trap_enabled),
		      "=l"(envp->accrued_except),
		      "=l"(envp->actual_except)
		     :/*no input*/ );
	envp->round_mode &= (3<<13);
	envp->trap_enabled &= 0x1F00;
	envp->accrued_except &= 0x1F;
	envp->accrued_except <<= 8;
	envp->actual_except &= 0x1F00;
}

#define feholdexcept(envp)        \
(                                         \
	fegetenv(envp);                   \
	fedisableexcept(FE_ALL_EXCEPT);   \
	feclearexcept(FE_ALL_EXCEPT);     \
	(0);                              \
)

#define fesetenv(envp)                \
({                                                  \
	/* Clear FRM & FTE field of SR */           \
	unsigned long clearSR = ( 127<<8 );         \
	asm volatile("andn SR, %0\n\t"              \
		     "or   SR, %1\n\t"              \
		     "or   SR, %2\n\t"              \
		     :/*no output*/                 \
		     :"l"(clearSR),                 \
		      "l"(envp->round_mode),        \
		      "l"(envp->trap_enabled) );    \
	asm volatile("andn G2, 0x1F1F\n\t"          \
		     "or   G2, %0\n\t"              \
		     "or   G2, %1\n\t"              \
		     :/*no output*/                 \
		     :"l"( envp->accrued_except >> 8),\
		     :"l"( envp->actual_except ) ); \
	(0); /* return 0 */                         \
})
		     
#define feupdateenv(envp)                           \
({                                                  \
	/* Clear FRM & FTE field of SR */           \
	asm volatile(/* We dont clear the prev SR*/ \
		     "or   SR, %1\n\t"              \
		     "or   SR, %2\n\t"              \
		     :/*no output*/                 \
		     :"l"(clearSR),                 \
		      "l"(envp->round_mode),        \
		      "l"(envp->accrued_except) );  \
	asm volatile(/* We dont clear the prev SR*/ \
		     "or   G2, %0\n\t"              \
		     "or   G2, %1\n\t"              \
		     :/*no output*/                 \
		     :"l"( envp->accrued_except >> 8),\
		     :"l"( envp->actual_except ) ); \
	(0); /* return 0 */                         \
})
		     

#endif /* __GNUC__ && !_SOFT_FLOAT */

