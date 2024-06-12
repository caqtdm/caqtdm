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
#include "limitsCartesianplotDialog.h"

limitsCartesianplotDialog::limitsCartesianplotDialog(caCartesianPlot *w, MutexKnobData *data, const QString &title, QWidget *parent) : QWidget(parent)
{
    bool ok1, ok2;
    QString xmin, xmax,  ymin, ymax;
    int thisWidth = 650;
    int thisHeight = 150;
    int showMax = false;
    CartesianPlot = w;
    monData = data;

    QGridLayout *Layout = new QGridLayout;
    Qt::WindowFlags flags = Qt::Dialog;
    setWindowFlags(flags);
    setWindowModality (Qt::WindowModal);

#if defined(MOBILE_IOS)
    if(qApp->primaryScreen()->size().height() < 500) {
        thisWidth=430;  // normal for iphone
        thisHeight=150;
    }
    Specials special;
    special.setNewStyleSheet(this, qApp->primaryScreen()->size(), 16, 10);
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, QSize(thisWidth,thisHeight), qApp->primaryScreen()->availableGeometry()));
#elif defined(MOBILE_ANDROID)
    QPalette palette;
    palette.setBrush(QPalette::Window, QColor(255,255,224,255));
    setPalette(palette);
    setAutoFillBackground(true);
    showMax = true;
#else
    move(parent->x() + parent->width() / 2 - thisWidth / 2 , parent->y() + parent->height() /2 -thisHeight/2);
#endif

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGroupBox *groupBox = new QGroupBox("cartesian plot scaling");

    // treat X
    QLabel *xLabel = new QLabel("X axis ");

    // add combobox and set correct item
    xComboBox = new QComboBox;
    xComboBox->addItem("auto");
    xComboBox->addItem("channel");
    xComboBox->addItem("user");

    xminLineEdit = new QLineEdit("");
    xmaxLineEdit = new QLineEdit("");
#if QWT_VERSION < 0x060100
    double Xmin = CartesianPlot->axisScaleDiv(caCartesianPlot::xBottom)->lowerBound();
    double Xmax = CartesianPlot->axisScaleDiv(caCartesianPlot::xBottom)->upperBound();
#else
    double Xmin = CartesianPlot->axisScaleDiv(caCartesianPlot::xBottom).lowerBound();
    double Xmax = CartesianPlot->axisScaleDiv(caCartesianPlot::xBottom).upperBound();
#endif
    xminLineEdit->setText(QString::number(Xmin, 'g'));
    xmaxLineEdit->setText(QString::number(Xmax, 'g'));

    if(CartesianPlot->getXscaling() == caCartesianPlot::Auto) xComboBox->setCurrentIndex(0);
    else if(CartesianPlot->getXscaling() == caCartesianPlot::Channel) xComboBox->setCurrentIndex(1);
    else if(CartesianPlot->getXscaling() == caCartesianPlot::User) xComboBox->setCurrentIndex(2);

    QLabel *xaxisPresent = new QLabel("axis visible");

    xCheckBox= new QCheckBox();
    xCheckBox->setChecked(CartesianPlot->getXaxisEnabled());

    XaxisType = new QComboBox;
    XaxisType->addItem("linear");
    XaxisType->addItem("log10");
    XaxisType->addItem("time");
    if(CartesianPlot->getXaxisType() == caCartesianPlot::linear) XaxisType->setCurrentIndex(0);
    else if(CartesianPlot->getXaxisType() == caCartesianPlot::log10) XaxisType->setCurrentIndex(1);
    else XaxisType->setCurrentIndex(2);

    // add all to layout
    Layout->addWidget(xLabel,    0, 0);
    Layout->addWidget(xComboBox, 0, 1);
    Layout->addWidget(xminLineEdit, 0, 2);
    Layout->addWidget(xmaxLineEdit, 0, 3);
    Layout->addWidget(xaxisPresent,  0, 5);
    Layout->addWidget(xCheckBox, 0, 6);
    Layout->addWidget(XaxisType, 0, 4);

    // treat Y
    QLabel *yLabel = new QLabel("Y axis ");

    // add combobox and set correct item
    yComboBox = new QComboBox;
    yComboBox->addItem("auto");
    yComboBox->addItem("channel");
    yComboBox->addItem("user");

    yminLineEdit = new QLineEdit("");
    ymaxLineEdit = new QLineEdit("");
