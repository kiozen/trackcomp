/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        CMainWidget.cpp

  Module:

  Description:

  Created:     08/16/2013

  (C) 2013 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CMainWidget.h"
#include "CGpx.h"
#include "CTrack.h"

#include <QtGui>
#include <QtXml/QDomElement>

class TreeWidgetItem : public QTreeWidgetItem
{
public:
    TreeWidgetItem(QTreeWidget *tree) : QTreeWidgetItem(tree)  {}
    TreeWidgetItem(QTreeWidget * parent, const QStringList & strings) : QTreeWidgetItem (parent,strings)  {}

    bool operator< (const QTreeWidgetItem &other) const
    {
        int sortCol = treeWidget()->sortColumn();
        if(sortCol < 2)
        {
            return text(sortCol) < other.text(sortCol);
        }
        else
        {
            double myNumber = text(sortCol).toDouble();
            double otherNumber = other.text(sortCol).toDouble();
            return myNumber < otherNumber;
        }
    }
};

CMainWidget::CMainWidget()
{
    setupUi(this);

    QSettings cfg;

    if ( cfg.contains("MainWindow/geometry"))
    {
        restoreGeometry(cfg.value("MainWindow/geometry").toByteArray());
    }
    else
    {
        setGeometry(0,0,800,600);
    }

    if ( cfg.contains("MainWindow/state"))
    {
        restoreState(cfg.value("MainWindow/state").toByteArray());
    }


    QString filename = cfg.value("Reference/file","").toString();
    if(!filename.isEmpty())
    {
        loadReference(filename);
    }

    QString path = cfg.value("Tracks/path",QDir::homePath()).toString();
    if(!path.isEmpty())
    {
        loadTrackList(path);
    }

    connect(toolRefTrack, SIGNAL(clicked()), this, SLOT(slotRefTrack()));
    connect(toolPathTracks, SIGNAL(clicked()), this, SLOT(slotPathTracks()));
    connect(pushCompare, SIGNAL(clicked()), this, SLOT(slotCompare()));

}

CMainWidget::~CMainWidget()
{
    QSettings cfg;

    cfg.setValue("MainWindow/state", saveState());
    cfg.setValue("MainWindow/geometry", saveGeometry());

}

void CMainWidget::slotRefTrack()
{
    QString path(QDir::homePath());

    if(!labelRefTrack->text().isEmpty())
    {
        QFileInfo fi(labelRefTrack->text());
        path = fi.absolutePath();
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Select reference track..."), path, "*.gpx");
    if(!filename.isEmpty())
    {
        loadReference(filename);
    }
}

void CMainWidget::slotPathTracks()
{
    QString path(QDir::homePath());

    if(!labelPathTracks->text().isEmpty())
    {
        QFileInfo fi(labelPathTracks->text());
        path = fi.absolutePath();
    }

    path = QFileDialog::getExistingDirectory(this, tr("Select path with tracks..."), path);
    if(!path.isEmpty())
    {
        loadTrackList(path);
    }

}

void CMainWidget::loadReference(const QString& filename)
{
    QSettings cfg;
    QDomElement gpxTrk;

    CGpx gpx(this);
    if(!gpx.load(filename))
    {
        goto loadReference_error;
    }
    gpxTrk = gpx.firstChildElement("gpx").firstChildElement("trk");
    if(gpxTrk.isElement())
    {
        if(!trackRef.load(gpxTrk))
        {
            goto loadReference_error;
        }
    }

    cfg.setValue("Reference/file", filename);
    labelRefTrack->setText(filename);
    return;

loadReference_error:
    cfg.setValue("Reference/file", "");
    labelRefTrack->setText("");
    return;
}

void CMainWidget::loadTrackList(const QString& path)
{
    QSettings cfg;
    QDomElement gpxTrk;

    QDir dir(path);

    treeTrackFiles->clear();
    qDeleteAll(tracks);

    QStringList files = dir.entryList(QStringList("*.gpx"), QDir::Files, QDir::Name);

    foreach(const QString& file, files)
    {
        CGpx gpx(this);
        if(!gpx.load(dir.absoluteFilePath(file)))
        {
            continue;
        }

        gpxTrk = gpx.firstChildElement("gpx").firstChildElement("trk");
        while(gpxTrk.isElement())
        {
            CTrack * track = new CTrack();

            if(track->load(gpxTrk))
            {
                TreeWidgetItem * item = new TreeWidgetItem(treeTrackFiles);
                item->setText(0,track->getName());
                item->setData(0, Qt::UserRole, track->getKey());
                item->setText(1,file);
                item->setData(1,Qt::UserRole, dir.absoluteFilePath(file));

                qDebug() << track->getName() << file;

                tracks << track;
            }
            else
            {
                delete track;
            }

            gpxTrk = gpxTrk.nextSiblingElement();
        }

    }

    treeTrackFiles->header()->resizeSections(QHeaderView::ResizeToContents);
    labelPathTracks->setText(path);

    cfg.setValue("Tracks/path", path);

}

void CMainWidget::slotCompare()
{
    int cnt = 0;
    QProgressDialog progress("Compare tracks...", "Abort Compare", 0, tracks.size(), this);
    progress.setWindowModality(Qt::WindowModal);


    foreach(CTrack *track, tracks)
    {
        progress.setValue(cnt++);

        track->setReference(trackRef);

        QTreeWidgetItem * item = findItemByKey(track->getKey());
        if(item)
        {
            item->setText(2, QString("%1").arg(track->getMean()));
            item->setText(3, QString("%1").arg(track->getVariance()));
            item->setText(4, QString("%1").arg(track->getOffTrackCnt()));
            item->setText(5, QString("%1").arg(track->getLessThan15m(),0,'f',0));
            item->setText(6, QString("%1").arg(track->getLessThan10m(),0,'f',0));
            item->setText(7, QString("%1").arg(track->getLessThan5m(),0,'f',0));
        }

//        CGpx gpx(this);
//        QDomElement root = gpx.documentElement();
//        QDomElement gpxTrk = gpx.createElement("trk");
//        root.appendChild(gpxTrk);
//        track->save(gpxTrk, gpx);
//        gpx.save(track->getName() + "_vs_ref.gpx");

        if(progress.wasCanceled())
        {
            break;
        }
    }

    treeTrackFiles->header()->resizeSections(QHeaderView::ResizeToContents);
}

QTreeWidgetItem  * CMainWidget::findItemByKey(const QString& key)
{
    for(int i = 0; i < treeTrackFiles->topLevelItemCount(); i++)
    {
        QTreeWidgetItem * item = treeTrackFiles->topLevelItem(i);
        if(item->data(0, Qt::UserRole).toString() == key)
        {
            return item;
        }
    }

    return 0;
}

void CMainWidget::slotReload()
{
    loadTrackList(labelPathTracks->text());
}
