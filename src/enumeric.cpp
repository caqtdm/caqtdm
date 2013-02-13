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

#if defined(_MSC_VER)
int round (double x) {
  int i = (int) x;
  if (x >= 0.0) {
    return ((x-i) >= 0.5) ? (i + 1) : (i);
  } else {
    return (-x+i >= 0.5) ? (i - 1) : (i);
  }
}
#endif

ENumeric::ENumeric(QWidget *parent, int id, int dd) : QFrame(parent), FloatDelegate()
{
    intDig = id;
    decDig = dd;
    digits = id + dd;
    data = 0;
    minVal = (int) -pow(10.0, digits) + 1;
    maxVal = (int) pow(10.0, digits) - 1;
#ifdef _MSC_VER
    d_minAsDouble = (double) round(minVal);
    d_maxAsDouble = (double) round(maxVal);
#else
    d_minAsDouble = (double) roundl(minVal);
    d_maxAsDouble = (double) roundl(maxVal);
#endif

    bup = NULL;
    bdown = NULL;
    box = NULL;
    text = NULL;
    d_fontScaleEnabled = false;
    setFrameShape(QFrame::NoFrame);
    setMinimumHeight(35);
    setMinimumWidth(15*digits);
    LeftClickWithModifiersEater *leftClickWithModifiersEater = new LeftClickWithModifiersEater(this);
    leftClickWithModifiersEater->setObjectName("leftClickWithModifiersEater");

    init();
    installEventFilter(this);
    writeAccessW(true);
}

void ENumeric::writeAccessW(int access)
{
     _AccessW = access;
}

