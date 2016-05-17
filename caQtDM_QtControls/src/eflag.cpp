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

#include "eflag.h"
#include <QGridLayout>
#include <QtAlgorithms> /* for qSort() */
#include <QtDebug>

EFlag::EFlag(QWidget *parent) : QWidget(parent)
{
    numRows = 3;
    numColumns = 3;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    arrangeCells();
    mask.clear();
}

void EFlag::setDisplayMask(QString slist)
{
    unsigned int tmp;
    bool ok;
    QList <unsigned int> uilist;
    QStringList stringlist = slist.split(",", QString::SkipEmptyParts);
    foreach(QString s, stringlist)
    {
        tmp = s.toUInt(&ok);
        if(ok )
            uilist.push_back(tmp );
        else
        {
            uilist.clear();
            mask.clear();
            string_mask = QString();
            return;
        }
    }
    string_mask = slist;
    /* Converted each element successfully: call the setDisplayMask() accepting a list of unsigned int */
    setDisplayMask(uilist);
}

void EFlag::setDisplayMask(QList<unsigned int> lui)
{
    /* initialize the mask */
    mask.clear();
    for(int i = 0; i < lui.size(); i++)
    {
        QList <unsigned int>lstmp;
        lstmp.clear();
        /* Add each element in a separate list containing only the element itself */
        lstmp.push_back(lui[i]);
        mask.push_back(lstmp);
    }
}

void EFlag::setDisplayMask(QList <QList <unsigned int> > ll)
{
    mask = ll;
}

void EFlag::initDisplayMask()
{
    mask.clear();
    for (int i = 0; i < numRows; i++)
    {
        for (int j = 0; j < numColumns; j++)
        {
            QList <unsigned int> tmplist;
            tmplist.push_back(i * numColumns + j);
            mask.push_back(tmplist);
        }
    }
}

void EFlag::setBooleanDisplay(int i, QString falseString, QString trueString, QColor falseColor, QColor trueColor)
{
    while(m_trueStrings.size() <= i)
        m_trueStrings << "One";
    while(m_falseStrings.size() <= i)
        m_falseStrings << "Zero";
    while(m_trueColors.size() <= i)
        m_trueColors << QColor(Qt::green);
    while(m_falseColors.size() <= i)
        m_falseColors << QColor(Qt::red);

    cells[i]->setBooleanDisplay(falseString, trueString, falseColor, trueColor);
    m_trueStrings[i] = trueString;
    m_falseStrings[i] = falseString;

    QVariant fc = falseColor, tc = trueColor;
    m_trueColors[i] = tc;
    m_falseColors[i] = fc;
}

void EFlag::setEnumDisplay(int cell, unsigned int value, QString label, QColor color)
{
    if(cells.size() > cell)
        cells[cell]->setEnumDisplay(value, label, color);
}

void EFlag::arrangeCells()
{
    QList <unsigned int> lelem;
    foreach(ELabel *l, cells)
    {
        grid->removeWidget(l);
        delete l;
    }
    cells.clear();
    data.clear();

    for (int i = 0; i < numRows; i++)
        for (int j = 0; j < numColumns; j++)
        {
            ELabel* temp = new ELabel(this);
            temp->setMargin(0);
            temp->setLineWidth(1);
            temp->setMidLineWidth(0);
            temp->setBotTopBorderWidth(0);
            temp->setFrameShape(QFrame::Panel);
            temp->setFrameShadow(QFrame::Sunken);

            grid->addWidget(temp, i, j);
            cells.push_back(temp);
            /* Initialize the mask with one-element lists */
            temp->show();
            while(m_trueStrings.size() < 1 + j + i * numRows)
                m_trueStrings << QString("One");
            while(m_falseStrings.size() < 1 + j + i * numRows)
                m_falseStrings << QString("Zero");
            //QVariant tc = EColor(Elettra::green), fc = EColor(Elettra::red);
            QVariant tc = QColor(Qt::green), fc = QColor(Qt::red);
            while(m_trueColors.size() < 1 + i * numRows + j)
                m_trueColors << tc;
            while(m_falseColors.size() < 1 + i * numRows + j)
                m_falseColors << fc;
        }
    configureCells();
    update();
}

