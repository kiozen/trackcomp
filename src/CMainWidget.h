/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CMainWidget.h

  Module:

  Description:

  Created:     08/16/2013

  (C) 2013 DSP Solutions. All rights reserved.


**********************************************************************************************/
#ifndef CMAINWIDGET_H
#define CMAINWIDGET_H

#include <QMainWindow>
#include "ui_IMainWidget.h"
#include "CTrack.h"

class CMainWidget : public QMainWindow, private Ui::IMainWidget
{
    Q_OBJECT;
    public:
        CMainWidget();
        virtual ~CMainWidget();


    private slots:
        void slotRefTrack();
        void slotPathTracks();
        void slotCompare();
        void slotReload();

    private:
        void loadReference(const QString& filename);
        void loadTrackList(const QString& path);

        QTreeWidgetItem  * findItemByKey(const QString& key);



        CTrack trackRef;

        QList<CTrack*> tracks;
};

#endif //CMAINWIDGET_H

