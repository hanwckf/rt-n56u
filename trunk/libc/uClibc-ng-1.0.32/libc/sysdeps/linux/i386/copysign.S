/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#define _ERRNO_H	1
#include <features.h>
#include <bits/errno.h>

.text
.global copysign
.type   copysign,%function
copysign:
	movl	16(%esp),%edx
	movl	8(%esp),%eax
	andl	$0x80000000,%edx
	andl	$0x7fffffff,%eax
	orl	%edx,%eax
	movl	%eax,8(%esp)
	fldl	4(%esp)
	ret
.size copysign,.-copysign

libc_hidden_def(copysign)

.global copysignf
.type   copysignf,%function
copysignf:
	movl	8(%esp),%edx
	movl	4(%esp),%eax
	andl	$0x80000000,%edx
	andl	$0x7fffffff,%eax
	orl	%edx,%eax
	movl	%eax,4(%esp)
	flds	4(%esp)
	ret
.size copysignf,.-copysignf

libc_hidden_def(copysignf)

.global copysignl
.type   copysignl,%function
copysignl:
	movl	24(%esp),%edx
	movl	12(%esp),%eax
	andl	$0x8000,%edx
	andl	$0x7fff,%eax
	orl	%edx,%eax
	movl	%eax,12(%esp)
	fldt	4(%esp)
	ret
.size copysignl,.-copysignl

libc_hidden_def(copysignl)

