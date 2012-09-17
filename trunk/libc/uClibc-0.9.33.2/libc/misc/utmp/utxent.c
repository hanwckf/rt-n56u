/*
 * utexent.c : Support for accessing user accounting database.
 * Copyright (C) 2010 STMicroelectronics Ltd.
 *
 * Author: Salvatore Cro <salvatore.cro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <features.h>
#include <string.h>
#include <utmpx.h>
#include <utmp.h>

void setutxent(void)
{
	setutent ();
}

void endutxent(void)
{
	endutent ();
}

struct utmpx *getutxent(void)
{
	return (struct utmpx *) getutent ();
}

struct utmpx *getutxid(const struct utmpx *utmp_entry)
{
	return (struct utmpx *) getutid ((const struct utmp *) utmp_entry);
}

struct utmpx *getutxline(const struct utmpx *utmp_entry)
{
	return (struct utmpx *) getutline ((const struct utmp *) utmp_entry);
}

struct utmpx *pututxline (const struct utmpx *utmp_entry)
{
	return (struct utmpx *) pututline ((const struct utmp *) utmp_entry);
}

int utmpxname (const char *new_ut_name)
{
	return utmpname (new_ut_name);
}

void updwtmpx (const char *wtmpx_file, const struct utmpx *utmpx)
{
	updwtmp (wtmpx_file, (const struct utmp *) utmpx);
}

/* Copy the information in UTMPX to UTMP. */
void getutmp (const struct utmpx *utmpx, struct utmp *utmp)
{
#if _HAVE_UT_TYPE - 0
	utmp->ut_type = utmpx->ut_type;
#endif
#if _HAVE_UT_PID - 0
	utmp->ut_pid = utmpx->ut_pid;
#endif
	memcpy (utmp->ut_line, utmpx->ut_line, sizeof (utmp->ut_line));
	memcpy (utmp->ut_user, utmpx->ut_user, sizeof (utmp->ut_user));
#if _HAVE_UT_ID - 0
	memcpy (utmp->ut_id, utmpx->ut_id, sizeof (utmp->ut_id));
#endif
#if _HAVE_UT_HOST - 0
	memcpy (utmp->ut_host, utmpx->ut_host, sizeof (utmp->ut_host));
#endif
#if _HAVE_UT_TV - 0
	utmp->ut_tv.tv_sec = utmpx->ut_tv.tv_sec;
	utmp->ut_tv.tv_usec = utmpx->ut_tv.tv_usec;
#else
	utmp->ut_time = utmpx->ut_time;
#endif
}

/* Copy the information in UTMP to UTMPX. */
void getutmpx (const struct utmp *utmp, struct utmpx *utmpx)
{
	memset (utmpx, 0, sizeof (struct utmpx));

#if _HAVE_UT_TYPE - 0
	utmpx->ut_type = utmp->ut_type;
#endif
#if _HAVE_UT_PID - 0
	utmpx->ut_pid = utmp->ut_pid;
#endif
	memcpy (utmpx->ut_line, utmp->ut_line, sizeof (utmp->ut_line));
	memcpy (utmpx->ut_user, utmp->ut_user, sizeof (utmp->ut_user));
#if _HAVE_UT_ID - 0
	memcpy (utmpx->ut_id, utmp->ut_id, sizeof (utmp->ut_id));
#endif
#if _HAVE_UT_HOST - 0
	memcpy (utmpx->ut_host, utmp->ut_host, sizeof (utmp->ut_host));
#endif
#if _HAVE_UT_TV - 0
	utmpx->ut_tv.tv_sec = utmp->ut_tv.tv_sec;
	utmpx->ut_tv.tv_usec = utmp->ut_tv.tv_usec;
#else
	utmpx->ut_time = utmp->ut_time;
#endif
}

