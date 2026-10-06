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

#include "tst_alhparser.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QXmlStreamReader>

static QStringList rtrimmedLines(const QString &text)
{
    QStringList out;
    foreach(const QString &line, text.split(QLatin1Char('\n'))) {
        QString l = line;
        while(l.endsWith(QLatin1Char(' '))) l.chop(1);
        out.append(l);
    }
    while(!out.isEmpty() && out.last().isEmpty()) out.removeLast();
    return out;
}

bool TestAlhParser::parseFixture(const QString &name, AlhModel &model, const AlhConfigParser::Options &options)
{
    AlhConfigParser parser(options);
    return parser.parseFile(QStringLiteral(":/alh/") + name, &model);
}

int TestAlhParser::countWarnings(const AlhModel &model, AlhWarning::Kind kind) const
{
    int n = 0;
    foreach(const AlhWarning &w, model.warnings()) if(w.kind == kind) n++;
    return n;
}

void TestAlhParser::maskRoundTrip()
{
    QCOMPARE(AlhMask().toString(), QStringLiteral("-----"));
    QVERIFY(AlhMask::fromString("-----").isEmpty());

    AlhMask m = AlhMask::fromString("CDATL");
    QVERIFY(m.cancel && m.disable && m.noAck && m.noAckTransient && m.noLog);
    QCOMPARE(m.toString(), QStringLiteral("CDATL"));

    // alSetMask is character based: order does not matter
    QCOMPARE(AlhMask::fromString("LTADC").toString(), QStringLiteral("CDATL"));
    QCOMPARE(AlhMask::fromString("-D---").toString(), QStringLiteral("-D---"));
    QCOMPARE(AlhMask::fromString("D").toString(), QStringLiteral("-D---"));

    QString unknown;
    AlhMask::fromString("-X-y-", &unknown);
    QCOMPARE(unknown, QStringLiteral("Xy"));
}

void TestAlhParser::forcePvInputs()
{
    double v = 0;
    QVERIFY(AlhForcePv::inputIsConstant("5", &v));
    QCOMPARE(v, 5.0);
    QVERIFY(AlhForcePv::inputIsConstant("2.5e1", &v));
    QCOMPARE(v, 25.0);
    QVERIFY(!AlhForcePv::inputIsConstant("0"));
    QVERIFY(!AlhForcePv::inputIsConstant("in:a"));
}

void TestAlhParser::upstreamExample()
{
    AlhModel model;
    QVERIFY(parseFixture("test.alhConfig", model));
    QVERIFY2(model.warnings().isEmpty(), qPrintable(model.warnings().isEmpty() ? QString() : model.warnings().first().toString()));
    QCOMPARE(model.groupCount(), 5);
    QCOMPARE(model.channelCount(), 10);
    QCOMPARE(model.roots().size(), 1);
    QCOMPARE(model.beepSeverity(), (int) AlhSevMajor);

    const AlhNode &root = model.node(model.roots().first());
    QCOMPARE(root.name, QStringLiteral("JBA_TEST_MAIN_GROUP"));
    QCOMPARE(root.command, QStringLiteral("xload"));
    QCOMPARE(root.guidanceText.size(), 2);
    QVERIFY(root.guidanceText.first().startsWith("This is the text guidance for JBA_TEST_MAIN_GROUP"));

    const int group = model.findByPath("JBA_TEST_MAIN_GROUP/GROUP");
    QVERIFY(group >= 0);
    QVERIFY(model.node(group).hasForcePv);
    QCOMPARE(model.node(group).forcePv.pv, QStringLiteral("testjbaai"));
    QCOMPARE(model.node(group).forcePv.mask.toString(), QStringLiteral("-D---"));
    QCOMPARE(model.node(group).forcePv.forceValue, 1.0);
    QCOMPARE(model.node(group).forcePv.resetValue, 0.0);
    QVERIFY(!model.node(group).forcePv.resetNE);
    QCOMPARE(model.node(group).command, QStringLiteral("xascii"));

    const int second = model.findByPath("JBA_TEST_MAIN_GROUP/SECONDGROUP");
    QVERIFY(second >= 0);
    QCOMPARE(model.node(second).alias, QStringLiteral("This is a test alias for the second group"));
    QCOMPARE(model.node(second).displayName(), model.node(second).alias);

    const int ch5 = model.findByPath("JBA_TEST_MAIN_GROUP/GROUP/SUBGROUP/jba:Example5");
    QVERIFY(ch5 >= 0);
    QVERIFY(model.node(ch5).isChannel());
    QVERIFY(model.node(ch5).mask.isEmpty());
    QVERIFY(model.isAncestorOrSelf(group, ch5));
    QVERIFY(!model.isAncestorOrSelf(second, ch5));
}

