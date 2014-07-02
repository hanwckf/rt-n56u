/*
 * This file Copyright (C) 2012-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: add-data.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef ADD_DATA_H
#define ADD_DATA_H

#include <QByteArray>
#include <QString>
#include <QUrl>

class AddData
{
  public:

    enum { NONE, MAGNET, URL, FILENAME, METAINFO };
    int type;

    QByteArray metainfo;
    QString filename;
    QString magnet;
    QUrl url;

  public:

    int set (const QString&);
    AddData (const QString& str) { set(str); }
    AddData (): type(NONE) {}

    QByteArray toBase64 () const;
    QString readableName () const;

  public:

    static bool isSupported (const QString& str) { return AddData(str).type != NONE; }
};

#endif