void EFlag::setValue(QVariant v, bool ref)
{
    last_val = v;

    if (v.type() == QVariant::List)
    {
        /* In this first case (a list is passed) we suppose
         * that the elements in the list are consistent with
         * the kind of data stored, e.g. all booleans
         */
        QList<QVariant> temp = v.toList();
        /* data is QList<QVariant> */
        data.clear();
        int i = 0;
        for(i = 0; i < mask.size(); i++ )
        {
            qSort(mask[i].begin(), mask[i].end() );
            for(int j = 0; j < mask[i].size(); j++)
            {
                data << temp.value(mask[i][j]);
            }
        }
        if(mask.size() == 0) /* no mask specified but list not empty, so reading something */
        {
            foreach(QVariant v, temp)
                data << v;
        }
    }
    else if (v.canConvert(QVariant::UInt)) /* an unsigned int is passed */
    {
        unsigned int temp = v.toUInt();
        unsigned int tmpmask = 0, value, abool = 0;
        if (data.size() != mask.size())
        {
            data.clear();
            for (int i = 0; i < mask.size(); i++)
                data << QVariant(false);
        }
        /* Iterate over the sub lists */
        for (int i = 0; i < mask.size(); i++)
        {
            /* See the desired bits and create the mask and
             * mask the value.
             * <3,4>, <14,15,16>, <18,20> are examples of
             * valid masks.
             */

            /* Let's sort each submask */
            qSort(mask[i].begin(), mask[i].end() );

            value = 0;
            int j;
            for (j = 0; j < mask[i].size(); j++)
            {
                if (mask[i][j] < 32)
                {
                    /* Create the bit mask */
                    tmpmask = 1 << mask[i][j];
                    abool = (temp & tmpmask)? 1: 0;
                    value += abool << j;
                }
                else
                    j = 0; /* This means we won't add anything */
            }
            if (j == 1) /* A mask of a single bit */
            {
                if(abool)
                    data[i] = QVariant(true);
                else
                    data[i] = QVariant(false);
            }
            else if (j > 1) /* A mask was given and was made up of two or more bits */
                data[i] = QVariant(value);
        }
        /* no mask specified: take the integer and put it as is in the data */
        if(mask.size() == 0 )
        {

            for(int n = 0; n < cells.size(); n++)
            {
                if(data.size() < n + 1)
                    data << temp;
                else
                    data[n] = temp;
            }
        }
    }

    QMutableListIterator<ELabel*> itCells(cells);
    QListIterator<QVariant>    itData(data);

    while (itCells.hasNext() && itData.hasNext())
        itCells.next()->setValue(itData.next(), ref);
}

void EFlag::configureCells()
{
    for (int i = 0; i < cells.size() && i < m_falseStrings.size() && i < m_trueStrings.size() &&
         i< m_falseColors.size() && i < m_trueColors.size(); i++)
    {
        setBooleanDisplay(i, m_falseStrings[i], m_trueStrings[i], m_falseColors[i].value<QColor>(), m_trueColors[i].value<QColor>());
        cells[i]->display();
    }
}

void EFlag::setMouseTracking(bool en)
{
    for (int i = 0; i < cells.size(); i++)
        cells[i]->setMouseTracking(en);
}

void EFlag::setTrueStrings(QString s)
{ 
    m_trueStrings = s.split(";");
    configureCells();
}

QString EFlag::trueStrings()
{
    return m_trueStrings.join(";");
}

void EFlag::setFalseStrings(QString s)
{
    m_falseStrings = s.split(";",QString::SkipEmptyParts);
    configureCells();
}

QString EFlag::falseStrings() 
{
    return m_falseStrings.join(";");
}

void EFlag::setTrueColors(QString c)
{
    m_trueColors.clear();
    QStringList l = c.split(";",QString::SkipEmptyParts);
    foreach(QString s, l)
        m_trueColors << QVariant(QColor(s.toUInt()));
    configureCells();
}

QString EFlag::trueColors()
{
    QStringList temp;
    foreach(QVariant c, m_trueColors)
        temp << QString().setNum((unsigned int) c.value<QColor>().rgba());
    return temp.join(";");
}

void EFlag::setFalseColors(QString c)
{
    m_falseColors.clear();
    QStringList l = c.split(";",QString::SkipEmptyParts);
    foreach(QString s, l)
        m_falseColors << QVariant(QColor(s.toUInt()));
    configureCells();
}

QString EFlag::falseColors()
{
    QStringList temp;
    foreach(QVariant c, m_falseColors)
        temp << QString().setNum((unsigned int) c.value<QColor>().rgba());
    return temp.join(";");
}

void EFlag::setTrueColorList(const QList<QColor> &cl)
{
    m_trueColors.clear();
    foreach(QColor c, cl)
        m_trueColors << QVariant(c);
    configureCells();
}

QList<QColor> EFlag::trueColorList() const
{
    QList<QColor> ret;
    foreach(QVariant c, m_trueColors)
        ret << c.value<QColor>();
    return ret;
}

void EFlag::setFalseColorList(const QList<QColor> &cl)
{
    m_falseColors.clear();
    foreach(QColor c, cl)
        m_falseColors << QVariant(c);
    configureCells();
}

QList<QColor> EFlag::falseColorList() const
{
    QList<QColor> ret;
    foreach(QVariant c, m_falseColors)
        ret << c.value<QColor>();
    return ret;
}

void EFlag::setFontScaleMode(ESimpleLabel::ScaleMode m)
{
    for (int i = 0; i < cells.size(); i++) {
        cells[i]->setFontScaleMode(m);
        if(m == ESimpleLabel::None) {  // in this case we may use font
            cells[i]->setFont(this->font());
        }
    }
}

ESimpleLabel::ScaleMode EFlag::fontScaleMode()
{
    if(cells.size())
        return cells.at(0)->fontScaleMode();
    return ESimpleLabel::None;
}

void EFlag::setTextAlignment(alignmentHor m)
{
    for (int i = 0; i < cells.size(); i++) {
        if(m == center) {
           cells[i]->setAlignment(Qt::AlignCenter);
        } else if (m == right) {
           cells[i]->setAlignment(Qt::AlignRight);
        } else if (m == left) {
           cells[i]->setAlignment(Qt::AlignLeft);
        }
    }
}

void EFlag::setFont(const QFont & font)
{
    for (int i = 0; i < cells.size(); i++) {
       cells[i]->setFont(font);
    }
}




