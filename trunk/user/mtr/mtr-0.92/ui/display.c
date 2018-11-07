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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include "mtr.h"
#include "display.h"
#include "report.h"
#include "select.h"
#include "raw.h"
#include "dns.h"
#include "asn.h"

#ifdef HAVE_CURSES
#include "mtr-curses.h"
#endif

#ifdef HAVE_GTK
#include "mtr-gtk.h"
#endif

#include "split.h"

#ifdef HAVE_CURSES
#define DEFAULT_DISPLAY DisplayCurses
#else
#define DEFAULT_DISPLAY DisplayReport
#endif

#ifdef HAVE_GTK
#define UNUSED_IF_NO_GTK        /* empty */
#else
#define UNUSED_IF_NO_GTK ATTRIBUTE_UNUSED
#endif

void display_detect(
    struct mtr_ctl *ctl,
    int *argc UNUSED_IF_NO_GTK,
    char ***argv UNUSED_IF_NO_GTK)
{
    ctl->DisplayMode = DEFAULT_DISPLAY;

#ifdef HAVE_GTK
    if (gtk_detect(argc, argv)) {
        ctl->DisplayMode = DisplayGTK;
    }
#endif
}


void display_open(
    struct mtr_ctl *ctl)
{
    switch (ctl->DisplayMode) {

    case DisplayReport:
        report_open();
        break;
    case DisplayTXT:
        txt_open();
        break;
    case DisplayJSON:
        json_open();
        break;
    case DisplayXML:
        xml_open();
        break;
    case DisplayCSV:
        csv_open();
        break;
#ifdef HAVE_CURSES
    case DisplayCurses:
        mtr_curses_open(ctl);
#ifdef HAVE_IPINFO
        asn_open(ctl);
#endif
        break;
#endif
    case DisplaySplit:
        split_open();
        break;
#ifdef HAVE_GTK
    case DisplayGTK:
        gtk_open(ctl);
#ifdef HAVE_IPINFO
        asn_open(ctl);
#endif
        break;
#endif
    }
}


void display_close(
    struct mtr_ctl *ctl)
{
    time_t now;

    now = time(NULL);

    switch (ctl->DisplayMode) {
    case DisplayReport:
        report_close(ctl);
        break;
    case DisplayTXT:
        txt_close(ctl);
        break;
    case DisplayJSON:
        json_close(ctl);
        break;
    case DisplayXML:
        xml_close(ctl);
        break;
    case DisplayCSV:
        csv_close(ctl, now);
        break;
#ifdef HAVE_CURSES
    case DisplayCurses:
#ifdef HAVE_IPINFO
        asn_close(ctl);
#endif
        mtr_curses_close();
        break;
#endif
    case DisplaySplit:
        split_close();
        break;
#ifdef HAVE_GTK
    case DisplayGTK:
        gtk_close();
        break;
#endif
    }
}


void display_redraw(
    struct mtr_ctl *ctl)
{
    switch (ctl->DisplayMode) {

#ifdef HAVE_CURSES
    case DisplayCurses:
        mtr_curses_redraw(ctl);
        break;
#endif

    case DisplaySplit:
        split_redraw(ctl);
        break;

#ifdef HAVE_GTK
    case DisplayGTK:
        gtk_redraw(ctl);
        break;
#endif
    }
}


int display_keyaction(
    struct mtr_ctl *ctl)
{
    switch (ctl->DisplayMode) {
#ifdef HAVE_CURSES
    case DisplayCurses:
        return mtr_curses_keyaction(ctl);
#endif

    case DisplaySplit:
        return split_keyaction();

#ifdef HAVE_GTK
    case DisplayGTK:
        return gtk_keyaction();
#endif
    }
    return 0;
}


void display_rawxmit(
    struct mtr_ctl *ctl,
    int host,
    int seq)
{
    if (ctl->DisplayMode == DisplayRaw)
        raw_rawxmit(host, seq);
}


void display_rawping(
    struct mtr_ctl *ctl,
    int host,
    int msec,
    int seq)
{
    if (ctl->DisplayMode == DisplayRaw)
        raw_rawping(ctl, host, msec, seq);
}


void display_rawhost(
    struct mtr_ctl *ctl,
    int host,
    ip_t * ip_addr)
{
    if (ctl->DisplayMode == DisplayRaw)
        raw_rawhost(ctl, host, ip_addr);
}


void display_loop(
    struct mtr_ctl *ctl)
{
#ifdef HAVE_GTK
    if (ctl->DisplayMode == DisplayGTK)
        gtk_loop(ctl);
    else
#endif
        select_loop(ctl);
}


void display_clear(
    struct mtr_ctl *ctl)
{
#ifdef HAVE_CURSES
    if (ctl->DisplayMode == DisplayCurses)
        mtr_curses_clear(ctl);
#endif
}
