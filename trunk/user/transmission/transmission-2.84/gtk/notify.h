/*
 * This file Copyright (C) 2008-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: notify.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef GTR_NOTIFY_H
#define GTR_NOTIFY_H

#include "tr-core.h"

void gtr_notify_init (void);

void gtr_notify_torrent_added   (const char * name);

void gtr_notify_torrent_completed (TrCore * core, int torrent_id);

#endif
