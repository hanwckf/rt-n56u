/*
    mtr  --  a network diagnostic tool
    Copyright (C) 1998  R.E.Wolff@BitWizard.nl

    raw.h -- raw output (for logging for later analysis)

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

/*  Prototypes for raw.c  */
extern void raw_rawxmit(
    int host,
    int seq);
extern void raw_rawping(
    struct mtr_ctl *ctl,
    int host,
    int msec,
    int seq);
extern void raw_rawhost(
    struct mtr_ctl *ctl,
    int host,
    ip_t * addr);
