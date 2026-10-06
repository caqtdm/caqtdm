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

#include "alhconfigparser.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

static const QRegularExpression &whitespace()
{
    static const QRegularExpression re(QStringLiteral("[ \\t\\f\\v]+"));
    return re;
}

// remainder of the line after the first n whitespace separated tokens
static QString restAfterTokens(const QString &line, int n)
{
    int pos = 0;
    const int len = line.length();
    for(int t = 0; t < n; t++) {
        while(pos < len && line.at(pos).isSpace()) pos++;
        while(pos < len && !line.at(pos).isSpace()) pos++;
    }
    return line.mid(pos).trimmed();
}

AlhConfigParser::AlhConfigParser(const Options &options)
    : m_opt(options), m_model(Q_NULLPTR), m_curGroup(-1), m_curNode(-1)
{
}

QString AlhConfigParser::expandMacros(const QString &text, const QMap<QString, QString> &macros)
{
    if(macros.isEmpty()) return text;
    static const QRegularExpression re(QStringLiteral("\\$\\(([^()]+)\\)"));
    QString out;
    int last = 0;
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out += text.mid(last, m.capturedStart() - last);
        const QString name = m.captured(1);
        out += macros.contains(name) ? macros.value(name) : m.captured(0);   // unknown: keep
        last = m.capturedEnd();
    }
    out += text.mid(last);
    return out;
}

bool AlhConfigParser::parseFile(const QString &path, AlhModel *model)
{
    m_model = model;
    m_curGroup = m_curNode = -1;
    m_includeStack.clear();
    const QFileInfo fi(path);
    m_includeDir = m_opt.includeDir.isEmpty() ? fi.absolutePath() : m_opt.includeDir;
    model->setMainFile(fi.absoluteFilePath());
    return readFile(fi.absoluteFilePath(), 0, -1);
}

bool AlhConfigParser::parseText(const QString &text, const QString &virtualPath, AlhModel *model)
{
    m_model = model;
    m_curGroup = m_curNode = -1;
    m_includeStack.clear();
    m_includeDir = m_opt.includeDir.isEmpty() ? QFileInfo(virtualPath).absolutePath() : m_opt.includeDir;
    model->setMainFile(virtualPath);
    QString expanded = expandMacros(text, m_opt.macros);
    QTextStream in(&expanded, QIODevice::ReadOnly);
    parseStream(in, virtualPath, 0, -1);
    return true;
}

QString AlhConfigParser::decode(const QByteArray &raw) const
{
    const QString utf8 = QString::fromUtf8(raw);
    if(utf8.contains(QChar(0xFFFD))) return QString::fromLatin1(raw);   // legacy latin-1 files
    return utf8;
}

bool AlhConfigParser::readFile(const QString &path, int depth, int includeParent)
{
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)) {
        if(depth > 0) warn(AlhWarning::IncludeMissing, path, 0, path);
        else qCWarning(alhParserLog) << "cannot open" << path;
        return false;
    }
    QString key = QFileInfo(path).canonicalFilePath();
    if(key.isEmpty()) key = QFileInfo(path).absoluteFilePath();
    if(m_includeStack.contains(key)) {
        warn(AlhWarning::IncludeRecursion, path, 0, path);
        return false;
    }
    if(depth > m_opt.maxIncludeDepth) {
        warn(AlhWarning::IncludeRecursion, path, 0, QStringLiteral("include depth exceeded"));
        return false;
    }
    m_includeStack.append(key);
    if(depth > 0) m_model->addIncludedFile(path);
    QString text = expandMacros(decode(f.readAll()), m_opt.macros);
    f.close();
    QTextStream in(&text, QIODevice::ReadOnly);
    parseStream(in, path, depth, includeParent);
    m_includeStack.removeLast();
    return true;
}

void AlhConfigParser::warn(AlhWarning::Kind kind, const QString &file, int line, const QString &text)
{
    m_model->addWarning(AlhWarning(kind, file, line, text));
    qCDebug(alhParserLog) << QFileInfo(file).fileName() << line << AlhWarning(kind, file, line, text).kindString() << text;
}

QString AlhConfigParser::truncated(const QString &s, int size, const QString &what, const QString &file, int lineNo)
{
    if(s.length() <= size) return s;
    warn(AlhWarning::Truncated, file, lineNo, QString("%1 '%2' truncated to %3 characters").arg(what, s).arg(size));
    return s.left(size);
}