QSize ENumeric::sizeHint() const
{
    if(d_fontScaleEnabled)
    {
	QFont f = font();
	f.setPointSize(4); /* provide a size hint calculated on a minimum font of 4 points */
	QFontMetrics fm(f);
	int width = digits * fm.width("X") + fm.width("X"); /* in case there's the +/- sign */
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
    if(int1Label)
    {
/* do not make a differenz between the labels, they will get different sizes otherwise
	if(en)
            int1Label->setFontScaleMode(ESimpleLabel::Height);
        else */
            int1Label->setFontScaleMode(ESimpleLabel::None);

	d_fontScaleEnabled = en;
        foreach(QLabel *l, findChildren<QLabel *>(QRegExp("layoutmember*"))) {
            l->setFont(int1Label->font());
        }
    }
    else
        printf("did not find an ESimpleLabel\n");
    d_fontScaleEnabled = en;
    valueUpdated();
}

void ENumeric::clearContainers()
{
    if (box)
    {
        labels.clear();
        foreach(QWidget *child, this->findChildren<QWidget *>(QRegExp("layoutmember*")))
            delete child;
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
    LeftClickWithModifiersEater *lCWME = findChild<LeftClickWithModifiersEater *>("leftClickWithModifiersEater");
    setFocusPolicy(Qt::StrongFocus);

    box = new QGridLayout(this);
    box->setSpacing(0);
    box->setMargin(1);
    box->setRowStretch(0,1);
    box->setRowStretch(1,1);
    box->setRowStretch(2,1);
    bup = new QButtonGroup(this);
    bdown = new QButtonGroup(this);
    for (int i = 0; i < digits; i++)
    {
        QLabel *l;
        if (i == intDig)
        {
            pointLabel = new QLabel(".", this);
            pointLabel->setAlignment(Qt::AlignCenter);
            pointLabel->setObjectName("layoutmember.");
            box->addWidget(pointLabel, 1, intDig + 1);
        }

        QPushButton *temp = new QPushButton(this);
        temp->setObjectName(QString("layoutmember") + QString().setNum(i));
        temp->installEventFilter(lCWME);
        bup->addButton(temp);

        if(i == intDig - 1)
        {
            l = new ESimpleLabel(QString().setNum(i), this);
/*
            if(d_fontScaleEnabled)
                dynamic_cast<ESimpleLabel *>(l)->setScaleMode(ESimpleLabel::Height);
            else
*/
                dynamic_cast<ESimpleLabel *>(l)->setScaleMode(ESimpleLabel::None);
        }
        else
            l = new QLabel(QString().setNum(i), this);
        l->setObjectName(QString("layoutmember") + QString().setNum(i));
        labels.push_back(l);

        QPushButton *temp2 = new QPushButton(this);
        temp2->setObjectName(QString("layoutmember") + QString().setNum(i));
        temp2->installEventFilter(lCWME);
        bdown->addButton(temp2);

        formatDigit(temp, l, temp2);
        if (i < intDig)
        {
            box->addWidget(temp, 0, i + 1);
            box->addWidget(l, 1, i + 1);
            box->addWidget(temp2, 2, i + 1);
        }
        else if (i >= intDig)
        {
            box->addWidget(temp, 0, i + 2);
            box->addWidget(l, 1, i + 2);
            box->addWidget(temp2, 2, i + 2);
        }

        if (i == 0)
        {
            /* messo qui per evitare casini col designer */
            signLabel = new QLabel("+", this);
            signLabel->setAlignment(Qt::AlignCenter);
            signLabel->setObjectName("layoutmember+");
            box->addWidget(signLabel, 1, 0);
        }
    }
    for (int i = 0; i < box->rowCount(); i++)
        box->setRowStretch(i, 10);
    for (int i = 0; i < box->columnCount(); i++)
        box->setColumnStretch(i, 10);
    box->setColumnStretch(0, 3);
    box->setColumnStretch(intDig+1, 1);

    showData();
    connect(bup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(upData(QAbstractButton*)));
    connect(bdown, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(downData(QAbstractButton*)));
}

void ENumeric::setValue(double v)
{ 
    long long temp = (long long) round(v * pow(10.0, decDig));
    if ((temp >= minVal) && (temp <= maxVal))
    {
        bool valChanged = data != temp;
        data = temp;
        /* call this before emitting value changed to be sure that the value is up to date
         * in the labels of the TNumeric.
         */
        showData();
        if (valChanged)
            emit valueChanged(temp*pow(10.0, -decDig));
    }
}

void ENumeric::silentSetValue(double v)
{
    long long temp = (long long) round(v * pow(10.0, decDig));
    //if ((temp >= minVal) && (temp <= maxVal))
    {
        data = temp;
        showData();
    }
}

void ENumeric::setMaximum(double v)
{ 
    if (v >= d_minAsDouble)
    {
        d_maxAsDouble = v;
        maxVal = (long long) round(v* (long)pow(10.0, decDig));
    }
}

void ENumeric::setMinimum(double v)
{ 
    if (v <= d_maxAsDouble)
    {
        d_minAsDouble = v;
        minVal = (long long) round(v* (long)pow(10.0, decDig));
    }
}

void ENumeric::setIntDigits(int i)
{ 
    if (i < 1) return;
    clearContainers();
    intDig = i;
    digits = intDig + decDig;
    init();
}

void ENumeric::setDecDigits(int d)
{
    if (d < 0) return;
    clearContainers();
    data = (long long) (data * pow(10.0, d - decDig));
    maxVal = (long long) (maxVal * pow(10.0, d - decDig));
    minVal = (long long) (minVal * pow(10.0, d - decDig));
    decDig = d;
    digits = intDig + decDig;
    /* when changing decimal digits, minimum and maximum need to be recalculated, to avoid
	 * round issues. So, recalculating maximum and minimum is required  to obtain precision
	 */
    setMinimum(d_minAsDouble);
    setMaximum(d_maxAsDouble);
    init();
}

void ENumeric::upData(QAbstractButton* b)
{

    if(!_AccessW) return;
/* rounding errors on windows
    int id = b->objectName().remove("layoutmember").toInt();
    if ((data + pow(10.0, digits-id-1)) <= maxVal)
    {
        data += (long long) pow(10.0, digits-id-1);
        emit valueChanged(data * pow(10.0, -decDig));
        showData();
    }
    if (text != NULL)
        text->hide();

*/
    int id = b->objectName().remove("layoutmember").toInt();
    double datad = (double) data;
    double power =  pow(10.0, digits-id-1);
    datad = datad + power;
    if (datad <= (double)maxVal)
    {
        data = (long long) datad;
        power = pow(10.0, -decDig);
        datad = datad * power;
        emit valueChanged(datad);
        showData();
    }
    if (text != NULL)
        text->hide();

}

void ENumeric::downData(QAbstractButton* b)
{
    if(!_AccessW) return;
/* rounding errors on windows
    int id = b->objectName().remove("layoutmember").toInt();
    if ((data - pow(10.0, digits-id-1)) >= minVal)
    {
        data -= (long long) pow(10.0, digits-id-1);
        emit valueChanged(data * pow(10.0, -decDig));
        showData();
    }
    if (text != NULL)
        text->hide();
*/
    int id = b->objectName().remove("layoutmember").toInt();
    double datad = (double) data;
    double power =  pow(10.0, digits-id-1);
    datad = datad - power;
    if (datad >= (double)minVal)
    {
        data = (long long) datad;
        power = pow(10.0, -decDig);
        datad = datad * power;
        emit valueChanged(datad);
        showData();
    }
    if (text != NULL)
        text->hide();
}

void ENumeric::showData()
{
/* rounding errors on windows
    long long temp = data;
    int num = 0;
    if (data < 0)
        signLabel->setText(QString("-"));
    else
        signLabel->setText(QString("+"));

    for (int i = 0; i < digits; i++)
    {
        num = (long long) (temp / (long) pow(10.0, digits-i-1));
        temp -= num * (long) pow(10.0, digits-i-1);
        labels[i]->setText(QString().setNum(abs(num)));
        labels[i]->setText
    }
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
*/
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
        labels[i]->setText(QString().setNum(abs((int) num)));
    }
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
}

void ENumeric::valueUpdated()
{
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
    if (text == NULL)
    {
        text = new QLineEdit(this);
        connect(text, SIGNAL(returnPressed()), this, SLOT(dataInput()));
        connect(text, SIGNAL(lostFocus()), text, SLOT(hide()));
    }
    text->setGeometry(QRect(box->cellRect(1, 0).topLeft(), box->cellRect(1, box->columnCount() - 1).bottomRight()));
    text->setFont(signLabel->font());
    text->setAlignment(Qt::AlignRight);
    text->setMaxLength(digits+2);
    text->setText("");
    text->setFocus();
    text->show();
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
    } else if(event->type() == QEvent::MouseButtonDblClick) {
        if(!_AccessW) return true;
    }
   return QObject::eventFilter(obj, event);
}


