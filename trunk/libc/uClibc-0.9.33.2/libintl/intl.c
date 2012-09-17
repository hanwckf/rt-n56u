/* Copyright (C) 2003     Manuel Novoa III
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 *  Stub version of libintl.
 *
 *  Aug 30, 2003
 *  Add some hidden names to support locale-enabled libstd++.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#undef __OPTIMIZE__
#include <libintl.h>

/**********************************************************************/
#ifdef L_gettext

char *gettext(const char *msgid)
{
	return (char *) msgid;
}

#endif
/**********************************************************************/
#ifdef L_dgettext

char *dgettext(const char *domainname,
				 const char *msgid)
{
	return (char *) msgid;
}

#endif
/**********************************************************************/
#ifdef L_dcgettext

char *dcgettext(const char *domainname,
				  const char *msgid, int category)
{
	return (char *) msgid;
}

#endif
/**********************************************************************/
#ifdef L_ngettext

char *ngettext(const char *msgid1, const char *msgid2,
			   unsigned long int n)
{
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

#endif
/**********************************************************************/
#ifdef L_dngettext

char *dngettext(const char *domainname, const char *msgid1,
				const char *msgid2, unsigned long int n)
{
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

#endif
/**********************************************************************/
#ifdef L_dcngettext

char *dcngettext(const char *domainname, const char *msgid1,
				 const char *msgid2, unsigned long int n,
				 int category)
{
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

#endif
/**********************************************************************/
#ifdef L_textdomain

char *textdomain(const char *domainname)
{
	static const char default_str[] = "messages";

	if (domainname && *domainname && strcmp(domainname, default_str)) {
		__set_errno(EINVAL);
		return NULL;
	}
	return (char *) default_str;
}

#endif
/**********************************************************************/
#ifdef L_bindtextdomain

char *bindtextdomain(const char *domainname, const char *dirname)
{
	static const char dir[] = "/";

	if (!domainname || !*domainname
		|| (dirname
#if 1
			&& ((dirname[0] != '/') || dirname[1])
#else
			&& strcmp(dirname, dir)
#endif
			)
		) {
		__set_errno(EINVAL);
		return NULL;
	}

	return (char *) dir;
}

#endif
/**********************************************************************/
#ifdef L_bind_textdomain_codeset

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
char *bind_textdomain_codeset(const char *domainname, const char *codeset)
{
	if (!domainname || !*domainname || codeset) {
		__set_errno(EINVAL);
	}
	return NULL;
}

#endif
/**********************************************************************/
#ifdef L__nl_expand_alias

/* glibc-ism */

const char *_nl_expand_alias(const char * name);
const char *_nl_expand_alias(const char * name)
{
	return NULL;		 /* uClibc does not support locale aliases. */
}

#endif
/**********************************************************************/
#ifdef L__nl_msg_cat_cntr

/* glibc-ism */

int _nl_msg_cat_cntr = 0;

#endif
/**********************************************************************/
