/*
    mtr  --  a network diagnostic tool
    Copyright (C) 1997,1998  Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "mtr.h"

extern void asn_open(
    struct mtr_ctl *ctl);
extern void asn_close(
    struct mtr_ctl *ctl);
extern char *fmt_ipinfo(
    struct mtr_ctl *ctl,
    ip_t * addr);
extern ATTRIBUTE_CONST size_t get_iiwidth_len(
    void);
extern ATTRIBUTE_CONST int get_iiwidth(
    int ipinfo_no);
extern int is_printii(
    struct mtr_ctl *ctl);
