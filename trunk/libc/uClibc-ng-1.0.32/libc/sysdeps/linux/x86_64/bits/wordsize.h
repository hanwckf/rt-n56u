/* Determine the wordsize from the preprocessor defines.  */

#if defined __x86_64__ && !defined __ILP32__
# define __WORDSIZE	64
#else
# define __WORDSIZE	32
#endif

#ifdef __x86_64__
/* This makes /var/run/utmp compatible with 32-bit environment: */
# define __WORDSIZE_TIME64_COMPAT32	1
#endif
