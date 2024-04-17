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
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QFrame>
#include <QScrollArea>
#include <math.h>
#include "cainclude.h"
#include "searchfile.h"
#include "fileFunctions.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QRegExp>
#else
    #include <QRegularExpression>
#endif

caInclude::caInclude(QWidget *parent) : QWidget(parent)
{
    thisLoadedWidgets.clear();
    thisParent = parent;
    newFileName="";
    thisLineSize = 1;
    thisItemCount = 1;
    thisMaxLines = 1;
    thisMaxColumns = 1;
    thisStacking = Row;
    // I have to change the previous default of this parameter
    prvAdjust= false;
    thisAdjust = true;
    thisSpacingVertical = thisSpacingHorizontal=0;
    thisFrameColor = Qt::gray;
    setVisibility(StaticV);

    thisPalette = palette();

    boxLayout = Q_NULLPTR;
    boxLayout = new QHBoxLayout();
    if (boxLayout)SETMARGIN_QT456(boxLayout,0);
    if (boxLayout)boxLayout->setSpacing(0);

    this->setLineSize(0);
    frame = new QFrame();
    thisFrameShape = NoFrame;
    thisFrameShadow = QFrame::Plain;
    thisFrameLineWidth = 1;

    if (boxLayout) boxLayout->addWidget(frame);

    gridLayout = Q_NULLPTR;
    gridLayout = new QGridLayout();
    if (gridLayout) SETMARGIN_QT456(gridLayout,0);
    if (gridLayout) gridLayout->setSpacing(0);

    effectiveSize = QSize(100,100);
    if (gridLayout) frame->setLayout(gridLayout);

    thisXfactor = thisYfactor = 1.0;
    thisXresizefactor=thisYresizefactor=1.0;

    setPropertyVisible(maximumLines, false);
    setPropertyVisible(xCorrectionFactor, false);
    setPropertyVisible(yCorrectionFactor, false);

    // when called from designer, we would like to see the included widgets
    loadIncludes = false;
    QVariant source = qApp->property("APP_SOURCE").value<QVariant>();

    // next code will only be executed when in designer
    // in other apps we load everything there while taking care of the macros
    if(source.isValid()) {
        if(!source.isNull()) {
            QString test = source.toString();
            if(test.contains("DESIGNER")) {
                loadIncludes = true;
            }
        }
    }
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    // This is needed for debugging
    connect(this, &QObject::objectNameChanged, this, &caInclude::caincludeNameChanged);
#endif
    clearChildsList();
}

caInclude::~ caInclude()
{
    if (boxLayout) delete boxLayout;
    frame->deleteLater();
    if (gridLayout) delete gridLayout;
}

void caInclude::caincludeNameChanged(const QString &objectName){
    if (frame) frame->setObjectName("caIncFrame_"+objectName);
    if (gridLayout) gridLayout->setObjectName("caIncgrLay_"+objectName);
    if (boxLayout) boxLayout->setObjectName("caIncBoxLay_"+objectName);
}



bool caInclude::isPropertyVisible(Properties property)
{
    return designerVisible[property];
}

void caInclude::setPropertyVisible(Properties property, bool visible)
{
    designerVisible[property] = visible;
}

void caInclude::removeIncludedWidgets()
{
    if(thisLoadedWidgets.count() > 0) {

        // remove from gridlayout and / or qframe
        foreach(QWidget *l, thisLoadedWidgets) {
            if (gridLayout) gridLayout->removeWidget(l);
            l->hide();
            l->setParent(NULL);
            l->deleteLater();
        }
        thisLoadedWidgets.clear();
    }
    prvFileName = "";
}

