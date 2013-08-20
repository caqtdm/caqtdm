#include <QtGui>

#include "limitsCartesianplotDialog.h"

limitsCartesianplotDialog::limitsCartesianplotDialog(caCartesianPlot *w, MutexKnobData *data, const QString &title, QWidget *parent) : QDialog(parent)
{
    bool ok1, ok2;
    QString xmin, xmax,  ymin, ymax;
    QStringList list;
    CartesianPlot = w;
    monData = data;
    QGridLayout *Layout = new QGridLayout;

    // treat X
    QLabel *xLabel = new QLabel("X axis ");

    // add combobox and set correct item
    xComboBox = new QComboBox;
    xComboBox->addItem("auto");
    xComboBox->addItem("channel");
    xComboBox->addItem("user");

    if(CartesianPlot->getXscaling() == caCartesianPlot::Auto) xComboBox->setCurrentIndex(0);
    else if(CartesianPlot->getXscaling() == caCartesianPlot::Channel) xComboBox->setCurrentIndex(1);
    else if(CartesianPlot->getXscaling() == caCartesianPlot::User) xComboBox->setCurrentIndex(2);

    // add linedits with correct values
    QString xLimits = CartesianPlot->getXaxisLimits();
    list = xLimits.split(";", QString::SkipEmptyParts);
    xminLineEdit = new QLineEdit(list.at(0));
    xmaxLineEdit = new QLineEdit(list.at(1));

    xCheckBox= new QCheckBox();
    xCheckBox->setChecked(CartesianPlot->getXaxisEnabled());

    // add all to layout
    Layout->addWidget(xLabel,    0, 0);
    Layout->addWidget(xComboBox, 0, 1);
    Layout->addWidget(xminLineEdit, 0, 2);
    Layout->addWidget(xmaxLineEdit, 0, 3);
    Layout->addWidget(xCheckBox, 0, 4);

    // treat Y
    QLabel *yLabel = new QLabel("Y axis ");

    // add combobox and set correct item
    yComboBox = new QComboBox;
    yComboBox->addItem("auto");
    yComboBox->addItem("channel");
    yComboBox->addItem("user");

    if(CartesianPlot->getYscaling() == caCartesianPlot::Auto) yComboBox->setCurrentIndex(0);
    else if(CartesianPlot->getYscaling() == caCartesianPlot::Channel) yComboBox->setCurrentIndex(1);
    else if(CartesianPlot->getYscaling() == caCartesianPlot::User) yComboBox->setCurrentIndex(2);

    // // add linedits with correct values
    QString yLimits = CartesianPlot->getYaxisLimits();
    list = yLimits.split(";", QString::SkipEmptyParts);
    yminLineEdit = new QLineEdit(list.at(0));
    ymaxLineEdit = new QLineEdit(list.at(1));

    yCheckBox = new QCheckBox();
    yCheckBox->setChecked(CartesianPlot->getYaxisEnabled());

    // add all to layout
    Layout->addWidget(yLabel,    1, 0);
    Layout->addWidget(yComboBox, 1, 1);
    Layout->addWidget(yminLineEdit, 1, 2);
    Layout->addWidget(ymaxLineEdit, 1, 3);
    Layout->addWidget(yCheckBox, 1, 4);

    // box with buttons
    QDialogButtonBox *box = new QDialogButtonBox( Qt::Horizontal );
    QPushButton *button = new QPushButton( "Return" );
    connect( button, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
    box->addButton(button, QDialogButtonBox::RejectRole );

    button = new QPushButton( "Apply" );
    connect( button, SIGNAL(clicked()), this, SLOT(applyClicked()) );
    box->addButton(button, QDialogButtonBox::ApplyRole );

    Layout->addWidget(box, 2, 0);

    setLayout(Layout);

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
}

void limitsCartesianplotDialog::cancelClicked()
{
    reject();
}

void limitsCartesianplotDialog::applyClicked()
{
    bool ok1, ok2;
    QString xLimits, yLimits;
    QString xmin, xmax,  ymin, ymax;
    QStringList list;
    int indx = xComboBox->currentIndex();
    int indy = yComboBox->currentIndex();

    xmin = xminLineEdit->text().trimmed();
    (void) xmin.toDouble(&ok1);
    xmax = xmaxLineEdit->text().trimmed();
    (void) xmax.toDouble(&ok2);
    if(ok1 && ok2) {
        xLimits = xmin; xLimits.append(";"); xLimits.append(xmax);
    } else {
        xLimits = CartesianPlot->getXaxisLimits();
        list = xLimits.split(";", QString::SkipEmptyParts);
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
        list = yLimits.split(";", QString::SkipEmptyParts);
        yminLineEdit->setText(list.at(0));
        ymaxLineEdit->setText(list.at(1));
    }

    // set x scale
    if(indx == 0) { // auto
        CartesianPlot->setXscaling(caCartesianPlot::Auto);
    } else if(indx == 1) { // channel
        CartesianPlot->setXscaling(caCartesianPlot::Channel);

        QString pvs = CartesianPlot->getPV(0);
        QStringList vars = pvs.split(";");
        if((vars.size()== 2) || (vars.at(0).trimmed().length() > 0)) {
            knobData *kPtr = monData->getMutexKnobDataPV(vars.at(0).trimmed());
             if(kPtr != (knobData*) 0) {
                if(kPtr->edata.lower_disp_limit != kPtr->edata.upper_disp_limit) {
                    //qDebug() << "set to channel limits" << kPtr->edata.lower_disp_limit << kPtr->edata.upper_disp_limit;
                    CartesianPlot->setScaleX(kPtr->edata.lower_disp_limit, kPtr->edata.upper_disp_limit);
                } else {
                    //qDebug() << "set to auto";
                    CartesianPlot->setXscaling(caCartesianPlot::Auto);
                }
             }
        }

    } else if(indx == 2) { // user
        CartesianPlot->setXscaling(caCartesianPlot::User);
        //qDebug() << "set xlimits to" << xLimits;
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
            knobData *kPtr = monData->getMutexKnobDataPV(vars.at(1).trimmed());
             if(kPtr != (knobData*) 0) {
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
        //qDebug() << "set ylimits to" << yLimits;
        CartesianPlot->setYaxisLimits(yLimits);
    }

    CartesianPlot->setXaxisEnabled(xCheckBox->isChecked());
    CartesianPlot->setYaxisEnabled(yCheckBox->isChecked());
}

