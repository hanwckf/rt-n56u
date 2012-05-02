/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * $Id: types.h 11092 2010-08-01 20:36:13Z charles $
 */

#ifndef QTR_TYPES_H
#define QTR_TYPES_H

#include <QVariant>

class TrTypes
{
    public:

        enum
        {
            TrackerStatsList = QVariant::UserType,
            PeerList = QVariant::UserType,
            FileList,
            FilterModeType,
            SortModeType
        };
};

#endif