#if QWT_VERSION < 0x060100
    double Ymin = CartesianPlot->axisScaleDiv(caCartesianPlot::yLeft)->lowerBound();
    double Ymax = CartesianPlot->axisScaleDiv(caCartesianPlot::yLeft)->upperBound();
#else
    double Ymin = CartesianPlot->axisScaleDiv(caCartesianPlot::yLeft).lowerBound();
    double Ymax = CartesianPlot->axisScaleDiv(caCartesianPlot::yLeft).upperBound();
#endif
    yminLineEdit->setText(QString::number(Ymin, 'g'));
    ymaxLineEdit->setText(QString::number(Ymax, 'g'));

    if(CartesianPlot->getYscaling() == caCartesianPlot::Auto) yComboBox->setCurrentIndex(0);
    else if(CartesianPlot->getYscaling() == caCartesianPlot::Channel) yComboBox->setCurrentIndex(1);
    else if(CartesianPlot->getYscaling() == caCartesianPlot::User) yComboBox->setCurrentIndex(2);

    QLabel *yaxisPresent = new QLabel("axis visible");

    yCheckBox = new QCheckBox();
    yCheckBox->setChecked(CartesianPlot->getYaxisEnabled());

    YaxisType = new QComboBox;
    YaxisType->addItem("linear");
    YaxisType->addItem("log10");
    if(CartesianPlot->getYaxisType() == caCartesianPlot::log10) YaxisType->setCurrentIndex(1);
    else YaxisType->setCurrentIndex(0);

    // add all to layout
    Layout->addWidget(yLabel,    1, 0);
    Layout->addWidget(yComboBox, 1, 1);
    Layout->addWidget(yminLineEdit, 1, 2);
    Layout->addWidget(ymaxLineEdit, 1, 3);
    Layout->addWidget(yaxisPresent,  1, 5);
    Layout->addWidget(yCheckBox, 1, 6);
    Layout->addWidget(YaxisType, 1, 4);

    // box with buttons
    buttonBox = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Return" );
    connect( button, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::RejectRole );

    button = new QPushButton( "Apply" );
    connect( button, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    buttonBox->addButton(button, QDialogButtonBox::ApplyRole );

    Layout->addWidget(buttonBox, 2, 0, 1, -1);

    groupBox->setLayout(Layout);
    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

    // when one of the limits is given by a channel, we do not allow to change anything

    xmin = xminLineEdit->text().trimmed();
    (void) xmin.toDouble(&ok1);
    xmax = xmaxLineEdit->text().trimmed();
    (void) xmax.toDouble(&ok2);
    if((!ok1 || !ok2) && (CartesianPlot->getXscaling() == caCartesianPlot::Channel)) {
        qDebug() << "not valid values, probably channels";
        xComboBox->setEnabled(false);
        xminLineEdit->setEnabled(false);
        xmaxLineEdit->setEnabled(false);
    }
    ymin = yminLineEdit->text();
    (void) ymin.toDouble(&ok1);
    ymax = ymaxLineEdit->text();
    (void) ymax.toDouble(&ok2);
    if((!ok1 || !ok2) && (CartesianPlot->getYscaling() == caCartesianPlot::Channel)) {
        qDebug() << "not valid values, probably channels";
        yComboBox->setEnabled(false);
        yminLineEdit->setEnabled(false);
        ymaxLineEdit->setEnabled(false);
    }

    setWindowTitle(title);

    if(!showMax) showNormal();
    else showMaximized();
}

void limitsCartesianplotDialog::cancelClicked()
{
    close();
}

