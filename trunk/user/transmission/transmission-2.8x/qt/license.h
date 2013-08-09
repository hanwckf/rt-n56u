/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * $Id: license.h 13944 2013-02-03 19:40:20Z jordan $
 */

#ifndef LICENSE_DIALOG_H
#define LICENSE_DIALOG_H

#include <QDialog>

class LicenseDialog: public QDialog
{
    Q_OBJECT

  public:
    LicenseDialog (QWidget * parent = 0);
    ~LicenseDialog () {}
};

#endif

