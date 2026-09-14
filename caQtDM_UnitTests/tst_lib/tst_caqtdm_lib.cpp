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

#include "tst_caqtdm_lib.h"

#include <caapplynumeric.h>
#include <cainclude.h>
#include <canumeric.h>
#include <caspinbox.h>
#include <gensoftpv.h>

#include <cfloat>
#include <climits>

// registers a genSoftPV-defined internal channel and pumps one publish cycle
static genSoftPV *registerInternalGenSoftPV(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                            const QString &variable, const QString &hopr,
                                            const QString &lopr, const QString &drvl = QString(),
                                            const QString &drvh = QString(), int precision = -1)
{
    genSoftPV *softpv = new genSoftPV(host);
    softpv->setVariable(variable);
    softpv->setDataType(genSoftPV::Double);
    if(!hopr.isEmpty()) softpv->setHopr(hopr);
    if(!lopr.isEmpty()) softpv->setLopr(lopr);
    if(!drvl.isEmpty()) softpv->setDrvl(drvl);
    if(!drvh.isEmpty()) softpv->setDrvh(drvh);
    if(precision >= 0) softpv->setPrecision(precision);
    lib->HandleWidget(softpv, "", true, true);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);
    return softpv;
}

// writes a field of a running internal channel through the real plugin API
// (the caput equivalent) and lets it publish the resulting change
static void writeInternalField(InternalPlugin *plugin, const QString &variable, const QString &field,
                               double value)
{
    // no "internal://" scheme prefix: CaQtDM_Lib::addMonitor() strips that
    // before ever reaching the plugin interface, and InternalChannel::splitField()
    // expects the already-stripped form
    QByteArray pv = (variable + "." + field).toLatin1();
    plugin->pvSetValue(pv.data(), value, 0, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, 0);
}

// shared across caApplyNumeric/caNumeric/caSpinbox; each caller needs a unique
// variable name since InternalPlugin caches channels by base name
template<class WidgetT>
static void checkChannelDispLimits(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                    const QString &variable)
{
    registerInternalGenSoftPV(lib, plugin, host, variable, "90", "10");

    WidgetT *w = new WidgetT(host);
    w->setLimitsMode(WidgetT::Channel);
    w->setPV("internal://" + variable);
    lib->HandleWidget(w, "", false, false);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);

    QCOMPARE(w->getMinValue(), 10.0);
    QCOMPARE(w->getMaxValue(), 90.0);

    // the actual purpose of HOPR/LOPR: values outside the range are rejected
    w->setValue(50.0);
    QCOMPARE(w->value(), 50.0);
    w->setValue(500.0);   // above HOPR
    QCOMPARE(w->value(), 50.0);
    w->setValue(-500.0);  // below LOPR
    QCOMPARE(w->value(), 50.0);

    delete w;
}

template<class WidgetT>
static void checkFallbackWithoutLimits(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                        const QString &variable)
{
    // neither HOPR/LOPR nor DRVL/DRVH configured
    registerInternalGenSoftPV(lib, plugin, host, variable, "", "");

    WidgetT *w = new WidgetT(host);
    w->setLimitsMode(WidgetT::Channel);
    w->setPV("internal://" + variable);
    lib->HandleWidget(w, "", false, false);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);

    QCOMPARE(w->getMinValue(), -100000.0);
    QCOMPARE(w->getMaxValue(), 100000.0);

    delete w;
}

template<class WidgetT>
static void checkUserModeIgnoresChannel(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                        const QString &variable)
{
    registerInternalGenSoftPV(lib, plugin, host, variable, "90", "10");

    WidgetT *w = new WidgetT(host);
    w->setLimitsMode(WidgetT::User);
    w->setMinValue(1.0);
    w->setMaxValue(2.0);
    w->setPV("internal://" + variable);
    lib->HandleWidget(w, "", false, false);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);

    QCOMPARE(w->getMinValue(), 1.0);
    QCOMPARE(w->getMaxValue(), 2.0);

    delete w;
}

