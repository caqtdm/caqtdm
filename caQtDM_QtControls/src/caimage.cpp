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
#include <QPainter>
#include <QResizeEvent>
//#include <QElapsedTimer>

Q_LOGGING_CATEGORY(caImageLog, "caqtdm.widgets.caimage")

caImage::caImage(QWidget* parent) : QWidget(parent)
{
    messagequeue = new messageQueue();
    _container = new QLabel();
    _layout = new QVBoxLayout(this);
    thisAngle = 0;
    thisFrame = prevFrame = 0;
    setVisibility(StaticV);
    timerId = 0;
    thisDelay = 500;
    thisSmoothScaling = false;
}

caImage::~caImage() {

   delete _animation;
   delete messagequeue;
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

void caImage::init(const QString& filename, const bool isProvisional) {
    // this will check for file existence and when an url is defined, download the file from a http server
    fileFunctions filefunction;
    int success = filefunction.checkFileAndDownload(filename);
    if(filefunction.lastInfo().length() > 0) messagequeue->enqueue(filefunction.lastInfo());
    if(!success) {
        if (isProvisional) {
            qCDebug(caImageLog) << "caimage:" << tr("Info: could not find or download provisional file %1, however continue; %2").arg(filename).arg(qasc(filefunction.lastInfo()));
        } else {
            if(filefunction.lastError().length() > 0) messagequeue->enqueue(filefunction.lastError());
            messagequeue->enqueue(tr("Info: could not find or download file %1, however continue").arg(filename));
            qCWarning(caImageLog) << "caimage:" << tr("Info: could not find or download file %1, however continue; %2").arg(filename).arg(qasc(filefunction.lastInfo()));
        }
    }

    searchFile *s = new searchFile(filename);
    QString fileNameFound = s->findFile();
    if(fileNameFound.isNull()) {
        if (isProvisional) {
            qCDebug(caImageLog) << "provisional file" << filename << "could not be found";
        } else {
            qCCritical(caImageLog) << "file" << filename << "does not exist";
        }
        delete s;
        return;
    }

    sourcePixmap = QPixmap();
    _animation = new QMovie(fileNameFound, 0, this);
    _animation->setCacheMode(QMovie::CacheAll);
    _animation->jumpToFrame(0);
    connect(_animation, SIGNAL(frameChanged(int)), this, SLOT(OnFrameChanged(int)));
    delete s;
    if( _animation.isNull()) return;
    // display the movie
    _layout->setSpacing(0);
    SETMARGIN_QT456(_layout,0);
    _layout->addWidget(_container);
    setLayout(_layout);

    renderCurrentFrame();

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
      _container->clear();
      oldColor = c;
    }
}

void caImage::setValid()
{
    QColor c;
    if(oldColor == Qt::gray) return;
    c = oldColor = Qt::gray;
    QString style = "color: rgb(%1, %2, %3); background-color: rgba(%4, %5, %6, %7);";
    style = style.arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.red()).arg(c.green()).arg(c.blue()).arg(0);
    _container->setStyleSheet(style);
    renderCurrentFrame();
}

void caImage::setFileName(QString filename, bool isProvisional)
{
    thisFileName = filename;
    init(thisFileName, isProvisional);
}

void caImage::setFrame(int frame)
{
    thisFrame = frame;
    if( _animation.isNull()) return;
    (void)_animation->jumpToFrame(frame);
    prevFrame= thisFrame;
}

void caImage::setAngle( int angle)
{
    if (angle >= 0 && angle <= 360) {
        thisAngle = angle;
        OnFrameChanged(thisFrame);
    }
}

void caImage::slotTiltAngle(int angle)
{
    setAngle(angle);
}

void caImage::slotTiltAngle(double angle)
{
    setAngle(qRound(angle));
}

void caImage::OnFrameChanged(int frame)
{
    Q_UNUSED(frame)
    if( _animation.isNull()) return;
    sourcePixmap = _animation->currentPixmap();
    renderFrame(sourcePixmap);
}

void caImage::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    renderCurrentFrame();
}

void caImage::renderCurrentFrame()
{
    if(sourcePixmap.isNull()) {
        if(_animation.isNull()) return;
        sourcePixmap = _animation->currentPixmap();
    }
    renderFrame(sourcePixmap);
}

void caImage::renderFrame(const QPixmap &frame)
{
    if(_container.isNull() || frame.isNull()) return;

    const QSize targetSize = contentsRect().size();
    const QSize sourceSize = frame.size();
    if(targetSize.isEmpty() || sourceSize.isEmpty()) return;

    QPixmap rendered(targetSize);
    rendered.fill(QColor::fromRgb(0, 0, 0, 0));

    QPainter painter(&rendered);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, thisSmoothScaling);
    painter.scale((qreal) targetSize.width() / sourceSize.width(),
                  (qreal) targetSize.height() / sourceSize.height());

    if(thisAngle != 0) {
        // Keep the historical source-canvas pivot and clipping behaviour.
        painter.translate(sourceSize.height()/2, sourceSize.height()/2);
        painter.rotate(thisAngle);
        painter.translate(-sourceSize.height()/2, -sourceSize.height()/2);
    }

    painter.drawPixmap(0, 0, frame);
    _container->setPixmap(rendered);
}