void ENumeric::reconstructGeometry()
{

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
    if (temp)
    {
        QPixmap pix(temp->size());
        pix.fill(palette().color(QPalette::Background));
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
        pen.setColor(palette().color(QPalette::Foreground));
        p.setPen(pen);
        QLinearGradient linearGradient(0, 0, w, h);
        linearGradient.setColorAt(0.0, palette().color(QPalette::Light));
        linearGradient.setColorAt(1.0, palette().color(QPalette::Dark));
        p.setBrush(linearGradient);
        p.drawConvexPolygon(poly);
        p.end();

        foreach (QAbstractButton* but, bup->buttons())
        {
            temp = qobject_cast<QPushButton *>(but);
            if (temp)
            {
                temp->setIconSize(pix.size());
                temp->setIcon(pix);
            }
        }
        //QPixmap pix2 = pix.transformed(QMatrix().rotate(180)).transformed(QMatrix().translate(-20,-20));
        QPixmap pix2 = pix.transformed(QMatrix().rotate(-180));
        foreach (QAbstractButton* but, bdown->buttons())
        {
            temp = qobject_cast<QPushButton *>(but);
            if (temp)
            {
                temp->setIconSize(pix2.size());
                temp->setIcon(pix2);
            }
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
    QFont labelFont;
    ESimpleLabel *l1 = findChild<ESimpleLabel *>();
    labelFont = l1->font();
    if(d_fontScaleEnabled && intDig > 0)
    {
        // this can not work correctly when resizing continously, characters will grow
        // and the layout will not be respected. I will calculate the fontsize exactly here
        //double fontSize = l1->calculateFontPointSizeF(l1->text(), l1->size());
        double fontSize = 80;
        fontSize = qMin((int) fontSize, size().height() / 4 - 2);
        fontSize = qMin((int) fontSize, size().width() / (digits+1));
        labelFont.setPointSizeF(fontSize);
        //printf("digits=%d %s font size=%f\n", digits, l1->text().toAscii().constData(), fontSize);
    }
    /* all fonts equal */
    if(d_fontScaleEnabled)
    {
        foreach(QLabel *l, findChildren<QLabel *>()) {
            l->setFont(labelFont);
        }
    }

    QWidget::resizeEvent(e);
}


void ENumeric::formatDigit(QPushButton *up, QLabel *l, QPushButton *down)
{
    up->setText("");
    // 	up->setAutoRepeat(true); /* used to cause infinite loop */
    up->setMinimumSize(QSize(MIN_SIZE,MIN_SIZE));
    up->setFlat(true);
    //up->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    up->setFocusPolicy(Qt::NoFocus);

    //l->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->setAlignment(Qt::AlignCenter);
    l->setMinimumSize(QSize(MIN_SIZE,2*MIN_SIZE));

    down->setText("");
    // 	down->setAutoRepeat(true); /* used to cause infinite loop */
    down->setMinimumSize(QSize(MIN_SIZE, MIN_SIZE));
    down->setFlat(true);
    down->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //down->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
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
    QTimer::singleShot(1000, this, SLOT(valueUpdated()));
    QWidget::showEvent(e);
}
