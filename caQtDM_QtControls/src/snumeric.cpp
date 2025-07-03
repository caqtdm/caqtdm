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
 *    Yannick Wernle
 *  Contact details:
 *    anton.mezger@psi.ch
 *    yannick.wernle@psi.ch
 */

#include "snumeric.h"
#include "econstants.h"
#include "leftclick_with_modifiers_eater.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <ESimpleLabel>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QPainter>
#include <QTimer>
#include <QtDebug>
#include <QApplication>

#define MIN_FONT_SIZE 3
#define PREC_LIMIT_NUMERIC 15

#if (_MSC_VER == 1600)
extern int round (double x);
#endif

SNumeric::SNumeric(QWidget *parent, int id, int dd) : QFrame(parent), FloatDelegate()
{
    lastLabelOnTab = lastLabel = -1;
    intDig = id;
    decDig = dd;
    digits = id + dd;
    data = 0;
    csValue = 0.0;
    minVal = (int) -pow(10.0, digits) + 1;
    maxVal = (int) pow(10.0, digits) - 1;
#ifdef _MSC_VER
    d_minAsDouble = (double) round(minVal);
    d_maxAsDouble = (double) round(maxVal);
#else
    d_minAsDouble = (double) roundl(minVal);
    d_maxAsDouble = (double) roundl(maxVal);
#endif

    QColor cText = this->palette().color(QPalette::Text);
    roundingColor = QColor(180 - cText.red(), 180 - cText.green(), 180 - cText.blue(), 255);
    bup = NULL;
    bdown = NULL;
    box = NULL;
    text = NULL;
    d_fontScaleEnabled = false;
    setFrameShape(QFrame::NoFrame);
    setMinimumHeight(20);
    setMinimumWidth(15*digits);
    LeftClickWithModifiersEater *leftClickWithModifiersEater = new LeftClickWithModifiersEater(this);
    leftClickWithModifiersEater->setObjectName("leftClickWithModifiersEater");

    init();
    installEventFilter(this);
    writeAccessW(true);
}

void SNumeric::writeAccessW(bool access)
{
    _AccessW = access;
}

QSize SNumeric::sizeHint() const
{
    if(d_fontScaleEnabled) {
        QFont f = font();
        f.setPointSize(4); /* provide a size hint calculated on a minimum font of 4 points */
        QFontMetrics fm(f);
        int width = digits * QMETRIC_QT456_FONT_WIDTH(fm,"X") + QMETRIC_QT456_FONT_WIDTH(fm,"X"); /* in case there's the +/- sign */
        return QSize(width, fm.height());
    }
    return QWidget::sizeHint();
}

QSize SNumeric::minimumSizeHint() const
{
    return sizeHint();
}

