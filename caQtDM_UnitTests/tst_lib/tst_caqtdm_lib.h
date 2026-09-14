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
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

#ifndef TST_CAQTDM_LIB_H
#define TST_CAQTDM_LIB_H

#include "fakefileopenwindow.h"

#include <caqtdm_lib.h>
#include <internal_plugin.h>

#include <QObject>
#include <QTest>

class caInclude;

class TestCaQtDM_Lib : public QObject
{
    Q_OBJECT
public:
    TestCaQtDM_Lib() = default;

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void cleanup();
    void checkJsonStringWorks();
    void parseForDisplayRateWorks();
    void parseForQRectConstWorks();
    void treatMacroWorks();
    void getLongValueFromStringWorks();
    void getDoubleValueFromStringWorks();
    void computeNumericMaxMinPrecUsesChannelDispLimits();
    void computeNumericMaxMinPrecFallsBackWithoutLimits();
    void computeNumericMaxMinPrecIgnoresChannelInUserMode();
    void computeNumericMaxMinPrecHonoursMaxChannelPrecision();
    void computeNumericMaxMinPrecUpdatesWhenChannelChanges();

private:
    FakeFileOpenWindow *m_fakeFileOpenWindow;
    caInclude *m_parentAS;
    MutexKnobData *m_mutexKnobData;
    InternalPlugin *m_internalPlugin;
    CaQtDM_Lib *m_caQtDM_Lib;
};

#endif // TST_CAQTDM_LIB_H
