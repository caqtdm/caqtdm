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
 *  Copyright (c) 2010 - 2026
 *
 *  Author:
 *    Helge Brands
 */

#ifndef TST_NUMERIC_SUITE_H
#define TST_NUMERIC_SUITE_H

#include "QtWidgets/qlayout.h"
#include <QApplication>
#include <QEvent>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>
#include <qnumeric.h>
#include <cfloat>
#include <cmath>
#include <cstdlib>

/* Shared reliability test suite for the fixed point digit widgets
 * (caNumeric/ENumeric and caSpinbox/SNumeric).
 * Mathematically generated values (pi, e, sqrt(2), rationals, sin/exp series,
 * powers of ten) are compared against pure integer/string oracles; failure
 * messages mark wrong digits with '^' and digits beyond double precision with '~'.
 */

/* mathematical constants with 25 significant digits, used as "true value" oracles */
static const char *const NUM_PI_STR    = "3.1415926535897932384626434";
static const char *const NUM_E_STR     = "2.7182818284590452353602875";
static const char *const NUM_SQRT2_STR = "1.4142135623730950488016887";
static const char *const NUM_LN2_STR   = "0.6931471805599453094172321";

/*
 * ---------------------------------------------------------------------------
 *  oracles — pure integer/string arithmetic, independent of the widgets
 * ---------------------------------------------------------------------------
 */

static inline long long numPow10ll(int n)
{
    long long r = 1;
    for (int i = 0; i < n; i++) r *= 10;
    return r;
}

/* correctly rounded, locale independent decimal representation of the double */
static inline QString numOracleDecimalString(double v, int decDig)
{
    return QString::number(v, 'f', decDig);
}

static inline long long numFixedPointFromDecimalString(const QString &s, int decDig,
                                                       bool *ok = Q_NULLPTR)
{
    if (ok) *ok = true;
    QString str = s.trimmed();
    bool neg = str.startsWith(QLatin1Char('-'));
    if (neg || str.startsWith(QLatin1Char('+'))) str.remove(0, 1);
    QString intPart = str.section(QLatin1Char('.'), 0, 0);
    QString fracPart = str.section(QLatin1Char('.'), 1, 1);
    if (intPart.isEmpty()) intPart = QLatin1String("0");
    while (fracPart.size() <= decDig) fracPart.append(QLatin1Char('0'));

    bool ok1 = true, ok2 = true;
    long long result = intPart.toLongLong(&ok1) * numPow10ll(decDig);
    if (decDig > 0) result += fracPart.left(decDig).toLongLong(&ok2);
    /* rounding half away from zero */
    if (fracPart.at(decDig).digitValue() >= 5) result += 1;
    if ((!ok1 || !ok2) && ok) *ok = false;
    return neg ? -result : result;
}

/* exact long division: the first fracDigits fractional digits of num/den */
static inline QString numDecimalExpansion(long long num, long long den, int fracDigits)
{
    QString out;
    long long r = num % den;
    for (int i = 0; i < fracDigits; i++) {
        r *= 10;
        out.append(QLatin1Char((char) ('0' + r / den)));
        r %= den;
    }
    return out;
}

/* multiply a decimal string by 10^shift (shift >= 0) — exact, no float involved */
static inline QString numShiftDecimalPoint(const QString &s, int shift)
{
    QString str = s;
    bool neg = str.startsWith(QLatin1Char('-'));
    if (neg) str.remove(0, 1);
    int p = str.indexOf(QLatin1Char('.'));
    if (p < 0) p = str.size(); else str.remove(p, 1);
    p += shift;
    while (str.size() < p) str.append(QLatin1Char('0'));
    QString res = str.left(p) + QLatin1String(".") + str.mid(p);
    if (res.startsWith(QLatin1Char('.'))) res.prepend(QLatin1Char('0'));
    if (res.endsWith(QLatin1Char('.'))) res.chop(1);
    /* strip leading zeros of the integer part, keep at least one digit */
    while (res.size() > 1 && res.at(0) == QLatin1Char('0') && res.at(1) != QLatin1Char('.'))
        res.remove(0, 1);
    return neg ? QLatin1String("-") + res : res;
}

static inline QString numExpectedDisplayString(long long data, int intDig, int decDig)
{
    long long mag = llabs(data);
    long long div = numPow10ll(decDig);
    QString intStr = QString::number(mag / div);
    if (intStr.size() < intDig) intStr.prepend(QString(intDig - intStr.size(), QLatin1Char('0')));
    /* leading zero suppression as done by the widgets: contiguous leading zeros
     * become blanks, the least significant integer position keeps its digit */
    for (int j = 0; j < intStr.size() - 1 && intStr.at(j) == QLatin1Char('0'); j++)
        intStr[j] = QLatin1Char(' ');
    QString out = (data < 0) ? QLatin1String("-") : QLatin1String("+");
    out += intStr;
    if (decDig > 0)
        out += QLatin1String(".") + QString("%1").arg(mag % div, decDig, 10, QLatin1Char('0'));
    return out;
}

static inline QString numReadDisplayedString(QWidget *w, int intDig, int decDig,
                                             bool *singleCharOk)
{
    *singleCharOk = true;
    QLabel *sign = w->findChild<QLabel *>("layoutmember+");
    QString signText = sign ? sign->text() : QString("?");
    if (signText.size() != 1) *singleCharOk = false;
    QString out = signText;
    for (int i = 0; i < intDig + decDig; i++) {
        if (i == intDig && decDig > 0) out += QLatin1String(".");
        QLabel *l = w->findChild<QLabel *>(QString("layoutmember%1").arg(i));
        QString t = l ? l->text() : QString("?");
        if (t.size() != 1) *singleCharOk = false;
        out += t;
    }
    return out;
}

static inline long long numParseDisplayedData(const QString &display, bool *ok)
{
    *ok = display.size() >= 2;
    if (!*ok) return 0;
    QChar signChar = display.at(0);
    QString digitsOnly = display.mid(1);
    digitsOnly.remove(QLatin1Char('.'));
    digitsOnly.replace(QLatin1Char(' '), QLatin1Char('0')); // blanks are suppressed leading zeros
    bool numOk = false;
    long long mag = digitsOnly.toLongLong(&numOk);
    if (!numOk || (signChar != QLatin1Char('+') && signChar != QLatin1Char('-'))) {
        *ok = false;
        return 0;
    }
    return (signChar == QLatin1Char('-')) ? -mag : mag;
}

/* error budget of the widget conversion in units of the last displayed digit */
static inline long long numIntegerTolerance(double v, int decDig, double kappa)
{
    return (long long) llround(kappa * fabs(v) * pow(10.0, (double) decDig) * DBL_EPSILON);
}

/* digit position p (0 = last decimal) is beyond double precision iff 10^p <= 2*tol */
static inline int numUnreliableDigitCount(long long tol)
{
    if (tol <= 0) return 0;
    return (int) floor(log10(2.0 * (double) tol)) + 1;
}

static inline QString numFormatDigitComparison(double v, int intDig, int decDig,
                                               const QString &expected, const QString &actual,
                                               long long tol)
{
    const int len = qMax(expected.size(), actual.size());
    const QString e = expected.leftJustified(len);
    const QString a = actual.leftJustified(len);
    QString carets(len, QLatin1Char(' '));
    QString tildes(len, QLatin1Char(' '));
    for (int i = 0; i < len; i++)
        if (e.at(i) != a.at(i)) carets[i] = QLatin1Char('^');
    const int nU = numUnreliableDigitCount(tol);
    int marked = 0;
    for (int i = len - 1; i >= 0 && marked < nU; --i) {
        const QChar c = e.at(i);
        if (c == QLatin1Char('.') || c == QLatin1Char('+') || c == QLatin1Char('-')) continue;
        tildes[i] = QLatin1Char('~');
        marked++;
    }
    QString msg;
    msg += QLatin1String("numeric display mismatch\n");
    msg += QString("  input value       : %1   (intDig=%2, decDig=%3)\n")
               .arg(v, 0, 'g', 17).arg(intDig).arg(decDig);
    msg += QLatin1String("  expected digits   : ") + e + QLatin1String("\n");
    msg += QLatin1String("  actual   digits   : ") + a + QLatin1String("\n");
    msg += QLatin1String("  wrong digits      : ") + carets + QLatin1String("\n");
    msg += QString("  beyond precision  : %1   (trailing %2 digit(s), integer tolerance T=%3)")
               .arg(tildes).arg(nU).arg(tol);
    return msg;
}