void limitsCartesianplotDialog::applyClicked()
{
    bool ok1, ok2;
    QString xLimits, yLimits;
    QString xmin, xmax,  ymin, ymax;
    QStringList list;

    int indx = XaxisType->currentIndex();
    if(indx == 0) CartesianPlot->setXaxisType(caCartesianPlot::linear);
    else if(indx == 1) CartesianPlot->setXaxisType(caCartesianPlot::log10);
    else if(indx == 2) CartesianPlot->setXaxisType(caCartesianPlot::time);

    int indy = YaxisType->currentIndex();
    if(indy == 0) CartesianPlot->setYaxisType(caCartesianPlot::linear);
    else if(indy == 1) CartesianPlot->setYaxisType(caCartesianPlot::log10);

    indx = xComboBox->currentIndex();
    indy = yComboBox->currentIndex();

    xmin = xminLineEdit->text().trimmed();
    (void) xmin.toDouble(&ok1);
    xmax = xmaxLineEdit->text().trimmed();
    (void) xmax.toDouble(&ok2);
    if(ok1 && ok2) {
        xLimits = xmin; xLimits.append(";"); xLimits.append(xmax);
    } else {
        xLimits = CartesianPlot->getXaxisLimits();
        list = xLimits.split(";", SKIP_EMPTY_PARTS);
        xminLineEdit->setText(list.at(0));
        xmaxLineEdit->setText(list.at(1));
    }

    ymin = yminLineEdit->text();
    (void) ymin.toDouble(&ok1);
    ymax = ymaxLineEdit->text();
    (void) ymax.toDouble(&ok2);
    if(ok1 && ok2) {
        yLimits = ymin; yLimits.append(";"); yLimits.append(ymax);
    } else {
        yLimits = CartesianPlot->getYaxisLimits();
        list = yLimits.split(";", SKIP_EMPTY_PARTS);
        yminLineEdit->setText(list.at(0));
        ymaxLineEdit->setText(list.at(1));
    }

    // set x scale
    if(indx == 0) { // auto
        CartesianPlot->setXscaling(caCartesianPlot::Auto);
        qDebug() << "set xlimits to auto";
    } else if(indx == 1) { // channel
        CartesianPlot->setXscaling(caCartesianPlot::Channel);
        qDebug() << "set xlimits to channel";
        QString pvs = CartesianPlot->getPV(0);
        QStringList vars = pvs.split(";");
        if((vars.size()== 2) || (vars.at(0).trimmed().length() > 0)) {
            knobData *kPtr = monData->getMutexKnobDataPV(CartesianPlot, vars.at(0).trimmed());
             if(kPtr != (knobData*) Q_NULLPTR) {
                if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) {
                    qDebug() << "set to channel limits" << kPtr->edata.lower_disp_limit << kPtr->edata.upper_disp_limit;
                    CartesianPlot->setScaleX(kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                } else {
                    //qDebug() << "set to auto";
                    CartesianPlot->setXscaling(caCartesianPlot::Auto);
                }
             }
        }

    } else if(indx == 2) { // user
        CartesianPlot->setXscaling(caCartesianPlot::User);
        qDebug() << "set xlimits to" << xLimits;
        CartesianPlot->setXaxisLimits(xLimits);
    }

    // set y scale
    if(indy == 0) { // auto
        CartesianPlot->setYscaling(caCartesianPlot::Auto);
    } else if(indy == 1) { // channel
        CartesianPlot->setYscaling(caCartesianPlot::Channel);

        QString pvs = CartesianPlot->getPV(0);
        QStringList vars = pvs.split(";");
        if((vars.size()== 2) || (vars.at(1).trimmed().length() > 0)) {
            knobData *kPtr = monData->getMutexKnobDataPV(CartesianPlot, vars.at(1).trimmed());
             if(kPtr != (knobData*) Q_NULLPTR) {
                if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) {
                    //qDebug() << "set to channel limits" << kPtr->edata.lower_disp_limit << kPtr->edata.upper_disp_limit;
                    CartesianPlot->setScaleY(kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                } else {
                    //qDebug() << "set to auto";
                    CartesianPlot->setYscaling(caCartesianPlot::Auto);
                }
             }
        }
    } else if(indy == 2) { // user
        CartesianPlot->setYscaling(caCartesianPlot::User);
        qDebug() << "set ylimits to" << yLimits;
        CartesianPlot->setYaxisLimits(yLimits);
    }

    CartesianPlot->setXaxisEnabled(xCheckBox->isChecked());
    CartesianPlot->setYaxisEnabled(yCheckBox->isChecked());
    CartesianPlot->updateLegendsPV();
}

void limitsCartesianplotDialog::exec()
{
    connect(buttonBox, SIGNAL(rejected()), &loop, SLOT(quit()) );
    connect(buttonBox, SIGNAL(accepted()), &loop, SLOT(quit()) );
    loop.exec();
    close();
    deleteLater();
}

void limitsCartesianplotDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    loop.quit();
}

void limitsCartesianplotDialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen(Qt::black, 3, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawRoundedRect(5, 5, width()-7, height()-7, 3, 3);

    QWidget::paintEvent(e);
}