// alGetConfig dispatcher: GROUP, CHANNEL, $, INCLUDE, empty, #, anything else is skipped
void AlhConfigParser::parseStream(QTextStream &in, const QString &file, int depth, int includeParent)
{
    int lineNo = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        lineNo++;
        if(line.endsWith(QLatin1Char('\r'))) line.chop(1);
        if(line.length() >= LineBufferSize) {
            warn(AlhWarning::Truncated, file, lineNo, QString("line longer than %1 characters").arg(LineBufferSize));
            line.truncate(LineBufferSize - 1);
        }
        int first = 0;
        while(first < line.length() && (line.at(first) == QLatin1Char(' ') || line.at(first) == QLatin1Char('\t'))) first++;
        const QString content = line.mid(first);
        if(content.isEmpty()) continue;
        if(content.startsWith(QLatin1String("GROUP"))) {
            handleGroup(content.split(whitespace(), ALH_SKIP_EMPTY), file, lineNo, includeParent);
        } else if(content.startsWith(QLatin1String("CHANNEL"))) {
            handleChannel(content.split(whitespace(), ALH_SKIP_EMPTY), file, lineNo);
        } else if(content.startsWith(QLatin1Char('$'))) {
            handleOptional(content.mid(1), in, file, lineNo);
        } else if(content.startsWith(QLatin1String("INCLUDE"))) {
            handleInclude(content.split(whitespace(), ALH_SKIP_EMPTY), file, lineNo, depth);
        } else if(content.startsWith(QLatin1Char('#'))) {
            continue;
        } else {
            warn(AlhWarning::InvalidInput, file, lineNo, content);
        }
    }
}

// parent must be on the ancestor chain of the current group context
int AlhConfigParser::resolveParent(const QString &parentName) const
{
    int id = m_curGroup;
    while(id >= 0) {
        if(m_model->node(id).name == parentName) return id;
        id = m_model->node(id).parentId;
    }
    return -1;
}

void AlhConfigParser::handleGroup(const QStringList &tok, const QString &file, int lineNo, int includeParent)
{
    if(tok.size() < 3) {
        warn(AlhWarning::InvalidInput, file, lineNo, QStringLiteral("Invalid GROUP line: ") + tok.join(QLatin1Char(' ')));
        return;
    }
    const QString parentName = truncated(tok.at(1), GroupNameSize, QStringLiteral("group parent"), file, lineNo);
    AlhNode node;
    node.type = AlhGroupNode;
    node.name = truncated(tok.at(2), GroupNameSize, QStringLiteral("group name"), file, lineNo);
    node.sourceFile = file;
    node.sourceLine = lineNo;
    if(parentName == QLatin1String("NULL")) {
        // alh: the first group is the root, later GROUP NULL lines hang below the current context
        node.parentId = m_curGroup;
        Q_UNUSED(includeParent);
    } else {
        node.parentId = resolveParent(parentName);
        if(node.parentId < 0) {
            warn(AlhWarning::ParentNotFound, file, lineNo, QString("GROUP %1 %2").arg(parentName, node.name));
            return;
        }
    }
    m_curGroup = m_curNode = m_model->addNode(node);
}

void AlhConfigParser::handleChannel(const QStringList &tok, const QString &file, int lineNo)
{
    if(tok.size() < 3) {
        warn(AlhWarning::InvalidInput, file, lineNo, QStringLiteral("Invalid CHANNEL line: ") + tok.join(QLatin1Char(' ')));
        return;
    }
    const QString parentName = truncated(tok.at(1), ChannelTokenSize, QStringLiteral("channel parent"), file, lineNo);
    AlhNode node;
    node.type = AlhChannelNode;
    node.name = truncated(tok.at(2).left(ChannelTokenSize), m_opt.pvNameSize, QStringLiteral("pv name"), file, lineNo);
    node.sourceFile = file;
    node.sourceLine = lineNo;
    node.parentId = resolveParent(parentName);
    if(node.parentId < 0) {
        warn(AlhWarning::ParentNotFound, file, lineNo, QString("CHANNEL %1 %2").arg(parentName, node.name));
        return;
    }
    if(tok.size() >= 4) {
        QString unknown;
        node.mask = AlhMask::fromString(tok.at(3), &unknown);
        if(!unknown.isEmpty())
            warn(AlhWarning::UnknownMaskChar, file, lineNo, QString("mask '%1' for %2").arg(tok.at(3), node.name));
    }
    m_curNode = m_model->addNode(node);
    m_curGroup = node.parentId;
}

