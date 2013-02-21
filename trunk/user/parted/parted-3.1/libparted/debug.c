/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000, 2005, 2007, 2009-2012 Free Software Foundation, Inc.

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

#include <config.h>
#include <parted/parted.h>
#include <parted/debug.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#ifdef DEBUG

#if HAVE_BACKTRACE
#include <execinfo.h>
#endif

static void default_handler ( const int level, const char* file, int line,
                const char* function, const char* msg );
static PedDebugHandler* debug_handler = &default_handler;


/**
 * Default debug handler.
 * Will print all information to stderr.
 */
static void default_handler ( const int level, const char* file, int line,
                const char* function, const char* msg )
{
        fprintf ( stderr, "[%d] %s:%d (%s): %s\n",
                        level, file, line, function, msg );
}

/**
 * Send a debug message.
 * Do not call this directly -- use PED_DEBUG() instead.
 *
 * level        log level, 0 ~= "print definitely"
 */
void ped_debug ( const int level, const char* file, int line,
                 const char* function, const char* msg, ... )
{
        va_list         arg_list;
        char*           msg_concat = ped_malloc(8192);

        va_start ( arg_list, msg );
                vsnprintf ( msg_concat, 8192, msg, arg_list );
        va_end ( arg_list );

        debug_handler ( level, file, line, function, msg_concat );

        free ( msg_concat );
}

/*
 * handler      debug handler; NULL for default handler
 */
void ped_debug_set_handler ( PedDebugHandler* handler )
{
        debug_handler = ( handler ? handler : default_handler );
}

/*
 * Check an assertion.
 * Do not call this directly -- use PED_ASSERT() instead.
 */
void ped_assert (const char* cond_text,
                 const char* file, int line, const char* function)
{
#if HAVE_BACKTRACE
        /* Print backtrace stack */
        void *stack[20];
        char **strings, **string;
        int size = backtrace(stack, 20);
        strings = backtrace_symbols(stack, size);

        if (strings) {
                printf(_("Backtrace has %d calls on stack:\n"), size);

                for (string = strings; size > 0; size--, string++)
                        printf("  %d: %s\n", size, *string);

                free(strings);
        }
#endif

        /* Throw the exception */
        ped_exception_throw (
                PED_EXCEPTION_BUG,
                PED_EXCEPTION_FATAL,
                _("Assertion (%s) at %s:%d in function %s() failed."),
                cond_text, file, line, function);
        abort ();
}

#endif /* DEBUG */
