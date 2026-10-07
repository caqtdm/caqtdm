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
#include <cmath>
#include <limits>

#define MIN_FONT_SIZE 3
#define NUMERIC_ROUNDING_WARN_DIGITS 15
/* long long storage limit: 10^19 - 1 does not fit any more */
#define MAX_NUMERIC_DIGITS ((int) SNumeric::MaxTotalDigits)

/* exact integer power of ten, n in [0, MAX_NUMERIC_DIGITS] */
static long long pow10ll(int n)
{
    long long r = 1;
    for (int i = 0; i < n; i++) r *= 10;
    return r;
}

Q_LOGGING_CATEGORY(sNumericLog, "caqtdm.widgets.snumeric")

SNumeric::SNumeric(QWidget *parent, int id, int dd) : QFrame(parent), FloatDelegate()
{
    lastLabelOnTab = lastLabel = -1;
    intDig = qBound(1, id, MAX_NUMERIC_DIGITS);
    decDig = qBound(0, dd, MAX_NUMERIC_DIGITS - intDig);
    orig_intDig = intDig;
    orig_decDig = decDig;
    digits = intDig + decDig;
    data = 0;
    csValue = 0.0;
    thisFixedFormat = false;
    maxVal = pow10ll(digits) - 1;
    minVal = -maxVal;
    d_minAsDouble = (double) minVal;
    d_maxAsDouble = (double) maxVal;

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
        qCWarning(sNumericLog) << "did not find an ESimpleLabel";
    }
    d_fontScaleEnabled = en;
    valueUpdated();
}