void TestAlhParser::treeDumpFormat()
{
    AlhModel model;
    QVERIFY(parseFixture("test.alhConfig", model));
    const QStringList expected = QStringList()
            << "JBA_TEST_MAIN_GROUP"
            << "|    jba:Example1"
            << "|    jba:Example2"
            << "+--GROUP"
            << "|  |    jba:Example3"
            << "|  |    jba:Example4"
            << "|  +--SUBGROUP"
            << "|  |       jba:Example5"
            << "|  |       jba:Example6"
            << "|  +--SECONDSUBGROUP"
            << "|          jba:Example7"
            << "|          jba:Example8"
            << "+--SECONDGROUP"
            << "        jba:Example9"
            << "        jba:Example10";
    QCOMPARE(rtrimmedLines(model.treeDump()), expected);
    // names are padded to 28 characters like %-28s
    QVERIFY(model.treeDump().startsWith(QStringLiteral("JBA_TEST_MAIN_GROUP").leftJustified(28) + "\n"));
}

void TestAlhParser::optionalLines()
{
    AlhModel model;
    QVERIFY(parseFixture("optional.alhConfig", model));

    // globals
    QCOMPARE(model.beepSeverity(), (int) AlhSevMajor);                 // last wins
    QVERIFY(model.hasHeartbeat());                                     // first wins
    QCOMPARE(model.heartbeatPv(), QStringLiteral("hb:pv"));
    QCOMPARE(model.heartbeatRate(), 30.0);
    QCOMPARE(model.heartbeatValue(), 2);

    const int rootId = model.findByPath("ROOT");
    QVERIFY(rootId >= 0);
    const AlhNode &root = model.node(rootId);
    QCOMPARE(root.alias, QStringLiteral("Root alias"));                // first wins
    QCOMPARE(root.command, QStringLiteral("first command arg1"));      // first wins
    QCOMPARE(root.beepSevr, (int) AlhSevMinor);                        // last wins
    QCOMPARE(root.sevrPv, QStringLiteral("root:sevr"));                // first wins
    QCOMPARE(root.guidanceLocation, QStringLiteral("http://example.org/root"));
    QCOMPARE(root.sevrCommands.size(), 3);
    QVERIFY(root.statCommands.isEmpty());                              // channels only
    QVERIFY(!root.hasCountFilter);                                     // channels only

    const int ch1 = model.findByPath("ROOT/test:ch1");
    QVERIFY(ch1 >= 0);
    const AlhNode &c1 = model.node(ch1);
    QVERIFY(c1.mask.disable && c1.mask.noAck && !c1.mask.cancel && !c1.mask.noAckTransient && !c1.mask.noLog);
    QVERIFY(c1.hasCountFilter);
    QCOMPARE(c1.countFilter.count, 3);                                 // first wins
    QCOMPARE(c1.countFilter.seconds, 10);
    QCOMPARE(c1.ackPv, QStringLiteral("ack:pv2"));                     // last wins
    QCOMPARE(c1.ackValue, QStringLiteral("2"));
    QCOMPARE(c1.statCommands.size(), 2);
    QCOMPARE(c1.statCommands.first().status, QStringLiteral("HIHI"));
    QCOMPARE(c1.statCommands.first().command, QStringLiteral("echo hihi"));
    QCOMPARE(c1.guidanceText, QStringList() << "line one" << "  line two indented");

    const int ch2 = model.findByPath("ROOT/test:ch2");
    QVERIFY(ch2 >= 0);
    const AlhNode &c2 = model.node(ch2);
    QCOMPARE(c2.mask.toString(), QStringLiteral("CDATL"));
    QVERIFY(c2.hasForcePv);
    QVERIFY(c2.forcePv.isCalc);
    QCOMPARE(c2.forcePv.mask.toString(), QStringLiteral("-D---"));
    QCOMPARE(c2.forcePv.forceValue, 1.0);
    QVERIFY(c2.forcePv.resetNE);
    QCOMPARE(c2.forcePv.calcExpr, QStringLiteral("A>B"));
    QCOMPARE(c2.forcePv.calcInput[0], QStringLiteral("in:a"));        // first wins per letter
    QCOMPARE(c2.forcePv.calcInput[1], QStringLiteral("5"));
    QVERIFY(c2.forcePv.calcInput[2].isEmpty());

    const int sub = model.findByPath("ROOT/SUB");
    QVERIFY(sub >= 0);
    QVERIFY(model.node(sub).hasForcePv);
    QCOMPARE(model.node(sub).forcePv.pv, QStringLiteral("force:pv"));
    QCOMPARE(model.node(sub).forcePv.forceValue, 1.0);
    QCOMPARE(model.node(sub).forcePv.resetValue, 0.0);

    // duplicates and misplaced lines are reported, never fatal
    QVERIFY(countWarnings(model, AlhWarning::DuplicateIgnored) >= 7);
    QCOMPARE(countWarnings(model, AlhWarning::UnsupportedFeature), 5); // 3 sevr + 2 stat commands stored
    QVERIFY(countWarnings(model, AlhWarning::InvalidOptional) >= 3);   // group STATCOMMAND/COUNTFILTER, FOO status
    QCOMPARE(model.unsupportedFeatures().size(), 5);
}

