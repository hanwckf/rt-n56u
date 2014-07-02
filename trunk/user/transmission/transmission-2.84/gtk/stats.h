/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: stats.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef GTR_STATS_DIALOG_H
#define GTR_STATS_DIALOG_H

#include <gtk/gtk.h>
#include "tr-core.h"

GtkWidget* gtr_stats_dialog_new (GtkWindow * parent, TrCore * core);

#endif /* GTR_STATS_DIALOG_H */
