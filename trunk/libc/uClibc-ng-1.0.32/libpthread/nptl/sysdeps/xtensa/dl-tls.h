/* Thread-local storage handling in the ELF dynamic linker.  Xtensa version.
   Copyright (C) 2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _XTENSA_DL_TLS_H
#define _XTENSA_DL_TLS_H 1

/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;

extern void *__tls_get_addr (tls_index *ti);

/* Type used to represent a TLS descriptor.  */
struct tlsdesc
{
  union
    {
      void *pointer;
      long value;
    } argument;
  ptrdiff_t (*entry)(struct tlsdesc *);
};

/* Type used as the argument in a TLS descriptor for a symbol that
   needs dynamic TLS offsets.  */
struct tlsdesc_dynamic_arg
{
  tls_index tlsinfo;
  size_t gen_count;
};

extern ptrdiff_t attribute_hidden
  _dl_tlsdesc_return(struct tlsdesc_dynamic_arg *);

extern void *_dl_make_tlsdesc_dynamic (struct link_map *map, size_t ti_offset);
extern ptrdiff_t attribute_hidden
  _dl_tlsdesc_dynamic(struct tlsdesc_dynamic_arg *);

#endif
