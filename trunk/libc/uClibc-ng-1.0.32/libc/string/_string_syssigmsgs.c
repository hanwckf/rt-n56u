/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#ifdef __UCLIBC_HAS_SIGNUM_MESSAGES__

attribute_hidden
const char _string_syssigmsgs[] = {
	/*   0:    0,  1 */ "\0"
	/*   1:    1,  7 */ "Hangup\0"
	/*   2:    8, 10 */ "Interrupt\0"
	/*   3:   18,  5 */ "Quit\0"
	/*   4:   23, 20 */ "Illegal instruction\0"
	/*   5:   43, 22 */ "Trace/breakpoint trap\0"
	/*   6:   65,  8 */ "Aborted\0"
	/*   7:   73, 10 */ "Bus error\0"
	/*   8:   83, 25 */ "Floating point exception\0"
	/*   9:  108,  7 */ "Killed\0"
	/*  10:  115, 22 */ "User defined signal 1\0"
	/*  11:  137, 19 */ "Segmentation fault\0"
	/*  12:  156, 22 */ "User defined signal 2\0"
	/*  13:  178, 12 */ "Broken pipe\0"
	/*  14:  190, 12 */ "Alarm clock\0"
	/*  15:  202, 11 */ "Terminated\0"
	/*  16:  213, 12 */ "Stack fault\0"
	/*  17:  225, 13 */ "Child exited\0"
	/*  18:  238, 10 */ "Continued\0"
	/*  19:  248, 17 */ "Stopped (signal)\0"
	/*  20:  265,  8 */ "Stopped\0"
	/*  21:  273, 20 */ "Stopped (tty input)\0"
	/*  22:  293, 21 */ "Stopped (tty output)\0"
	/*  23:  314, 21 */ "Urgent I/O condition\0"
	/*  24:  335, 24 */ "CPU time limit exceeded\0"
	/*  25:  359, 25 */ "File size limit exceeded\0"
	/*  26:  384, 22 */ "Virtual timer expired\0"
	/*  27:  406, 24 */ "Profiling timer expired\0"
	/*  28:  430, 15 */ "Window changed\0"
	/*  29:  445, 13 */ "I/O possible\0"
	/*  30:  458, 14 */ "Power failure\0"
	/*  31:  472, 16 */ "Bad system call"
#if defined SIGEMT
	/*  32:  488,  9 */ "\0EMT trap"
#endif
};

#endif
