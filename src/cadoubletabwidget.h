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

#ifndef DOUBLEWIDGET_H
#define DOUBLEWIDGET_H
#include <QtGui>
#include <QWidget>
#include <QTabBar>
#include <QPushButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>

#include <qtcontrols_global.h>

typedef char string40[40];

class QTCON_EXPORT caDoubleTabWidget : public QWidget
{

    Q_OBJECT

    Q_PROPERTY(QString itemsHorizontal READ getItemsHor WRITE setItemsHor)
    Q_PROPERTY(QString itemsVertical READ getItemsVer WRITE setItemsVer)
    Q_PROPERTY(QString itemsVerticalPadding READ getItemsPadding WRITE setItemsPadding)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)

public:

    caDoubleTabWidget(QWidget *parent);


    QString getItemsHor() const {return thisHorItems.join(";");}
    void setItemsHor(QString const &items) {thisHorItems = items.split(";"); removeTabs(0); addSampleWidget(0);}

    QString getItemsVer() const {return thisVerItems.join(";");}
    void setItemsVer(QString const &items) {thisVerItems = items.split(";"); removeTabs(1); addSampleWidget(1);}

    QString getItemsPadding() const {return thisVerPadding.join(";");}
    void setItemsPadding(QString const &items);

    void addSampleWidget(int);
    void removeTabs(int);

    int count() const;
    int currentIndex() const;
    QWidget *widget(int index);
    int  lookupPage(int row, int col);
    void storePage(int index, int row, int col);
    void setFont(int dir);

public slots:
    void setCurrentIndex(int index);
    void setRow(int r);
    void setCol(int c);
    void addPage(QWidget *page);
    void insertPage(int index, QWidget *page);
    void removePage(int index);

signals:
    void currentChanged(int indx);
    void currentIndexChanged(int index);

protected:
    virtual void fontChange(const QFont & oldFont);

private:
    struct twoInts {
        int r;
        int c;
    };

    QStringList	thisHorItems;
    QStringList	thisVerItems;
    QStringList	thisVerPadding;
    int row;
    int col;
    QTabBar *hTabBar;
    QButtonGroup *vTabBar;
    QStackedWidget *viewPort;
    QMap<int, twoInts> lookup;
    QLineEdit *tableIndex;
    bool addPages;
    QVBoxLayout*  buttonLayout;
    int vCount;
};

#endif
