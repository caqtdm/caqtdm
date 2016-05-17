/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */

#include <pipereader.h>

PipeReader::PipeReader(QEventLoop *loop, QObject *parent) : QObject(parent)
{
    evLoop = loop;
    std_in = new QFile();
    std_in->open(0,QIODevice::ReadOnly);
    notifier = new QSocketNotifier(0, QSocketNotifier::Read);
    connect(notifier,SIGNAL(activated(int)),this,SLOT(DataReadyOnStdin()));

    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(Quit()));
    timer->start();
}

PipeReader::~PipeReader()
{
}

void PipeReader:: Quit()
{
    //qDebug() << "timeout";
    timer->stop();
    // get rid of notifier
    disconnect(notifier, SIGNAL(activated(int)), 0, 0 );
    notifier->deleteLater();
    emit evLoop->quit();
}

void PipeReader::DataReadyOnStdin() {

    timer->stop();
    QByteArray newData = std_in->readAll();
    std_in->close();
    //qDebug() << data << data.size();
    if(newData.size() == 0) {

    } else {
        QTemporaryFile file(QDir::tempPath()+"/qt-tempFile");
        file.setAutoRemove(false);
        if(file.open()) {
            qDebug() << file.fileName();
            file.write(newData);
            file.close();
            filename = file.fileName() + ".ui";
            QFile::rename(file.fileName(), filename);
        }
    }
    // get rid of notifier
    disconnect(notifier, SIGNAL(activated(int)), 0, 0 );
    notifier->deleteLater();

    // say we should have received data and have written a file
    emit evLoop->quit();
}

QString PipeReader::getTemporaryFilename()
{
    //qDebug() << "getTemporaryFilename" << filename;
    return filename;
}