void AlhConfigParser::handleInclude(const QStringList &tok, const QString &file, int lineNo, int depth)
{
    if(tok.size() < 3) {
        warn(AlhWarning::InvalidInput, file, lineNo, QStringLiteral("Invalid INCLUDE line: ") + tok.join(QLatin1Char(' ')));
        return;
    }
    int parent = -1;
    if(tok.at(1) == QLatin1String("NULL")) {
        if(m_model->nodeCount() > 0) {                       // alh: "Missing parent"
            warn(AlhWarning::ParentNotFound, file, lineNo, QString("INCLUDE %1 %2 (NULL only before the first group)").arg(tok.at(1), tok.at(2)));
            return;
        }
    } else {
        parent = resolveParent(tok.at(1));
        if(parent < 0) {
            warn(AlhWarning::ParentNotFound, file, lineNo, QString("INCLUDE %1 %2").arg(tok.at(1), tok.at(2)));
            return;
        }
    }
    QString path = tok.at(2);
    if(!QFileInfo(path).isAbsolute() && !path.startsWith(QLatin1Char(':')))
        path = QDir(m_includeDir).filePath(path);
    // the included file starts with the include parent as context; afterwards alh continues
    // with the include parent (GetIncludeLine: *pglink = parent_link)
    m_curGroup = m_curNode = parent;
    readFile(path, depth + 1, parent);
    m_curGroup = m_curNode = parent;
}

void AlhConfigParser::readGuidanceBlock(QTextStream &in, const QString &file, int &lineNo, AlhNode &node)
{
    while(!in.atEnd()) {
        QString line = in.readLine();
        lineNo++;
        if(line.endsWith(QLatin1Char('\r'))) line.chop(1);
        int i = 0;
        while(i < line.length() && (line.at(i) == QLatin1Char(' ') || line.at(i) == QLatin1Char('\t'))) i++;
        if(i < line.length() && line.at(i) == QLatin1Char('$')) {
            const QString rest = line.mid(i);
            if(!rest.startsWith(QLatin1String("$END")) && !rest.startsWith(QLatin1String("$End")))
                warn(AlhWarning::InvalidEnd, file, lineNo, rest);    // alh: block ends, line is consumed
            return;
        }
        node.guidanceText.append(line);
    }
}