template<class WidgetT>
static void checkWidgetPicksUpFieldWrite(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                         const QString &variable)
{
    // HOPR/LOPR inside DRVL/DRVH: the widget follows HOPR/LOPR first
    registerInternalGenSoftPV(lib, plugin, host, variable, "90", "10", "0", "100");

    WidgetT *w = new WidgetT(host);
    w->setLimitsMode(WidgetT::Channel);
    w->setPV("internal://" + variable);
    lib->HandleWidget(w, "", false, false);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);

    QCOMPARE(w->getMinValue(), 10.0);
    QCOMPARE(w->getMaxValue(), 90.0);

    // reset HOPR/LOPR to 0 through the real plugin write path (like a caput
    // to NAME.HOPR/NAME.LOPR); the widget must recognize the change and fall
    // back to DRVL/DRVH
    writeInternalField(plugin, variable, "HOPR", 0.0);
    writeInternalField(plugin, variable, "LOPR", 0.0);

    QCOMPARE(w->getMinValue(), 0.0);
    QCOMPARE(w->getMaxValue(), 100.0);

    delete w;
}

// maxChannelPrecision caps the decimals taken from the channel; default 4 is
// what caQtDM_Lib had hard coded before the property existed
template<class WidgetT>
static void checkMaxChannelPrecision(CaQtDM_Lib *lib, InternalPlugin *plugin, QWidget *host,
                                     const QString &variable)
{
    // no HOPR/LOPR: the limits fall back to +-100000
    registerInternalGenSoftPV(lib, plugin, host, variable, "", "", QString(), QString(), 8);

    WidgetT *w = new WidgetT(host);
    w->setPrecisionMode(WidgetT::Channel);
    w->setPV("internal://" + variable);
    lib->HandleWidget(w, "", false, false);

    QMetaObject::invokeMethod(plugin, "updateChannels", Qt::DirectConnection);

    QCOMPARE(w->getMaxChannelPrecision(), 4);
    QCOMPARE(w->decDigits(), 4);

    // raising the cap lets the channel precision through on the next initialize
    w->setMaxChannelPrecision(10);
    writeInternalField(plugin, variable, "PREC", 8.0);
    QCOMPARE(w->decDigits(), 8);

    delete w;
}

void TestCaQtDM_Lib::initTestCase()
{
    // code to be executed before the first test function

    m_fakeFileOpenWindow = Q_NULLPTR;
    m_parentAS = Q_NULLPTR;
    m_mutexKnobData = Q_NULLPTR;
    m_internalPlugin = Q_NULLPTR;
    m_caQtDM_Lib = Q_NULLPTR;
}

void TestCaQtDM_Lib::init()
{
    // code to be executed before each test function

    m_fakeFileOpenWindow = new FakeFileOpenWindow(Q_NULLPTR);
    m_parentAS = new caInclude(m_fakeFileOpenWindow);
    m_mutexKnobData = new MutexKnobData();
    m_mutexKnobData->UpdateMechanism(MutexKnobData::UpdateDirect); // synchronous widget updates

    m_internalPlugin = new InternalPlugin();
    m_internalPlugin->initCommunicationLayer(m_mutexKnobData, Q_NULLPTR, QMap<QString, QString>());

    QMap<QString, ControlsInterface *> interfaces;
    interfaces.insert("internal", qobject_cast<ControlsInterface *>(m_internalPlugin));

    m_caQtDM_Lib = new CaQtDM_Lib(m_fakeFileOpenWindow,
                                  "",
                                  "",
                                  m_mutexKnobData,
                                  interfaces,
                                  Q_NULLPTR,
                                  false,
                                  m_parentAS,
                                  {});
}

void TestCaQtDM_Lib::cleanupTestCase()
{
    // code to be executed after the last test function
}

void TestCaQtDM_Lib::cleanup()
{
    // code to be executed after each test function

    delete m_caQtDM_Lib;
    delete m_internalPlugin;
    delete m_mutexKnobData;
    delete m_fakeFileOpenWindow;
}