void SNumeric::setDigitsFontScaleEnabled(bool en)
{
    ESimpleLabel *int1Label = findChild<ESimpleLabel *>();
    if(int1Label) {
        int1Label->setFontScaleMode(ESimpleLabel::None);
        d_fontScaleEnabled = en;
        QString pattern="layoutmember*";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        foreach(QLabel *l, findChildren<QLabel *>(QRegExp(pattern))) {
#else
        foreach(QLabel *l, findChildren<QLabel *>(QRegularExpression(pattern))) {
#endif
            l->setFont(int1Label->font());
        }
    } else {
        printf("did not find an ESimpleLabel\n");
    }
    d_fontScaleEnabled = en;
    valueUpdated();
}

void SNumeric::clearContainers()
{
    if (box) {
        labels.clear();
        QString pattern="layoutmember*";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        foreach(QWidget *child, this->findChildren<QWidget *>(QRegExp(pattern))) delete child;
#else
        foreach(QWidget *child, this->findChildren<QWidget *>(QRegularExpression(pattern))) delete child;
#endif
        delete box;
        box = Q_NULLPTR;
    }
    if (bup) {
        delete bup;
        bup = Q_NULLPTR;
    }
    if (bdown) {
        delete bdown;
        bdown = Q_NULLPTR;
    }
}

void SNumeric::init()
{
    LeftClickWithModifiersEater *lCWME = findChild<LeftClickWithModifiersEater *>("leftClickWithModifiersEater");
    setFocusPolicy(Qt::StrongFocus);

        if(box == NULL) box = new QGridLayout(this);
        if(bup == NULL) bup = new QButtonGroup(this);
        if(bdown == NULL) bdown = new QButtonGroup(this);
        SETMARGIN_QT456(box,1);
        SETSPACING_QT456(box,0);

        box->setRowStretch(0,1);
        //box->setRowStretch(1,1);
        //box->setRowStretch(2,1);

    for (int i = 0; i < digits; i++) {
        QLabel *l;
        if (i == intDig) {
            pointLabel = new QLabel(".", this);
            pointLabel->setAlignment(Qt::AlignCenter);
            pointLabel->setObjectName("layoutmember.");
            box->addWidget(pointLabel, 0, intDig + 1);
        }

        if(i == intDig - 1) {
            l = new ESimpleLabel(QString().setNum(i), this);
            dynamic_cast<ESimpleLabel *>(l)->setScaleMode(ESimpleLabel::None);
        } else {
            l = new QLabel(QString().setNum(i), this);
        }

        l->setObjectName(QString("layoutmember") + QString().setNum(i));
        labels.push_back(l);

        formatLabel(l);

        if (i < intDig) {
            box->addWidget(l, 0, i + 1);
        } else if (i >= intDig) {
            box->addWidget(l, 0, i + 2);
        }

        if (i == 0) {
            /* messo qui per evitare casini col designer */
            signLabel = new QLabel("+", this);
            signLabel->setAlignment(Qt::AlignCenter);
            signLabel->setObjectName("layoutmember+");
            box->addWidget(signLabel, 0, 0);
        }
    }

    QGridLayout *box1 = new QGridLayout();
    box->addLayout(box1,0,digits+2);
    QPushButton *temp = new QPushButton(this);
    temp->setObjectName(QString("layoutmember") + QString().setNum(0));
    temp->installEventFilter(lCWME);

    bup->addButton(temp);

    QPushButton *temp2 = new QPushButton(this);
    temp2->setObjectName(QString("layoutmember") + QString().setNum(0));
    temp2->installEventFilter(lCWME);
    bdown->addButton(temp2);


    formatButton(temp);
    formatButton(temp2);
    box1->addWidget(temp, 0, 1);
    box1->addWidget(temp2, 1, 1);

    for (int i = 0; i < box->rowCount(); i++)   box->setRowStretch(i, 10);
    for (int i = 0; i < box->columnCount(); i++) box->setColumnStretch(i, 10);
    box->setColumnStretch(0, 3);
    box->setColumnStretch(intDig+1, 1);

    showData();

    connect(bup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(upData(QAbstractButton*)));
    connect(bdown, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(downData(QAbstractButton*)));
}

void SNumeric::setValue(double v)
{
    long long temp = transformNumberSpace(v, decDig);
    if ((temp >= minVal) && (temp <= maxVal))
    {
        bool valChanged = data != temp;
        data = temp;
        /* call this before emitting value changed to be sure that the value is up to date
         * in the labels of the TNumeric.
         */
        showData();
        if (valChanged)
            emit valueChanged(transformNumberSpace(temp, -decDig));
    }
}

// written from CS
void SNumeric::silentSetValue(double v)
{
    int intDigits = QString().number((long long) v).length();
    if(orig_decDig == -1) orig_decDig = decDig;
    if(orig_intDig == -1) orig_intDig = intDig;

    // Only do after initialized (or above threshold) to avoid shortening digits when not needed
    if(!valueChangedByButton && (isInitialized || digits > 15)){
        // Shift digits towards integers if the integer digits over EPICS is bigger than those displayed currently
        while (intDigits > intDig) {
            intDig++;
        }
        // Shift back if the opposite is the case
        while(intDigits < intDig){
            intDig--;
            if(orig_decDig > decDig) decDig++;
            if(orig_decDig < decDig) decDig--;
        }
        digits = intDig + decDig;
    }
    setValuesFromChannel(v);
    valueChangedByButton = false;
}

void SNumeric::setValuesFromChannel(double v)
{
    csValue = v;
    // Adjust min/max if set by channel
    setDecDigits(decDig);
    long long temp = transformNumberSpace(v, decDig);
    if(!isInitialized) isInitialized = true;
    data = temp;
    showData();
}

void SNumeric::setMaximum(double v)
{
    if (v >= d_minAsDouble) {
        d_maxAsDouble = v;
        maxVal = transformNumberSpace(v, decDig);
    }
    long long temp = (long long) round(csValue * pow(10.0, decDig));
    data = temp;
}

long long SNumeric::transformNumberSpace(double value, int dig){
    double f1;
    double f = std::modf(value, &f1);
    long long pw = f *pow(10.0, dig);
    long long pw1 = f1 *pow(10.0, dig);

    return pw+pw1;
}

double SNumeric::transformNumberSpace(long long value, int dig){
    double pw = value;
    while (dig > 10) {
        dig -= 10;
        pw *= pow(10.0, 10);
    }

    pw *= pow(10.0, dig);
    return pw;
}


void SNumeric::setMinimum(double v)
{
    if (v <= d_maxAsDouble)
    {
        d_minAsDouble = v;
        minVal = transformNumberSpace(v, decDig);
    }
    long long temp = (long long) round(csValue * pow(10.0, decDig));
    data = temp;
}

void SNumeric::setIntDigits(int i)
{
    if (i < 1) return;
    clearContainers();
    intDig = i;
    digits = intDig + decDig;
    init();
}

void SNumeric::setDecDigits(int d)
{
    if (d < 0) return;
    data = (long long) (data * pow(10.0, d - decDig));
    maxVal = (long long) (maxVal * pow(10.0, d - decDig));
    minVal = (long long) (minVal * pow(10.0, d - decDig));
    decDig = d;

    // shift digits around if max/minvalue get too big.
    int ddig = decDig + intDig;
    while (ddig > 17) {
        decDig--;
        ddig--;
    }
    digits = decDig + intDig;

    if(decDig != orig_decDig || intDig != orig_intDig){
        clearContainers();
        /* when changing decimal digits, minimum and maximum need to be recalculated, to avoid
         * round issues. So, recalculating maximum and minimum is required  to obtain precision
         */
        setMinimum(d_minAsDouble);
        setMaximum(d_maxAsDouble);

        init();
    }
}

void SNumeric::upData(QAbstractButton* b)
{
    valueChangedByButton = true;
    Q_UNUSED(b);
    if(lastLabel > -1) upDataIndex(lastLabel);
}

void SNumeric::upDataIndex(int id)
{
    if(!_AccessW) return;
    if(id == -1) return;
    long long datad = data;
    long long const currentData = data;
    long long power =  qPow(10.0, digits-id-1);
    datad = datad + power;
    // (long long) power overflows between 10^18 and 10^19
    if (datad <= ((double)maxVal) && digits-id-1 < 19) {
        data = datad;
        double dataDouble = (double) datad;
        dataDouble *= pow(10.0, -decDig);

        if (currentData <= datad) {
            emit valueChanged(dataDouble);
            showData();
        }
    }
    if (text != NULL) text->hide();
}

void SNumeric::downData(QAbstractButton* b)
{
    valueChangedByButton = true;
    Q_UNUSED(b);
    if(lastLabel > -1) downDataIndex(lastLabel);
}

void SNumeric::downDataIndex(int id)
{
    if(!_AccessW) return;
    if(id == -1) return;
    long long datad = data;
    long long const currentData = data;
    long long power =  pow(10.0, digits-id-1);
    datad = datad - power;
    if (datad >= minVal) {
        data = datad;
        double dataDouble = (double) datad;
        dataDouble *= pow(10.0, -decDig);

        if(currentData >= datad){
            emit valueChanged(dataDouble);
            showData();
        }
    }
    if (text != NULL) text->hide();
}

void SNumeric::showData()
{
    int thisDigit, prvDigit;
    bool suppress = true;
    long long temp = data;
    double num = 0;
    if (data < 0)
        signLabel->setText(QString("-"));
    else
        signLabel->setText(QString("+"));

    for (int i = 0; i < digits; i++)
    {
        double power =  pow(10.0, digits-i-1);
        double numd = (double) temp / power;
        if(numd >=0)
            num = floor(numd);
        else
            num = ceil(numd);
        numd = num * power;
        temp = temp - (long long) numd;

        thisDigit = abs((int) num);
        if(i>0 && prvDigit == 0 && suppress) labels[i-1]->setText(" ");
        labels[i]->setText(QString().setNum(abs((int) num)));
        prvDigit = thisDigit;
        if(thisDigit != 0) suppress = false;
        if(i >= intDig-1)  suppress = false;
    }
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
    triggerRoundColorUpdate();
}

void SNumeric::triggerRoundColorUpdate(){
    for(int i = 1; i < digits ; i++){
        updateRoundColors(i);
    }
}

void SNumeric::updateRoundColors(int i) {
    QColor currColor = labels[i]->palette().color(QPalette::Text);

    QString valueString = "";
    if(signLabel->text() == "-") valueString += signLabel->text();
    for(int i = 0; i < digits; i++){
        if(i == intDig) valueString += ".";
        QString txt = labels[i]->text();
        if(txt != " ") valueString += txt;

    }
    int digitsToColorFromEnd = (valueString.length() - PREC_LIMIT_NUMERIC);
    if (i > PREC_LIMIT_NUMERIC ||  (i > (digits-digitsToColorFromEnd) && valueString.length() > (PREC_LIMIT_NUMERIC +1))) {
        if(currColor != roundingColor){
            labels[i]->setStyleSheet("QLabel {color:" + roundingColor.name() + ";}");
        }else{
            labels[i]->setStyleSheet("QLabel {color:" + currColor.name() + ";}");
        }
    }
}

void SNumeric::valueUpdated()
{
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
    delete re;
}

bool SNumeric::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        lastLabelOnTab = lastLabel;
        lastLabel = -1;
        long long temp = (long long) round(csValue * pow(10.0, decDig));
        data = temp;
        showData();
        QApplication::restoreOverrideCursor();
        valueUpdated();
        updateGeometry();
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *ev = (QMouseEvent *) event;
        for (int i = 0; i < digits; i++) {
            QRect widgetRect = labels[i]->geometry();
            if(widgetRect.contains(ev->pos())) {
                lastLabel = i;
                valueUpdated();
                break;
            }
        }

    // this prevents a parent scrollbar to react to the up/down keys
    } else if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ev = (QKeyEvent*) event;
         if(ev->key() ==Qt::Key_Down || ev->key() ==Qt::Key_Up) return true;

    } else if(event->type() == QEvent::KeyRelease)   {
        QKeyEvent *ev = (QKeyEvent *) event;
        if(ev->key() == Qt::Key_Escape) if (text != NULL) text->hide();
        if(ev->key() == Qt::Key_Up) upDataIndex(lastLabel);
        if(ev->key() == Qt::Key_Down) downDataIndex(lastLabel);
        if(ev->key() == Qt::Key_Left) {
            lastLabel--;
            if(lastLabel < 0) lastLabel = 0;
            valueUpdated();
        }
        if(ev->key() == Qt::Key_Right) {
            lastLabel++;
            if(lastLabel > (digits-1)) lastLabel = digits-1;
            valueUpdated();
        }
        // move cursor with tab focus
        if(ev->key() == Qt::Key_Tab) {
            QCursor *cur = new QCursor;
            QPoint p = QWidget::mapToGlobal(QPoint(this->width()/2, this->height()/2));
            lastLabel = lastLabelOnTab;
            cur->setPos( p.x(), p.y());
            setFocus();
            valueUpdated();
        }
    }
    return QObject::eventFilter(obj, event);
}


