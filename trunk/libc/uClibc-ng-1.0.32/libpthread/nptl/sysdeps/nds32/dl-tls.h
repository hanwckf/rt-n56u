/* Thread-local storage handling in the ELF dynamic linker.  NDS32 version.
   Copyright (C) 2013 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _NDS32_DL_TLS_H
#define _NDS32_DL_TLS_H 1


/* Type used to represent a TLS descriptor.  */
struct tlsdesc
{
  ptrdiff_t (*entry)(struct tlsdesc *);
  union
    {
      void *pointer;
      long value;
    } argument;
};

/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;


/* Type used as the argument in a TLS descriptor for a symbol that
 *    needs dynamic TLS offsets.  */
struct tlsdesc_dynamic_arg
{
  tls_index tlsinfo;
  size_t gen_count;
};


extern void *__tls_get_addr (tls_index *ti);

extern ptrdiff_t attribute_hidden
  _dl_tlsdesc_return(struct tlsdesc_dynamic_arg *);

extern void *_dl_make_tlsdesc_dynamic (struct link_map *map, size_t ti_offset);
extern ptrdiff_t attribute_hidden
  _dl_tlsdesc_dynamic(struct tlsdesc *);

#endif //_NDS32_DL_TLS_H