void TestCaQtDM_Lib::checkJsonStringWorks()
{
    QString emptyJson = "{}";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(emptyJson), true);

    QString validJson
        = R"({"root":{"children":[{"id":"node-1","metadata":{"tags":["alpha","beta"],"priority":7}},{"id":"node-2","metadata":{"tags":[],"priority":0}}],"configuration":{"featureFlags":{"enableQuantumMode":false,"useLegacyFluxCapacitor":true},"thresholds":[0.1,0.5,0.9]},"auditTrail":[{"timestamp":"1970-01-01T00:00:00Z","action":"INITIALIZE"},{"timestamp":"2099-12-31T23:59:59Z","action":"TERMINATE"}],"nullableField":null,"emptyObject":{},"emptyArray":[]}})";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(validJson), true);

    QString validMultilineJson = R"({
        "a": 1,
        "caqtdm_monitor": {
            "maxdisplayrate": 42
        },
        "b": 2
    })";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(validMultilineJson), true);

    QString emptyString = "";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(emptyString), false);

    QString invalidJson
        = R"({"root":{"children":[{"id":"node-1","metadata":{"tags":["alpha","beta"],"priority":7}},{"id":"node-2","metadata":{"tags":[],"priority":0}}],"configuration":{"featureFlags":{"enableQuantumMode":false,"useLegacyFluxCapacitor":true},"thresholds":[0.1,0.5,0.9]},"auditTrail":[{"timestamp":"1970-01-01T00:00:00Z","action"2099-12-31T23:59:59Z","action":"TERMINATE"}],"nullableField":null,"emptyObject":{},"emptyArray":[]}})";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(invalidJson), false);

    QString invalidJsonMissingBracket
        = R"({"root":{"children":[{"id":"node-1","metadata":{"tags":["alpha","beta"],"priority":7}},{"id":"node-2","metadata":{"tags":[],"priority":0}}],"configuration":{"featureFlags":{"enableQuantumMode":false,"useLegacyFluxCapacitor":true},"thresholds":[0.1,0.5,0.9]},"auditTrail":[{"timestamp":"1970-01-01T00:00:00Z","action":"INITIALIZE"},{"timestamp":"2099-12-31T23:59:59Z","action":"TERMINATE"}],"nullableField":null,"emptyObject":{},"emptyArray":[]})";
    QCOMPARE(m_caQtDM_Lib->checkJsonString(invalidJsonMissingBracket), false);
}

void TestCaQtDM_Lib::parseForDisplayRateWorks()
{
    QString json = R"({"a": 1,"b": 2})";
    int rate = -1;

    int result = m_caQtDM_Lib->parseForDisplayRate(json, rate);
    QCOMPARE(result, 0);
    QCOMPARE(rate, -1);

    json = R"({
        "a": 1,
        "caqtdm_monitor": {
            "maxdisplayrate": 42
        },
        "b": 2
    })";
    rate = 0;

    result = m_caQtDM_Lib->parseForDisplayRate(json, rate);
    QCOMPARE(result, 1);
    QCOMPARE(rate, 42);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QCOMPARE(doc.isObject(), true);

    QJsonObject root = doc.object();
    QCOMPARE(root.contains("caqtdm_monitor"), false);
    QCOMPARE(root["a"].toInt(), 1);
    QCOMPARE(root["b"].toInt(), 2);

    json = R"({"arr":{"e":-1,"i":1,"s":1},"caqtdm_monitor":{"maxdisplayrate":1},"dbnd":{"abs":0},"sync":{"before":""}})";
    rate = 0;

    result = m_caQtDM_Lib->parseForDisplayRate(json, rate);
    qDebug() << json;
    QCOMPARE(result, 1);
    QCOMPARE(rate, 1);

    doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QCOMPARE(doc.isObject(), true);

    root = doc.object();
    QCOMPARE(root.contains("caqtdm_monitor"), false);
    QCOMPARE(root["arr"].toObject(), QJsonObject({{"e", -1}, {"i", 1}, {"s", 1}}));
    QCOMPARE(root["dbnd"].toObject(), QJsonObject({{"abs", 0}}));
    QCOMPARE(root["sync"].toObject(), QJsonObject({{"before", ""}}));
}

void TestCaQtDM_Lib::parseForQRectConstWorks()
{
    QString json = R"({"badkey":[1,2,3]})";
    double valueArray[3] = {0.0, 0.0, 0.0};

    QCOMPARE(m_caQtDM_Lib->parseForQRectConst(json, valueArray), false);
    QCOMPARE(valueArray[0], 0.0);
    QCOMPARE(valueArray[1], 0.0);
    QCOMPARE(valueArray[2], 0.0);
    QCOMPARE(json, QString(R"({"badkey":[1,2,3]})"));

    json = R"({"valueconst":[1,2,3]})";
    valueArray[0] = 1.0;
    valueArray[1] = 2.0;
    valueArray[2] = 3.0;

    QCOMPARE(m_caQtDM_Lib->parseForQRectConst(json, valueArray), true);
    QCOMPARE(valueArray[0], 1.0);
    QCOMPARE(valueArray[1], 2.0);
    QCOMPARE(valueArray[2], 3.0);
    QCOMPARE(json, QString(R"({"valueconst":[1,2,3]})"));
}

