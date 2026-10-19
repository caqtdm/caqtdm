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

#include <QCoreApplication>
#include <QTest>
#include <clocale>

#include "tst_gensoftpv.h"
#include "tst_canumeric.h"
#include "tst_caspinbox.h"
#include "tst_caapplynumeric.h"
#include "tst_caimage.h"
#include "tst_pvdialog.h"
#include "tst_alhparser.h"
#include "tst_alarmtree_logic.h"
#include "tst_themecolors.h"
#include "tst_textwidgets_theme.h"
#include "tst_buttons_theme.h"
#include "tst_containers_theme.h"
#include "tst_painted_theme.h"

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    qputenv("QT_QPA_FONTDIR", QByteArrayLiteral("."));
    // as the viewer does: panel palettes propagate into stylesheet widgets
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
#endif

    QApplication app(argc, argv);
    QApplication::setOrganizationName("Paul Scherrer Institut");
    QApplication::setApplicationName("caQtDM-UnitTests-QtControls");
    // to avoid failing tests because of a comma as decimal separator
    setlocale(LC_NUMERIC, "C");
    int status = 0;

    {
        TestPVDialog tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestGenSoftPV tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestCaNumeric tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestCaSpinbox tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestCaApplyNumeric tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestCaImage tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestAlhParser tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestAlarmTreeLogic tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestThemeColors tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestTextWidgetsTheme tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestButtonsTheme tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestContainersTheme tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    {
        TestPaintedTheme tc;
        status |= QTest::qExec(&tc, argc, argv);
    }

    return status;
}