void caInclude::setStacking(Stacking stacking) {

    thisStacking = stacking;
    setFileName(newFileName);
    prvStacking = thisStacking;
    if(thisStacking == RowColumn) {
        setPropertyVisible(maximumLines, true);
        setPropertyVisible(maximumColumns, false);
    } else if(thisStacking == ColumnRow) {
        setPropertyVisible(maximumColumns, true);
        setPropertyVisible(maximumLines, false);
    } else {
        setPropertyVisible(maximumLines, false);
        setPropertyVisible(maximumColumns, false);
    }

    if(thisStacking == Positions) {
        setPropertyVisible(xCorrectionFactor, true);
        setPropertyVisible(yCorrectionFactor, true);
    } else {
        setPropertyVisible(xCorrectionFactor, false);
        setPropertyVisible(yCorrectionFactor, false);
    }
}

void caInclude::setFileName(QString const &filename)
{
    newFileName = filename.trimmed();

    // load widgets from includes
    if(loadIncludes) {

        QUiLoader loader;
        QString fileName;
        QStringList openFile;
        fileFunctions filefunction;

        int nbLines = thisMaxLines;
        int nbColumns = thisMaxColumns;
        int column = 0;
        int row = 0;
        int maxRows = 0;
        int maxColumns=0;
        int adjustMargin = 0;

        maximumX = maximumY = 0;

        //printf("cainclude -- setfilename %s for %s\n", qasc(filename), qasc(this->objectName()));

        if(newFileName.size() < 1) {
            removeIncludedWidgets();
            return;
        }

        if(!newFileName.contains(".")) {
            removeIncludedWidgets();
            return;
        }

        // modify stacking
        if(thisStacking != prvStacking || thisMaxLines != prvMaxLines || thisMaxColumns != prvMaxColumns || thisAdjust != prvAdjust ||
           thisSpacingHorizontal != prvSpacingHorizontal || thisSpacingVertical != prvSpacingVertical || thisFrameUpdate) {
            thisFrameUpdate = false;

            if (boxLayout) setLayout(boxLayout);
            SETMARGIN_QT456(gridLayout,0);

            switch(thisFrameShape) {
                case NoFrame:
                      frame->setFrameShape(QFrame::NoFrame);
                      break;
                case Box:
                      frame->setFrameShape(QFrame::Box);    
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                      gridLayout->setMargin(thisFrameLineWidth);
#endif
                      break;
                case Panel:
                      frame->setFrameShape(QFrame::Panel);
                      break;
                default:
                     frame->setFrameShape(QFrame::Panel);
            }

            frame->setFrameShadow(thisFrameShadow);
            frame->setLineWidth(thisFrameLineWidth);

            QColor thisLightColor = thisFrameColor.lighter();
            QColor thisDarkColor = thisFrameColor.darker();

            thisPalette.setColor(QPalette::WindowText, thisFrameColor);
            thisPalette.setColor(QPalette::Light, thisLightColor);
            thisPalette.setColor(QPalette::Dark, thisDarkColor);
            thisPalette.setColor(QPalette::Window, thisFrameColor);
            frame->setPalette(thisPalette);

            if (gridLayout) {
                gridLayout->setVerticalSpacing(thisSpacingVertical);
                gridLayout->setHorizontalSpacing(thisSpacingHorizontal);
            }
            if(thisLoadedWidgets.count() > 0) {
                int j = 0;

               adjustMargin = getMargin();

                // remove from gridlayout and / or qframe
                foreach(QWidget *l, thisLoadedWidgets) {
                    l->hide();
                    gridLayout->removeWidget(l);
                    l->setParent(NULL);
                }

                foreach(QWidget *l, thisLoadedWidgets) {
                    bool ok = false;
                    int posX = 0;
                    int posY = 0;
                    // find the row, column to add this widget
                    if(thisStacking == Row) {
                        if (gridLayout)  gridLayout->addWidget(l, j, 0);
                        row++;
                        maxRows = row;
                        maxColumns = 1;
                    } else if(thisStacking == Column) {
                        if (gridLayout) gridLayout->addWidget(l, 0, j);
                        column++;
                        maxColumns = column;
                        maxRows = 1;
                    } else if(thisStacking == RowColumn){
                        if(row >= nbLines) {
                            row=0;
                            column++;
                        }
                        if (gridLayout) gridLayout->addWidget(l, row, column);
                        row++;
                        if(row > maxRows) maxRows = row;
                        maxColumns = column + 1;
                    } else if(thisStacking == ColumnRow){
                        if(column >= nbColumns) {
                            row++;
                            column=0;
                        }
                        if (gridLayout) gridLayout->addWidget(l, row, column);
                        column++;
                        if(column > maxColumns) maxColumns = column;
                        maxRows = row + 1;
                    } else {
                        l->setParent(frame);
                        if(j<thisXpositionsList.count()) {
                            posX = thisXpositionsList.at(j).toInt(&ok);
                            if(!ok) posX = 0;
                        }
                        if(j<thisYpositionsList.count()) {
                            posY = thisYpositionsList.at(j).toInt(&ok);
                            if(!ok) posY = 0;
                        }

                        int xpos = qRound((double) posX * thisXfactor);
                        int ypos = qRound((double) posY * thisYfactor);
                        l->move(xpos + adjustMargin/2, ypos + adjustMargin/2);
                        int maxX = xpos + l->width();
                        int maxY = ypos + l->height();
                        if(maxX > maximumX) maximumX = maxX;
                        if(maxY > maximumY) maximumY = maxY;
                    }

                    j++;
                    l->show();
                }

                if(thisAdjust && (thisStacking != Positions)) {
                    int new_width=maxColumns * effectiveSize.width() + (maxColumns-1) * thisSpacingHorizontal + adjustMargin;
                    int new_height=maxRows * effectiveSize.height() + (maxRows-1) * thisSpacingVertical + adjustMargin;
                    if ((new_width>width()) || (new_height>height()))
                         resize(new_width,new_height);
                }
                    else if(thisAdjust && (thisStacking == Positions)) {
                    if ((maximumX>width()) || (maximumY>height()))
                         resize(maximumX + adjustMargin, maximumY + adjustMargin);
                }
                prvAdjust = thisAdjust;

                if(QScrollArea* scrollWidget = qobject_cast<QScrollArea *>(parent()->parent())) {
                    Q_UNUSED(scrollWidget);
                    QWidget *contents = (QWidget*) parent();
                    contents->setMinimumSize(maximumX + adjustMargin, maximumY + adjustMargin);
                }

            }
        }

        if(thisItemCount != prvItemCount) {
            //printf("count modified\n");

        } else if(!prvFileName.isNull() && !newFileName.isNull()) {
            //printf("filename did not change->return\n");
            int indx1 = prvFileName.indexOf(".");
            int indx2 = newFileName.indexOf(".");
            if(indx1 != -1 && indx2 != -1) {
                QString file1 = prvFileName.left(indx1);
                QString file2 = newFileName.left(indx2);
                if(file1 == file2) return;
            }
        }
        if(thisStacking != Positions){
            if (gridLayout) frame->setLayout(gridLayout);
        }
        if(newFileName.contains(".prc")) {
            fileName = newFileName;
            thisAdjust = false;
        } else {
            openFile = newFileName.split(".", SKIP_EMPTY_PARTS);
            fileName = openFile[0].append(".ui");
        }

        // this will check for file existence and when an url is defined, download the file from a http server
        filefunction.checkFileAndDownload(fileName);
        searchFile *s = new searchFile(fileName);
        QString fileNameFound = s->findFile();

        // file was not found, remove previous widget if any
        if(fileNameFound.isNull()) {
            removeIncludedWidgets();
            delete s;
            return;
        }
        delete s;

        // file was found, remove previous widget if any
        removeIncludedWidgets();

        adjustMargin = getMargin();

        // load file
        for(int j=0; j<thisItemCount; j++) {
            bool ok = false;
            int posX = 0;
            int posY = 0;
            QWidget * loadedWidget = (QWidget *) Q_NULLPTR;
            QWidget * tmp = (QWidget *) Q_NULLPTR;
            if(!fileName.contains(".prc")) {
                // load new file
                QFile *file = new QFile;
                file->setFileName(fileNameFound);
                file->open(QFile::ReadOnly);
                //symtomatic AFS check
                if (!file->isOpen()){
                    printf("can't open file %s\n",qasc(fileName));
                }else{
                    if (file->size()==0){
                        printf("file %s has size zero \n",qasc(fileName));
                    }else{
                        printf("effective load of file %s for widget %s\n", qasc(fileNameFound), qasc(this->objectName()));
                        tmp = loader.load(file, thisParent);
                    }
                }

                file->close();
                delete file;
                if(tmp == (QWidget*) Q_NULLPTR) return;
                thisLoadedWidgets.append(tmp);
                loadedWidget = tmp;
                effectiveSize= tmp->size();
                // pep file
            } else {
                ParsePepFile *parsefile = new ParsePepFile(fileNameFound);
                printf("effective load of file %s for widget %s\n", qasc(fileNameFound), qasc(this->objectName()));
                QWidget *tmp= parsefile->load(thisParent);
                if(tmp == (QWidget*) Q_NULLPTR) return;
                thisLoadedWidgets.append(tmp);
                loadedWidget = tmp;
                effectiveSize= tmp->size();
            }

            // find the row, column to add this widget
            if(thisStacking == Row) {
                if (gridLayout)  gridLayout->addWidget(loadedWidget, j, 0);
                row++;
                maxRows = row;
                maxColumns = 1;
            } else if(thisStacking == Column) {
                if (gridLayout) gridLayout->addWidget(loadedWidget, 0, j);
                column++;
                maxColumns = column;
                maxRows = 1;
            } else if(thisStacking == RowColumn){
                if(row >= nbLines) {
                    row=0;
                    column++;
                }
                if (gridLayout) gridLayout->addWidget(loadedWidget, row, column);
                row++;
                if(row > maxRows) maxRows = row;
                maxColumns = column + 1;

            } else if(thisStacking == ColumnRow){
                if(column >= nbColumns) {
                    row++;
                    column=0;
                }
                gridLayout->addWidget(loadedWidget, row, column);
                column++;
                if(column > maxColumns) maxColumns = column;
                maxRows = row + 1;
            } else {
                loadedWidget->setParent(frame);

                if(j<thisXpositionsList.count()) {
                    posX = thisXpositionsList.at(j).toInt(&ok);
                    if(!ok) posX = 0;
                }
                if(j<thisYpositionsList.count()) {
                    posY = thisYpositionsList.at(j).toInt(&ok);
                    if(!ok) posY = 0;
                }

                int xpos = qRound((double) posX * thisXfactor);
                int ypos = qRound((double) posY * thisYfactor);
                loadedWidget->move(xpos + adjustMargin/2, ypos + adjustMargin/2);
                int maxX = xpos + loadedWidget->width();
                int maxY = ypos + loadedWidget->height();
                if(maxX > maximumX) maximumX = maxX;
                if(maxY > maximumY) maximumY = maxY;
            }

            // show it
            loadedWidget->show();
        }

        if(thisAdjust  && (thisStacking != Positions)) {
            int new_width=maxColumns * effectiveSize.width() + (maxColumns-1) * thisSpacingHorizontal + adjustMargin;
            int new_height=maxRows * effectiveSize.height() + (maxRows-1) * thisSpacingVertical + adjustMargin;
            if ((new_width>width()) || (new_height>height()))   resize(new_width,new_height);
        }
        else
            if(thisAdjust && (thisStacking == Positions)) {
                if ((maximumX>width()) || (maximumY>height()))
                    resize(maximumX + adjustMargin, maximumY + adjustMargin);
            }

        if(QScrollArea* scrollWidget = qobject_cast<QScrollArea *>(parent()->parent())) {
            Q_UNUSED(scrollWidget);
            QWidget *contents = (QWidget*) parent();
            contents->setMinimumSize(maximumX + adjustMargin, maximumY + adjustMargin);
        }
        update_geometrysave();
        prvFileName = newFileName;
        prvStacking = thisStacking;
        prvItemCount = thisItemCount;
        prvMaxLines = thisMaxLines;
        prvMaxColumns = thisMaxColumns;
        prvSpacingHorizontal = thisSpacingHorizontal;
        prvSpacingVertical = thisSpacingVertical;
        prvAdjust = thisAdjust;
    }
}
void caInclude::update_position(QWidget* w,int x,int y){
//qDebug()<<"update_position:"<<w<<x<<y;


if (w){
    QVariant var=w->property("GeometryList");
    if (!var.isNull()){
        //qDebug() << "!var.isNull()"<<this->objectName();
        QVariantList integerList = var.toList();

        integerList.replace(0, x);
        integerList.replace(1, y);
        w->setProperty("GeometryList", integerList);
      }
    }
}