void TestCaQtDM_Lib::treatMacroWorks()
{
    m_caQtDM_Lib->unknownMacrosList.clear();
    m_caQtDM_Lib->level = 0;
    m_caQtDM_Lib->savedFile[0] = "test.ui";

    {
        QMap<QString, QString> map;
        map["A"] = "$(B)";
        map["B"] = "$(C)";
        map["C"] = "final";

        QString text = "prefix $(A) suffix";
        bool doNothing = true;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget"),
                 QString("prefix final suffix"));
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.isEmpty(), true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        QMap<QString, QString> map;
        map["A"] = "literal";

        QString text = R"(pv $(A{"regex":"literal","value":"JSON"}))";
        bool doNothing = true;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget"), QString("pv JSON"));
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.isEmpty(), true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        QMap<QString, QString> map;
        map["A"] = "prefix-123-suffix";

        QString text = R"(pv $(A{"regex":"[0-9]+","value":"456"}))";
        bool doNothing = true;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget"),
                 QString("pv prefix-456-suffix"));
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.isEmpty(), true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        QMap<QString, QString> map;
        map["A"] = "prefix-123-suffix";
        map["B"] = "456";

        QString text = R"lim(pv $(A{"regex":"[0-9]+","value":"$(B)"}))lim";
        bool doNothing = true;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget"),
                 QString("pv prefix-456-suffix"));
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.isEmpty(), true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        QMap<QString, QString> map;
        map["A"] = "one";

        QString text = "pv $(MISSING)";
        bool doNothing = false;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget42"),
                 QString("pv $(MISSING)"));
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.size(), 1);

        QString key = "(MISSING)###widget42###test.ui";
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.contains(key), true);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.value(key), QString("test.ui"));
    }

    {
        QMap<QString, QString> map;
        QString text = "pv $(A)";
        bool doNothing = false;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget"), QString("pv $(A)"));
        QCOMPARE(doNothing, true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        // Cases taken from the manual display test caQtDM_Tests/macrotest.ui,
        // which pairs each expression with its expected result for the macros
        // TEST=12345, TEST1=abcd, TESTLEER=<empty>; TEST2..TEST5 are undefined.
        QMap<QString, QString> map;
        map["TEST"] = "12345";
        map["TEST1"] = "abcd";
        map["TESTLEER"] = "";

        const struct {
            const char *input;
            const char *expected;
        } cases[] = {
            {R"lim($(TEST))lim", "12345"},
            {R"lim($(TEST)$(TEST1)$(TEST)$(TEST1)$(TEST)$(TEST1))lim",
             "12345abcd12345abcd12345abcd"},
            {R"lim(xyz$$876_$(TEST{"regex":"([1,2]+)([3])","value":"A\\1"})______)lim",
             "xyz$$876_A1245______"},
            {R"lim($(TEST{Bla})$(TEST1)$(TEST))lim", "12345abcd12345"},
            {R"lim($(TEST{"regex":"\\S+","value":"HUI2HUIHUI"}) $(TEST{Bla}) $(TEST1)$(TEST))lim",
             "HUI2HUIHUI 12345 abcd12345"},
            {R"lim($(TEST{"regex":"([1,2]+)([3])","value":"A"}))lim", "A45"},
            {R"lim($(TEST{"regex":"([1,2]+)([3])","value":"A\\2"}))lim", "A345"},
            {R"lim($(TEST5=This) $(TEST5=is) $(TEST5=a) $(TEST5=macro) $(TEST5=test))lim",
             "This is a macro test"},
            {R"lim( $(TEST=Error!!))lim", " 12345"},
            {R"lim($(TEST5{"regex":"\\S+","value":"$(TEST1)"}=RegExConst))lim", "RegExConst"},
            {R"lim($(TEST5{"regex":"\\S+","value":"$(TEST1)"}=$(TEST)))lim", "12345"},
            {R"lim($(TEST5=$(TEST4=$(TEST3=chain constant test))))lim", "chain constant test"},
            {R"lim($(TEST5=$(TEST4=$(TEST3=$(TEST1)))))lim", "abcd"},
            {R"lim($(TEST5=->$(TEST=$(TEST3=$(TEST4)))<-))lim", "->12345<-"},
            {R"lim($(TESTLEER=1$(TEST=2$(TEST3=3$(TEST4)3)2)1))lim", ""},
            {R"lim($(TESTLEER)<-LEER)lim", "<-LEER"},
            {R"lim(->$(TESTLEER=not OK)$(TEST=$(TEST3=Boing))$(TEST4=)<-)lim", "->12345<-"},
            {R"lim($(TESTLEER)-$(TEST))lim", "-12345"},
            {R"lim($(TEST)-$(TESTLEER)$(TEST)=0)lim", "12345-12345=0"},
            {R"lim(->$(TESTLEER=Error)<-)lim", "-><-"},
            {R"lim($(TEST2=1);$(TEST2=2);$(TEST2=3);$(TEST2=4);$(TEST2=5);$(TEST2=6))lim",
             "1;2;3;4;5;6"},
        };

        for (const auto &testCase : cases) {
            bool doNothing = true;
            QString result = m_caQtDM_Lib->treatMacro(map,
                                                      QString::fromUtf8(testCase.input),
                                                      &doNothing,
                                                      "widget");
            QVERIFY2(result == QString::fromUtf8(testCase.expected),
                     qPrintable(QString("input '%1' gave '%2', expected '%3'")
                                    .arg(testCase.input, result, testCase.expected)));
            QCOMPARE(doNothing, false);
        }
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.isEmpty(), true);
    }

    m_caQtDM_Lib->unknownMacrosList.clear();

    {
        // also from macrotest.ui: an undefined macro without default value
        // stays untouched and is registered as unknown
        QMap<QString, QString> map;
        map["TEST"] = "12345";
        map["TEST1"] = "abcd";
        map["TESTLEER"] = "";

        QString text = R"lim($(TEST2),not Resolved Macro=OK)lim";
        bool doNothing = true;

        QCOMPARE(m_caQtDM_Lib->treatMacro(map, text, &doNothing, "widget42"), text);
        QCOMPARE(doNothing, false);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.size(), 1);
        QCOMPARE(m_caQtDM_Lib->unknownMacrosList.contains("(TEST2)###widget42###test.ui"), true);
    }
}