void TestAlhParser::toleranceContinuesParsing()
{
    AlhModel model;
    QVERIFY(parseFixture("tolerance.alhConfig", model));
    QCOMPARE(model.groupCount(), 2);
    QCOMPARE(model.channelCount(), 3);

    QCOMPARE(countWarnings(model, AlhWarning::LogicNoContext), 1);
    QCOMPARE(countWarnings(model, AlhWarning::InvalidInput), 2);       // garbage, short CHANNEL
    QCOMPARE(countWarnings(model, AlhWarning::ParentNotFound), 2);     // ORPHAN, MISSING
    QCOMPARE(countWarnings(model, AlhWarning::UnknownMaskChar), 1);
    QCOMPARE(countWarnings(model, AlhWarning::InvalidEnd), 1);
    QCOMPARE(countWarnings(model, AlhWarning::Truncated), 1);
    QCOMPARE(countWarnings(model, AlhWarning::InvalidOptional), 2);    // $UNKNOWNKEY, $FORCEPV<tab>

    const int pv1 = model.findByPath("MAIN/ok:pv1");
    QVERIFY(pv1 >= 0);
    QCOMPARE(model.node(pv1).guidanceText, QStringList() << "some text");
    QVERIFY(model.node(pv1).alias.isEmpty());                          // consumed by the broken block
    QVERIFY(model.findByPath("MAIN/ok:pv3") >= 0);

    const int sub = model.findByPath("MAIN/SUB");
    QVERIFY(sub >= 0);
    QVERIFY(!model.node(sub).hasForcePv);
    QCOMPARE(model.node(sub).children.size(), 1);
    QCOMPARE(model.node(model.node(sub).children.first()).name.length(), 65);
}

void TestAlhParser::includeFiles()
{
    AlhModel model;
    QVERIFY(parseFixture("include_main.alhConfig", model));
    QCOMPARE(model.roots().size(), 1);                                 // TOP only
    QCOMPARE(model.includedFiles().size(), 1);                         // the loop file is never read
    QCOMPARE(countWarnings(model, AlhWarning::IncludeMissing), 1);
    QCOMPARE(countWarnings(model, AlhWarning::ParentNotFound), 1);     // INCLUDE NULL after groups: "Missing parent"

    // GROUP NULL inside an include hangs below the include parent
    QVERIFY(model.findByPath("TOP/SUBROOT/sub:ch1") >= 0);
    QVERIFY(model.findByPath("TOP/SUBROOT/DEEP/sub:ch2") >= 0);
    // after the include alh continues with the include parent as context
    QVERIFY(model.findByPath("TOP/AFTER") >= 0);
    QVERIFY(model.findByPath("LOOP") < 0);

    // INCLUDE NULL before the first group: the included root becomes the first root; the context is
    // gone afterwards, so a following GROUP NULL is a second root (alh would even lose the included tree)
    AlhModel first;
    QVERIFY(parseFixture("include_first.alhConfig", first));
    QCOMPARE(first.roots().size(), 2);
    QCOMPARE(first.node(first.roots().first()).name, QStringLiteral("SUBROOT"));
    QCOMPARE(first.node(first.roots().last()).name, QStringLiteral("AFTERINCLUDE"));
    QVERIFY(first.findByPath("SUBROOT/DEEP/sub:ch2") >= 0);
    QCOMPARE(countWarnings(first, AlhWarning::IncludeRecursion), 0);

    // a later GROUP NULL in the main file hangs below the current context (alh), it is no second root
    AlhModel ctx;
    AlhConfigParser parser;
    QVERIFY(parser.parseText("GROUP NULL A\nGROUP A B\nCHANNEL B b:ch\nGROUP NULL C\nCHANNEL C c:ch\n", "ctx.alhConfig", &ctx));
    QCOMPARE(ctx.roots().size(), 1);
    QVERIFY(ctx.findByPath("A/B/C/c:ch") >= 0);
}