void caInclude::update_geometrysave(){
//    foreach(QWidget *l, thisLoadedWidgets) {


    foreach(QWidget *l, this->getChildsList()) {
        if (l){
            QString className(l->metaObject()->className());

            QList<QVariant> integerList;
            integerList.insert(0, l->geometry().x()*(1/thisXresizefactor));
            integerList.insert(1, l->geometry().y()*(1/thisYresizefactor));
            integerList.insert(2, l->geometry().width()*(1/thisXresizefactor));
            integerList.insert(3, l->geometry().height()*(1/thisYresizefactor));
            l->setProperty("GeometryList", integerList);
        //qDebug() << className<< l->objectName() << integerList;
        }
    }

    QString className(this->metaObject()->className());


    QList<QVariant> integerList;
    integerList.insert(0, this->geometry().x()*(1/thisXresizefactor));
    integerList.insert(1, this->geometry().y()*(1/thisYresizefactor));
    integerList.insert(2, this->geometry().width()*(1/thisXresizefactor));
    integerList.insert(3, this->geometry().height()*(1/thisYresizefactor));
    this->setProperty("GeometryList", integerList);
    integerList.clear();
    integerList.insert(0, 0);
    integerList.insert(1, 0);
    integerList.insert(2, this->geometry().width()*(1/thisXresizefactor));
    integerList.insert(3, this->geometry().height()*(1/thisYresizefactor));
    if (frame) frame->setProperty("GeometryList", integerList);

    //qDebug() << className << integerList;

}
QRect caInclude::scanChildsneededArea(){
    foreach(QWidget* l ,this->findChildren<QWidget *>()){
       QRect resizedata=l->childrenRect();
       if ((resizedata.width()>0) && (resizedata.height()>0))
         l->resize(resizedata.width(),resizedata.height());
    }
    return this->childrenRect();
}
void caInclude::childResizeCall(double factX,double factY){
    thisXresizefactor=factX;
    thisYresizefactor=factY;


    //foreach(QWidget *l, thisLoadedWidgets) {
//    if (getStacking()== caInclude::Positions) qDebug() << "childResizeCall :"<<this->objectName()<< thisLoadedWidgets.count()<< this->findChildren<QWidget *>().count();
//    foreach(QObject *l, this->findChildren<QObject *>()){
//        QString className(l->metaObject()->className());
//        qDebug() << className;
//        if(QGridLayout *testWidget = qobject_cast<QGridLayout *>(l)){
//            qDebug() <<"QGridLayout:" <<testWidget->contentsRect();

//        }
//        if(QHBoxLayout *testWidget = qobject_cast<QHBoxLayout *>(l)){
//            qDebug() <<"QHBoxLayout:" <<testWidget->contentsRect();
//        }
//        if(QFrame *testWidget = qobject_cast<QFrame *>(l)){
//            //testWidget->updateGeometry();
//            qDebug() <<"QFrame:"<< testWidget->objectName()<<testWidget->pos()<<testWidget->contentsRect();
//            qDebug() <<"Parent:" <<testWidget->parentWidget()->objectName()<<testWidget->parentWidget()->pos()<<testWidget->parentWidget()->contentsRect();
//        }

//    }

    //QObject
    foreach(QWidget *l, this->findChildren<QWidget *>()){
        if (l){
            QString className(l->metaObject()->className());
            //qDebug() << className;
            if(     className.contains("QMainWindow")||

                    className.contains("QWidget")  ){

                QVariant var=l->property("GeometryList");
                //qDebug() << var;
                double x,y,width,height;
                if (!var.isNull()){
                    //qDebug() << "!var.isNull()"<<this->objectName();
                    QVariantList list = var.toList();
                    x = (double) list.at(0).toInt() * factX;
                    y = (double) list.at(1).toInt() * factY;
                    width = (double) list.at(2).toInt() *factX;
                    height = (double) list.at(3).toInt() *factY;
                    //qDebug()<<"childResizeCall:"<<factX<<factY<<list.at(0).toInt()<< list.at(1).toInt()<< list.at(2).toInt()<<list.at(3).toInt();
                }else{
                    x = 0;//(double) l->x() * factX;
                    y = 0;//(double) l->y() * factY;
                    width = (double) l->width() *factX;
                    height = (double)l->height() *factY;
                }
//                if (getStacking()== caInclude::Positions) {
//                    qDebug() << "caInclude::Positions";
//                    if (this->objectName().contains("AR_LLM")){
//                        qDebug() << "Include :" << qRound(x)<< qRound(y)<< qRound(width)<< qRound(height)<<l;

//                    }
//                    //else qDebug() <<this->objectName();
//                }
                l->setGeometry(QRect(qRound(x), qRound(y), qRound(width), qRound(height)));
                l->updateGeometry();

            }
        }
    }

}


