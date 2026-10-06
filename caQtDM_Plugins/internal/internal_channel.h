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
 *  Contact details:
 *    helge.brands@psi.ch
 */
#ifndef INTERNAL_CHANNEL_H
#define INTERNAL_CHANNEL_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include <stdint.h>

#include "knobData.h"

// a limit coming from the JSON configuration that may or may not be defined;
// reads nicer than parallel has* flags
struct OptionalLimit
{
    double value;
    bool defined;

    OptionalLimit() : value(0.0), defined(false) {}
    void set(double newValue) { value = newValue; defined = true; }
};

// the current value in its native EPICS type (like the epics3 plugin);
// only the member matching the channel fieldtype is used
struct NativeValue
{
    qint16  int16Value;   // caINT   (dbr_short_t)
    qint32  int32Value;   // caLONG  (dbr_long_t)
    float   floatValue;   // caFLOAT (dbr_float_t)
    double  doubleValue;  // caDOUBLE (dbr_double_t), also the index of string regex channels
    quint16 enumValue;    // caENUM  (dbr_enum_t)
    quint8  charValue;    // caCHAR  (dbr_char_t)

    NativeValue()
        : int16Value(0), int32Value(0), floatValue(0.0f), doubleValue(0.0)
        , enumValue(0), charValue(0) {}
};

/*
 * One simulated channel of the "internal" plugin. The JSON configuration uses
 * the EPICS field names (val, step, period, drvl/drvh, hopr/lopr, low/lolo/high/hihi,
 * nelm/nord, units, prec, enums, regex, overflow, persistent, mode, type) and is
 * assembled by the genSoftPV widget.
 */
class InternalChannel
{
public:
    enum Mode { Constant = 0, Counter };

    // record fields addressable as internal://NAME.FIELD, VAL is the default
    enum Field { FieldVal = 0, FieldSevr, FieldStat, FieldLow, FieldLolo, FieldHigh, FieldHihi,
                 FieldDrvl, FieldDrvh, FieldHopr, FieldLopr, FieldPrec, FieldEgu, FieldNelm, FieldNord };

    InternalChannel();

    // false for unknown extensions: such channels are not handled at all
    static bool splitField(const QString &pv, QString *base, Field *field);
    static QString baseName(const QString &pv);
    static QString jsonPart(const QString &pv);
    // canonical (compact, sorted keys) form of a configuration for comparisons
    static QString normalizedJson(const QString &json);

    // parses the JSON configuration; on error the previous state stays untouched
    bool configure(const QString &json, QString *errorString = Q_NULLPTR);
    bool isConfigured() const { return m_configured; }

    // accumulates elapsed time and executes counter steps; returns true when the value changed
    bool advance(int elapsedMs);
    // one single counter step, independent of the period (used by advance and the tests)
    void tick();

    // the native value converted to double / stored from double (with the
    // truncation and wrap around of the native EPICS type)
    double currentValue() const;
    void setCurrentValue(double newValue);

    // write access (pvSetValue / pvSetWave)
    void setValue(double rdata, qint32 idata, const QString &sdata);
    void setWave(const QVector<double> &values);
    void setFieldValue(Field field, double rdata, qint32 idata, const QString &sdata);

    void fillKnobData(knobData *kData) const;
    void fillKnobDataField(knobData *kData, Field field) const;
    QVariant fieldVariant(Field field) const;

    void alarmState(double checkValue, short *severity, short *status) const;

    // a matrix channel (JSON "dim": [rows, cols]) has a fixed element count
    // and an always materialized, row-major element store (m_waveOverride)
    bool isMatrix() const { return dimRows > 0; }

    // string matching the regex pattern for the given enumeration index (string channels)
    QString generatedString(qint64 index) const;
    // number of different strings the regex pattern can produce (0 = no pattern)
    qint64 combinations() const { return m_combinations; }

    // configuration, plain data (also convenient for the unit tests), EPICS field names
    short fieldtype;            // caType from knobDefines.h
    Mode mode;
    double val;                 // VAL: initial value
    double step;
    int periodMs;
    OptionalLimit drvl;         // DRVL/DRVH: drive limits (counter range, control limits)
    OptionalLimit drvh;
    OptionalLimit hopr;         // HOPR/LOPR: operating range, display/input limits, clamped to DRVL/DRVH
    OptionalLimit lopr;
    OptionalLimit low;          // LOW/LOLO/HIGH/HIHI: alarm limits
    OptionalLimit lolo;
    OptionalLimit high;
    OptionalLimit hihi;
    bool overflow;
    bool persistent;            // keeps the channel alive without any monitor
    int nelm;                   // NELM: maximum array size
    int nord;                   // NORD: number of elements actually used (<= nelm)
    int dimRows;                // matrix shape, 0 = plain scalar/waveform
    int dimCols;
    QString units;
    short precision;
    QStringList enums;
    QString text;
    QStringList textArray;      // string waveform content given as "val" array
    QString regexPattern;
    QString configJson;         // accepted configuration (normalized), later definitions are compared to it
    QString definedBy;          // origin of the accepted configuration (widget, file), set by the plugin

    // state; severity/status are re-evaluated from the limits on every value change,
    // a SEVR/STAT write sets them directly until then (NOTCONNECTED = 99)
    NativeValue native;
    bool needsPublish;
    // a control-info field (limits, precision, units) changed since the last
    // publish; the plugin uses this to force initialize=true on the next
    // update, mirroring an EPICS DBE_PROPERTY event
    bool controlInfoChanged;
    short severity;
    short status;

private:
    double elementValue(int i) const;
    void counterRange(double *rangeLow, double *rangeHigh) const;
    double clampToDriveLimits(double value) const;
    void updateAlarmState();
    static bool parseRegexPattern(const QString &pattern, QList<QStringList> *segments,
                                  qint64 *combinations, QString *errorString);

    QList<QStringList> m_segments;  // regex pattern broken into enumerable segments
    qint64 m_combinations;
    QString m_regexReplace;         // replacement template for written strings (the configured val)
    QVector<double> m_waveOverride;
    int m_elapsedMs;
    bool m_configured;
};

#endif // INTERNAL_CHANNEL_H