/* counts QEvent::StyleChange on the watched widget */
class NumStyleChangeCounter : public QObject
{
public:
    int count = 0;
protected:
    bool eventFilter(QObject *obj, QEvent *ev)
    {
        if (ev->type() == QEvent::StyleChange) count++;
        return QObject::eventFilter(obj, ev);
    }
};

static const int s_numDigitConfigs[][2] = {
    {1, 0}, {2, 1}, {3, 2}, {5, 3}, {8, 6}, {9, 8},
    {10, 7}, {1, 16}, {2, 15}, {17, 1}, {9, 9}
};
static const int s_numDigitConfigCount =
    (int) (sizeof(s_numDigitConfigs) / sizeof(s_numDigitConfigs[0]));

/*
 * ---------------------------------------------------------------------------
 *  shared fixture and test bodies; WidgetT = ca widget, BaseT = its base class
 * ---------------------------------------------------------------------------
 */

template <class WidgetT, class BaseT>
class NumericSuiteBase
{
protected:
    NumericSuiteBase() : m_host(Q_NULLPTR), m_num(Q_NULLPTR) {}
    virtual ~NumericSuiteBase() {}

    /* the widget that receives synthetic key/mouse events; composite widgets
     * (caApplyNumeric) override this with their embedded numeric child */
    virtual QWidget *inputTarget() { return m_num; }

    void t_init()
    {
        /* the widget must have a parent: the event filters dereference parent() */
        m_host = new QWidget();
        m_num = new WidgetT(m_host);
    }

    void t_cleanup()
    {
        delete m_host;
        m_host = Q_NULLPTR;
        m_num = Q_NULLPTR;
    }

    /* largest value the digit labels can represent: (10^(I+D) - 1) * 10^-D */
    double capacity(int intDig, int decDig) const
    {
        return (double) (numPow10ll(intDig + decDig) - 1) * pow(10.0, -(double) decDig);
    }

    void configure(int intDig, int decDig)
    {
        m_num->setMaxValue(1.0);
        m_num->setMinValue(-1.0);
        /* both counts at once: they share the total digit budget, setting them
         * one after the other would clamp each against the other's old value */
        m_num->setDigits(intDig, decDig);
        /* the suite exercises the automatic digit shift, which is opt-in */
        m_num->setAutoDigitShift(true);
        const double cap = capacity(intDig, decDig);
        m_num->setMaxValue(cap);
        m_num->setMinValue(-cap);
    }

    /* mathematically generated doubles covering the displayable range */
    QList<double> mathValues(int intDig, int decDig) const
    {
        const double cap = capacity(intDig, decDig);
        QList<double> candidates;
        candidates << 0.0 << 1.0 << -1.0
                   << QString(NUM_PI_STR).toDouble() << -QString(NUM_PI_STR).toDouble()
                   << QString(NUM_E_STR).toDouble()
                   << QString(NUM_SQRT2_STR).toDouble()
                   << QString(NUM_LN2_STR).toDouble()
                   << 1.0 / 3.0 << 2.0 / 3.0
                   << 0.1 + 0.2;
        for (int k = 1; k <= 6; k++) candidates << (double) k / 7.0;
        for (int k = 1; k <= 5; k++) candidates << sin((double) k);
        for (int k = 1; k <= 4; k++) candidates << exp((double) k);
        for (int k = -decDig; k < intDig; k++) {
            const double p = pow(10.0, (double) k);
            candidates << p << -p << p / 3.0 << -p / 3.0;
        }
        candidates << cap << -cap
                   << (double) (numPow10ll(intDig + decDig) - 2) * pow(10.0, -(double) decDig);

        /* keep only values whose correctly rounded fixed point representation
         * fits the display capacity (a double comparison against cap is not
         * exact: for 16+ digits cap itself rounds up beyond the capacity) */
        QList<double> result;
        const long long capData = numPow10ll(intDig + decDig) - 1;
        foreach (const double v, candidates) {
            bool ok = true;
            const long long fixed =
                numFixedPointFromDecimalString(numOracleDecimalString(v, decDig), decDig, &ok);
            if (ok && llabs(fixed) <= capData) result << v;
        }
        return result;
    }

    void addConfigValueColumns()
    {
        QTest::addColumn<int>("intDig");
        QTest::addColumn<int>("decDig");
        QTest::addColumn<double>("value");

        for (int c = 0; c < s_numDigitConfigCount; c++) {
            const int I = s_numDigitConfigs[c][0];
            const int D = s_numDigitConfigs[c][1];
            int n = 0;
            foreach (const double v, mathValues(I, D)) {
                QTest::newRow(qPrintable(QString("I%1.D%2.#%3 v=%4")
                                             .arg(I).arg(D).arg(n++).arg(v, 0, 'g', 17)))
                    << I << D << v;
            }
        }
    }

    /* verify the oracles against hard coded cases, so that an oracle bug is not
     * mistaken for a widget bug */
    void t_helperSelfTest()
    {
        QCOMPARE(numPow10ll(0), Q_INT64_C(1));
        QCOMPARE(numPow10ll(18), Q_INT64_C(1000000000000000000));

        QCOMPARE(numDecimalExpansion(1, 7, 12), QString("142857142857"));
        QCOMPARE(numDecimalExpansion(1, 3, 5), QString("33333"));

        QCOMPARE(numShiftDecimalPoint("3.14159", 3), QString("3141.59"));
        QCOMPARE(numShiftDecimalPoint("3.14", 5), QString("314000"));
        QCOMPARE(numShiftDecimalPoint("-0.5", 1), QString("-5"));
        QCOMPARE(numShiftDecimalPoint("0.05", 1), QString("0.5"));
        QCOMPARE(numShiftDecimalPoint("0.0625", 2), QString("6.25"));

        bool ok = false;
        QCOMPARE(numFixedPointFromDecimalString("12.5", 1, &ok), Q_INT64_C(125));
        QVERIFY(ok);
        QCOMPARE(numFixedPointFromDecimalString("12.34", 2), Q_INT64_C(1234));
        QCOMPARE(numFixedPointFromDecimalString("-3.14159", 2), Q_INT64_C(-314));
        QCOMPARE(numFixedPointFromDecimalString("2.675", 2), Q_INT64_C(268)); // half away
        QCOMPARE(numFixedPointFromDecimalString("-0.5", 0), Q_INT64_C(-1));   // half away
        QCOMPARE(numFixedPointFromDecimalString("1.9999", 0), Q_INT64_C(2));
        QCOMPARE(numFixedPointFromDecimalString("0.1", 3), Q_INT64_C(100));

        QCOMPARE(numExpectedDisplayString(125, 3, 1), QString("+ 12.5"));
        QCOMPARE(numExpectedDisplayString(0, 3, 1), QString("+  0.0"));
        QCOMPARE(numExpectedDisplayString(-905, 2, 1), QString("-90.5"));
        QCOMPARE(numExpectedDisplayString(5, 1, 0), QString("+5"));
        QCOMPARE(numExpectedDisplayString(13, 3, 0), QString("+ 13"));

        QCOMPARE(numParseDisplayedData(QString("+ 12.5"), &ok), Q_INT64_C(125));
        QVERIFY(ok);
        QCOMPARE(numParseDisplayedData(QString("-90.5"), &ok), Q_INT64_C(-905));
        QVERIFY(ok);
        QCOMPARE(numParseDisplayedData(QString("+  0.0"), &ok), Q_INT64_C(0));
        QVERIFY(ok);

        QCOMPARE(numUnreliableDigitCount(0), 0);
        QCOMPARE(numUnreliableDigitCount(1), 1);
        QCOMPARE(numUnreliableDigitCount(28), 2);
        QCOMPARE(numUnreliableDigitCount(500), 4);
    }

