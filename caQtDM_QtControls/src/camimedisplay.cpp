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

#include "camimedisplay.h"
#include <QApplication>

caMimeDisplay::caMimeDisplay(QWidget *parent) : caRowColMenu(parent)
{
    setImage("mime.png");
    setStacking(Menu);
    setElevation(on_top);
    installEventFilter(this);
    connect(this, SIGNAL(clicked(int)), this, SLOT(Callback_Clicked(int)));
    connect(this, SIGNAL(triggered(int)), this, SLOT(Callback_Clicked(int)));
}

void caMimeDisplay::Callback_Clicked(int indx)
{
    QStringList Urls = getFiles().split(";");

    if(indx <  Urls.count()) {
        QUrl url(Urls.at(indx));

        // file contains things like http:// or file:// or ...
        if(Urls.at(indx).contains("://")) {
            // test if local file exists
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
            if(url.isLocalFile()) {
#else
            if (url.toString().compare(QLatin1String("file"), Qt::CaseInsensitive) != 0) {
#endif
                QString filePath = url.toLocalFile();
                QFile file(filePath);
                if (!file.exists()) {
                    QMessageBox::critical(0, tr("caQtDM"), tr("local file does not exist '%1'").arg(Urls.at(indx)));
                    return;
                }
                // call file as specified
            }
            printf("call file %s as specified\n;", qasc(Urls.at(indx)));
            bool success = QDesktopServices::openUrl (QUrl(Urls.at(indx)));
            if(!success) QMessageBox::critical(0, tr("caQtDM"), tr("could not start mime application with file '%1'").arg(Urls.at(indx)));
            return;

            // must be a local file we have to search its location (application path, CAQTDM_DISPLAY_PATH, CAQTDM_MIME_PATH
        } else {
            QString fileName = Urls.at(indx);
            printf("%s\n", qasc(fileName));
            // find from application path
            QFile filePath(fileName);
            if(filePath.exists()) {
                bool success = QDesktopServices::openUrl (QUrl(filePath.fileName()));
                if(!success) QMessageBox::critical(0, tr("caQtDM"), tr("could not start mime application with file '%1'").arg(filePath.fileName()));
                return;
            }

            // find in caQtDM_DISPLAY_PATH
            QString path = (QString) qgetenv("CAQTDM_DISPLAY_PATH");
            QStringList paths = path.split(pathSeparator);
            QString pathFile;
            for(int i=0; i< paths.count(); i++) {
                pathFile = paths[i] + "/" + fileName;
                QFile filePath(pathFile);
                if(filePath.exists()) {
                    bool success = QDesktopServices::openUrl (QUrl(filePath.fileName()));
                    if(!success) QMessageBox::critical(0, tr("caQtDM"), tr("could not start mime application with file '%1'").arg(filePath.fileName()));
                    return;
                }
            }

            // find in caQtDM_MIME_PATH
            path = (QString) qgetenv("CAQTDM_MIME_PATH");
            paths = path.split(pathSeparator);
            for(int i=0; i< paths.count(); i++) {
                pathFile = paths[i] + "/" + fileName;
                QFile filePath(pathFile);
                if(filePath.exists()) {
                    bool success = QDesktopServices::openUrl (QUrl(filePath.fileName()));
                    if(!success) QMessageBox::critical(0, tr("caQtDM"), tr("could not start mime application with file '%1'").arg(filePath.fileName()));
                    return;
                }
            }
            QMessageBox::critical(0, tr("caQtDM"), tr("could not start mime application with file '%1'").arg(fileName));
        }
    }
}

bool caMimeDisplay::eventFilter(QObject *obj, QEvent *event)
{
    // intercept space key, so that no keyboard spacebar will trigger when button has focus
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *me = static_cast<QKeyEvent *>(event);
        if(me->key() == Qt::Key_Space) {
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}


