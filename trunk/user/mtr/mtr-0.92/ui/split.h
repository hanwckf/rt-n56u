/*
    mtr  --  a network diagnostic tool

    split.h -- raw output (for inclusion in KDE Network Utilities)
    Copyright (C) 1998  Bertrand Leconte <B.Leconte@mail.dotcom.fr>

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

/*  Prototypes for split.c  */
extern void split_open(
    void);
extern void split_close(
    void);
extern void split_redraw(
    struct mtr_ctl *ctl);
extern int split_keyaction(
    void);
