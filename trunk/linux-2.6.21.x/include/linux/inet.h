/*
 *		Swansea University Computer Society NET3
 *
 *	This work is derived from NET2Debugged, which is in turn derived
 *	from NET2D which was written by:
 * 		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This work was derived from Ross Biro's inspirational work
 *		for the LINUX operating system.  His version numbers were:
 *
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 * 		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 * 		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 * 		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 * 		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *		$Id: inet.h,v 1.1.1.1 2007-05-25 06:50:12 bruce Exp $
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_INET_H
#define _LINUX_INET_H

#ifdef __KERNEL__
#include <linux/types.h>

extern __be32 in_aton(const char *str);
extern int in4_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
extern int in6_pton(const char *src, int srclen, u8 *dst, int delim, const char **end);
#endif
#endif	/* _LINUX_INET_H */
