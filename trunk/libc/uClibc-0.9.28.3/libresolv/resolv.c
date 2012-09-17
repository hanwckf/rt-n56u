/* vi: set sw=4 ts=4: */
/*
 * libresolv for uClibc
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <ctype.h>


/*
 * Pack domain name 'exp_dn' in presentation form into 'comp_dn'.
 * Return the size of the compressed name or -1.
 * 'length' is the size of the array pointed to by 'comp_dn'.
 */
int
dn_comp(const char *src, u_char *dst, int dstsiz,
        u_char **dnptrs, u_char **lastdnptr)
{
        return (ns_name_compress(src, dst, (size_t)dstsiz,
                                 (const u_char **)dnptrs,
                                 (const u_char **)lastdnptr));
}

