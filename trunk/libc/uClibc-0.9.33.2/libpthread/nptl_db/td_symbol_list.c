/* Return list of symbols the library can request.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#ifndef __UCLIBC__
#include <gnu/lib-names.h>
#endif
#include "thread_dbP.h"


#ifdef HAVE_ASM_GLOBAL_DOT_NAME
# define DOT "."		/* PPC64 requires . prefix on code symbols.  */
#else
# define DOT			/* No prefix.  */
#endif

static const char *symbol_list_arr[] =
{
# define DB_STRUCT(type) \
  [SYM_SIZEOF_##type] = "_thread_db_sizeof_" #type,
# define DB_STRUCT_FIELD(type, field) \
  [SYM_##type##_FIELD_##field] = "_thread_db_" #type "_" #field,
# define DB_SYMBOL(name) \
  [SYM_##name] = #name,
# define DB_FUNCTION(name) \
  [SYM_##name] = DOT #name,
# define DB_VARIABLE(name) \
  [SYM_##name] = #name, \
  [SYM_DESC_##name] = "_thread_db_" #name,
# include "structs.def"
# undef DB_STRUCT
# undef DB_FUNCTION
# undef DB_SYMBOL
# undef DB_VARIABLE

  [SYM_TH_UNIQUE_CONST_THREAD_AREA] = "_thread_db_const_thread_area",
  [SYM_TH_UNIQUE_REGISTER64] = "_thread_db_register64",
  [SYM_TH_UNIQUE_REGISTER32] = "_thread_db_register32",
  [SYM_TH_UNIQUE_REGISTER32_THREAD_AREA] = "_thread_db_register32_thread_area",
  [SYM_TH_UNIQUE_REGISTER64_THREAD_AREA] = "_thread_db_register64_thread_area",

  [SYM_NUM_MESSAGES] = NULL
};


const char **
td_symbol_list (void)
{
  return symbol_list_arr;
}


ps_err_e
td_lookup (struct ps_prochandle *ps, int idx, psaddr_t *sym_addr)
{
  ps_err_e result;
  assert (idx >= 0 && idx < SYM_NUM_MESSAGES);
  result = ps_pglobal_lookup (ps, LIBPTHREAD_SO, symbol_list_arr[idx],
			      sym_addr);

#ifdef HAVE_ASM_GLOBAL_DOT_NAME
  /* For PowerPC, 64-bit uses dot symbols but 32-bit does not.
     We could be a 64-bit libthread_db debugging a 32-bit libpthread.  */
  if (result == PS_NOSYM && symbol_list_arr[idx][0] == '.')
    result = ps_pglobal_lookup (ps, LIBPTHREAD_SO, &symbol_list_arr[idx][1],
				sym_addr);
#endif

  return result;
}
