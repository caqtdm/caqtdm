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

#include "caimage.h"
#include "searchfile.h"
#include "fileFunctions.h"

caImage::caImage(QWidget* parent) : QWidget(parent)
{
    messagequeue = new messageQueue();
    _container = new QLabel(this);
    _layout = new QVBoxLayout(this);
    thisFrame = prevFrame = 0;
    setVisibility(StaticV);
    timerId = 0;
    thisDelay = 500;
}

caImage::~caImage() {

   delete _animation;
}

QString caImage::getMessages()
{
    if(!messagequeue->isEmpty()) {
        return messagequeue->dequeue();
    } else {
      return NULL;
    }
}

bool caImage::anyMessages()
{
    return !messagequeue->isEmpty();
}

void caImage::init(const QString& filename) {

    // this will check for file existence and when an url is defined, download the file from a http server
    fileFunctions filefunction;
    int success = filefunction.checkFileAndDownload(filename);
    if(filefunction.lastInfo().length() > 0) messagequeue->enqueue(filefunction.lastInfo());
    if(!success) {
        if(filefunction.lastError().length() > 0) messagequeue->enqueue(filefunction.lastError());
        messagequeue->enqueue(tr("Info: could not find or download file %1, however continue").arg(filename));
    }

    searchFile *s = new searchFile(filename);
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        qDebug() << "file" << filename << "does not exist";
        delete s;
        return;
    }

    _animation = new QMovie(fileNameFound, 0, this);
    _animation->setCacheMode(QMovie::CacheAll);
    _animation->jumpToFrame(0);
    delete s;
    if( _animation.isNull()) return;
    // display the movie
    _container->setScaledContents(true);
    _container->setMovie(_animation);

    _layout->setSpacing(0);
    _layout->setMargin(0);
    _layout->addWidget(_container);
    setLayout(_layout);

    setHidden(false);
}

int caImage::getFrameCount()
{
    if( _animation.isNull()) return 0;
    return _animation->frameCount();
}

void caImage::timerEvent(QTimerEvent *)
{
    if( _animation.isNull()) return;
    if(thisFrame > (_animation->frameCount()-1)) {
        thisFrame=0;
    }
    // display only when frame changed
    if(thisFrame != prevFrame) {
      (void)_animation->jumpToFrame(thisFrame);
      prevFrame = thisFrame;
    }
    thisFrame++;
}

void caImage::startMovie()
{
    // kill default timer
    if(timerId != 0) killTimer(timerId);
    //start timer, but 0 milliseconds means no timer
    if(thisDelay > 0) timerId = startTimer(thisDelay);
}

void caImage::setInvalid(QColor c)
{
    if(c != oldColor) {
      QString style = "color: rgb(%1, %2, %3); background-color: rgb(%4, %5, %6);";
      style = style.arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.red()).arg(c.green()).arg(c.blue());
      _container->setStyleSheet(style);
      _container->setMovie(NULL);
      oldColor = c;
    }
}

void caImage::setValid()
{
    QColor c;
    if(oldColor == Qt::gray) return;
    _container->setMovie(_animation);
    c = oldColor = Qt::gray;
    QString style = "color: rgb(%1, %2, %3); background-color: rgba(%4, %5, %6, %7);";
    style = style.arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.red()).arg(c.green()).arg(c.blue()).arg(0);
    _container->setStyleSheet(style);
}

void caImage::setFileName(QString filename)
{
    thisFileName = filename;
    init(thisFileName);
}

void caImage::setFrame(int frame)
{
    thisFrame = frame;
    if( _animation.isNull()) return;
    (void)_animation->jumpToFrame(frame);
    prevFrame= thisFrame;
}