void TestCaQtDM_Lib::getLongValueFromStringWorks()
{
    // the function may modify the buffer ('O' -> '0'), so writable buffers are required
    char buffer[SMALL_STRING_LENGTH];
    char *end = Q_NULLPTR;

    qstrncpy(buffer, "17", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::octal, &end), 15L);
    QCOMPARE(*end, '\0');

    // caQtDM represents octal values with an 'O' prefix
    qstrncpy(buffer, "O17", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::octal, &end), 15L);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "ff", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::hexadecimal, &end), 255L);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "0x1F", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::hexadecimal, &end), 31L);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "42", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 42L);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "-42", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), -42L);
    QCOMPARE(*end, '\0');

    // hexadecimal and octal notations are auto-detected in decimal mode
    qstrncpy(buffer, "0x1F", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 31L);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "O17", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 15L);
    QCOMPARE(*end, '\0');

    // trailing garbage: end points to the first unparsed character
    qstrncpy(buffer, "42abc", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 42L);
    QCOMPARE(*end, 'a');

    // nothing parsed at all: end stays at the beginning of the buffer
    qstrncpy(buffer, "abc", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 0L);
    QCOMPARE(end, buffer);

    // limits of long (size is platform dependent, so the inputs are generated)
    qstrncpy(buffer, QString::number(LONG_MAX).toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             LONG_MAX);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, QString::number(LONG_MIN).toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             LONG_MIN);
    QCOMPARE(*end, '\0');

    // decimal overflow saturates at the limits (strtol semantics)
    qstrncpy(buffer, "9999999999999999999999999", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             LONG_MAX);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "-9999999999999999999999999", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             LONG_MIN);
    QCOMPARE(*end, '\0');

    // all bits set in hexadecimal is ULONG_MAX (strtoul), which maps to -1 as long
    QString allF(int(2 * sizeof(long)), QChar('f'));
    qstrncpy(buffer, allF.toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getLongValueFromString(buffer, CaQtDM_Lib_Interface::hexadecimal, &end),
             -1L);
    QCOMPARE(*end, '\0');
}

