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

#include "enumeric.h"
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

#define MIN_FONT_SIZE 5
/* shown significant digits beyond this threshold get the rounding color,
 * same value as in snumeric.cpp (caSpinbox); not to be confused with
 * MaxTotalDigits, which is the storage capacity */
#define NUMERIC_ROUNDING_WARN_DIGITS 15
/* qint64 storage limit: 10^19 - 1 does not fit any more */
#define MAX_NUMERIC_DIGITS ((int) ENumeric::MaxTotalDigits)

/* exact integer power of ten, n in [0, MAX_NUMERIC_DIGITS] */
static qint64 pow10ll(int n)
{
    qint64 r = 1;
    for (int i = 0; i < n; i++) r *= 10;
    return r;
}

Q_LOGGING_CATEGORY(eNumericLog, "caqtdm.widgets.enumeric")

ENumeric::ENumeric(QWidget *parent, int id, int dd) : QFrame(parent), FloatDelegate()
{
    lastLabelOnTab = lastLabel = -1;
    intDig = qBound(1, id, MAX_NUMERIC_DIGITS);
    decDig = qBound(0, dd, MAX_NUMERIC_DIGITS - intDig);
    orig_intDig = intDig;
    orig_decDig = decDig;
    digits = intDig + decDig;
    qCDebug(eNumericLog) << "ENumeric::ENumeric" << digits;

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
    setMinimumWidth(5*digits);
    LeftClickWithModifiersEater *leftClickWithModifiersEater = new LeftClickWithModifiersEater(this);
    leftClickWithModifiersEater->setObjectName("leftClickWithModifiersEater");
    init();
    installEventFilter(this);
    writeAccessW(true);
}

void ENumeric::writeAccessW(bool access)
{
    _AccessW = access;
}

QSize ENumeric::sizeHint() const
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

QSize ENumeric::minimumSizeHint() const
{
    return sizeHint();
}

void ENumeric::setDigitsFontScaleEnabled(bool en)
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
        qCWarning(eNumericLog) << "did not find an ESimpleLabel";
    }
    d_fontScaleEnabled = en;
    valueUpdated();
}

void ENumeric::clearContainers()
{
    qCDebug(eNumericLog) << "clearContainers";
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
        box = NULL;
    }
    if (bup) {
        delete bup;
        bup = NULL;
    }
    if (bdown) {
        delete bdown;
        bdown = NULL;
    }
}

