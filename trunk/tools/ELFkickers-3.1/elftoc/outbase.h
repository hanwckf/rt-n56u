/* outbase.c: The lowest-level output functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _outbase_h_
#define _outbase_h_

/* Set the current output file. Call this before calling any output
 * functions. (The FILE* argument is given as a void* so as to not
 * include stdio.h here.)
 */
extern void setoutputfile(void *file);

/* Set the current output line width.
 */
extern void setoutputwidth(int width);
extern void setoutputindent(int width);

/* Begins a C block. If brk is true, the opening brace will be placed
 * on a separate line from the block's contents.
 */
extern void beginblock(int brk);

/* Closes the current C block.
 */
extern void endblock(void);

/* Adds a line break to the output.
 */
extern void linebreak(void);

/* Outputs a string representing a single element. If the output is
 * within a block, the string will be separated from its siblings by
 * commas.
 */
extern void out(char const *str);

/* Outputs text as a C comment. If output is currently inside a block,
 * then the comment will be separated from its left sibling by a
 * comma.
 */
extern void outcomment(char const *str);

/* Outputs an arbitrary string in the form of a C string literal, as a
 * single element. Long strings are broken up so as to not exceed the
 * line width. The string does not have to be NUL-terminated. However,
 * the function does assume that it is safe to examine the byte at
 * str[length].
 */
extern void outstring(char const *str, long length);

/* Returns the number of output characters a string will require.
 */
extern long outstringsize(char const *str, long length);

#endif
