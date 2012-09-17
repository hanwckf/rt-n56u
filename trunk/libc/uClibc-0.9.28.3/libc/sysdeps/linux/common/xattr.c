/* 
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /home/cvsroot/RT288x_SDK/source/lib/libc/sysdeps/linux/common/xattr.c,v 1.1.1.1 2007-01-09 06:46:10 steven Exp $
 *
 * This file provides the following Extended Attribute system calls to uClibc.
 *
 *	setxattr(), lsetxattr(), fsetxattr(),
 *	getxattr(), lgetxattr(), fgetxattr(),
 *	listxattr(), llistxattr(), flistxattr(),
 *	removexattr(), lremovexattr(), fremovexattr()
 *
 * Dec 2004 - <solar@gentoo.org>
 */

/* Taken from the manpage.
 * On success, a positive number is returned indicating the size of the
 * extended attribute name list. On failure, -1 is returned and errno
 * is set appropriately. If extended attributes are not supported by the
 * filesystem, or are disabled, errno is set to ENOSYS.
 */

#include "syscalls.h"
#include <unistd.h>

/* sets */
#ifdef __NR_setxattr
_syscall5(int, setxattr, const char *, path, const char *, name,
	const void *, value, size_t, size, int, flags);
#else
int setxattr(__const char *__path, __const char *__name,
	__const void *__value, size_t __size, int __flags)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_lsetxattr
_syscall5(int, lsetxattr, const char *, path, const char *, name,
	const void *, value, size_t, size, int, flags);
#else
int lsetxattr(__const char *__path, __const char *__name,
	__const void *__value, size_t __size, int __flags)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_fsetxattr
_syscall5(int, fsetxattr, int, filedes, const char *, name, const void *,
	value, size_t, size, int, flags);
#else
int fsetxattr(int __fd, __const char *__name, __const void *__value,
	size_t __size, int __flags)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

/* gets */
#ifdef __NR_getxattr
_syscall4(ssize_t, getxattr, const char *, path, const char *, name,
	void *, value, size_t, size);
#else
ssize_t getxattr(__const char *__path, __const char *__name, void *__value,
	size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_lgetxattr
_syscall4(ssize_t, lgetxattr, const char *, path, const char *, name,
	void *, value, size_t, size);
#else
ssize_t lgetxattr(__const char *__path, __const char *__name,
	void *__value, size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_fgetxattr
_syscall4(ssize_t, fgetxattr, int, filedes, const char *, name, void *,
	value, size_t, size);
#else
ssize_t fgetxattr(int __fd, __const char *__name, void *__value,
	size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

/* list */
#ifdef __NR_listxattr
_syscall3(ssize_t, listxattr, const char *, path, char *, list, size_t,
	size);
#else
ssize_t listxattr(__const char *__path, char *__list, size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_llistxattr
_syscall3(ssize_t, llistxattr, const char *, path, char *, list, size_t,
	size);
#else
ssize_t llistxattr(__const char *__path, char *__list, size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_flistxattr
_syscall3(size_t, flistxattr, int, filedes, char *, list, size_t, size);
#else
ssize_t flistxattr(int __fd, char *__list, size_t __size)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

/* remove */
#ifdef __NR_removexattr
_syscall2(int, removexattr, const char *, path, const char *, name);
#else
int removexattr(__const char *__path, __const char *__name)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_lremovexattr
_syscall2(int, lremovexattr, const char *, path, const char *, name);
#else
int lremovexattr(__const char *__path, __const char *__name)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif

#ifdef __NR_fremovexattr
_syscall2(int, fremovexattr, int, filedes, const char *, name);
#else
int fremovexattr(int __fd, __const char *__name)
{
    __set_errno(ENOSYS);
    return -1;
}
#endif
