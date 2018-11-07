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

/*  Prototypes for curses.c  */
extern void mtr_curses_open(
    struct mtr_ctl *ctl);
extern void mtr_curses_close(
    void);
extern void mtr_curses_redraw(
    struct mtr_ctl *ctl);
extern int mtr_curses_keyaction(
    struct mtr_ctl *ctl);
extern void mtr_curses_clear(
    struct mtr_ctl *ctl);
