/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2000, 2007-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file exception.c */

/**
 * \addtogroup PedException
 *
 * \brief Exception handling.
 *
 * There are a few types of exceptions: PED_EXCEPTION_INFORMATION,
 * PED_EXCEPTION_WARNING, PED_EXCEPTION_ERROR, PED_EXCEPTION_FATAL,
 * PED_EXCEPTION_BUG.
 *
 * They are "thrown" when one of the above events occur while executing
 * a libparted function. For example, if ped_device_open() fails
 * because the device doesn't exist, an exception will be thrown.
 * Exceptions contain text describing what the event was. It will give
 * at least one option for resolving the exception: PED_EXCEPTION_FIX,
 * PED_EXCEPTION_YES, PED_EXCEPTION_NO, PED_EXCEPTION_OK, PED_EXCEPTION_RETRY,
 * PED_EXCEPTION_IGNORE, PED_EXCEPTION_CANCEL. After an exception is thrown,
 * there are two things that can happen:
 *
 * -# an exception handler is called, which selects how the exception should be
 *    resolved (usually by asking the user). Also note: an exception handler may
 *    choose to return PED_EXCEPTION_UNHANDLED. In this case, a default action
 *    will be taken by libparted (respectively the code that threw the
 *    exception). In general, a default action will be "safe".
 * -# the exception is not handled, because the caller of the function wants to
 *    handle everything itself. In this case, PED_EXCEPTION_UNHANDLED is
 *    returned.
 *
 * @{
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/exception.h>

#define N_(String) String
#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int				ped_exception = 0;

static PedExceptionOption default_handler (PedException* ex);

static PedExceptionHandler*	ex_handler = default_handler;
static PedException*		ex = NULL;
static int			ex_fetch_count = 0;

static const char *const type_strings [] = {
	N_("Information"),
	N_("Warning"),
	N_("Error"),
	N_("Fatal"),
	N_("Bug"),
	N_("No Implementation")
};

static const char *const option_strings [] = {
	N_("Fix"),
	N_("Yes"),
	N_("No"),
	N_("OK"),
	N_("Retry"),
	N_("Ignore"),
	N_("Cancel")
};

/**
 *  Return a string describing an exception type.
 */
char*
ped_exception_get_type_string (PedExceptionType ex_type)
{
	return (char *) type_strings [ex_type - 1];
}

/* FIXME: move this out to the prospective math.c */
/* FIXME: this can probably be done more efficiently */
static int _GL_ATTRIBUTE_PURE
ped_log2 (int n)
{
	int x;

        PED_ASSERT (n > 0);

	for (x=0; 1 << x <= n; x++);

	return x - 1;
}

/**
 * Return a string describing an exception option.
 */
char*
ped_exception_get_option_string (PedExceptionOption ex_opt)
{
	return (char *) option_strings [ped_log2 (ex_opt)];
}

static PedExceptionOption
default_handler (PedException* e)
{
	if (e->type == PED_EXCEPTION_BUG)
		fprintf (stderr,
			_("A bug has been detected in GNU Parted.  "
			"Refer to the web site of parted "
			"http://www.gnu.org/software/parted/parted.html "
			"for more information of what could be useful "
			"for bug submitting!  "
			"Please email a bug report to "
			"%s containing at least the "
			"version (%s) and the following message:  "),
			 PACKAGE_BUGREPORT, VERSION);
	else
		fprintf (stderr, "%s: ",
			 ped_exception_get_type_string (e->type));
	fprintf (stderr, "%s\n", e->message);

	switch (e->options) {
		case PED_EXCEPTION_OK:
		case PED_EXCEPTION_CANCEL:
		case PED_EXCEPTION_IGNORE:
			return e->options;

		default:
			return PED_EXCEPTION_UNHANDLED;
	}
}

/**
 * Set the exception handler.
 *
 * The exception handler should return ONE of the options set in ex->options,
 * indicating the way the event should be resolved.
 */
void
ped_exception_set_handler (PedExceptionHandler* handler)
{
	if (handler)
		ex_handler = handler;
	else
		ex_handler = default_handler;
}

/**
 * Get the current exception handler.
 */
PedExceptionHandler *
ped_exception_get_handler (void)
{
	if (ex_handler)
		return ex_handler;
	return default_handler;
}

/**
 * Assert that the current exception has been resolved.
 */
void
ped_exception_catch ()
{
        if (ped_exception) {
                ped_exception = 0;
                free (ex->message);
                free (ex);
                ex = NULL;
        }
}

static PedExceptionOption
do_throw ()
{
	PedExceptionOption	ex_opt;

	ped_exception = 1;

	if (ex_fetch_count) {
		return PED_EXCEPTION_UNHANDLED;
	} else {
		ex_opt = ex_handler (ex);
		ped_exception_catch ();
		return ex_opt;
	}
}

/**
 * Throw an exception.
 *
 * You can also use this in a program using libparted.
 * "message" is a printf-like format string, so you can do
 *
 * \code
 * ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_RETRY_CANCEL,
 *      "Can't open %s", file_name);
 * \endcode
 *
 * Returns the option selected to resolve the exception. If the exception was
 * unhandled, PED_EXCEPTION_UNHANDLED is returned.
 */
PedExceptionOption
ped_exception_throw (PedExceptionType ex_type,
		     PedExceptionOption ex_opts, const char* message, ...)
{
	va_list		arg_list;
	int result;
	static int size = 1000;

	if (ex)
		ped_exception_catch ();

	ex = (PedException*) malloc (sizeof (PedException));
	if (!ex)
		goto no_memory;

	ex->type = ex_type;
	ex->options = ex_opts;

	while (message) {
			ex->message = (char*) malloc (size * sizeof (char));
			if (!ex->message)
					goto no_memory;

			va_start (arg_list, message);
			result = vsnprintf (ex->message, size, message, arg_list);
			va_end (arg_list);

			if (result > -1 && result < size)
					break;

			size += 10;
			free (ex->message);
	}

	return do_throw ();

no_memory:
	fputs ("Out of memory in exception handler!\n", stderr);

	va_start (arg_list, message);
	vfprintf (stderr, message, arg_list);
	va_end (arg_list);

	return PED_EXCEPTION_UNHANDLED;
}

/**
 * Rethrow an unhandled exception.
 * This means repeating the last ped_exception_throw() statement.
 */
PedExceptionOption
ped_exception_rethrow ()
{
	return do_throw ();
}

/**
 * Indicates that exceptions should not go to the exception handler, but
 * passed up to the calling function(s).  All calls to
 * ped_exception_throw() will return PED_EXCEPTION_UNHANDLED.
 */
void
ped_exception_fetch_all ()
{
	ex_fetch_count++;
}

/**
 * Indicates that the calling function does not want to accept any
 * responsibility for exceptions any more.
 *
 * \note a caller of that function may still want responsibility, so
 *      ped_exception_throw() may not invoke the exception handler.
 *
 * \warning every call to this function must have a preceding
 *      ped_exception_fetch_all().
 */
void
ped_exception_leave_all ()
{
	PED_ASSERT (ex_fetch_count > 0);
	ex_fetch_count--;
}

/** @} */
