/*
 * This file Copyright (C) 2010-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: about.h 14241 2014-01-21 03:10:30Z jordan $
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
    AboutDialog (QWidget * parent = 0);
    ~AboutDialog () {}
    QWidget * createAboutTab ();
    QWidget * createAuthorsTab ();
    QWidget * createLicenseTab ();

  public slots:
    void showCredits ();

};

#endif
