/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

/**
 * \addtogroup PedException
 * @{
 */

/** \file exception.h */

#ifndef PED_EXCEPTION_H_INCLUDED
#define PED_EXCEPTION_H_INCLUDED

typedef struct _PedException PedException;

/**
 * Exception type
 */
enum _PedExceptionType {
	PED_EXCEPTION_INFORMATION=1,
	PED_EXCEPTION_WARNING=2,
	PED_EXCEPTION_ERROR=3,
	PED_EXCEPTION_FATAL=4,
	PED_EXCEPTION_BUG=5,
	PED_EXCEPTION_NO_FEATURE=6,
};
typedef enum _PedExceptionType PedExceptionType;

/**
 * Option for resolving the exception
 */
enum _PedExceptionOption {
	PED_EXCEPTION_UNHANDLED=0,
	PED_EXCEPTION_FIX=1,
	PED_EXCEPTION_YES=2,
	PED_EXCEPTION_NO=4,
	PED_EXCEPTION_OK=8,
	PED_EXCEPTION_RETRY=16,
	PED_EXCEPTION_IGNORE=32,
	PED_EXCEPTION_CANCEL=64,
};
typedef enum _PedExceptionOption PedExceptionOption;
#define PED_EXCEPTION_OK_CANCEL	    (PED_EXCEPTION_OK + PED_EXCEPTION_CANCEL)
#define PED_EXCEPTION_YES_NO	    (PED_EXCEPTION_YES + PED_EXCEPTION_NO)
#define PED_EXCEPTION_YES_NO_CANCEL (PED_EXCEPTION_YES_NO \
				     + PED_EXCEPTION_CANCEL)
#define PED_EXCEPTION_IGNORE_CANCEL (PED_EXCEPTION_IGNORE \
				     + PED_EXCEPTION_CANCEL)
#define PED_EXCEPTION_RETRY_CANCEL  (PED_EXCEPTION_RETRY + PED_EXCEPTION_CANCEL)
#define PED_EXCEPTION_RETRY_IGNORE_CANCEL (PED_EXCEPTION_RETRY \
					   + PED_EXCEPTION_IGNORE_CANCEL)
#define PED_EXCEPTION_OPTION_FIRST PED_EXCEPTION_FIX
#define PED_EXCEPTION_OPTION_LAST PED_EXCEPTION_CANCEL

/**
 * Structure with information about exception
 */
struct _PedException {
	char*			message;	/**< text describing what the event was */
	PedExceptionType	type;		/**< type of exception */
	PedExceptionOption	options;	/**< ORed list of options that
						   the exception handler can
						   return (the ways an exception
						   can be resolved) */
};

typedef PedExceptionOption (PedExceptionHandler) (PedException* ex);

extern int ped_exception;	/* set to true if there's an exception */

extern char* ped_exception_get_type_string (PedExceptionType ex_type)
    
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__const__))
#endif
;
extern char* ped_exception_get_option_string (PedExceptionOption ex_opt)
    
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;

extern void ped_exception_set_handler (PedExceptionHandler* handler);
extern PedExceptionHandler *ped_exception_get_handler(void)
    
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;

extern PedExceptionOption ped_exception_default_handler (PedException* ex);

extern PedExceptionOption	ped_exception_throw (PedExceptionType ex_type,
						     PedExceptionOption ex_opt,
						     const char* message,
						     ...);
/* rethrows an exception - i.e. calls the exception handler, (or returns a
   code to return to pass up higher) */
extern PedExceptionOption	ped_exception_rethrow ();

/* frees an exception, indicating that the exception has been handled.
   Calling an exception handler counts. */
extern void			ped_exception_catch ();

/* indicate that exceptions should not go to the exception handler, but passed
   up to the calling function(s) */
extern void			ped_exception_fetch_all ();

/* indicate that exceptions should invoke the exception handler */
extern void			ped_exception_leave_all ();

#endif /* PED_EXCEPTION_H_INCLUDED */

/** @} */
