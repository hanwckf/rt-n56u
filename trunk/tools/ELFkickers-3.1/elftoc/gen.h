/* gen.h: Generic functions and definitions used throughout.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_gen_h_
#define	_gen_h_

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/* These functions display error message. err() always returns zero;
 * fail() exits the program. warn() adds the text "warning:" to the
 * message, and prints nothing if warnings are disabled.
 */
extern int warn(char const *fmt, ...);
extern int err(char const *fmt, ...);
extern void fail(char const *fmt, ...);

/* These functions set a program name and a file name, respectively,
 * to prefix error messages.
 */
extern void setprogramname(char const *str);
extern void setfilename(char const *str);

/* This function turns the display of warnings on or off.
 */
extern void enablewarnings(int flag);

/* Memory allocation functions. These functions either succeed or exit
 * the program.
 */
extern void *allocate(unsigned int size);
extern void *reallocate(void *ptr, unsigned int size);
extern char *strallocate(char const *str);
extern void deallocate(void *ptr);

#endif