void SNumeric::clearContainers()
{
    /* both are deleted below, avoid dangling pointers */
    signLabel = Q_NULLPTR;
    pointLabel = Q_NULLPTR;
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
#ifndef MOBILE
    temp->setAutoRepeat(true);
    temp->setAutoRepeatInterval(200);
    temp->setAutoRepeatDelay(500);
#endif
    bup->addButton(temp);

    QPushButton *temp2 = new QPushButton(this);
    temp2->setObjectName(QString("layoutmember") + QString().setNum(0));
    temp2->installEventFilter(lCWME);
#ifndef MOBILE
    temp2->setAutoRepeat(true);
    temp2->setAutoRepeatInterval(200);
    temp2->setAutoRepeatDelay(500);
#endif
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

    /* delayed resize for the freshly built layout */
    scheduleValueUpdated();
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

bool SNumeric::canEdit(){
    /* the channel value must fit the available digit positions; the decimal
     * places only count when they can actually be traded for integer ones */
    if (!std::isfinite(csValue)) return false;
    const bool canShift = thisAutoDigitShift && !thisFixedFormat;
    const int intCapacity = canShift ? digits : intDig;
    if (fabs(csValue) >= (double) pow10ll(intCapacity)) {
        qCDebug(sNumericLog) << "canEdit=false:" << csValue << "does not fit" << intCapacity << "integer digits";
        return false;
    }
    return true;
}

void SNumeric::suppressUserInput(){
    suppressInput = true;
    /* an empty stylesheet/tooltip is a valid state to return to, so the flag
     * decides, not the emptiness of the backup */
    if (!haveBackup) {
        backupStylesheet = this->styleSheet();
        backupToolTip = this->toolTip();
        haveBackup = true;
    }
    for(int i = 0; i < digits && i < labels.length(); i++){
        labels[i]->setText("*");
        labels[i]->setStyleSheet("QLabel {color:red;}");
    }
    if (signLabel) signLabel->setText("");

    this->setStyleSheet("* {color: red;}");
    this->setToolTip("input broke widget: awaiting processable input");
    update();
}

void SNumeric::restoreUserInput(){
    suppressInput = false;
    if (haveBackup) {
        setStyleSheet(backupStylesheet);
        setToolTip(backupToolTip);
        backupStylesheet = "";
        backupToolTip = "";
        haveBackup = false;
    }
    for (int i = 0; i < digits && i < labels.length(); i++) {
        labels[i]->setStyleSheet("");
    }
}

/* after a layout or format change: enter, refresh or leave the suppression state */
void SNumeric::updateSuppressionState(){
    if (!canEdit()) {
        suppressUserInput(); /* also re-stars freshly rebuilt labels */
        return;
    }
    if (suppressInput) {
        restoreUserInput();
        /* the last channel value becomes displayable again */
        const long long capacity = pow10ll(digits) - 1;
        data = qBound(-capacity, transformNumberSpace(csValue, decDig), capacity);
        showData();
    }
}

/* resolve the displayed digits from the configured baseline and the channel
 * value: trade decimal digits for integer digits when the magnitude needs them */
void SNumeric::updateDigitLayout(){
    if (thisFixedFormat) return; /* frozen: keep the displayed layout */
    if (!thisAutoDigitShift) {
        /* shifting is opt-in: without it the displayed layout is the baseline */
        applyDigitLayout(orig_intDig, orig_decDig);
        return;
    }
    int newIntDig = orig_intDig;
    if (std::isfinite(csValue)) {
        /* integer digits needed by the magnitude */
        double mag = fabs(csValue);
        int needed = 1;
        while (mag >= 10.0 && needed < MAX_NUMERIC_DIGITS) {
            mag /= 10.0;
            needed++;
        }
        newIntDig = qBound(orig_intDig, needed, orig_intDig + orig_decDig);
    }
    applyDigitLayout(newIntDig, orig_intDig + orig_decDig - newIntDig);
}

/* rebuild the digit layout, rescaling the stored value to the new scale */
bool SNumeric::applyDigitLayout(int newIntDig, int newDecDig){
    if (newIntDig == intDig && newDecDig == decDig) return false;
    qCDebug(sNumericLog) << "applyDigitLayout:" << objectName() << "intDig" << intDig << "->" << newIntDig
                         << "decDig" << decDig << "->" << newDecDig;
    /* rescale to the new scale, rounding half away from zero */
    if (newDecDig > decDig) {
        data = data * pow10ll(newDecDig - decDig);
    } else if (newDecDig < decDig) {
        const long long divisor = pow10ll(decDig - newDecDig);
        const long long half = divisor / 2;
        data = (data >= 0) ? (data + half) / divisor : (data - half) / divisor;
    }
    clearContainers();
    intDig = newIntDig;
    decDig = newDecDig;
    digits = intDig + decDig;
    /* a shrunk layout voids a stale digit selection */
    if (lastLabel >= digits) lastLabel = -1;
    if (lastLabelOnTab >= digits) lastLabelOnTab = -1;
    /* re-clamp the limits to the new scale */
    setMinimum(d_minAsDouble);
    setMaximum(d_maxAsDouble);
    init();
    return true;
}

void SNumeric::silentSetValue(double v)
{
    csValue = v;

    if (!canEdit()) {
        if (!suppressInput) suppressUserInput();
        return;
    }

    /* value is processable (again): restore the normal display */
    if (suppressInput) restoreUserInput();

    updateDigitLayout(); /* may rebuild the digit layout */

    /* channel values bypass the limits but not the display capacity */
    const long long capacity = pow10ll(digits) - 1;
    data = qBound(-capacity, transformNumberSpace(v, decDig), capacity);
    showData();
}

void SNumeric::setMaximum(double v)
{
    if (v >= d_minAsDouble) {
        d_maxAsDouble = v;
        /* clamped to the display capacity */
        maxVal = qMin(transformNumberSpace(v, decDig), pow10ll(digits) - 1);
    }
}

long long SNumeric::transformNumberSpace(double value, int dig){
    /* correctly rounded double -> fixed point conversion via the decimal text
     * representation; unrepresentable values saturate to the long long range */
    if (dig < 0) dig = 0;
    if (!std::isfinite(value)) {
        return (value < 0.0) ? std::numeric_limits<long long>::min()
                             : std::numeric_limits<long long>::max();
    }
    QByteArray s = QByteArray::number(value, 'f', dig);
    const bool neg = s.startsWith('-');
    if (neg) s.remove(0, 1);
    const int dot = s.indexOf('.');
    const QByteArray intPart = (dot < 0) ? s : s.left(dot);
    const QByteArray fracPart = (dot < 0) ? QByteArray() : s.mid(dot + 1);
    if (intPart.size() + fracPart.size() > MAX_NUMERIC_DIGITS) {
        return neg ? std::numeric_limits<long long>::min()
                   : std::numeric_limits<long long>::max();
    }
    long long mag = intPart.toLongLong() * pow10ll(fracPart.size());
    if (!fracPart.isEmpty()) mag += fracPart.toLongLong();
    return neg ? -mag : mag;
}

double SNumeric::transformNumberSpace(long long value, int dig){
    /* dividing by the exact power of ten gives the correctly rounded double */
    if (dig < 0) return (double) value / (double) pow10ll(-dig);
    return (double) value * (double) pow10ll(dig);
}

double SNumeric::value() const
{
    /* dividing by the exact power of ten gives the correctly rounded double */
    return (double) data / (double) pow10ll(decDig);
}

void SNumeric::setMinimum(double v)
{
    if (v <= d_maxAsDouble) {
        d_minAsDouble = v;
        /* clamped to the display capacity */
        minVal = qMax(transformNumberSpace(v, decDig), -(pow10ll(digits) - 1));
    }
}

void SNumeric::setDigits(int i, int d)
{
    /* caQtDM_Lib computes width-prec-1, which can be 0, so clamp up to 1;
     * integer digits win the budget, the magnitude has to stay displayable */
    i = qBound(1, i, MAX_NUMERIC_DIGITS);
    d = qBound(0, d, MAX_NUMERIC_DIGITS - i);
    if (i == orig_intDig && d == orig_decDig) return;
    qCDebug(sNumericLog) << "setDigits" << objectName() << i << d;
    orig_intDig = i;
    orig_decDig = d;
    /* under fixed format the new baseline is applied directly */
    if (thisFixedFormat) applyDigitLayout(orig_intDig, orig_decDig);
    else updateDigitLayout();
    updateSuppressionState();
}

void SNumeric::setIntDigits(int i)
{
    setDigits(i, orig_decDig);
}

void SNumeric::setDecDigits(int d)
{
    setDigits(orig_intDig, d);
}

void SNumeric::setAutoDigitShift(bool f)
{
    if (thisAutoDigitShift == f) return;
    qCDebug(sNumericLog) << "setAutoDigitShift" << objectName() << f;
    thisAutoDigitShift = f;
    /* true resolves the layout against the value, false returns to the baseline */
    updateDigitLayout();
    updateSuppressionState();
}

void SNumeric::setFixedFormat(bool f)
{
    if (thisFixedFormat == f) return;
    qCDebug(sNumericLog) << "setFixedFormat" << objectName() << f;
    thisFixedFormat = f;
    /* true freezes the displayed layout, false resolves it again */
    if (!f) updateDigitLayout();
    updateSuppressionState();
}

void SNumeric::upData(QAbstractButton* b)
{
    Q_UNUSED(b);
    if(!suppressInput){
        if(lastLabel > -1) upDataIndex(lastLabel);
    }
}

void SNumeric::upDataIndex(int id)
{
    if(!_AccessW) return;
    if(id < 0 || id >= digits) return;
    /* pure long long arithmetic, exact also for |data| > 2^53 */
    const long long power = pow10ll(digits - id - 1);
    const long long datad = data + power;
    if (datad <= maxVal) {
        data = datad;
        emit valueChanged(transformNumberSpace(data, -decDig));
        showData();
    }
    if (text != NULL) text->hide();
}

void SNumeric::downData(QAbstractButton* b)
{
    Q_UNUSED(b);
    if(!suppressInput){
        if(lastLabel > -1) downDataIndex(lastLabel);
    }
}

void SNumeric::downDataIndex(int id)
{
    if(!_AccessW) return;
    if(id < 0 || id >= digits) return;
    /* pure long long arithmetic, exact also for |data| > 2^53 */
    const long long power = pow10ll(digits - id - 1);
    const long long datad = data - power;
    if (datad >= minVal) {
        data = datad;
        emit valueChanged(transformNumberSpace(data, -decDig));
        showData();
    }
    if (text != NULL) text->hide();
}

void SNumeric::showData()
{
    if (suppressInput) return;

    if (signLabel) signLabel->setText(data < 0 ? QString("-") : QString("+"));

    /* integer digit decomposition, exact also for |data| > 2^53 */
    long long mag = (data < 0) ? -data : data;
    for (int i = digits - 1; i >= 0; i--) {
        if (i < labels.length()) labels[i]->setText(QString::number((int) (mag % 10)));
        mag /= 10;
    }
    /* blank leading zeros, the last integer digit always stays */
    for (int i = 0; i < intDig - 1 && i < labels.length(); i++) {
        if (labels[i]->text() != " " && labels[i]->text() != "0") break;
        labels[i]->setText(" ");
    }

    triggerRoundColorUpdate();
}

void SNumeric::triggerRoundColorUpdate(){
    /* from 0: the first digit can never carry the rounding color, but it can
     * carry the selection border, which has to be maintained here as well */
    for(int i = 0; i < digits ; i++){
        updateRoundColors(i);
    }
}

void SNumeric::updateRoundColors(int i) {
    if (suppressInput) return;
    if (i < 0 || i >= labels.length()) return;

    /* count the shown digits (sign, point and blanks do not count) */
    int shownDigits = 0;
    for (int k = 0; k < digits && k < labels.length(); k++) {
        if (labels[k]->text() != " ") shownDigits++;
    }

    const int digitsToColorFromEnd = shownDigits - NUMERIC_ROUNDING_WARN_DIGITS;
    QString style;
    if (digitsToColorFromEnd > 0 && i >= digits - digitsToColorFromEnd) {
        style = "QLabel {color:" + roundingColor.name() + ";}";
    }
    /* the border marks the selected digit and acts as the user's cursor: it
     * shares the stylesheet with the rounding color and must survive a value
     * change, so both are composed here instead of overwriting each other */
    style = getStylesheetUpdate(style, i != lastLabel);
    if (labels[i]->styleSheet() != style) labels[i]->setStyleSheet(style);
}

/* coalesce: at most one delayed resize in flight */
void SNumeric::scheduleValueUpdated()
{
    if (resizePending) return;
    resizePending = true;
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
}

void SNumeric::valueUpdated()
{
    resizePending = false;
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
        /* Enter sets the override cursor unconditionally, so it has to be
         * released unconditionally too - otherwise it stays for the whole
         * application while the input is suppressed */
        QApplication::restoreOverrideCursor();
        /* the digit selection is the mouse cursor of this widget: dropping it
         * on leave must happen even while the input is suppressed, otherwise
         * the border reappears on recovery with the mouse long gone */
        lastLabelOnTab = lastLabel;
        lastLabel = -1;
        if(!suppressInput) {
            /* fall back to the last channel value */
            const long long capacity = pow10ll(digits) - 1;
            data = qBound(-capacity, transformNumberSpace(csValue, decDig), capacity);
            showData();
            valueUpdated();
            updateGeometry();
        }
    } else if (event->type() == QEvent::MouseButtonPress && !suppressInput) {
        QMouseEvent *ev = (QMouseEvent *) event;
        for (int i = 0; i < digits; i++) {
            QRect widgetRect = labels[i]->geometry();
            if(widgetRect.contains(ev->pos())) {
                lastLabel = i;
                valueUpdated();
                break;
            }
        }

    // this prevents a parent scrollbar to react to the up/down keys; the step
    // itself happens on release, so holding a key does not ramp the channel
    } else if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ev = (QKeyEvent*) event;
         if(ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up) return true;

    } else if(event->type() == QEvent::KeyRelease && !suppressInput)   {
        QKeyEvent *ev = (QKeyEvent *) event;
        if(ev->key() == Qt::Key_Escape) if (text != NULL) text->hide();
        if(ev->key() == Qt::Key_Up) upDataIndex(lastLabel);
        if(ev->key() == Qt::Key_Down) downDataIndex(lastLabel);
        if(ev->key() == Qt::Key_Left) {
            lastLabel--;
            if(lastLabel < 0) lastLabel = 0;
            valueUpdated();
        }
        if(ev->key() == Qt::Key_Right && !suppressInput) {
            lastLabel++;
            if(lastLabel > (digits-1)) lastLabel = digits-1;
            valueUpdated();
        }
        // move cursor with tab focus
        if(ev->key() == Qt::Key_Tab && !suppressInput) {
            QPoint p = QWidget::mapToGlobal(QPoint(this->width()/2, this->height()/2));
            lastLabel = lastLabelOnTab;
            QCursor::setPos(p.x(), p.y());
            setFocus();
            valueUpdated();
        }
    }
    return QObject::eventFilter(obj, event);
}


void SNumeric::reconstructGeometry()
{

}

void SNumeric::changeEvent(QEvent *event)
{
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        QResizeEvent resize(size(), size());
        resizeEvent(&resize);
    }
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
        const QColor arrowBackground = property("caqtdmNumericArrowBackground").value<QColor>();
        pix.fill(arrowBackground.isValid() ? arrowBackground : palette().color(QPalette::Button));
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
        for(int j=0; j< digits && j < labels.length(); j++) {
            labels[j]->setStyleSheet(getStylesheetUpdate(labels[j]->styleSheet(), true));
        }
        // bounds check: a digit layout change can leave a stale selection
        if(lastLabel >= 0 && lastLabel < labels.length()){
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
    if(l1 != NULL) labelFont = l1->font();
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
        qCDebug(sNumericLog) << "digits=" << digits << l1->text() << "font size=" << fontSize;
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
    scheduleValueUpdated();
    QWidget::showEvent(e);
}