// GetOptionalLine: keywords are matched by prefix in alh order
void AlhConfigParser::handleOptional(const QString &content, QTextStream &in, const QString &file, int &lineNo)
{
    const QStringList tok = content.split(whitespace(), ALH_SKIP_EMPTY);
    const QString keyword = tok.isEmpty() ? QString() : tok.first();

    if(content.startsWith(QLatin1String("BEEPSEVERITY"))) {
        const int sev = tok.size() >= 2 ? AlhModel::severityFromToken(tok.at(1)) : -1;
        if(sev <= 0) { warn(AlhWarning::InvalidOptional, file, lineNo, QLatin1Char('$') + content); return; }   // NO_ALARM not allowed
        m_model->setBeepSeverity(sev);
        return;
    }
    if(content.startsWith(QLatin1String("HEARTBEATPV"))) {
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, QLatin1Char('$') + content); return; }
        if(m_model->hasHeartbeat()) { warn(AlhWarning::DuplicateIgnored, file, lineNo, QLatin1Char('$') + content); return; }
        bool ok = true;
        double rate = 1.0;     // alh defaults: 1 s, value 1
        int value = 1;
        if(tok.size() >= 3) { rate = tok.at(2).toDouble(&ok); if(!ok) rate = 1.0; }
        if(tok.size() >= 4) { value = tok.at(3).toInt(&ok, 0); if(!ok) value = 1; }
        m_model->setHeartbeat(truncated(tok.at(1), ForcePvNameSize, QStringLiteral("heartbeat pv"), file, lineNo), rate, value);
        return;
    }

    if(m_curNode < 0) {
        warn(AlhWarning::LogicNoContext, file, lineNo, QLatin1Char('$') + content);
        return;
    }
    AlhNode &node = m_model->node(m_curNode);
    const QString dollarLine = QLatin1Char('$') + content;

    if(content.startsWith(QLatin1String("BEEPSEVR"))) {
        const int sev = tok.size() >= 2 ? AlhModel::severityFromToken(tok.at(1)) : -1;
        if(sev <= 0) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        node.beepSevr = sev;                                            // last wins
    } else if(content.startsWith(QLatin1String("FORCEPV "))) {
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(node.hasForcePv) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        AlhForcePv f;
        f.pv = truncated(tok.at(1), ForcePvNameSize, QStringLiteral("force pv"), file, lineNo);
        f.isCalc = (f.pv == QLatin1String("CALC"));
        if(tok.size() >= 3) {
            QString unknown;
            f.mask = AlhMask::fromString(tok.at(2).left(ForcePvMaskSize), &unknown);
            if(!unknown.isEmpty()) warn(AlhWarning::UnknownMaskChar, file, lineNo, dollarLine);
        }
        if(tok.size() >= 4) {
            bool ok;
            const double v = tok.at(3).toDouble(&ok);
            if(ok) f.forceValue = v; else warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine);
        }
        if(tok.size() >= 5) {
            if(tok.at(4).startsWith(QLatin1String("NE")) || tok.at(4).startsWith(QLatin1String("ne"))) f.resetNE = true;
            else f.resetValue = tok.at(4).toDouble();                   // atof semantics
        }
        node.hasForcePv = true;
        node.forcePv = f;
    } else if(content.startsWith(QLatin1String("FORCEPV_CALC "))) {
        if(!node.hasForcePv || !node.forcePv.isCalc)
            warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (no $FORCEPV CALC before)"));
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(tok.size() > 2) warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (alh reads the first word only)"));
        node.forcePv.calcExpr = tok.at(1);                              // last wins
    } else if(content.startsWith(QLatin1String("FORCEPV_CALC_"))) {
        const QChar letter = keyword.length() > 13 ? keyword.at(13) : QChar();
        const int idx = letter.toLatin1() - 'A';
        if(idx < 0 || idx > 5 || tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(!node.hasForcePv || !node.forcePv.isCalc)
            warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (no $FORCEPV CALC before)"));
        if(!node.forcePv.calcInput[idx].isEmpty()) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        node.forcePv.calcInput[idx] = tok.at(1);                        // first wins per letter
    } else if(content.startsWith(QLatin1String("SEVRPV"))) {
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(!node.sevrPv.isEmpty()) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        node.sevrPv = truncated(tok.at(1), ForcePvNameSize, QStringLiteral("sevr pv"), file, lineNo);
    } else if(content.startsWith(QLatin1String("COMMAND"))) {
        const QString cmd = restAfterTokens(content, 1);
        if(cmd.isEmpty()) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(!node.command.isEmpty()) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        node.command = cmd;
    } else if(content.startsWith(QLatin1String("SEVRCOMMAND"))) {
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        AlhSevrCommand c;
        c.token = tok.at(1);
        c.command = restAfterTokens(content, 2);
        const int len = c.token.startsWith(QLatin1Char('D')) ? 5 : 3;  // addNewSevrCommand
        c.up = (len == 3);
        const QString sevPart = c.token.mid(len);
        const int sev = AlhModel::severityFromToken(sevPart);
        c.severity = sev >= 0 ? sev : (int) AlhSevCount;
        c.anyAlarm = sevPart.startsWith(QLatin1String("ALARM"));
        if(sev < 0 && !c.anyAlarm && !sevPart.startsWith(QLatin1String("ANY")))
            warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (unknown severity keyword, treated as ANY like alh)"));
        node.sevrCommands.append(c);
        warn(AlhWarning::UnsupportedFeature, file, lineNo, dollarLine);
    } else if(content.startsWith(QLatin1String("STATCOMMAND"))) {
        if(tok.size() < 2) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(node.isGroup()) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (channels only)")); return; }
        AlhStatCommand c;
        c.status = tok.at(1);
        c.command = restAfterTokens(content, 2);
        if(AlhModel::statusFromToken(c.status) < 0)
            warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (unknown status keyword)"));
        node.statCommands.append(c);
        warn(AlhWarning::UnsupportedFeature, file, lineNo, dollarLine);
    } else if(content.startsWith(QLatin1String("ALIAS"))) {
        const QString alias = restAfterTokens(content, 1);
        if(alias.isEmpty()) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(!node.alias.isEmpty()) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        node.alias = alias;
    } else if(content.startsWith(QLatin1String("ALARMCOUNTFILTER"))) {
        if(node.isGroup()) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (channels only)")); return; }
        AlhCountFilter f;                                               // alh defaults: 1 1
        f.count = 1;
        f.seconds = 1;
        bool ok = true;
        if(tok.size() >= 2) { const int v = tok.at(1).toInt(&ok, 0); if(ok) f.count = v; }
        if(ok && tok.size() >= 3) { const int v = tok.at(2).toInt(&ok, 0); if(ok) f.seconds = v; }
        if(!ok) warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (bad number, alh defaults used)"));
        if(node.hasCountFilter) { warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine); return; }
        node.hasCountFilter = true;
        node.countFilter = f;
    } else if(content.startsWith(QLatin1String("GUIDANCE"))) {
        const QString location = restAfterTokens(content, 1);
        if(location.isEmpty()) readGuidanceBlock(in, file, lineNo, node);
        else if(node.guidanceLocation.isEmpty()) node.guidanceLocation = location;
        else warn(AlhWarning::DuplicateIgnored, file, lineNo, dollarLine);
    } else if(content.startsWith(QLatin1String("ACKPV"))) {
        if(tok.size() < 3) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine); return; }
        if(node.isGroup()) { warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine + QStringLiteral(" (channels only)")); return; }
        node.ackPv = truncated(tok.at(1), AckPvNameSize, QStringLiteral("ack pv"), file, lineNo);   // last wins
        node.ackValue = truncated(tok.at(2), AckPvValueSize, QStringLiteral("ack value"), file, lineNo);
    } else {
        warn(AlhWarning::InvalidOptional, file, lineNo, dollarLine);
    }
}