void caInclude::setLineSize(int size )
{
    thisLineSize = size;
    update();
}

void caInclude::paintEvent( QPaintEvent *event)
{
    Q_UNUSED(event);
    if(thisLineSize > 0) {
        QPainter painter( this );
        painter.setPen( QPen(thisFrameColor, thisLineSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter.drawRect(0, 0, width() - 1, height() -1);
    }
}

void caInclude::setMacro(QString const &newMacro) {
    QString Macro = newMacro.simplified();
    Macro.replace(" ", "");
    QStringList splitted = Macro.split(";");
    setMacroAndPositionsFromMacroStringList(splitted);
/*
    for(int i=0; i< thisMacro.count(); i++) {
        printf("%s\n", qasc(thisMacro[i]));
    }
    */
}

void caInclude::setMacroAndPositionsFromMacroStringList(QStringList macroList) {

    // compose the real lists (macro and x,y lists) by decoding the macro list containing macros and or positions
    thisMacro.clear();

    QStringList XpositionsList;
    QStringList YpositionsList;

    for(int i=0; i<macroList.count(); i++) {

        QString Macro = macroList[i].simplified();
        Macro.replace(" ", "");
        //printf(" MacroOrg: %s\n",qasc(Macro));
        QString pattern = QString("(?:,+|^)\\[([^,]*[^\\]]*)\\]");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRegExp rx(pattern);
        if (rx.indexIn(Macro) != -1){
         QStringList capTxt = rx.capturedTexts();

#else
        QRegularExpression rx(pattern);
        QRegularExpressionMatch match = rx.match(Macro);
        if (match.hasMatch()){
           QStringList capTxt = match.capturedTexts();
#endif

         //printf(" capTxt(%d): %s\n",capTxt.count(),qasc(capTxt[1]));
         //handle only the first one
         QStringList MacroPartPos = capTxt[1].split(",");

         if(MacroPartPos.count() > 1) {
             XpositionsList.append(MacroPartPos[0]);
             YpositionsList.append(MacroPartPos[1]);
             QString pattern2 = QString("(,+|^)\\[[^,]*[^\\]]*\\],?");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
             rx=QRegExp(pattern2);
             int index_rx=rx.indexIn(Macro);
             int length_rx=rx.matchedLength();
#else
             rx=QRegularExpression(pattern2);
             QRegularExpressionMatch match = rx.match(Macro);
             qsizetype index_rx=match.capturedStart();
             qsizetype length_rx=match.capturedLength();

#endif

             if (index_rx==0){
                Macro.remove(rx);
             }else{
                 if (Macro.length()>(index_rx+length_rx)){
                    Macro.replace(rx,",");
                 }else{
                    Macro.remove(rx);
                 }
             }

             //printf("Remove Macro: %s\n",qasc(Macro));
         }else {
             XpositionsList.append("undef");
             YpositionsList.append("undef");
         }

        }else {
            XpositionsList.append("undef");
            YpositionsList.append("undef");
        }
        //printf(" Macro: %s\n",qasc(Macro));
        thisMacro.append(Macro);
        //fflush(stdout);
    }
    setXpositionsList(XpositionsList);
    setYpositionsList(YpositionsList);
}

QStringList caInclude::getMacroList() const {
    QStringList newList;
    for(int i=0; i<thisMacro.count(); i++) {
        if(!thisXpositionsList[i].contains("undef") && !thisYpositionsList[i].contains("undef")) {
            if(thisMacro[i].length() > 0) {
               newList.append(thisMacro[i] + ", [" + thisXpositionsList[i] + "," + thisYpositionsList[i] + "]");
            } else {
               newList.append("[" + thisXpositionsList[i] + "," + thisYpositionsList[i] + "]");
            }
        } else {
            newList.append(thisMacro[i]);
        }
    }
    return newList;
}

void caInclude::setMacroList(QStringList list) {
    thisMacro = list;
    setMacroAndPositionsFromMacroStringList(thisMacro);
    updatePropertyEditorItem(this, "macro");
}

void caInclude::setXpositionsList(QStringList list)
{
    thisXpositionsList = list;
    thisFrameUpdate = true;
    setFileName(newFileName);
}

void caInclude::setYpositionsList(QStringList list)
{
    thisYpositionsList = list;
    thisFrameUpdate = true;
    setFileName(newFileName);
}

void caInclude::updateXpositionsList(int pos, int value)
{
    if(pos < thisXpositionsList.count()) {
        thisXpositionsList[pos] = QString::number(value);
    }
}

void caInclude::updateYpositionsList(int pos, int value)
{
    if(pos < thisYpositionsList.count()) {
        thisYpositionsList[pos] = QString::number(value);
    }
}

bool caInclude::getXposition(int indx, int &posX, int width, QString &pos) {
    Q_UNUSED(width);
    //qDebug()<< "thisXpositionsList: "<< thisXpositionsList;
    if(indx < thisXpositionsList.count()) {
        bool ok;
        pos =  thisXpositionsList[indx];
        if(pos.contains("undef")) {
            posX = 0;
            return true;
        }
        posX = pos.toInt(&ok);
        if(!ok) {
            posX = 0;
            return false;
        } else {
            return true;
        }
    }
    return false;
}

bool caInclude::getYposition(int indx, int &posY, int height, QString &pos) {
    Q_UNUSED(height);
    if(indx < thisYpositionsList.count()) {
        bool ok;
        pos =  thisYpositionsList[indx];
        if(pos.contains("undef")) {
            posY = 0;
            return true;
        }
        posY = pos.toInt(&ok);
        if(!ok) {
            posY = 0;
            return false;
        } else {
            return true;
        }
    }
    return false;
}

int caInclude::getXmaximum()
{
    int maximum = 0;
    QString pos;
    int posX=0;

    for(int i=0; i <  thisXpositionsList.count(); i++) {
        getXposition(i, posX, 0, pos);
        posX = qRound((double) posX * getXcorrection());
        int maxX = posX + getMargin();
        if(maxX > maximum) maximum = maxX;
    }
    return maximum;
}

int caInclude::getYmaximum() {
    int maximum = 0;
    QString pos;
    int posY=0;

    for(int i=0; i <  thisYpositionsList.count(); i++) {
        getYposition(i, posY, 0, pos);
        posY = qRound((double) posY * getYcorrection());
        int maxY = posY + getMargin();
        if(maxY > maximum) maximum = maxY;
    }
    return maximum;
}

int caInclude::getMargin() {
    int adjustMargin = 0;
    if(getFrameShape() == caInclude::Box) adjustMargin = 4*getFrameLineWidth();
    else if(getFrameShape() == caInclude::NoFrame) adjustMargin = 0;
    else adjustMargin = 2*getFrameLineWidth();
    return adjustMargin;
}




