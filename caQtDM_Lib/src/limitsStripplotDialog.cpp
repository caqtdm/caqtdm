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

#include <QtGui>
#include "limitsStripplotDialog.h"

limitsStripplotDialog::limitsStripplotDialog(caStripPlot *w, MutexKnobData *data, const QString &title, QWidget *parent) : QWidget(parent)
{
    bool showMax = false;
    int thisWidth = 650;
    int thisHeight = 150;
    StripPlot = w;
    monData = data;

    QGridLayout *Layout = new QGridLayout;
    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

    QString text = StripPlot->getPVS();
    vars = text.split(";", QString::SkipEmptyParts);

#if defined(MOBILE_IOS)
    Specials special;
    if(qApp->desktop()->size().height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=100 + vars.size() * 13;
    } else {
        thisHeight=100 + vars.size() * 20;
    }
    special.setNewStyleSheet(this, qApp->desktop()->size());
    QPalette palette;
    palette.setBrush(QPalette::Background, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(thisWidth,thisHeight), qApp->desktop()->availableGeometry()));
#elif defined(MOBILE_ANDROID)
    QPalette palette;
    palette.setBrush(QPalette::Background, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    showMax = true;
#else
    move(parent->x() + parent->width() / 2 - thisWidth / 2 , parent->y() + parent->height() /2 -thisHeight/2);
#endif

    for(int i=0; i< vars.size(); i++) {
        QString pv = vars.at(i).trimmed();
        if(pv.size() > 0) {
            QLabel *channelLabel = new QLabel(pv);

            minComboBox[i] = new QComboBox;
            minComboBox[i]->addItem("channel");
            minComboBox[i]->addItem("user");
            if(StripPlot->getYscalingMin(i) == caStripPlot::Channel) minComboBox[i]->setCurrentIndex(0);
            else minComboBox[i]->setCurrentIndex(1);

            double minY = StripPlot->getYaxisLimitsMin(i);
            text = text.setNum(minY);
            minLineEdit[i] = new QLineEdit(text);

            maxComboBox[i] = new QComboBox;
            maxComboBox[i]->addItem("channel");
            maxComboBox[i]->addItem("user");
            if(StripPlot->getYscalingMax(i) == caStripPlot::Channel) maxComboBox[i]->setCurrentIndex(0);
            else maxComboBox[i]->setCurrentIndex(1);

            double maxY = StripPlot->getYaxisLimitsMax(i);
            text = text.setNum(maxY);
            maxLineEdit[i] = new QLineEdit(text);

            Layout->addWidget(channelLabel,    i, 0);
            Layout->addWidget(minComboBox[i],  i, 1);
            Layout->addWidget(minLineEdit[i],  i, 2);
            Layout->addWidget(maxComboBox[i],  i, 3);
            Layout->addWidget(maxLineEdit[i],  i, 4);
        }
    }

    QLabel *YaxisScalingLabel = new QLabel("Y Scaling :");
    YaxisScaling = new QComboBox;
    YaxisScaling->addItem("fixedScale");
    YaxisScaling->addItem("autoScale");
    if(StripPlot->getYaxisScaling() == caStripPlot::fixedScale) YaxisScaling->setCurrentIndex(0);
    else YaxisScaling->setCurrentIndex(1);

    QLabel *YaxisTypeLabel = new QLabel("Y axis :");
    YaxisType = new QComboBox;
    YaxisType->addItem("linear");
    YaxisType->addItem("log10");
    if(StripPlot->getYaxisType() == caStripPlot::log10) YaxisType->setCurrentIndex(1);
    else YaxisType->setCurrentIndex(0);

    Layout->addWidget(YaxisScalingLabel,  vars.size(), 1);
    Layout->addWidget(YaxisScaling,  vars.size(), 2);

    Layout->addWidget(YaxisTypeLabel,  vars.size(), 3);
    Layout->addWidget(YaxisType,  vars.size(), 4);

    buttonBox = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Return" );
    connect( button, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::RejectRole );

    button = new QPushButton( "Apply" );
    connect( button, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::ApplyRole );

    Layout->addWidget(buttonBox, vars.size(), 0);

    setLayout(Layout);
    setWindowTitle(title);

    if(!showMax) showNormal();
    else showMaximized();
}

void limitsStripplotDialog::cancelClicked()
{
    close();
}

void limitsStripplotDialog::applyClicked()
{
    bool ok;
    QVariant var=StripPlot->property("MonitorList");
    QVariantList list = var.toList();
    int nbMonitors = list.at(0).toInt();

    for(int i=0; i< qMin(vars.size(), nbMonitors); i++) {
        QString pv = vars.at(i).trimmed();
        if(pv.size() > 0) {

            knobData *ptr = monData->GetMutexKnobDataPtr(list.at(i+1).toInt());
            if(ptr == (knobData *) 0) break;

            int indx = minComboBox[i]->currentIndex();
            QString text = minLineEdit[i]->text();
            double minY = text.toDouble(&ok);
            if(ok) {
                if(indx == 1) { // user
                        StripPlot->setYaxisLimitsMin(i, minY);
                        StripPlot->setYscalingMin(i, caStripPlot::User);
                } else {
                        StripPlot->setYscalingMin(i, caStripPlot::Channel);
                        StripPlot->setYaxisLimitsMin(i, ptr->edata.lower_disp_limit);
                }
            }

            indx = maxComboBox[i]->currentIndex();
            text = maxLineEdit[i]->text();
            double maxY = text.toDouble(&ok);
            if(ok) {
                if(indx == 1) { // user
                        StripPlot->setYaxisLimitsMax(i, maxY);
                        StripPlot->setYscalingMax(i, caStripPlot::User);

                } else {
                        StripPlot->setYscalingMax(i, caStripPlot::Channel);
                        StripPlot->setYaxisLimitsMax(i, ptr->edata.upper_disp_limit);
                }
            }
            if(StripPlot->getYaxisLimitsMin(i) == StripPlot->getYaxisLimitsMax(i)) {
                StripPlot->setYaxisLimitsMin(i, 0.0);
                StripPlot->setYaxisLimitsMax(i, 10.0);
            }
            if(i==0) {
                double ymin = StripPlot->getYaxisLimitsMin(0);
                double ymax = StripPlot->getYaxisLimitsMax(0);
                StripPlot->setYscale(ymin, ymax);
            }
        }
    }

    int indx = YaxisType->currentIndex();
    if(indx == 0) StripPlot->setYaxisType(caStripPlot::linear);
    else if(indx == 1) StripPlot->setYaxisType(caStripPlot::log10);

    indx = YaxisScaling->currentIndex();
    if(indx == 0) StripPlot->setYaxisScaling(caStripPlot::fixedScale);
    else if(indx == 1) StripPlot->setYaxisScaling(caStripPlot::autoScale);

    StripPlot->UpdateScaling();
}

void limitsStripplotDialog::exec()
{
    connect(buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()) );
    connect(buttonBox, SIGNAL(accepted()), &loop, SLOT(quit()) );
    loop.exec();
    close();
    deleteLater();
}

void limitsStripplotDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

void limitsStripplotDialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-7, 3, 3);

    QWidget::paintEvent(e);
}