void ENumeric::init()
{
    qCDebug(eNumericLog) << "init";
    LeftClickWithModifiersEater *lCWME = findChild<LeftClickWithModifiersEater *>("leftClickWithModifiersEater");
    setFocusPolicy(Qt::StrongFocus);
        if(box == NULL) box = new QGridLayout(this);
        if(bup == NULL) bup = new QButtonGroup(this);
        if(bdown == NULL) bdown = new QButtonGroup(this);
        box->setSpacing(0);
        SETMARGIN_QT456(box,1);
        box->setRowStretch(0,1);
        box->setRowStretch(1,1);
        box->setRowStretch(2,1);

    for (int i = 0; i < digits; i++) {
        QLabel *l;
        if (i == intDig) {
            pointLabel = new QLabel(".", this);
            pointLabel->setAlignment(Qt::AlignCenter);
            pointLabel->setObjectName("layoutmember.");
            box->addWidget(pointLabel, 1, intDig + 1);
        }

        QPushButton *temp = new QPushButton(this);
        temp->setObjectName(QString("layoutmember") + QString().setNum(i));
        temp->installEventFilter(lCWME);
#ifndef MOBILE
        /* one step per click: holding the button must not ramp a real channel */
        temp->setAutoRepeat(false);
        temp->setAutoRepeatInterval(200);
        temp->setAutoRepeatDelay(500);
#endif
        bup->addButton(temp);

        if(i == intDig - 1) {
            l = new ESimpleLabel(QString().setNum(i), this);
            dynamic_cast<ESimpleLabel *>(l)->setScaleMode(ESimpleLabel::None);
        } else {
            l = new QLabel(QString().setNum(i), this);
        }

        l->setObjectName(QString("layoutmember") + QString().setNum(i));
        labels.push_back(l);

        QPushButton *temp2 = new QPushButton(this);
        temp2->setObjectName(QString("layoutmember") + QString().setNum(i));
        temp2->installEventFilter(lCWME);
#ifndef MOBILE
        /* one step per click: holding the button must not ramp a real channel */
        temp2->setAutoRepeat(false);
        temp2->setAutoRepeatInterval(200);
        temp2->setAutoRepeatDelay(500);
#endif
        bdown->addButton(temp2);

        formatDigit(temp, l, temp2);

        if (i < intDig) {
            box->addWidget(temp, 0, i + 1);
            box->addWidget(l, 1, i + 1);
            box->addWidget(temp2, 2, i + 1);
        } else if (i >= intDig) {
            box->addWidget(temp, 0, i + 2);
            box->addWidget(l, 1, i + 2);
            box->addWidget(temp2, 2, i + 2);
        }

        if (i == 0) {
            /* messo qui per evitare casini col designer */
            signLabel = new QLabel("+", this);
            signLabel->setAlignment(Qt::AlignCenter);
            signLabel->setObjectName("layoutmember+");
            box->addWidget(signLabel, 1, 0);
        }
    }
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

void ENumeric::setValue(double v)
{
    /* user driven value: out-of-range values are ignored */
    qint64 temp = transformNumberSpace(v, decDig);
    if ((temp >= minVal) && (temp <= maxVal)) {
        bool valChanged = data != temp;
        data = temp;
        /* update the labels before emitting */
        showData();
        if (valChanged) emit valueChanged(transformNumberSpace(temp, -decDig));
    }
}

QString ENumeric::generate_valueString(){
    QString valueString="";
    if(signLabel && (signLabel->text() == "-")) valueString += signLabel->text();
    for(int i = 0; i < digits; i++){
        if(i == intDig) valueString += ".";
        if (i<labels.length()){
            QString txt = labels[i]->text();
            if(txt != " ") valueString += txt;
        }
    }
    return valueString;
}




bool ENumeric::canEdit(){
    /* the channel value must fit the available digit positions; the decimal
     * places only count when they can actually be traded for integer ones */
    if (!std::isfinite(csValue)) return false;
    const bool canShift = thisAutoDigitShift && !thisFixedFormat;
    const int intCapacity = canShift ? digits : intDig;
    if (fabs(csValue) >= (double) pow10ll(intCapacity)) {
        qCDebug(eNumericLog) << "canEdit=false:" << csValue << "does not fit" << intCapacity << "integer digits";
        return false;
    }
    return true;
}

void ENumeric::suppressUserInput(){
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

void ENumeric::restoreUserInput(){
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
void ENumeric::updateSuppressionState(){
    if (!canEdit()) {
        suppressUserInput(); /* also re-stars freshly rebuilt labels */
        return;
    }
    if (suppressInput) {
        restoreUserInput();
        /* the last channel value becomes displayable again */
        const qint64 capacity = pow10ll(digits) - 1;
        data = qBound(-capacity, transformNumberSpace(csValue, decDig), capacity);
        showData();
    }
}

/* resolve the displayed digits from the configured baseline and the channel
 * value: trade decimal digits for integer digits when the magnitude needs them */
void ENumeric::updateDigitLayout(){
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
bool ENumeric::applyDigitLayout(int newIntDig, int newDecDig){
    if (newIntDig == intDig && newDecDig == decDig) return false;
    qCDebug(eNumericLog) << "applyDigitLayout:" << objectName() << "intDig" << intDig << "->" << newIntDig
                         << "decDig" << decDig << "->" << newDecDig;
    /* rescale to the new scale, rounding half away from zero */
    if (newDecDig > decDig) {
        data = data * pow10ll(newDecDig - decDig);
    } else if (newDecDig < decDig) {
        const qint64 divisor = pow10ll(decDig - newDecDig);
        const qint64 half = divisor / 2;
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

void ENumeric::silentSetValue(double v)
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
    const qint64 capacity = pow10ll(digits) - 1;
    data = qBound(-capacity, transformNumberSpace(v, decDig), capacity);
    showData();
}

void ENumeric::setMaximum(double v)
{
    if (v >= d_minAsDouble) {
        d_maxAsDouble = v;
        /* clamped to the display capacity */
        maxVal = qMin(transformNumberSpace(v, decDig), pow10ll(digits) - 1);
    }
}

void ENumeric::setMinimum(double v)
{
    if (v <= d_maxAsDouble) {
        d_minAsDouble = v;
        /* clamped to the display capacity */
        minVal = qMax(transformNumberSpace(v, decDig), -(pow10ll(digits) - 1));
    }
}

void ENumeric::setDigits(int i, int d)
{
    /* caQtDM_Lib computes width-prec-1, which can be 0, so clamp up to 1;
     * integer digits win the budget, the magnitude has to stay displayable */
    i = qBound(1, i, MAX_NUMERIC_DIGITS);
    d = qBound(0, d, MAX_NUMERIC_DIGITS - i);
    if (i == orig_intDig && d == orig_decDig) return;
    qCDebug(eNumericLog) << "setDigits" << objectName() << i << d;
    orig_intDig = i;
    orig_decDig = d;
    /* under fixed format the new baseline is applied directly */
    if (thisFixedFormat) applyDigitLayout(orig_intDig, orig_decDig);
    else updateDigitLayout();
    updateSuppressionState();
}

void ENumeric::setIntDigits(int i)
{
    setDigits(i, orig_decDig);
}

void ENumeric::setDecDigits(int d)
{
    setDigits(orig_intDig, d);
}

void ENumeric::setFixedFormat(bool f)
{
    if (thisFixedFormat == f) return;
    qCDebug(eNumericLog) << "setFixedFormat" << objectName() << f;
    thisFixedFormat = f;
    /* true freezes the displayed layout, false resolves it again */
    if (!f) updateDigitLayout();
    updateSuppressionState();
}

void ENumeric::setAutoDigitShift(bool f)
{
    if (thisAutoDigitShift == f) return;
    qCDebug(eNumericLog) << "setAutoDigitShift" << objectName() << f;
    thisAutoDigitShift = f;
    /* true resolves the layout against the value, false returns to the baseline */
    updateDigitLayout();
    updateSuppressionState();
}

void ENumeric::upData(QAbstractButton* b)
{
    if(!suppressInput){
        int id = b->objectName().remove("layoutmember").toInt();
        upDataIndex(id);
    }
}

void ENumeric::upDataIndex(int id)
{
    if(!_AccessW) return;
    if(id < 0 || id >= digits) return;
    /* pure qint64 arithmetic, exact also for |data| > 2^53 */
    const qint64 power = pow10ll(digits - id - 1);
    const qint64 datad = data + power;
    qCDebug(eNumericLog) << "upDataIndex:" << id << datad << maxVal << power;
    if (datad <= maxVal) {
        data = datad;
        emit valueChanged(transformNumberSpace(data, -decDig));
        showData();
    }
    if (text != NULL) text->hide();
}

qint64 ENumeric::transformNumberSpace(double value, int dig){
    /* correctly rounded double -> fixed point conversion via the decimal text
     * representation; unrepresentable values saturate to the qint64 range */
    if (dig < 0) dig = 0;
    if (!std::isfinite(value)) {
        return (value < 0.0) ? std::numeric_limits<qint64>::min()
                             : std::numeric_limits<qint64>::max();
    }
    QByteArray s = QByteArray::number(value, 'f', dig);
    const bool neg = s.startsWith('-');
    if (neg) s.remove(0, 1);
    const int dot = s.indexOf('.');
    const QByteArray intPart = (dot < 0) ? s : s.left(dot);
    const QByteArray fracPart = (dot < 0) ? QByteArray() : s.mid(dot + 1);
    if (intPart.size() + fracPart.size() > MAX_NUMERIC_DIGITS) {
        return neg ? std::numeric_limits<qint64>::min()
                   : std::numeric_limits<qint64>::max();
    }
    qint64 mag = intPart.toLongLong() * pow10ll(fracPart.size());
    if (!fracPart.isEmpty()) mag += fracPart.toLongLong();
    return neg ? -mag : mag;
}

double ENumeric::transformNumberSpace(qint64 value, int dig){
    /* dividing by the exact power of ten gives the correctly rounded double */
    if (dig < 0) return (double) value / (double) pow10ll(-dig);
    return (double) value * (double) pow10ll(dig);
}

double ENumeric::value() const
{
    /* dividing by the exact power of ten gives the correctly rounded double */
    return (double) data / (double) pow10ll(decDig);
}

void ENumeric::downData(QAbstractButton* b)
{
    if(!suppressInput){
        int id = b->objectName().remove("layoutmember").toInt();
        downDataIndex(id);
    }
}

void ENumeric::downDataIndex(int id)
{
    if(!_AccessW) return;
    if(id < 0 || id >= digits) return;
    /* pure qint64 arithmetic, exact also for |data| > 2^53 */
    const qint64 power = pow10ll(digits - id - 1);
    const qint64 datad = data - power;
    qCDebug(eNumericLog) << "downDataIndex:" << id << datad << minVal << power;
    if (datad >= minVal) {
        data = datad;
        emit valueChanged(transformNumberSpace(data, -decDig));
        showData();
    }
    if (text != NULL) text->hide();
}

void ENumeric::showData()
{
    if (suppressInput) return;

    if (signLabel) signLabel->setText(data < 0 ? QString("-") : QString("+"));

    /* integer digit decomposition, exact also for |data| > 2^53 */
    qint64 mag = (data < 0) ? -data : data;
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

void ENumeric::triggerRoundColorUpdate(){
    for(int i = 1; i < digits ; i++){
        updateRoundColors(i);
    }
}

void ENumeric::updateRoundColors(int i) {
    if (suppressInput) return;
    if (i < 0 || i >= labels.length()) return;

    /* count the shown digits (sign, point and blanks do not count) */
    int shownDigits = 0;
    for (int k = 0; k < digits && k < labels.length(); k++) {
        if (labels[k]->text() != " ") shownDigits++;
    }

    const int digitsToColorFromEnd = shownDigits - NUMERIC_ROUNDING_WARN_DIGITS;
    if (digitsToColorFromEnd > 0 && i >= digits - digitsToColorFromEnd) {
        labels[i]->setStyleSheet("QLabel {color:" + roundingColor.name() + ";}");
    } else if (!labels[i]->styleSheet().isEmpty()) {
        labels[i]->setStyleSheet("");
    }
}

/* coalesce: at most one delayed resize in flight */
void ENumeric::scheduleValueUpdated()
{
    if (resizePending) return;
    resizePending = true;
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
}

void ENumeric::valueUpdated()
{
    resizePending = false;
    QResizeEvent *re = new QResizeEvent(size(), size());
    resizeEvent(re);
    delete re;
}

void ENumeric::dataInput()
{
    setFocus();
    bool ok;
    double val;
    val = text->text().toDouble(&ok);
    if (ok)
        this->setValue(val);
    text->hide();
}

void ENumeric::mouseDoubleClickEvent(QMouseEvent*)
{
    if(!suppressInput){
        if (text == NULL) {
            qCDebug(eNumericLog) << "generate new QLineEdit";
            text = new QLineEdit(this);
            /* connect only once, otherwise the connections accumulate */
            connect(text, SIGNAL(returnPressed()), this, SLOT(dataInput()));
            connect(text, SIGNAL(editingFinished()), text, SLOT(hide()));
        } else {
            qCDebug(eNumericLog) << "reuse existing QLineEdit and raise it to front";
        }
        text->raise();
        text->show();

        QString valueString = generate_valueString();
        text->setText(valueString);

        text->setGeometry(QRect(box->cellRect(1, 0).topLeft(), box->cellRect(1, box->columnCount() - 1).bottomRight()));
        text->setFont(signLabel->font());
        text->setAlignment(Qt::AlignRight);
        text->setMaxLength(digits+2);
        text->setSelection(0, valueString.length());
        text->setFocus();
    }
}

bool ENumeric::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if(!_AccessW) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        } else {
            QApplication::restoreOverrideCursor();
        }
    } else if(event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
        if(!suppressInput) {
            QString ParentClassName = parent()->metaObject()->className();
            if(ParentClassName.contains("caApplyNumeric")) {
                qCDebug(eNumericLog) << "do nothing";
            } else {
                lastLabelOnTab = lastLabel;
                lastLabel = -1;
                /* fall back to the last channel value */
                const qint64 capacity = pow10ll(digits) - 1;
                data = qBound(-capacity, transformNumberSpace(csValue, decDig), capacity);
                showData();

                valueUpdated();
                updateGeometry();
            }
        }
    } else if(event->type() == QEvent::MouseButtonDblClick) {
        if(!_AccessW) return true;
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
    } else if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ev = (QKeyEvent*) event;
         if(ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up) return true;

    } else if(event->type() == QEvent::KeyRelease && !suppressInput)    {
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

void ENumeric::reconstructGeometry()
{
}

void ENumeric::changeEvent(QEvent *event)
{
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        QResizeEvent resize(size(), size());
        resizeEvent(&resize);
    }
}

void ENumeric::resizeEvent(QResizeEvent *e)
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
        QPixmap pix1(temp->size() * 0.9);
        const QColor arrowBackground = property("caqtdmNumericArrowBackground").value<QColor>();
        pix1.fill(arrowBackground.isValid() ? arrowBackground : palette().color(QPalette::Button));
        QPainter *p = new QPainter(&pix1);
        p->setRenderHint(QPainter::Antialiasing);
        hmargin = (int) (pix1.width() * MARGIN);
        vmargin = (int) (pix1.height() * MARGIN);
        if (hmargin < MIN_MARGIN)
            hmargin = MIN_MARGIN;
        if (vmargin < MIN_MARGIN)
            vmargin = MIN_MARGIN;
        int h = pix1.height(), w = pix1.width();
        QPolygon poly(3);
        poly.setPoint(0, (int) (w * .5), vmargin);
        poly.setPoint(1, w - hmargin, h - vmargin);
        poly.setPoint(2, hmargin, h - vmargin);
        QPen pen;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        pen.setColor(palette().color(QPalette::Foreground));
#else
        pen.setColor(palette().color(QPalette::Text));
#endif
        p->setPen(pen);
        QLinearGradient linearGradient(0, 0, w, h);
        linearGradient.setColorAt(0.0, palette().color(QPalette::Light));
        linearGradient.setColorAt(1.0, palette().color(QPalette::Dark));
        p->setBrush(linearGradient);
        p->drawConvexPolygon(poly);
        p->end();
        delete p;
        // down pixmap
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QPixmap pix2 = pix1.transformed(QMatrix().rotate(-180));
#else
        QPixmap pix2 = pix1.transformed(QTransform().rotate(-180));
#endif
        // pixmap up with red border
        QPixmap pix1Red = pix1;
        QPainter paint1R(&pix1Red);
        pen.setColor(Qt::red);
        paint1R.setBrush(Qt::NoBrush);
        paint1R.setPen(pen);
        paint1R.drawRect(0, 0, pix1Red.width() -1, pix1Red.height() -1);
        paint1R.end();

        // pixmap down with red border
        QPixmap pix2Red = pix2;
        QPainter paint2R(&pix2Red);
        pen.setColor(Qt::red);
        paint2R.setBrush(Qt::NoBrush);
        paint2R.setPen(pen);
        paint2R.drawRect(0, 0, pix2Red.width() -1, pix2Red.height() -1);
        paint2R.end();

        int i=0;
        foreach (QAbstractButton* but, bup->buttons()) {
            temp = qobject_cast<QPushButton *>(but);
            if (temp) {
                if(lastLabel == i) {
                    temp->setIconSize(pix1Red.size());
                    temp->setIcon(pix1Red);
                } else {
                    temp->setIconSize(pix1.size());
                    temp->setIcon(pix1);
                }
            }
            i++;
        }

        i = 0;
        foreach (QAbstractButton* but, bdown->buttons()) {
            temp = qobject_cast<QPushButton *>(but);
            if (temp) {
                temp->setIconSize(pix2.size());
                temp->setIcon(pix2);
                if(lastLabel == i) {
                    temp->setIconSize(pix2Red.size());
                    temp->setIcon(pix2Red);
                } else {
                    temp->setIconSize(pix2.size());
                    temp->setIcon(pix2);
                }
            }
            i++;
        }
    }

    if (text != NULL)  {
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
    if(l1 != NULL)labelFont = l1->font();
    if(d_fontScaleEnabled && intDig > 0)
    {
        // this can not work correctly when resizing continously, characters will grow
        // and the layout will not be respected. I will calculate the fontsize exactly here
        //double fontSize = l1->calculateFontPointSizeF(l1->text(), l1->size());
        double fontSize = 80;
        fontSize = qMin((int) fontSize, size().height() / 4 - 2);
        qCDebug(eNumericLog) << "h" << fontSize << "digits=" << digits << intDig << decDig;
        if(decDig > 0) {
           fontSize = qMin((int) fontSize, size().width() / (digits+2));
        } else {
           fontSize = qMin((int) fontSize, size().width() / (digits+1));
        }
        qCDebug(eNumericLog) << "w" << fontSize;
        if(fontSize < MIN_FONT_SIZE) fontSize = MIN_FONT_SIZE;
        labelFont.setPointSizeF(fontSize);
        signFont.setPointSizeF(fontSize);

         CorrectFontIfAndroid(labelFont);
         CorrectFontIfAndroid(signFont);
         qCDebug(eNumericLog) << "digits=" << digits << l1->text() << "font size=" << fontSize;
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

void ENumeric::formatDigit(QPushButton *up, QLabel *l, QPushButton *down)
{

    up->setText("");
    up->setMinimumSize(QSize(MIN_SIZE,MIN_SIZE));
    up->setFlat(true);
    up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    up->setFocusPolicy(Qt::NoFocus);

    l->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->setAlignment(Qt::AlignCenter);
    l->setMinimumSize(QSize(MIN_SIZE,2*MIN_SIZE));

    down->setText("");
    down->setMinimumSize(QSize(MIN_SIZE, MIN_SIZE));
    down->setFlat(true);
    down->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    down->setFocusPolicy(Qt::NoFocus);
}

void ENumeric::setEnabled(bool b)
{
    QWidget::setEnabled(b);
    update();
}

void ENumeric::setDisabled(bool b)
{
    QWidget::setDisabled(b);
    update();
}

void ENumeric::showEvent(QShowEvent *e)
{
    scheduleValueUpdated();
    QWidget::showEvent(e);
}