void SNumeric::reconstructGeometry()
{

}

void SNumeric::resizeEvent(QResizeEvent *e)
{
    int hmargin, vmargin;
    QPushButton *temp;
    if(bup == NULL) return;
    if(bdown == NULL) return;
    if(box == NULL) return;
    if(bup->buttons().count() == 0) return;

    // this leads to a dangling pointer, do it in two steps
    //temp = qobject_cast<QPushButton *>(bup->buttons().front());

    QList<QAbstractButton *> list  = bup->buttons();
    temp =  qobject_cast<QPushButton *>(list.front());
    if (temp) {
        QPixmap pix(temp->size() * 0.9);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        pix.fill(palette().color(QPalette::Background));
#else
        pix.fill(palette().color(QPalette::Window));
#endif
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        hmargin = (int) (pix.width() * MARGIN);
        vmargin = (int) (pix.height() * MARGIN);
        if (hmargin < MIN_MARGIN)
            hmargin = MIN_MARGIN;
        if (vmargin < MIN_MARGIN)
            vmargin = MIN_MARGIN;
        int h = pix.height(), w = pix.width();
        QPolygon poly(3);
        poly.setPoint(0, (int) (w * .5), vmargin);
        poly.setPoint(1, w - hmargin, h - vmargin);
        poly.setPoint(2, hmargin, h - vmargin);
        QPen	pen;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        pen.setColor(palette().color(QPalette::Foreground));
#else
        pen.setColor(palette().color(QPalette::Text));
#endif
        p.setPen(pen);
        QLinearGradient linearGradient(0, 0, w, h);
        linearGradient.setColorAt(0.0, palette().color(QPalette::Light));
        linearGradient.setColorAt(1.0, palette().color(QPalette::Dark));
        p.setBrush(linearGradient);
        p.drawConvexPolygon(poly);
        p.end();

        int i=0;

        // put a border around selected digit
        for(int j=0; j< digits; j++) {
            labels[j]->setStyleSheet(getStylesheetUpdate(labels[j]->styleSheet(), true));
        }
        if(lastLabel != -1){
            labels[lastLabel]->setStyleSheet(getStylesheetUpdate(labels[lastLabel]->styleSheet(), false));
        }

        foreach (QAbstractButton* but, bup->buttons()) {
            temp = qobject_cast<QPushButton *>(but);
            if (temp) {
                temp->setIconSize(pix.size());
                temp->setIcon(pix);
            }
            i++;
        }

        i = 0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QPixmap pix2 = pix.transformed(QMatrix().rotate(-180));
#else
        QPixmap pix2 = pix.transformed(QTransform().rotate(-180));
#endif
        foreach (QAbstractButton* but, bdown->buttons()) {
            temp = qobject_cast<QPushButton *>(but);
            if (temp) {
                temp->setIconSize(pix2.size());
                temp->setIcon(pix2);
            }
            i++;
        }
    }

    if (text != Q_NULLPTR)  {
        text->setGeometry(QRect(box->cellRect(1, 0).topLeft(), box->cellRect(1, box->columnCount() - 1).bottomRight()));
    }

    /* rescale font if required. Take the only ESimpleLabel we have, then ask it to calculateFontPointSizeF
     * providing its text and its size as input parameters to the public method calculateFontPointSizeF of
     * the class FontScalingWidget, which ESimpleLabel inherits. We must provide text and size because the
     * method belongs to FontScalingWidget, not to ESimpleLabel.
     */
    QFont signFont("Monospace");  // + and - should have same size
    QFont labelFont;
    ESimpleLabel *l1 = findChild<ESimpleLabel *>();
    labelFont = l1->font();
    if(d_fontScaleEnabled && intDig > 0)
    {
        // this can not work correctly when resizing continously, characters will grow
        // and the layout will not be respected. I will calculate the fontsize exactly here
        //double fontSize = l1->calculateFontPointSizeF(l1->text(), l1->size());
        double fontSize = 80;
        fontSize = qMin((int) fontSize, size().height()/2+2);
        fontSize = qMin((int) fontSize, size().width() / (digits+1));
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        labelFont.setPointSizeF(fontSize);
        signFont.setPointSizeF(fontSize);

        CorrectFontIfAndroid(labelFont);
        CorrectFontIfAndroid(signFont);
        //printf("digits=%d %s font size=%f\n", digits, qasc(l1->text()), fontSize);
    }
    /* all fonts equal */
    if(d_fontScaleEnabled){
        foreach(QLabel *l, findChildren<QLabel *>()) {
            l->setFont(labelFont);
            if(l->objectName().contains("layoutmember+")) l->setFont(signFont);
        }
    }

    QWidget::resizeEvent(e);
}

QString SNumeric::getStylesheetUpdate(QString styleSheet, bool resetBorder){
    QString borderStyle = "border: 2px solid red;";
    if (styleSheet.length() > 0) {
        if (resetBorder) {
            styleSheet = styleSheet.replace(borderStyle, "");
        } else {
            if(styleSheet.contains("}")) styleSheet = styleSheet.replace("}", (borderStyle +" }"));
        }
    }else if(!resetBorder) styleSheet = borderStyle;
    return styleSheet;
}

void  SNumeric::formatButton(QPushButton *button) {
    button->setText("");
    button->setMinimumSize(QSize(MINSIZE,MINSIZE));
    button->setFlat(true);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    button->setFocusPolicy(Qt::NoFocus);
}

void SNumeric::formatLabel(QLabel *l)
{
    l->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->setAlignment(Qt::AlignCenter);
    l->setMinimumSize(QSize(MINSIZE,MINSIZE));
}

void SNumeric::setEnabled(bool b)
{
    QWidget::setEnabled(b);
    update();
}

void SNumeric::setDisabled(bool b)
{
    QWidget::setDisabled(b);
    update();
}

void SNumeric::showEvent(QShowEvent *e)
{
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
    QWidget::showEvent(e);
}
