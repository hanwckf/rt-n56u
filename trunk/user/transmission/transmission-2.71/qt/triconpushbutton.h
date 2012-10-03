/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * $Id: triconpushbutton.h 11092 2010-08-01 20:36:13Z charles $
 */

#ifndef QTR_IconPushButton_H
#define QTR_IconPushButton_H

#include <QPushButton>

class QIcon;

class TrIconPushButton: public QPushButton
{
        Q_OBJECT

    public:
        TrIconPushButton( QWidget * parent = 0 );
        TrIconPushButton( const QIcon&, QWidget * parent = 0 );
        virtual ~TrIconPushButton( ) { }
        QSize sizeHint () const;

    protected:
        void paintEvent( QPaintEvent * event );
};

#endif // QTR_IconPushButton_H
