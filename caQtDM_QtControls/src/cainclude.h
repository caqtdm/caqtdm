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

#ifndef CAINCLUDE_H
#define CAINCLUDE_H

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <qtcontrols_global.h>

#include "caPropHandleDefs.h"

#define PRC 1

#ifdef PRC
   #include "parsepepfile.h"
#endif

class  QTCON_EXPORT caInclude : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString macro READ getMacro WRITE setMacro  DESIGNABLE inactiveButVisible())
    Q_PROPERTY(QStringList macroList READ getMacroList WRITE setMacroList STORED false)

    Q_PROPERTY(QString filename READ getFileName WRITE setFileName)
    Q_PROPERTY(Stacking stacking READ getStacking WRITE setStacking)
    Q_PROPERTY(int numberOfItems READ getItemCount WRITE setItemCount)
    Q_PROPERTY(int maximumLines READ getMaxLines WRITE setMaxLines DESIGNABLE isPropertyVisible(maximumLines))
    Q_PROPERTY(int maximumColumns READ getMaxColumns WRITE setMaxColumns DESIGNABLE isPropertyVisible(maximumColumns))
    Q_ENUMS(Stacking)
    Q_PROPERTY(bool adjustSizeToContents READ getAdjustSize WRITE setAdjustSize)
    Q_PROPERTY(int verticalSpacing READ getSpacingVertical WRITE setSpacingVertical)
    Q_PROPERTY(int horizontalSpacing READ getSpacingHorizontal WRITE setSpacingHorizontal)

    Q_ENUMS(myShapes)
    Q_PROPERTY(myShapes frameShape READ getFrameShape WRITE setFrameShape)
    Q_PROPERTY(QFrame::Shadow frameShadow READ getFrameShadow WRITE setFrameShadow)
    Q_PROPERTY(int frameLineWidth READ getFrameLineWidth WRITE setFrameLineWidth)
    Q_PROPERTY(QColor frameColor READ getFrameColor WRITE setFrameColor)

    // this will prevent user interference
     Q_PROPERTY(QString styleSheet READ styleSheet WRITE noStyle DESIGNABLE false)

#include "caVisibProps.h"
#include "caVisibDefs.h"

public:
#include "caPropHandle.h"

    void noStyle(QString style) {Q_UNUSED(style);}

    enum myShapes {NoFrame=0, Box, Panel};

    enum Properties { maximumLines = 0, numberofItems, maximumColumns};
    enum Stacking {Row=0, Column, RowColumn, ColumnRow};
    Stacking getStacking() const { return thisStacking; }
    void setStacking(Stacking stacking);

    bool getAdjustSize() { return thisAdjust; }
    void setAdjustSize(bool adjust) { thisAdjust = adjust; setFileName(newFileName);}

    int getItemCount() const { return thisItemCount;}
    void setItemCount(int count) {if(count > 0) thisItemCount = count; else thisItemCount=1; setFileName(newFileName); prvItemCount = thisItemCount;}

    int getMaxLines() const { return thisMaxLines;}
    void setMaxLines(int count) {if(count > 0) thisMaxLines = count; else thisMaxLines=1; setFileName(newFileName); prvMaxLines = thisMaxLines;}

    int getMaxColumns() const { return thisMaxColumns;}
    void setMaxColumns(int count) {if(count > 0) thisMaxColumns = count; else thisMaxColumns=1; setFileName(newFileName); prvMaxColumns = thisMaxColumns;}

    int getSpacingVertical() const {return thisSpacingVertical;}
    int getSpacingHorizontal() const {return thisSpacingHorizontal;}
    void setSpacingVertical(int spacing) {thisSpacingVertical = spacing; setAdjustSize(thisAdjust); prvSpacingVertical = thisSpacingVertical;}
    void setSpacingHorizontal(int spacing) {thisSpacingHorizontal = spacing; setAdjustSize(thisAdjust); prvSpacingHorizontal = thisSpacingHorizontal;}

    myShapes getFrameShape() const {return thisFrameShape;}
    void setFrameShape(myShapes shape) {thisFrameShape = shape; thisFrameUpdate = true; setFileName(newFileName);}
    QFrame::Shadow getFrameShadow() const {return thisFrameShadow;}
    void setFrameShadow(QFrame::Shadow shadow) {thisFrameShadow = shadow; thisFrameUpdate = true; setFileName(newFileName);}
    int getFrameLineWidth() const {return thisFrameLineWidth;}
    void setFrameLineWidth(int lineWidth) {thisFrameLineWidth = lineWidth; thisFrameUpdate = true; setFileName(newFileName);}
    void setFrameColor(QColor c) {thisFrameColor = c; thisFrameUpdate = true; setFileName(newFileName);}
    QColor getFrameColor() const {return thisFrameColor;}

    caInclude( QWidget *parent = 0 );
    ~caInclude();

    QString getFileName() const {return newFileName;}
    void setFileName(QString const &filename);

    QString getMacro() const {return thisMacro.join(";");}
    void setMacro(QString const &newMacro) {thisMacro = newMacro.split(";");}
    QStringList getMacroList() const {return thisMacro;}
    void setMacroList(QStringList list) {thisMacro = list; updatePropertyEditorItem(this, "macro");}

    void setLineSize( int size );

    void removeIncludedWidgets();

    bool isPropertyVisible(Properties property);
    void setPropertyVisible(Properties property, bool visible);

public slots:
    void animation(QRect p) {
#include "animationcode.h"
    }

protected:
      void paintEvent( QPaintEvent* );

private:

    bool designerVisible[10];
    QString newFileName;
    QString prvFileName;
    QStringList thisMacro;
    QColor thisBackColor;
    QWidget *thisParent;
    int thisLineSize;
    QGridLayout *gridLayout;
    QList<QWidget *> thisLoadedWidgets;
    bool loadIncludes;
    Stacking thisStacking, prvStacking;
    int thisItemCount, prvItemCount;
    int thisMaxLines, prvMaxLines;
    int thisMaxColumns, prvMaxColumns;
    QSize effectiveSize;
    bool thisAdjust, prvAdjust;
    int thisSpacingVertical, thisSpacingHorizontal;
    int prvSpacingVertical, prvSpacingHorizontal;
    QFrame *frame;
    QHBoxLayout *boxLayout;
    myShapes thisFrameShape;
    QFrame::Shadow thisFrameShadow;
    QColor thisFrameColor;
    int thisFrameLineWidth;
    bool thisFrameUpdate;
    QPalette thisPalette;

#ifdef PRC
    ParsePepFile *pepfile;
#endif
};

#endif
