#ifndef __TEMPNAME_H__
#define __TEMPNAME_H__

#define	__need_size_t
#include <stddef.h>
#include <sys/types.h>

/* Disable support for $TMPDIR */
extern int ___path_search (char *tmpl, size_t tmpl_len, const char *dir,
	        const char *pfx /*, int try_tmpdir */) attribute_hidden;
#define __path_search(tmpl, tmpl_len, dir, pfx, try_tmpdir) ___path_search(tmpl, tmpl_len, dir, pfx)

extern int __gen_tempname (char *__tmpl, int __kind, int flags,
                           int suffixlen, mode_t mode) attribute_hidden;

/* The __kind argument to __gen_tempname may be one of: */
#define __GT_FILE     0       /* create a file */
#define __GT_BIGFILE  1       /* create a file, using open64 */
#define __GT_DIR      2       /* create a directory */
#define __GT_NOCREATE 3       /* just find a name not currently in use */

#endif
