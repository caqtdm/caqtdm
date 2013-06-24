//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#include "imagepushbutton.h"
#include <QResizeEvent>
#include <QPainter>

ImagePushButton::ImagePushButton(const QString & text, const QString& image, QWidget *parent): EPushButton(text, parent)
{
    myText = text;
    myImage= image;
    iconPresent = false;
    invisible = false;
    searchFile *s = new searchFile(image);
    QString fileNameFound = s->findFile();

    if(fileNameFound.isNull()) {
        //printf("file <%s> does not exist\n", qPrintable(image));
        iconOK = false;
    } else {
        pixmap.load(fileNameFound);
        iconOK = true;
    }
    delete s;
}

void ImagePushButton:: setLabelText(const QString& text) {
    myText = text;
    update();
}

void ImagePushButton::setIconVisible(bool b) {
    iconPresent = b;
    update();
}

void ImagePushButton::setInVisible(QColor bg, QColor fg, QColor bc) {
    invisible = true;
    thisbg = bg;
    thisfg = fg;
    thisbc = bc;
    update();
}

void ImagePushButton::resizeEvent(QResizeEvent *e) {
    QSize size = e->size();
    setGeometry(0, 0, size.width(), size.height());
    EPushButton::resizeEvent(e);
}

void ImagePushButton::paintEvent( QPaintEvent* event) {

    int x, y, w, h;
    EPushButton::paintEvent(event);
    QPainter p(this);

    p.setRenderHint( QPainter::Antialiasing );

    QRect r = this->geometry();
    x=r.x(); y=r.y(); w=r.width(); h=r.height();
    //printf("%d %d %d %d\n", x, y, w, h);

    if(invisible) {
        thisbg.setAlpha(0);
        thisfg.setAlpha(0);
        p.setBackground(thisbg);
        p.setPen(thisfg);
        p.drawRect(QRect(0, 0, w, h));

    }  else {
        if(iconPresent && iconOK) {
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            p.drawPixmap( 1, y+h/2-pixh/2, pixmap );
            x += pixw + 4;
            w -= pixw + 4;
        }
        r.setRect(x, y, w, h);

        //p.drawText(r, Qt::AlignCenter, myText);
    }
}

