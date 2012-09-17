/* FR-V FDPIC ELF shared library loader suppport
 * Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Lots of code copied from ../i386/elfinterp.c, so:
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *               David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2002, Erik Andersen
 * All rights reserved.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

static const char * const _dl_reltypes_tab[] =
{
  [0]	"R_FRV_NONE",		"R_FRV_32",
  [2]	"R_FRV_LABEL16",	"R_FRV_LABEL24",
  [4]	"R_FRV_LO16",		"R_FRV_HI16",
  [6]	"R_FRV_GPREL12",	"R_FRV_GPRELU12",
  [8]	"R_FRV_GPREL32",	"R_FRV_GPRELHI",	"R_FRV_GPRELLO",
  [11]	"R_FRV_GOT12",		"R_FRV_GOTHI",		"R_FRV_GOTLO",
  [14]	"R_FRV_FUNCDESC",
  [15]	"R_FRV_FUNCDESC_GOT12",	"R_FRV_FUNCDESC_GOTHI",	"R_FRV_FUNCDESC_GOTLO",
  [18]	"R_FRV_FUNCDESC_VALUE", "R_FRV_FUNCDESC_GOTOFF12",
  [20]	"R_FRV_FUNCDESC_GOTOFFHI", "R_FRV_FUNCDESC_GOTOFFLO",
  [22]	"R_FRV_GOTOFF12",	"R_FRV_GOTOFFHI",	"R_FRV_GOTOFFLO",
#if 0
  [200]	"R_FRV_GNU_VTINHERIT",	"R_FRV_GNU_VTENTRY"
#endif
};
