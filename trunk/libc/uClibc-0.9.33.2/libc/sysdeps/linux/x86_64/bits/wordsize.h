/* Determine the wordsize from the preprocessor defines.  */

#if defined __x86_64__
# define __WORDSIZE	64
/* This makes /var/run/utmp compatible with 32-bit environment: */
# define __WORDSIZE_COMPAT32	1
#else
# define __WORDSIZE	32
#endif
