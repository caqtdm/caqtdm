QRect q;
QRect qold = geometry();
if(p.x() < 0)  q.setX(x()); else q.setX(p.x());
if(p.y() < 0)  q.setY(y()); else q.setY(p.y());
if(p.width() < 0)  q.setWidth(width()); else q.setWidth(p.width());
if(p.height() < 0)  q.setHeight(height()); else q.setHeight(p.height());
#if QWT_VERSION >= 0x060100
setProperty("SIGNALDRIVEN", true);
#else
setProperty("SIGNALDRIVEN", QVariant(true));
#endif

// when any change, set new geometry and adjust its scrollarea parent if any

if(q != qold) {
    //printf("change animation\n");
    setGeometry(q);

    if(QScrollArea* scrollWidget = qobject_cast<QScrollArea *>(parent()->parent()->parent())) {
        int maxX=300;
        int maxY=200;
        QList<QWidget *> allW = scrollWidget->findChildren<QWidget *>();
        foreach(QWidget* widget, allW) {
            if(widget->x() + widget->width() > maxX) maxX = widget->x() + widget->width();
            if(widget->y() + widget->height() > maxY) maxY = widget->y() + widget->height();
        }

        QWidget *contents = (QWidget*) parent();
        if(contents != (QWidget *) Q_NULLPTR) {
            QSize sizew = contents->minimumSize();
            if(maxX > sizew.width() || maxY > sizew.height()) {
                contents->setMinimumSize(maxX, maxY);
                //printf("change scroll size when animation\n");
            }
        }
    }
} else {
    //printf("nochange animation\n");
}

