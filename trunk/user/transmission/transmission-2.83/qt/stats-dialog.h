/*
 * This file Copyright (C) 2009-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: stats-dialog.h 14241 2014-01-21 03:10:30Z jordan $
 */

#ifndef STATS_DIALOG_H
#define STATS_DIALOG_H

#include <QDialog>

class Session;
class QLabel;
class QTimer;

class StatsDialog: public QDialog
{
    Q_OBJECT

  signals:
    void accepted ();

  public slots:
    void updateStats ();

  private slots:
    void onTimer ();

  public:
    StatsDialog (Session&, QWidget * parent = 0);
    ~StatsDialog ();
    virtual void setVisible (bool visible);

  private:
    Session & mySession;
    QTimer * myTimer;
    QLabel * myCurrentUp;
    QLabel * myCurrentDown;
    QLabel * myCurrentRatio;
    QLabel * myCurrentDuration;
    QLabel * myStartCount;
    QLabel * myTotalUp;
    QLabel * myTotalDown;
    QLabel * myTotalRatio;
    QLabel * myTotalDuration;
};

#endif
