/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * $Id: about.h 11092 2010-08-01 20:36:13Z charles $
 */

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>

class AboutDialog: public QDialog
{
        Q_OBJECT

    private:
        QDialog * myLicenseDialog;

    public:
        AboutDialog( QWidget * parent = 0 );
        ~AboutDialog( ) { }
        QWidget * createAboutTab( );
        QWidget * createAuthorsTab( );
        QWidget * createLicenseTab( );

    public slots:
        void showCredits( );

};

#endif
