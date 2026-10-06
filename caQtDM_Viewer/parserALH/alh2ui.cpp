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

#include <cstdio>
#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QTextStream>
#include <QtUiTools/QUiLoader>

#include "alhconfigparser.h"    // caQtDM_Parsers/alhParserSrc
#include "alhuigenerator.h"

// caQtDM-wide idiom, locally defined (standalone CLI)
#define qasc(x) x.toLatin1().constData()

// exit codes: 0 ok, 1 usage/input, 2 empty output, 3 not writable, 4 verify failed, 5 warnings (--strict)
enum { RcOk = 0, RcUsage = 1, RcEmpty = 2, RcWrite = 3, RcVerify = 4, RcWarnings = 5 };
enum Mode { ModeUi, ModeTree, ModeWarn, ModeJson };

static void usage()
{
    fprintf(stderr, "usage: alh2ui [mode] [options] <config.alhConfig> [output]\n");
    fprintf(stderr, "  debug/test tool for alh configurations; caQtDM loads .alhConfig directly\n");
    fprintf(stderr, "  modes (default --ui):\n");
    fprintf(stderr, "  --ui             .ui document as loaded by caQtDM\n");
    fprintf(stderr, "  --tree           tree dump in the format of alh (alConfigTreePrint)\n");
    fprintf(stderr, "  --warn           skipped lines and parsed but unsupported attributes\n");
    fprintf(stderr, "  --json           model as JSON (tests, diffs)\n");
    fprintf(stderr, "  options:\n");
    fprintf(stderr, "  -I <dir>         include directory (alh configDir), default: directory of the config\n");
    fprintf(stderr, "  -m \"A=x,B=y\"     macros substituted before parsing\n");
    fprintf(stderr, "  --pvname-size N  alh PVNAME_SIZE used for truncation (default 65)\n");
    fprintf(stderr, "  --verify         --ui only: load the generated document with QUiLoader\n");
    fprintf(stderr, "  --strict         exit code 5 when the config produced warnings\n");
    fprintf(stderr, "  output: file name, default stdout\n");
}

static QMap<QString, QString> parseMacros(const QString &text)
{
    QMap<QString, QString> map;
    foreach(const QString &part, text.split(QLatin1Char(','), ALH_SKIP_EMPTY)) {
        const int eq = part.indexOf(QLatin1Char('='));
        if(eq > 0) map.insert(part.left(eq).trimmed(), part.mid(eq + 1).trimmed());
    }
    return map;
}

static int writeOut(const QByteArray &data, const QString &outputFile)
{
    if(data.isEmpty()) {
        fprintf(stderr, "alh2ui error: no output produced\n");
        return RcEmpty;
    }
    if(outputFile.isEmpty()) {
        fwrite(data.constData(), 1, data.size(), stdout);
        fflush(stdout);
        return RcOk;
    }
    QFile out(outputFile);
    if(!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fprintf(stderr, "alh2ui error: cannot write %s\n", qasc(outputFile));
        return RcWrite;
    }
    out.write(data);
    out.close();
    return RcOk;
}

static int verifyUi(const QByteArray &ui)
{
    QBuffer buffer;
    buffer.setData(ui);
    buffer.open(QIODevice::ReadOnly);
    QUiLoader loader;
    QWidget *widget = loader.load(&buffer, Q_NULLPTR);
    if(widget == Q_NULLPTR) {
        fprintf(stderr, "alh2ui error: QUiLoader cannot load the document (%s)\n", qasc(loader.errorString()));
        return RcVerify;
    }
    // no delete, see prc2ui: the top-level widget is cleaned up by QApplication
    return RcOk;
}

int main(int argc, char *argv[])
{
    // --verify loads widgets; run headless unless overridden
    if(qgetenv("QT_QPA_PLATFORM").isEmpty()) qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    // warnings are printed by the tool itself; QT_LOGGING_RULES still overrides
    QLoggingCategory::setFilterRules(QStringLiteral("caqtdm.parsers.alh.debug=false"));

    Mode mode = ModeUi;
    bool verify = false, strict = false;
    QString inputFile, outputFile;
    AlhConfigParser::Options options;

    for(int i = 1; i < argc; i++) {
        const QString arg = QString(argv[i]);
        if(arg == "-h" || arg == "--help") { usage(); return RcOk; }
        else if(arg == "--ui") mode = ModeUi;
        else if(arg == "--tree") mode = ModeTree;
        else if(arg == "--warn") mode = ModeWarn;
        else if(arg == "--json") mode = ModeJson;
        else if(arg == "--verify") verify = true;
        else if(arg == "--strict") strict = true;
        else if(arg == "-I" && i + 1 < argc) options.includeDir = QString(argv[++i]);
        else if(arg == "-m" && i + 1 < argc) options.macros = parseMacros(QString(argv[++i]));
        else if(arg == "--pvname-size" && i + 1 < argc) options.pvNameSize = QString(argv[++i]).toInt();
        else if(arg.startsWith("-")) {
            fprintf(stderr, "alh2ui error: unknown option %s\n", qasc(arg));
            usage();
            return RcUsage;
        }
        else if(inputFile.isEmpty()) inputFile = arg;
        else if(outputFile.isEmpty()) outputFile = arg;
        else { usage(); return RcUsage; }
    }

    if(inputFile.isEmpty()) { usage(); return RcUsage; }
    if(!QFile::exists(inputFile)) {
        fprintf(stderr, "alh2ui error: input file %s does not exist\n", qasc(inputFile));
        return RcUsage;
    }

    AlhModel model;
    AlhConfigParser parser(options);
    if(!parser.parseFile(inputFile, &model)) {
        fprintf(stderr, "alh2ui error: cannot read %s\n", qasc(inputFile));
        return RcUsage;
    }

    QByteArray data;
    int rc = RcOk;
    switch(mode) {
    case ModeUi: {
        AlhUiGenerator::Params p;
        p.configFile = QFileInfo(inputFile).absoluteFilePath();
        p.includeDir = options.includeDir;
        QStringList macros;
        for(QMap<QString, QString>::const_iterator it = options.macros.constBegin(); it != options.macros.constEnd(); ++it)
            macros.append(it.key() + "=" + it.value());
        p.macros = macros.join(",");
        data = AlhUiGenerator::generate(p);
        if(verify) rc = verifyUi(data);
        break;
    }
    case ModeTree:
        data = model.treeDump().toUtf8();
        break;
    case ModeJson:
        data = model.toJson().toJson(QJsonDocument::Indented);
        break;
    case ModeWarn: {
        QString text;
        QTextStream out(&text);
        out << "config: " << model.mainFile() << "\n";
        foreach(const QString &f, model.includedFiles()) out << "include: " << f << "\n";
        out << "groups: " << model.groupCount() << ", channels: " << model.channelCount()
            << ", warnings: " << model.warnings().size() << "\n";
        foreach(const AlhWarning &w, model.warnings()) out << w.toString() << "\n";
        foreach(const QString &u, model.unsupportedFeatures()) out << "unsupported: " << u << "\n";
        data = text.toUtf8();
        break;
    }
    }

    if(rc == RcOk) rc = writeOut(data, outputFile);
    if(mode != ModeWarn) {
        foreach(const AlhWarning &w, model.warnings()) fprintf(stderr, "alh2ui warning: %s\n", qasc(w.toString()));
    }
    if(rc == RcOk && strict && !model.warnings().isEmpty()) rc = RcWarnings;
    return rc;
}
