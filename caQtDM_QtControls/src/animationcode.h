        QRect q;
        if(p.x() < 0)  q.setX(x()); else q.setX(p.x());
        if(p.y() < 0)  q.setY(y()); else q.setY(p.y());
        if(p.width() < 0)  q.setWidth(width()); else q.setWidth(p.width());
        if(p.height() < 0)  q.setHeight(height()); else q.setHeight(p.height());
        setGeometry(q);