void TestAlhParser::alhDefaults()
{
    AlhModel model;
    AlhConfigParser parser;
    const QString text = "$HEARTBEATPV hb:pv\n"
                         "GROUP NULL G\n"
                         "$FORCEPV CALC\n"
                         "$FORCEPV_CALC A > B\n"
                         "$BEEPSEVR NO_ALARM\n"
                         "CHANNEL G c:1\n"
                         "$ALARMCOUNTFILTER\n"
                         "CHANNEL G c:2\n"
                         "$ALARMCOUNTFILTER 4\n"
                         "$SEVRCOMMAND DOWN_ANY echo any\n";
    QVERIFY(parser.parseText(text, "defaults.alhConfig", &model));
    QCOMPARE(model.heartbeatRate(), 1.0);                              // alh: 1 s, value 1
    QCOMPARE(model.heartbeatValue(), 1);
    const AlhNode &g = model.node(model.findByPath("G"));
    QVERIFY(g.hasForcePv);
    QVERIFY(g.forcePv.mask.isEmpty());                                 // no mask given
    QCOMPARE(g.forcePv.calcExpr, QStringLiteral("A"));                 // alh reads one word
    QCOMPARE(g.beepSevr, -1);                                          // NO_ALARM is not a beep severity
    const AlhNode &c1 = model.node(model.findByPath("G/c:1"));
    QVERIFY(c1.hasCountFilter);
    QCOMPARE(c1.countFilter.count, 1);
    QCOMPARE(c1.countFilter.seconds, 1);
    const AlhNode &c2 = model.node(model.findByPath("G/c:2"));
    QCOMPARE(c2.countFilter.count, 4);
    QCOMPARE(c2.countFilter.seconds, 1);
    QCOMPARE(c2.sevrCommands.size(), 1);
    QCOMPARE(c2.sevrCommands.first().severity, (int) AlhSevCount);     // ANY
    QCOMPARE(countWarnings(model, AlhWarning::InvalidOptional), 2);    // FORCEPV_CALC blanks, BEEPSEVR NO_ALARM
}

void TestAlhParser::macros()
{
    AlhConfigParser::Options options;
    options.macros.insert("P", "MY");
    options.macros.insert("R", "DEV");
    AlhModel model;
    QVERIFY(parseFixture("macros.alhConfig", model, options));
    QVERIFY(model.findByPath("MYGROUP/MYDEV:ch") >= 0);
    QVERIFY(model.findByPath("MYGROUP/$(UNDEF):ch") >= 0);             // unknown macros stay

    QCOMPARE(AlhConfigParser::expandMacros("a $(P) b $(P)$(R)", options.macros), QStringLiteral("a MY b MYDEV"));
    QCOMPARE(AlhConfigParser::expandMacros("$(X)", QMap<QString, QString>()), QStringLiteral("$(X)"));
}

