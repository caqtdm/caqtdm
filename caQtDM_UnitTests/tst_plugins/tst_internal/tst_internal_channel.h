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
#ifndef TST_INTERNAL_CHANNEL_H
#define TST_INTERNAL_CHANNEL_H

#include <QObject>
#include <QTest>

#include "internal_channel.h"

class TestInternalChannel : public QObject
{
    Q_OBJECT
public:
    TestInternalChannel() = default;

private slots:
    void baseNameAndJsonPartWork();
    void fieldSyntaxWorks();
    void splitFieldStripsSchemePrefix();
    void counterStopsWhenInvalidOrDisconnected();
    void fieldWritesAndForcingWork();
    void fillKnobDataFieldWorks();
    void configureParsesAllFields();
    void configureUsesDefaults();
    void configureRejectsInvalidInput();
    void counterTicksAndWraps();
    void nativeTypesWrapLikeEpics();
    void advanceRespectsPeriod();
    void regexGeneratorWorks();
    void regexProcessesWrittenStrings();
    void alarmLimitsWork();
    void fillKnobDataScalarTypesWork();
    void fillKnobDataArraysWork();
    void hoprLoprWork();
    void drvlDrvhClampTheValue();
    void nordAndNelmWorkLikeEpics();
    void setValueWorks();
    void setWaveWorks();
    void unconfiguredChannelIsNotConnected();
    void configureMatrixWorks();
    void matrixWritesKeepShape();
    void matrixCounterTicks();
    void fillKnobDataMatrixWorks();
    void configJsonIsStoredNormalized();
};

#endif // TST_INTERNAL_CHANNEL_H
