#ifndef _BITS_SIGACTION_STRUCT_H
#define _BITS_SIGACTION_STRUCT_H

/* We have a separate header file here because we do not support
   SA_RESTORER on hppa. */

/* This is the sigaction struction from the Linux 2.1.20 kernel.  */
/* Blah.  This is bogus.  We don't ever use it. */
struct old_kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_mask;
	unsigned long sa_flags;
};

#endif
