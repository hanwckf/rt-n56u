#if __GNUC_PREREQ (4, 1)
#warning !!! gcc 4.1 and later have problems with __always_inline so redefined as inline
# ifdef __always_inline
# undef __always_inline
# define __always_inline __inline__
# endif
#endif
