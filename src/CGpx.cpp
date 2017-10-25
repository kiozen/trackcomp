/**********************************************************************************************
    Copyright (C) 2006, 2007 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/
#include "CGpx.h"
#include "CTrack.h"


#include <QtGui>

CGpx::CGpx(QObject * parent)
: QObject(parent)
, QDomDocument()
{
    writeMetadata();

}


CGpx::~CGpx()
{

}

void CGpx::writeMetadata()
{
    QDomElement root = createElement("gpx");
    appendChild(root);
    root.setAttribute("version","1.1");
    root.setAttribute("creator","TrackComp");

    QDomElement metadata = createElement("metadata");
    root.appendChild(metadata);

    QDomElement time = createElement("time");
    metadata.appendChild(time);
    QDomText _time_ = createTextNode(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd'T'hh:mm:ss'Z'"));
    time.appendChild(_time_);
}



bool CGpx::load(const QString& filename)
{
    clear();
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << tr("Failed to open: ") + filename;
        return false;
    }

    QString msg;
    int line;
    int column;
    if(!setContent(&file, true, &msg, &line, &column))
    {
        file.close();
        qDebug() << tr("Failed to read: %1\nline %2, column %3:\n %4").arg(filename).arg(line).arg(column).arg(msg);
        return false;
    }
    file.close();

    const  QDomElement& docElem = documentElement();
    if(docElem.tagName() != "gpx")
    {
        qDebug() << tr("Not a GPX file: ") + filename;
        file.close();
        return false;
    }

    if (!docElem.hasAttribute("creator"))
    {
        qDebug() << tr("GPX schema violation: no \"creator\" attribute.");
        return false;
    }

    return true;

}

void CGpx::save(const QString& filename)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << tr("Failed to create %1").arg(filename);
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" << endl;;
    out << toString();
    file.close();
    if(file.error() != QFile::NoError)
    {
        qDebug() << tr("Failed to write %1").arg(filename);
        return;
    }
}

