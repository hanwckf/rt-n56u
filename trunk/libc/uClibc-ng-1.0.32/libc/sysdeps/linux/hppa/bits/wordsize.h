/* Determine the wordsize from the preprocessor defines.  */

#ifdef __LP64__
# define __WORDSIZE	64
#else
# define __WORDSIZE	32
#endif