    void t_roundTripValue_data()
    {
        addConfigValueColumns();
    }

    void t_roundTripValue()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(double, value);

        configure(intDig, decDig);
        m_num->setValue(value);

        bool oracleOk = true;
        const long long oracleData = numFixedPointFromDecimalString(
            numOracleDecimalString(value, decDig), decDig, &oracleOk);
        QVERIFY(oracleOk);
        const long long T = numIntegerTolerance(value, decDig, 4.0);
        /* correctly rounded reference: one exact integer division */
        const double expected = (double) oracleData / (double) numPow10ll(decDig);
        double tol = (double) T / (double) numPow10ll(decDig);
        if (T > 0) tol += 4.0 * DBL_EPSILON * qMax(1.0, fabs(expected));
        const double actual = m_num->value();

        QVERIFY2(fabs(actual - expected) <= tol,
                 qPrintable(QString("value() round trip failed for %1 (intDig=%2, decDig=%3):\n"
                                    "  expected %4\n  actual   %5\n"
                                    "  |diff| %6 > tolerance %7 (integer tolerance T=%8)")
                                .arg(value, 0, 'g', 17).arg(intDig).arg(decDig)
                                .arg(expected, 0, 'g', 17).arg(actual, 0, 'g', 17)
                                .arg(fabs(actual - expected), 0, 'g', 6).arg(tol, 0, 'g', 6)
                                .arg(T)));
    }

    void t_displayIntegrity_data()
    {
        addConfigValueColumns();
        /* fixed point values beyond 2^53: exact only with integer digit decomposition */
        QTest::newRow("big I16.D1 pi*1e15")
            << 16 << 1 << numShiftDecimalPoint(NUM_PI_STR, 15).toDouble();
        QTest::newRow("big I17.D1 pi*1e16")
            << 17 << 1 << numShiftDecimalPoint(NUM_PI_STR, 16).toDouble();
        QTest::newRow("big I18.D0 pi*1e17")
            << 18 << 0 << numShiftDecimalPoint(NUM_PI_STR, 17).toDouble();
        QTest::newRow("big I9.D9 pi*1e8")
            << 9 << 9 << numShiftDecimalPoint(NUM_PI_STR, 8).toDouble();
    }

    void t_displayIntegrity()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(double, value);

        configure(intDig, decDig);
        m_num->setValue(value);

        /* the labels must render exactly the correctly rounded fixed point value */
        bool oracleOk = true;
        const long long expectedStored = numFixedPointFromDecimalString(
            numOracleDecimalString(value, decDig), decDig, &oracleOk);
        QVERIFY(oracleOk);
        const QString expected = numExpectedDisplayString(expectedStored, intDig, decDig);
        bool singleOk = true;
        const QString actual = numReadDisplayedString(m_num, intDig, decDig, &singleOk);

        QVERIFY2(singleOk && actual == expected,
                 qPrintable(numFormatDigitComparison(value, intDig, decDig, expected, actual, 0)));
    }

    void t_displayedValueCorrectness_data()
    {
        addConfigValueColumns();
        /* decimal ties: correct rounding differs from a naive round(v * pow(10, D)) */
        QTest::newRow("tie 0.15@D1") << 2 << 1 << 0.15;
        QTest::newRow("tie 0.35@D1") << 2 << 1 << 0.35;
        QTest::newRow("tie 2.675@D2") << 3 << 2 << 2.675;
        QTest::newRow("tie 8.835@D2") << 3 << 2 << 8.835;
        QTest::newRow("tie 1.005@D2") << 3 << 2 << 1.005;
        QTest::newRow("tie 0.045@D2") << 3 << 2 << 0.045;
    }

    void t_displayedValueCorrectness()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(double, value);

        configure(intDig, decDig);
        m_num->setValue(value);

        bool oracleOk = true;
        const long long oracleData = numFixedPointFromDecimalString(
            numOracleDecimalString(value, decDig), decDig, &oracleOk);
        QVERIFY(oracleOk);
        const long long T = numIntegerTolerance(value, decDig, 4.0);

        bool singleOk = true, parseOk = true;
        const QString actual = numReadDisplayedString(m_num, intDig, decDig, &singleOk);
        const long long displayed = numParseDisplayedData(actual, &parseOk);
        const QString expected = numExpectedDisplayString(oracleData, intDig, decDig);

        QVERIFY2(singleOk && parseOk && llabs(displayed - oracleData) <= T,
                 qPrintable(numFormatDigitComparison(value, intDig, decDig, expected, actual, T)));
    }

    void t_precisionBoundary_data()
    {
        QTest::addColumn<int>("intDig");
        QTest::addColumn<int>("decDig");
        QTest::addColumn<QString>("trueDigits");

        /* constants with 25 significant digits at increasing total digit counts:
         * beyond ~16 significant digits a double cannot hold the displayed digits */
        QTest::newRow("pi I2.D8") << 2 << 8 << QString(NUM_PI_STR);
        QTest::newRow("e I2.D8") << 2 << 8 << QString(NUM_E_STR);
        QTest::newRow("pi I2.D12") << 2 << 12 << QString(NUM_PI_STR);
        QTest::newRow("sqrt2 I2.D12") << 2 << 12 << QString(NUM_SQRT2_STR);
        QTest::newRow("pi I2.D14") << 2 << 14 << QString(NUM_PI_STR);
        QTest::newRow("pi I2.D15") << 2 << 15 << QString(NUM_PI_STR);
        QTest::newRow("e I2.D15") << 2 << 15 << QString(NUM_E_STR);
        QTest::newRow("pi I2.D16") << 2 << 16 << QString(NUM_PI_STR);
        QTest::newRow("1/3 I1.D16") << 1 << 16 << QString("0.") + numDecimalExpansion(1, 3, 25);
        QTest::newRow("2/7 I1.D16") << 1 << 16 << QString("0.") + numDecimalExpansion(2, 7, 25);
        QTest::newRow("ln2 I1.D16") << 1 << 16 << QString(NUM_LN2_STR);
        QTest::newRow("pi*1e4 I6.D12") << 6 << 12 << numShiftDecimalPoint(NUM_PI_STR, 4);
        QTest::newRow("pi*1e7 I9.D9") << 9 << 9 << numShiftDecimalPoint(NUM_PI_STR, 7);
        QTest::newRow("pi*1e10 I12.D6") << 12 << 6 << numShiftDecimalPoint(NUM_PI_STR, 10);
        QTest::newRow("pi*1e15 I17.D1") << 17 << 1 << numShiftDecimalPoint(NUM_PI_STR, 15);
        /* pi * 10^k sweep at a constant total digit count of 18 */
        for (int k = 0; k <= 7; k++) {
            QTest::newRow(qPrintable(QString("pi*1e%1 I%2.D%3").arg(k).arg(k + 2).arg(16 - k)))
                << (k + 2) << (16 - k) << numShiftDecimalPoint(NUM_PI_STR, k);
        }
    }

    void t_precisionBoundary()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(QString, trueDigits);

        configure(intDig, decDig);
        const double v = trueDigits.toDouble();
        QVERIFY(fabs(v) <= capacity(intDig, decDig));
        m_num->setValue(v);

        bool oracleOk = true;
        const long long trueData = numFixedPointFromDecimalString(trueDigits, decDig, &oracleOk);
        QVERIFY(oracleOk);
        /* one extra unit for the representation error of the constant itself */
        const long long T = numIntegerTolerance(v, decDig, 4.0) + 1;

        bool singleOk = true, parseOk = true;
        const QString actual = numReadDisplayedString(m_num, intDig, decDig, &singleOk);
        const long long displayed = numParseDisplayedData(actual, &parseOk);
        const QString expected = numExpectedDisplayString(trueData, intDig, decDig);
        const QString report = numFormatDigitComparison(v, intDig, decDig, expected, actual, T);

        /* always make the double precision boundary visible in the test output */
        if (numUnreliableDigitCount(T) > 0)
            qInfo("true value %s:\n%s", qPrintable(trueDigits), qPrintable(report));

        QVERIFY2(singleOk && parseOk && llabs(displayed - trueData) <= T, qPrintable(report));
    }

    void t_outOfRangeValueIsIgnored()
    {
        configure(3, 2); // capacity 999.99
        m_num->setValue(12.34);
        bool singleOk = true;
        const QString before = numReadDisplayedString(m_num, 3, 2, &singleOk);
        QVERIFY(singleOk);

        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        m_num->setValue(1000.0);
        m_num->setValue(-1000.0);

        QVERIFY2(fabs(m_num->value() - 12.34) < 1e-9,
                 qPrintable(QString("out-of-range setValue changed the value to %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        const QString after = numReadDisplayedString(m_num, 3, 2, &singleOk);
        QCOMPARE(after, before);
        QCOMPARE(spy.count(), 0);
    }

    void t_limitsAreClampedToDisplayCapacity()
    {
        /* default widget: 2 integer digits, 1 decimal digit, constructor limits
         * +-100000; the effective limits must be clamped to the display capacity */
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());

        m_num->setValue(1234.5); /* beyond the 99.9 capacity: must be ignored */
        QVERIFY2(fabs(m_num->value()) < 1e-9,
                 qPrintable(QString("value beyond the display capacity was accepted: value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(spy.count(), 0);

        m_num->setValue(99.9); /* the capacity itself must be accepted */
        QVERIFY2(fabs(m_num->value() - 99.9) < 1e-9,
                 qPrintable(QString("capacity value 99.9 was rejected: value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(spy.count(), 1);

        /* with more integer digits the same value fits and must be displayed */
        m_num->setIntDigits(5);
        m_num->setMaxValue(100000.0);
        m_num->setValue(1234.5);
        QVERIFY2(fabs(m_num->value() - 1234.5) < 1e-9,
                 qPrintable(QString("1234.5 rejected after setIntDigits(5): value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        bool singleOk = true;
        const QString disp = numReadDisplayedString(m_num, 5, 1, &singleOk);
        QVERIFY2(singleOk && disp == numExpectedDisplayString(12345, 5, 1),
                 qPrintable(numFormatDigitComparison(1234.5, 5, 1,
                                                     numExpectedDisplayString(12345, 5, 1),
                                                     disp, 0)));
    }

    void t_ctorDigitsOverflow()
    {
        /* the base widget constructor must compute the fixed point limits with
         * integer arithmetic; an (int) cast of pow(10, digits) overflows for
         * digits >= 10 */
        BaseT e(m_host, 10, 0);
        e.setValue(5.0e9);
        const double got = e.value();
        QVERIFY2(fabs(got - 5.0e9) <= 1.0,
                 qPrintable(QString("base widget with 10 integer digits cannot take the value "
                                    "5e9: value() = %1 — constructor limits overflow")
                                .arg(got, 0, 'g', 17)));
    }

    void t_setDecDigitsRescaling_data()
    {
        QTest::addColumn<int>("intDig");
        QTest::addColumn<int>("startD");
        QTest::addColumn<int>("newD");
        QTest::addColumn<double>("startValue");
        QTest::addColumn<long long>("expectedData");

        QTest::newRow("increase D1->D3 keeps 12.5") << 2 << 1 << 3 << 12.5 << Q_INT64_C(12500);
        QTest::newRow("decrease D1->D0 must round 12.9 to 13")
            << 2 << 1 << 0 << 12.9 << Q_INT64_C(13);
        QTest::newRow("decrease D16->D15 pi")
            << 2 << 16 << 15 << QString(NUM_PI_STR).toDouble() << Q_INT64_C(3141592653589793);
    }

    void t_setDecDigitsRescaling()
    {
        QFETCH(int, intDig);
        QFETCH(int, startD);
        QFETCH(int, newD);
        QFETCH(double, startValue);
        QFETCH(long long, expectedData);

        configure(intDig, startD);
        m_num->setValue(startValue);
        m_num->setDecDigits(newD);

        bool singleOk = true, parseOk = true;
        const QString disp = numReadDisplayedString(m_num, intDig, newD, &singleOk);
        const long long actualData = numParseDisplayedData(disp, &parseOk);
        QVERIFY2(singleOk && parseOk && actualData == expectedData,
                 qPrintable(QString("setDecDigits(%1 -> %2) starting from %3: expected "
                                    "fixed-point data %4, widget shows \"%5\" (data %6) — "
                                    "the rescaling must round half away from zero")
                                .arg(startD).arg(newD).arg(startValue, 0, 'g', 17)
                                .arg(expectedData).arg(disp).arg(actualData)));

        /* data == expectedData was just verified: value() must match exactly */
        const double expVal = (double) expectedData / (double) numPow10ll(newD);
        QVERIFY2(m_num->value() == expVal,
                 qPrintable(QString("value() after setDecDigits is %1, expected %2")
                                .arg(m_num->value(), 0, 'g', 17).arg(expVal, 0, 'g', 17)));

        /* the limits must still work after the rescaling */
        m_num->setValue(1.5);
        const QString disp2 = numReadDisplayedString(m_num, intDig, newD, &singleOk);
        const long long data2 = numParseDisplayedData(disp2, &parseOk);
        QVERIFY2(singleOk && parseOk && data2 == numFixedPointFromDecimalString("1.5", newD),
                 qPrintable(QString("setValue(1.5) after setDecDigits(%1) shows \"%2\"")
                                .arg(newD).arg(disp2)));
    }

    void t_valueChangedEmissionSemantics()
    {
        configure(3, 2); // capacity 999.99

        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());

        m_num->setValue(1.25);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDouble(), m_num->value());

        m_num->setValue(1.25); // unchanged value: no signal
        QCOMPARE(spy.count(), 0);

        m_num->setValue(5000.0); // out of range: silently ignored, no signal
        QCOMPARE(spy.count(), 0);

        m_num->silentSetValue(7.5); // changes the value but must not emit
        QCOMPARE(spy.count(), 0);
        QVERIFY2(fabs(m_num->value() - 7.5) < 1e-9,
                 qPrintable(QString("silentSetValue(7.5) resulted in value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
    }

    void t_autoShiftOnChannelValue()
    {
        /* channel values (silentSetValue) may shift decimal digits into integer
         * digits when the value needs them, and shift back when it shrinks again;
         * user values (setValue) must never change the configuration */
        configure(2, 4);
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());

        m_num->silentSetValue(12345.6); /* needs 5 integer digits */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);
        QVERIFY2(fabs(m_num->value() - 12345.6) < 1e-9,
                 qPrintable(QString("value() after auto shift is %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        bool singleOk = true;
        QString disp = numReadDisplayedString(m_num, 5, 1, &singleOk);
        QVERIFY2(singleOk && disp == numExpectedDisplayString(123456, 5, 1),
                 qPrintable(numFormatDigitComparison(12345.6, 5, 1,
                                                     numExpectedDisplayString(123456, 5, 1),
                                                     disp, 0)));

        m_num->setValue(3.5); /* user value: the configuration must stay (5,1) */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);
        QVERIFY(fabs(m_num->value() - 3.5) < 1e-9);

        m_num->silentSetValue(1.25); /* small again: back to the original (2,4) */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        QVERIFY2(fabs(m_num->value() - 1.25) < 1e-9,
                 qPrintable(QString("value() after shifting back is %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        disp = numReadDisplayedString(m_num, 2, 4, &singleOk);
        QVERIFY2(singleOk && disp == numExpectedDisplayString(12500, 2, 4),
                 qPrintable(numFormatDigitComparison(1.25, 2, 4,
                                                     numExpectedDisplayString(12500, 2, 4),
                                                     disp, 0)));

        QCOMPARE(spy.count(), 1); /* only the user setValue may emit */
    }

    void t_suppressUserInputOnUnrepresentableValue()
    {
        configure(3, 2);
        m_num->silentSetValue(1.0e19); /* does not fit any digit configuration */

        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QLabel *last = m_num->template findChild<QLabel *>("layoutmember4");
        QLabel *sign = m_num->template findChild<QLabel *>("layoutmember+");
        QVERIFY(first && last && sign);
        QCOMPARE(first->text(), QString("*"));
        QCOMPARE(last->text(), QString("*"));
        QCOMPARE(sign->text(), QString(""));

        m_num->silentSetValue(42.5); /* processable again: the display must recover */
        bool singleOk = true;
        const QString disp = numReadDisplayedString(m_num, 3, 2, &singleOk);
        QVERIFY2(singleOk && disp == numExpectedDisplayString(4250, 3, 2),
                 qPrintable(numFormatDigitComparison(42.5, 3, 2,
                                                     numExpectedDisplayString(4250, 3, 2),
                                                     disp, 0)));
        QVERIFY(fabs(m_num->value() - 42.5) < 1e-9);
    }

    void t_roundingColorsMarkDigitsBeyondPrecision()
    {
        /* shown significant digits beyond the 15 digit precision threshold are
         * colored with the rounding color (light gray) */
        configure(9, 9);
        m_num->setValue(numShiftDecimalPoint(QString(NUM_PI_STR), 8).toDouble());
        for (int i = 0; i < 18; i++) {
            QLabel *l = m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(i));
            QVERIFY(l);
            if (i >= 15) { /* 18 shown digits: the last three exceed the threshold */
                QVERIFY2(l->styleSheet().startsWith("QLabel {color:"),
                         qPrintable(QString("label %1 should carry the rounding color, "
                                            "stylesheet is \"%2\"").arg(i).arg(l->styleSheet())));
            } else {
                QVERIFY2(l->styleSheet().isEmpty(),
                         qPrintable(QString("label %1 unexpectedly colored: \"%2\"")
                                        .arg(i).arg(l->styleSheet())));
            }
        }

        /* a small value on the same widget must clear the previous coloring */
        m_num->setValue(1.5);
        for (int i = 0; i < 18; i++) {
            QLabel *l = m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(i));
            QVERIFY(l);
            QVERIFY2(l->styleSheet().isEmpty(),
                     qPrintable(QString("label %1 still colored after a small value: \"%2\"")
                                    .arg(i).arg(l->styleSheet())));
        }

        /* few digits: nothing to color */
        configure(3, 2);
        m_num->setValue(42.5);
        for (int i = 0; i < 5; i++) {
            QLabel *l = m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(i));
            QVERIFY(l);
            QVERIFY(l->styleSheet().isEmpty());
        }

        /* label indices beyond 15 but only two significant digits: no coloring */
        configure(17, 1);
        m_num->setValue(1.5);
        for (int i = 0; i < 18; i++) {
            QLabel *l = m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(i));
            QVERIFY(l);
            QVERIFY2(l->styleSheet().isEmpty(),
                     qPrintable(QString("label %1 colored by index instead of significance: "
                                        "\"%2\"").arg(i).arg(l->styleSheet())));
        }
    }

    /* per digit up/down buttons (caNumeric, caApplyNumeric) */
    void t_incrementDecrementByButtonsPerDigit_data()
    {
        QTest::addColumn<int>("intDig");
        QTest::addColumn<int>("decDig");
        QTest::addColumn<int>("index");
        QTest::addColumn<bool>("up");
        QTest::addColumn<double>("startValue");
        QTest::addColumn<long long>("expectedDeltaData");
        QTest::addColumn<int>("expectSignals"); // -1: do not check

        QTest::newRow("LSD up") << 3 << 2 << 4 << true << 0.0 << Q_INT64_C(1) << 1;
        QTest::newRow("LSD down") << 3 << 2 << 4 << false << 0.0 << Q_INT64_C(-1) << 1;
        QTest::newRow("MSD up") << 3 << 2 << 0 << true << 0.0 << Q_INT64_C(10000) << 1;
        QTest::newRow("MSD down") << 3 << 2 << 0 << false << 0.0 << Q_INT64_C(-10000) << 1;
        QTest::newRow("down below minimum is ignored")
            << 1 << 0 << 0 << false << -9.0 << Q_INT64_C(0) << 0;
        /* data beyond 2^53: exact only with pure integer increments */
        QTest::newRow("LSD up beyond 2^53")
            << 17 << 1 << 17 << true << 9.0e15 << Q_INT64_C(1) << -1;
    }

    void t_incrementDecrementByButtonsPerDigit()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(int, index);
        QFETCH(bool, up);
        QFETCH(double, startValue);
        QFETCH(long long, expectedDeltaData);
        QFETCH(int, expectSignals);

        configure(intDig, decDig);
        QVERIFY(index >= 0 && index < intDig + decDig);

        QList<QPushButton *> btns =
            m_num->template findChildren<QPushButton *>(QString("layoutmember%1").arg(index));
        QCOMPARE(btns.size(), 2);

        /* self calibration: the up and the down button share the label's
         * objectName, so determine which is which by one click at a safe value */
        m_num->setValue(0.0);
        btns.at(0)->click();
        const double calib = m_num->value();
        QVERIFY2(calib != 0.0, "calibration click had no effect");
        QPushButton *upBtn = (calib > 0.0) ? btns.at(0) : btns.at(1);
        QPushButton *downBtn = (calib > 0.0) ? btns.at(1) : btns.at(0);

        m_num->setValue(startValue);
        const long long startData =
            numFixedPointFromDecimalString(numOracleDecimalString(startValue, decDig), decDig);
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());

        (up ? upBtn : downBtn)->click();

        bool singleOk = true, parseOk = true;
        const QString disp = numReadDisplayedString(m_num, intDig, decDig, &singleOk);
        const long long actualData = numParseDisplayedData(disp, &parseOk);
        QVERIFY2(singleOk && parseOk && (actualData - startData) == expectedDeltaData,
                 qPrintable(QString("digit %1 %2-click starting at %3 (intDig=%4, decDig=%5): "
                                    "expected fixed-point delta %6, got %7 (display \"%8\")")
                                .arg(index).arg(up ? "up" : "down")
                                .arg(startValue, 0, 'g', 17).arg(intDig).arg(decDig)
                                .arg(expectedDeltaData).arg(actualData - startData).arg(disp)));
        if (expectSignals >= 0)
            QCOMPARE(spy.count(), expectSignals);
    }

    void t_nanAndInfHandling()
    {
        configure(3, 2);
        m_num->setValue(12.5);
        bool singleOk = true;
        const QString before = numReadDisplayedString(m_num, 3, 2, &singleOk);
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());

        /* user values that are not finite are ignored */
        m_num->setValue(qQNaN());
        m_num->setValue(qInf());
        m_num->setValue(-qInf());
        QVERIFY2(fabs(m_num->value() - 12.5) < 1e-9,
                 qPrintable(QString("NaN/Inf changed the value to %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk), before);
        QCOMPARE(spy.count(), 0);

        /* channel values that are not finite suppress the input */
        m_num->silentSetValue(qQNaN());
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));

        m_num->silentSetValue(3.5); /* recovery */
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(350, 3, 2));

        m_num->silentSetValue(qInf());
        first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));
        m_num->silentSetValue(1.5);
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(150, 3, 2));
    }

    void t_negativeZeroAndTinyValues()
    {
        configure(3, 2);
        bool singleOk = true;
        m_num->setValue(-0.0);
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(0, 3, 2));
        QVERIFY(fabs(m_num->value()) < 1e-12);

        m_num->setValue(1e-310); /* subnormal: rounds to zero */
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(0, 3, 2));

        m_num->setValue(0.006); /* just above half of the last digit */
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(1, 3, 2));
        m_num->setValue(0.004); /* just below: rounds to zero */
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(0, 3, 2));
    }

    void t_asymmetricLimits()
    {
        configure(3, 2);
        m_num->setMinimum(-5.0);
        m_num->setMaximum(99.0);

        m_num->setValue(98.5);
        QVERIFY(fabs(m_num->value() - 98.5) < 1e-9);
        m_num->setValue(99.01); /* above maximum: ignored */
        QVERIFY2(fabs(m_num->value() - 98.5) < 1e-9,
                 qPrintable(QString("99.01 accepted despite maximum 99: value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        m_num->setValue(99.0); /* the maximum itself is allowed */
        QVERIFY(fabs(m_num->value() - 99.0) < 1e-9);

        m_num->setValue(-4.5);
        QVERIFY(fabs(m_num->value() + 4.5) < 1e-9);
        m_num->setValue(-5.5); /* below minimum: ignored */
        QVERIFY2(fabs(m_num->value() + 4.5) < 1e-9,
                 qPrintable(QString("-5.5 accepted despite minimum -5: value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        m_num->setValue(-5.0); /* the minimum itself is allowed */
        QVERIFY(fabs(m_num->value() + 5.0) < 1e-9);
    }

    void t_keyboardIncrementDecrement()
    {
        configure(3, 2);
        QWidget *tgt = inputTarget();
        QVERIFY(tgt);
        m_num->setValue(0.0);

        /* select the least significant digit (index 4) with arrow keys */
        for (int k = 0; k <= 4; k++) QTest::keyClick(tgt, Qt::Key_Right);
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        QTest::keyClick(tgt, Qt::Key_Up);
        QVERIFY2(fabs(m_num->value() - 0.01) < 1e-9,
                 qPrintable(QString("Key_Up on the LSD gave value() = %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDouble(), m_num->value());
        QTest::keyClick(tgt, Qt::Key_Down);
        QVERIFY(fabs(m_num->value()) < 1e-9);

        /* Key_Right clamps at the last digit */
        for (int k = 0; k < 10; k++) QTest::keyClick(tgt, Qt::Key_Right);
        QTest::keyClick(tgt, Qt::Key_Up);
        QVERIFY2(fabs(m_num->value() - 0.01) < 1e-9,
                 "Key_Right beyond the last digit must clamp the selection");

        /* Key_Left clamps at the first digit */
        for (int k = 0; k < 20; k++) QTest::keyClick(tgt, Qt::Key_Left);
        QTest::keyClick(tgt, Qt::Key_Up);
        QVERIFY2(fabs(m_num->value() - 100.01) < 1e-9,
                 "Key_Left beyond the first digit must clamp the selection");
    }

    void t_writeAccessBlocksInput()
    {
        configure(3, 2);
        m_num->setValue(1.0);
        QWidget *tgt = inputTarget();
        QVERIFY(tgt);
        for (int k = 0; k <= 4; k++) QTest::keyClick(tgt, Qt::Key_Right);

        m_num->setAccessW(false);
        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        QTest::keyClick(tgt, Qt::Key_Up);
        QCOMPARE(spy.count(), 0);
        QVERIFY2(fabs(m_num->value() - 1.0) < 1e-9,
                 "write access disabled but the value changed");

        m_num->setAccessW(true);
        QTest::keyClick(tgt, Qt::Key_Up);
        QCOMPARE(spy.count(), 1);
        QVERIFY(fabs(m_num->value() - 1.01) < 1e-9);
    }

    void t_suppressBlocksInteraction()
    {
        configure(3, 2);
        m_num->silentSetValue(1.0e19); /* suppression */
        QWidget *tgt = inputTarget();
        QVERIFY(tgt);

        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        for (int k = 0; k <= 4; k++) QTest::keyClick(tgt, Qt::Key_Right);
        QTest::keyClick(tgt, Qt::Key_Up);
        QCOMPARE(spy.count(), 0);
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));

        m_num->silentSetValue(5.0); /* recovery */
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(500, 3, 2));
    }

    void t_mouseLeaveRevertsToChannelValue(bool expectRevert)
    {
        configure(3, 2);
        m_num->silentSetValue(5.0);
        m_num->setValue(7.5); /* unconfirmed user value */
        QVERIFY(fabs(m_num->value() - 7.5) < 1e-9);

        QEvent leave(QEvent::Leave);
        QApplication::sendEvent(inputTarget(), &leave);

        const double expected = expectRevert ? 5.0 : 7.5;
        QVERIFY2(fabs(m_num->value() - expected) < 1e-9,
                 qPrintable(QString("after Leave the value is %1, expected %2")
                                .arg(m_num->value(), 0, 'g', 17).arg(expected, 0, 'g', 17)));
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 3, 2, &singleOk),
                 numExpectedDisplayString(expectRevert ? 500 : 750, 3, 2));
    }

    void t_signalArgumentOnIncrement()
    {
        configure(3, 2);
        m_num->setValue(1.0);
        QWidget *tgt = inputTarget();
        QVERIFY(tgt);
        for (int k = 0; k <= 4; k++) QTest::keyClick(tgt, Qt::Key_Right);

        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        QTest::keyClick(tgt, Qt::Key_Up);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDouble(), m_num->value());
        QVERIFY(fabs(m_num->value() - 1.01) < 1e-9);
    }

    void verifyPointBetween(int leftIdx, int rightIdx)
    {
        QLabel *point = m_num->template findChild<QLabel *>("layoutmember.");
        QLabel *left =
            m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(leftIdx));
        QLabel *right =
            m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(rightIdx));
        QVERIFY(point && left && right);
        const int px = point->geometry().center().x();
        QVERIFY2(left->geometry().center().x() < px && px < right->geometry().center().x(),
                 qPrintable(QString("decimal point label at x=%1 is not between digit %2 "
                                    "(x=%3) and digit %4 (x=%5)")
                                .arg(px).arg(leftIdx).arg(left->geometry().center().x())
                                .arg(rightIdx).arg(right->geometry().center().x())));
    }

    void t_decimalPointPosition()
    {
        /* the display string tests synthesize the decimal point themselves;
         * here the point label position is verified geometrically */
        configure(3, 2);
        m_num->setValue(12.34);
        /* the host has no layout: give the widget a real size explicitly,
         * otherwise the digit labels squeeze into overlapping positions */
        m_host->resize(420, 100);
        m_num->setGeometry(5, 5, 400, 80);
        m_host->show();
        QVERIFY(QTest::qWaitForWindowExposed(m_host));
        QApplication::processEvents();
        verifyPointBetween(2, 3);

        /* the point must move together with an automatic digit shift */
        m_num->silentSetValue(1234.5); /* (3,2) -> (4,1) */
        QCOMPARE(m_num->intDigits(), 4);
        QApplication::processEvents();
        QWidget *w = inputTarget();
        if (w && w->layout()) w->layout()->activate();
        QApplication::processEvents();
        QTest::qWait(20);
        verifyPointBetween(3, 4);
    }

    void t_libDigitPattern()
    {
        /* the sequence caQtDM_Lib uses: setIntDigits(width-prec-1); setDecDigits(prec) */
        struct Row { int width; int prec; int expI; int expD; double v; };
        const Row rows[] = {
            {10, 3, 6, 3, 12.345},
            {8, 7, 1, 7, 0.1234567},   /* width-prec-1 = 0: clamped to 1 */
            {25, 17, 7, 11, 12.5},     /* prec clamped by the 18 digit invariant */
        };
        for (size_t r = 0; r < sizeof(rows) / sizeof(rows[0]); r++) {
            t_cleanup();
            t_init();
            m_num->setIntDigits(rows[r].width - rows[r].prec - 1);
            m_num->setDecDigits(rows[r].prec);
            QCOMPARE(m_num->intDigits(), rows[r].expI);
            QCOMPARE(m_num->decDigits(), rows[r].expD);
            m_num->silentSetValue(rows[r].v);
            QVERIFY2(fabs(m_num->value() - rows[r].v) < 1e-6 * qMax(1.0, fabs(rows[r].v)),
                     qPrintable(QString("width=%1 prec=%2: value() = %3 after channel value %4")
                                    .arg(rows[r].width).arg(rows[r].prec)
                                    .arg(m_num->value(), 0, 'g', 17)
                                    .arg(rows[r].v, 0, 'g', 17)));
        }
    }

    void t_autoShiftKeepsConfiguredLimits()
    {
        /* documented behaviour: after an automatic shift the configured limits
         * stay in effect — increments beyond them are rejected, decrements
         * towards them are allowed */
        configure(2, 4); /* d_max = 99.9999 */
        m_num->silentSetValue(12345.6); /* shifts to (5,1) */
        QCOMPARE(m_num->intDigits(), 5);
        QWidget *tgt = inputTarget();
        QVERIFY(tgt);
        for (int k = 0; k <= 5; k++) QTest::keyClick(tgt, Qt::Key_Right); /* LSD */

        QSignalSpy spy(m_num, SIGNAL(valueChanged(double)));
        QVERIFY(spy.isValid());
        QTest::keyClick(tgt, Qt::Key_Up); /* beyond the configured limits */
        QVERIFY2(fabs(m_num->value() - 12345.6) < 1e-9,
                 qPrintable(QString("increment beyond the limits was accepted: %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(spy.count(), 0);

        QTest::keyClick(tgt, Qt::Key_Down); /* towards the limits */
        QVERIFY2(fabs(m_num->value() - 12345.5) < 1e-9,
                 qPrintable(QString("decrement towards the limits failed: %1")
                                .arg(m_num->value(), 0, 'g', 17)));
        QCOMPARE(spy.count(), 1);
    }

    void t_autoShiftNegativeValues()
    {
        configure(2, 4);
        m_num->silentSetValue(-12345.6); /* the sign does not count as a digit */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);
        QVERIFY(fabs(m_num->value() + 12345.6) < 1e-9);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 5, 1, &singleOk),
                 numExpectedDisplayString(-123456, 5, 1));

        m_num->silentSetValue(-1.25); /* back to the original configuration */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        QCOMPARE(numReadDisplayedString(m_num, 2, 4, &singleOk),
                 numExpectedDisplayString(-12500, 2, 4));
    }

    void t_fixedFormatDisablesAutoShift()
    {
        configure(2, 4);
        m_num->setFixedFormat(true);
        m_num->silentSetValue(12345.6); /* does not fit 2 integer digits: suppression */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));

        m_num->setFixedFormat(false);
        m_num->silentSetValue(12345.6); /* now the shift is allowed again */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 5, 1, &singleOk),
                 numExpectedDisplayString(123456, 5, 1));
    }

    void t_autoShiftIsOptIn()
    {
        /* opt-in: without the shift an oversized value suppresses instead */
        QVERIFY2(!m_num->getAutoDigitShift(),
                 "automatic digit shift must be off by default");

        configure(2, 4);
        m_num->setAutoDigitShift(false);
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);

        m_num->silentSetValue(12345.6); /* does not fit 2 integer digits */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QVERIFY2(first->text() == QString("*"),
                 "without the shift an oversized value must suppress the input");

        m_num->silentSetValue(1.5); /* fits again, still no shift */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 2, 4, &singleOk),
                 numExpectedDisplayString(15000, 2, 4));
    }

    void t_setDigitsIsAtomic()
    {
        /* setDigits must not clamp either count against the other's old value */
        configure(2, 15);
        m_num->setAutoDigitShift(false);
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 15);

        m_num->setDigits(7, 2);
        QVERIFY2(m_num->intDigits() == 7,
                 qPrintable(QString("setDigits(7,2) from (2,15) gave %1 integer digits, "
                                    "the decimal baseline still limits it")
                                .arg(m_num->intDigits())));
        QCOMPARE(m_num->decDigits(), 2);

        m_num->setDigits(2, 15); /* and back */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 15);
    }

    void t_fixedFormatFreezesCurrentLayout()
    {
        /* switching fixed format on at runtime freezes the layout as displayed,
         * switching it off resolves it again from the configured baseline */
        configure(2, 4);
        m_num->silentSetValue(12345.6); /* auto shift to (5,1) */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);

        m_num->setFixedFormat(true);
        m_num->silentSetValue(1.7); /* small value: the layout must stay frozen */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 5, 1, &singleOk),
                 numExpectedDisplayString(17, 5, 1));

        m_num->silentSetValue(123456.7); /* does not fit 5 integer digits: suppression */
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 1);

        m_num->silentSetValue(42.5); /* recovery, still frozen */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(numReadDisplayedString(m_num, 5, 1, &singleOk),
                 numExpectedDisplayString(425, 5, 1));

        m_num->setFixedFormat(false); /* resolves back to the (2,4) baseline */
        QCOMPARE(m_num->intDigits(), 2);
        QCOMPARE(m_num->decDigits(), 4);
        m_num->silentSetValue(1.7);
        QCOMPARE(numReadDisplayedString(m_num, 2, 4, &singleOk),
                 numExpectedDisplayString(17000, 2, 4));
    }

    void t_digitChangeRecoversFromSuppression()
    {
        /* a digit configuration change must resolve the suppression state in
         * both directions without waiting for the next channel value */
        configure(3, 2);
        m_num->setFixedFormat(true);
        m_num->silentSetValue(1234.5); /* does not fit 3 integer digits: stars */
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));

        m_num->setIntDigits(5); /* enough digits: must recover without a new value */
        QCOMPARE(m_num->intDigits(), 5);
        QCOMPARE(m_num->decDigits(), 2);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 5, 2, &singleOk),
                 numExpectedDisplayString(123450, 5, 2));

        m_num->setIntDigits(3); /* too small again: back to stars, no index digits */
        for (int i = 0; i < 5; i++) {
            QLabel *l = m_num->template findChild<QLabel *>(QString("layoutmember%1").arg(i));
            QVERIFY(l);
            QCOMPARE(l->text(), QString("*"));
        }

        m_num->setIntDigits(4); /* and recover once more */
        QCOMPARE(numReadDisplayedString(m_num, 4, 2, &singleOk),
                 numExpectedDisplayString(123450, 4, 2));
    }

    void t_suppressionRecoveryEdges_data()
    {
        QTest::addColumn<int>("intDig");
        QTest::addColumn<int>("decDig");
        QTest::addColumn<bool>("fixedFmt");
        QTest::addColumn<double>("badValue");
        QTest::addColumn<int>("recoverIntDig"); /* 0: skip the digit recovery step */
        QTest::addColumn<bool>("digitRecovers");
        QTest::addColumn<double>("goodValue");

        QTest::newRow("reported case") << 3 << 2 << true << 1234.5 << 5 << true << 42.5;
        QTest::newRow("smallest overflow") << 1 << 0 << true << 10.0 << 2 << true << 5.0;
        QTest::newRow("huge 18 digit value") << 3 << 2 << true << 9.99e17 << 16 << false << 0.05;
        QTest::newRow("full integer capacity") << 18 << 0 << false << 1.0e18 << 0 << false << 9.98e17;
        /* setDigits gives the integer digits priority over the shared budget:
         * asking for 3 of them yields 3 (and 15 decimals), so 100.0 becomes
         * displayable again — previously the request was clamped away by the
         * 16 configured decimals and the widget stayed suppressed */
        QTest::newRow("tiny decimals, full budget") << 2 << 16 << true << 100.0 << 3 << true << 6.0e-17;
        QTest::newRow("negative big") << 3 << 2 << true << -99999.9 << 5 << true << -0.5;
        QTest::newRow("dbl_max saturates") << 5 << 2 << true << DBL_MAX << 6 << false << 12.5;
        QTest::newRow("negative saturation") << 5 << 2 << true << -1.0e19 << 6 << false << -0.5;
        QTest::newRow("non-fixed overflow") << 3 << 2 << false << 1.0e7 << 0 << false << 1.5;
        QTest::newRow("subnormal recovery") << 2 << 4 << true << 1.0e19 << 0 << false << 1.0e-310;
    }

    void t_suppressionRecoveryEdges()
    {
        QFETCH(int, intDig);
        QFETCH(int, decDig);
        QFETCH(bool, fixedFmt);
        QFETCH(double, badValue);
        QFETCH(int, recoverIntDig);
        QFETCH(bool, digitRecovers);
        QFETCH(double, goodValue);

        configure(intDig, decDig);
        if (fixedFmt) m_num->setFixedFormat(true);

        m_num->silentSetValue(badValue); /* must enter the suppression state */
        QLabel *first = m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(first);
        QCOMPARE(first->text(), QString("*"));

        if (recoverIntDig > 0) {
            m_num->setIntDigits(recoverIntDig);
            first = m_num->template findChild<QLabel *>("layoutmember0"); /* maybe rebuilt */
            QVERIFY(first);
            if (digitRecovers) {
                bool ok = true, singleOk = true;
                const int D = m_num->decDigits();
                const long long expected = numFixedPointFromDecimalString(
                    numOracleDecimalString(badValue, D), D, &ok);
                QVERIFY(ok);
                QCOMPARE(numReadDisplayedString(m_num, m_num->intDigits(), D, &singleOk),
                         numExpectedDisplayString(expected, m_num->intDigits(), D));
                /* shrinking again suppresses with stars, not with index digits */
                m_num->setIntDigits(intDig);
                for (int i = 0; i < m_num->intDigits() + m_num->decDigits(); i++) {
                    QLabel *l = m_num->template findChild<QLabel *>(
                        QString("layoutmember%1").arg(i));
                    QVERIFY(l);
                    QCOMPARE(l->text(), QString("*"));
                }
            } else {
                QCOMPARE(first->text(), QString("*"));
            }
        }

        /* a processable channel value always recovers the display */
        m_num->silentSetValue(goodValue);
        bool ok = true, singleOk = true;
        const int I = m_num->intDigits(), D = m_num->decDigits();
        const long long expected = numFixedPointFromDecimalString(
            numOracleDecimalString(goodValue, D), D, &ok);
        QVERIFY(ok);
        QCOMPARE(numReadDisplayedString(m_num, I, D, &singleOk),
                 numExpectedDisplayString(expected, I, D));
        QVERIFY(singleOk);
    }

    void t_disconnectedColorUpdatesAreFiltered()
    {
        /* the update cycle re-sends the disconnected state permanently; the
         * widget must restyle (and force a repaint resize) only once */
        configure(3, 2);
        m_num->silentSetValue(12.5);

        const int modes[] = { 0 /* Static */, 1 /* Default */ };
        for (size_t m = 0; m < sizeof(modes) / sizeof(modes[0]); m++) {
            m_num->setColorMode((typename WidgetT::colMode) modes[m]);
            m_num->setConnectedColors(true);
            m_num->setConnectedColors(false); /* the first disconnect may restyle */
            if (modes[m] == 1)
                QVERIFY2(m_num->styleSheet().isEmpty(),
                         "Default mode must inherit the panel palette while disconnected");

            NumStyleChangeCounter counter;
            m_num->installEventFilter(&counter);
            for (int k = 0; k < 20; k++) m_num->setConnectedColors(false);
            m_num->removeEventFilter(&counter);
            QVERIFY2(counter.count == 0,
                     qPrintable(QString("colorMode %1: %2 style changes during 20 "
                                        "repeated disconnected updates")
                                    .arg(modes[m]).arg(counter.count)));

            /* the reconnect restores the configured colors */
            m_num->setConnectedColors(true);
            if (modes[m] == 1)
                QVERIFY(m_num->styleSheet().isEmpty());
            QVERIFY(!m_num->styleSheet().contains("rgba(255, 255, 255"));
        }
    }

    void t_channelUpdateStormDoesNotRebuild()
    {
        /* the caQtDM_Lib per-update sequence setIntDigits/setDecDigits/
         * silentSetValue must not rebuild the digit widgets once settled */
        configure(3, 15);
        m_num->silentSetValue(123456789.5); /* needs 9 integer digits */
        QCOMPARE(m_num->intDigits(), 9);
        QCOMPARE(m_num->decDigits(), 9);

        QPointer<QLabel> firstLabel =
            m_num->template findChild<QLabel *>("layoutmember0");
        QVERIFY(firstLabel);

        for (int k = 0; k < 20; k++) {
            m_num->setIntDigits(3);
            m_num->setDecDigits(15);
            m_num->silentSetValue(123456789.5 + k);
        }
        QVERIFY2(!firstLabel.isNull(),
                 "digit labels were rebuilt during a plain channel update storm");
        QCOMPARE(m_num->template findChild<QLabel *>("layoutmember0"),
                 firstLabel.data());
        QCOMPARE(m_num->intDigits(), 9);
        QCOMPARE(m_num->decDigits(), 9);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 9, 9, &singleOk),
                 numExpectedDisplayString(Q_INT64_C(123456808500000000), 9, 9));
    }

    void t_baselineChangeWhileShifted()
    {
        /* a baseline change while the auto shift is active must resolve against
         * the new baseline, not against the displayed digits */
        configure(3, 15);
        m_num->silentSetValue(123456789.5); /* shifted to (9,9) */
        QCOMPARE(m_num->intDigits(), 9);
        QCOMPARE(m_num->decDigits(), 9);

        m_num->setDecDigits(12); /* baseline (3,12), the value still needs 9 */
        QCOMPARE(m_num->intDigits(), 9);
        QCOMPARE(m_num->decDigits(), 6);

        m_num->silentSetValue(1.5); /* small again: settle on the new baseline */
        QCOMPARE(m_num->intDigits(), 3);
        QCOMPARE(m_num->decDigits(), 12);
        bool singleOk = true;
        QCOMPARE(numReadDisplayedString(m_num, 3, 12, &singleOk),
                 numExpectedDisplayString(Q_INT64_C(1500000000000), 3, 12));
    }

    void t_noDelayedResizeOnPlainValueUpdate()
    {
        /* a plain value update must not schedule the delayed synthetic resize
         * (it repaints all button pixmaps); only layout rebuilds may */
        configure(3, 2);
        m_num->silentSetValue(1.5);
        QTest::qWait(1200); /* drain the delayed resize of the initial build */

        QPushButton *btn = m_num->template findChild<QPushButton *>("layoutmember0");
        QVERIFY(btn);
        QVERIFY2(!btn->icon().isNull(), "delayed resize of the initial build did not run");
        const qint64 key = btn->icon().cacheKey();

        m_num->silentSetValue(2.5); /* same layout: no resize may be scheduled */
        QTest::qWait(1200);
        QCOMPARE(btn->icon().cacheKey(), key);
    }

    QWidget *m_host;
    WidgetT *m_num;
};

#endif // TST_NUMERIC_SUITE_H