void TestAlhParser::sevrCommandTokens()
{
    AlhModel model;
    AlhConfigParser parser;
    const QString text = "GROUP NULL G\n"
                         "$SEVRCOMMAND UP_MAJOR cmd1 a b\n"
                         "$SEVRCOMMAND DOWN_ANY cmd2\n"
                         "$SEVRCOMMAND UP_ALARM cmd3\n"
                         "$SEVRCOMMAND DOWN_NO_ALARM cmd4\n"
                         "$SEVRCOMMAND UP_INVALID\n";
    QVERIFY(parser.parseText(text, "inline.alhConfig", &model));
    const AlhNode &g = model.node(0);
    QCOMPARE(g.sevrCommands.size(), 5);                                // alh keeps an empty command too
    QVERIFY(g.sevrCommands.at(4).command.isEmpty());
    QVERIFY(g.sevrCommands.at(0).up);
    QCOMPARE(g.sevrCommands.at(0).severity, (int) AlhSevMajor);
    QCOMPARE(g.sevrCommands.at(0).command, QStringLiteral("cmd1 a b"));
    QVERIFY(!g.sevrCommands.at(1).up);
    QCOMPARE(g.sevrCommands.at(1).severity, (int) AlhSevCount);        // alh: no match, kept silently
    QVERIFY(g.sevrCommands.at(2).anyAlarm);
    QVERIFY(!g.sevrCommands.at(3).up);
    QCOMPARE(g.sevrCommands.at(3).severity, (int) AlhSevNoAlarm);

    QCOMPARE(AlhModel::severityFromToken("MAJOR_X"), (int) AlhSevMajor);
    QCOMPARE(AlhModel::severityFromToken("FOO"), -1);
    QCOMPARE(AlhModel::statusFromToken("READ_ACCESS"), 1);             // prefix match picks READ first, like alh
    QCOMPARE(AlhModel::statusFromToken("NOT_CONNECTED"), 22);
}

void TestAlhParser::jsonContainsTree()
{
    AlhModel model;
    QVERIFY(parseFixture("test.alhConfig", model));
    const QJsonObject root = model.toJson().object();
    QCOMPARE(root.value("file").toString(), QStringLiteral("test.alhConfig"));
    QCOMPARE(root.value("beepSeverity").toString(), QStringLiteral("MAJOR"));
    QCOMPARE(root.value("roots").toArray().size(), 1);
    QCOMPARE(root.value("warnings").toArray().size(), 0);
    const QJsonObject top = root.value("roots").toArray().first().toObject();
    QCOMPARE(top.value("name").toString(), QStringLiteral("JBA_TEST_MAIN_GROUP"));
    QCOMPARE(top.value("children").toArray().size(), 4);
    const QJsonObject ch = top.value("children").toArray().first().toObject();
    QCOMPARE(ch.value("type").toString(), QStringLiteral("channel"));
    QCOMPARE(ch.value("mask").toString(), QStringLiteral("-----"));
}

void TestAlhParser::uiGenerator()
{
    AlhUiGenerator::Params p;
    p.configFile = "/some/where/test.alhConfig";
    p.macros = "P=MY";
    QByteArray ui = AlhUiGenerator::generate(p);
    QVERIFY(!ui.isEmpty());

    QXmlStreamReader xml(ui);
    int customWidgets = 0, alarmTrees = 0, alarmLogs = 0;
    QString configFile, title;
    while(!xml.atEnd()) {
        xml.readNext();
        if(!xml.isStartElement()) continue;
        if(xml.name() == QLatin1String("customwidget")) customWidgets++;
        if(xml.name() == QLatin1String("widget")) {
            const QString klass = xml.attributes().value("class").toString();
            if(klass == "caAlarmTree") alarmTrees++;
            if(klass == "caAlarmLog") alarmLogs++;
        }
        if(xml.name() == QLatin1String("property")) {
            const QString name = xml.attributes().value("name").toString();
            if(name == "configFile" || name == "windowTitle") {
                const QString value = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
                if(name == "configFile") configFile = value; else title = value;
            }
        }
    }
    QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
    QCOMPARE(customWidgets, 2);
    QCOMPARE(alarmTrees, 1);
    QCOMPARE(alarmLogs, 1);
    QCOMPARE(configFile, p.configFile);
    QCOMPARE(title, QStringLiteral("test.alhConfig"));

    p.withLog = false;
    QXmlStreamReader xml2(AlhUiGenerator::generate(p));
    int cw = 0;
    while(!xml2.atEnd()) {
        xml2.readNext();
        if(xml2.isStartElement() && xml2.name() == QLatin1String("customwidget")) cw++;
    }
    QCOMPARE(cw, 1);
}