void TestCaQtDM_Lib::getDoubleValueFromStringWorks()
{
    char buffer[SMALL_STRING_LENGTH];
    char *end = Q_NULLPTR;

    qstrncpy(buffer, "17", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::octal, &end), 15.0);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "ff", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::hexadecimal, &end), 255.0);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "3.14", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 3.14);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "-2.5e3", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), -2500.0);
    QCOMPARE(*end, '\0');

    // hexadecimal notation is auto-detected in decimal mode
    qstrncpy(buffer, "0x10", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 16.0);
    QCOMPARE(*end, '\0');

    // nothing parsed at all: end stays at the beginning of the buffer
    qstrncpy(buffer, "abc", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end), 0.0);
    QCOMPARE(end, buffer);

    // limits of double survive a round trip through their string representation
    qstrncpy(buffer, QString::number(DBL_MAX, 'g', 17).toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             DBL_MAX);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, QString::number(-DBL_MAX, 'g', 17).toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             -DBL_MAX);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, QString::number(DBL_MIN, 'g', 17).toLatin1().constData(), sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             DBL_MIN);
    QCOMPARE(*end, '\0');

    // overflow yields infinity (strtod semantics)
    qstrncpy(buffer, "1e309", sizeof(buffer));
    double result = m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal,
                                                           &end);
    QVERIFY(qIsInf(result) && result > 0.0);
    QCOMPARE(*end, '\0');

    qstrncpy(buffer, "-1e309", sizeof(buffer));
    result = m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end);
    QVERIFY(qIsInf(result) && result < 0.0);
    QCOMPARE(*end, '\0');

    // underflow yields zero
    qstrncpy(buffer, "1e-999", sizeof(buffer));
    QCOMPARE(m_caQtDM_Lib->getDoubleValueFromString(buffer, CaQtDM_Lib_Interface::decimal, &end),
             0.0);
    QCOMPARE(*end, '\0');
}

void TestCaQtDM_Lib::computeNumericMaxMinPrecUsesChannelDispLimits()
{
    checkChannelDispLimits<caApplyNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                            "T_ApplyNumeric_Channel");
    checkChannelDispLimits<caNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                       "T_Numeric_Channel");
    checkChannelDispLimits<caSpinbox>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                       "T_Spinbox_Channel");
}

void TestCaQtDM_Lib::computeNumericMaxMinPrecFallsBackWithoutLimits()
{
    checkFallbackWithoutLimits<caApplyNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                                "T_ApplyNumeric_Fallback");
    checkFallbackWithoutLimits<caNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                           "T_Numeric_Fallback");
    checkFallbackWithoutLimits<caSpinbox>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                           "T_Spinbox_Fallback");
}

void TestCaQtDM_Lib::computeNumericMaxMinPrecIgnoresChannelInUserMode()
{
    checkUserModeIgnoresChannel<caApplyNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                                 "T_ApplyNumeric_User");
    checkUserModeIgnoresChannel<caNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                            "T_Numeric_User");
    checkUserModeIgnoresChannel<caSpinbox>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                            "T_Spinbox_User");
}

void TestCaQtDM_Lib::computeNumericMaxMinPrecHonoursMaxChannelPrecision()
{
    checkMaxChannelPrecision<caApplyNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                              "T_ApplyNumeric_Prec");
    checkMaxChannelPrecision<caNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                         "T_Numeric_Prec");
    checkMaxChannelPrecision<caSpinbox>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                         "T_Spinbox_Prec");
}

void TestCaQtDM_Lib::computeNumericMaxMinPrecUpdatesWhenChannelChanges()
{
    checkWidgetPicksUpFieldWrite<caApplyNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                                  "T_ApplyNumeric_Live");
    checkWidgetPicksUpFieldWrite<caNumeric>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                             "T_Numeric_Live");
    checkWidgetPicksUpFieldWrite<caSpinbox>(m_caQtDM_Lib, m_internalPlugin, m_parentAS,
                                             "T_Spinbox_Live");
}
