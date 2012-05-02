/* Follow glibc's example and use 1024 for CLK_TCK to implement sysconf and
 * clock.c instead of the normal default of 100.
 *
 * WARNING: It is assumed that this is a constant integer value usable in
 * preprocessor conditionals!!!
 */

#define __UCLIBC_CLK_TCK_CONST		1024
