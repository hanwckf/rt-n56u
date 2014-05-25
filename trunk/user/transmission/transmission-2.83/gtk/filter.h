/*
 * This file Copyright (C) 2012-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: filter.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef GTR_FILTER_H
#define GTR_FILTER_H

#include <gtk/gtk.h>
#include <libtransmission/transmission.h>

GtkWidget * gtr_filter_bar_new (tr_session     * session,
                                GtkTreeModel   * torrent_model,
                                GtkTreeModel  ** filter_model);

#endif
