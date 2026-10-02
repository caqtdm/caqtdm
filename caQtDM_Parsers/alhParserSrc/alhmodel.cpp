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

#include "alhmodel.h"
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(alhParserLog, "caqtdm.parsers.alh")

static const char *alhSeverityNames[AlhSevCount] = { "NO_ALARM", "MINOR", "MAJOR", "INVALID", "ERROR" };

// EPICS alarm.h status strings (alarmStatusString) + the three alh additions
static const char *alhStatusNames[] = {
    "NO_ALARM", "READ", "WRITE", "HIHI", "HIGH", "LOLO", "LOW", "STATE", "COS", "COMM",
    "TIMEOUT", "HWLIMIT", "CALC", "SCAN", "LINK", "SOFT", "BAD_SUB", "UDF", "DISABLE", "SIMM",
    "READ_ACCESS", "WRITE_ACCESS", "NOT_CONNECTED", "NO_READ_ACCESS", "NO_WRITE_ACCESS", Q_NULLPTR };

//---------------------------------------------------------------------------- AlhMask

AlhMask AlhMask::fromString(const QString &s, QString *unknownChars)
{
    AlhMask m;
    for(int i = 0; i < s.length(); i++) {
        const QChar c = s.at(i);
        if(c == QLatin1Char('T')) m.noAckTransient = true;
        else if(c == QLatin1Char('L')) m.noLog = true;
        else if(c == QLatin1Char('D')) m.disable = true;
        else if(c == QLatin1Char('C')) m.cancel = true;
        else if(c == QLatin1Char('A')) m.noAck = true;
        else if(c != QLatin1Char('-') && unknownChars != Q_NULLPTR) unknownChars->append(c);
    }
    return m;
}

QString AlhMask::toString() const
{
    QString s = QStringLiteral("-----");
    if(cancel) s[0] = QLatin1Char('C');
    if(disable) s[1] = QLatin1Char('D');
    if(noAck) s[2] = QLatin1Char('A');
    if(noAckTransient) s[3] = QLatin1Char('T');
    if(noLog) s[4] = QLatin1Char('L');
    return s;
}

bool AlhMask::isEmpty() const
{
    return !cancel && !disable && !noAck && !noAckTransient && !noLog;
}

bool AlhMask::operator==(const AlhMask &o) const
{
    return cancel == o.cancel && disable == o.disable && noAck == o.noAck &&
           noAckTransient == o.noAckTransient && noLog == o.noLog;
}

//---------------------------------------------------------------------------- AlhForcePv

// alh uses atof(): a leading number != 0 means constant, anything else is a pv
bool AlhForcePv::inputIsConstant(const QString &input, double *value)
{
    static const QRegularExpression leadingNumber(QStringLiteral("^\\s*[-+]?(\\d+\\.?\\d*|\\.\\d+)([eE][-+]?\\d+)?"));
    const QRegularExpressionMatch m = leadingNumber.match(input);
    if(!m.hasMatch()) return false;
    const double v = m.captured(0).trimmed().toDouble();
    if(v == 0.0) return false;
    if(value != Q_NULLPTR) *value = v;
    return true;
}

//---------------------------------------------------------------------------- AlhWarning

QString AlhWarning::kindString() const
{
    switch(kind) {
    case InvalidInput: return QStringLiteral("Invalid input line");
    case InvalidOptional: return QStringLiteral("Invalid Optional Line");
    case InvalidEnd: return QStringLiteral("Invalid End");
    case LogicNoContext: return QStringLiteral("Logic error: glink is NULL");
    case ParentNotFound: return QStringLiteral("Invalid parent");
    case IncludeMissing: return QStringLiteral("Ignoring Invalid INCLUDE file");
    case IncludeRecursion: return QStringLiteral("Recursive INCLUDE");
    case Truncated: return QStringLiteral("Truncated");
    case UnknownMaskChar: return QStringLiteral("Unknown mask character");
    case UnsupportedFeature: return QStringLiteral("Not supported by caQtDM");
    case DuplicateIgnored: return QStringLiteral("Duplicate ignored");
    }
    return QString();
}

QString AlhWarning::toString() const
{
    return QString("%1:%2: %3: %4").arg(QFileInfo(file).fileName()).arg(line).arg(kindString(), text);
}

//---------------------------------------------------------------------------- AlhModel

AlhModel::AlhModel()
{
    clear();
}

void AlhModel::clear()
{
    m_nodes.clear();
    m_roots.clear();
    m_beepSeverity = -1;
    m_hasHeartbeat = false;
    m_heartbeatPv.clear();
    m_heartbeatRate = 0.0;
    m_heartbeatValue = 0;
    m_mainFile.clear();
    m_includedFiles.clear();
    m_warnings.clear();
}

int AlhModel::addNode(const AlhNode &node)
{
    AlhNode n = node;
    n.id = m_nodes.size();
    n.children.clear();
    m_nodes.append(n);
    if(n.parentId >= 0 && n.parentId < m_nodes.size() - 1) m_nodes[n.parentId].children.append(n.id);
    else { m_nodes[n.id].parentId = -1; m_roots.append(n.id); }
    return n.id;
}

int AlhModel::groupCount() const
{
    int n = 0;
    foreach(const AlhNode &node, m_nodes) if(node.isGroup()) n++;
    return n;
}

int AlhModel::channelCount() const
{
    return m_nodes.size() - groupCount();
}

bool AlhModel::isAncestorOrSelf(int ancestorId, int id) const
{
    while(id >= 0) {
        if(id == ancestorId) return true;
        id = m_nodes.at(id).parentId;
    }
    return false;
}

QString AlhModel::nodePath(int id) const
{
    QStringList parts;
    while(id >= 0) {
        parts.prepend(m_nodes.at(id).name);
        id = m_nodes.at(id).parentId;
    }
    return parts.join(QLatin1Char('/'));
}

int AlhModel::findByPath(const QString &path) const
{
    for(int i = 0; i < m_nodes.size(); i++) if(nodePath(i) == path) return i;
    return -1;
}

int AlhModel::effectiveBeepSeverity() const
{
    return m_beepSeverity >= 0 ? m_beepSeverity : (int) AlhSevMinor;
}

void AlhModel::setHeartbeat(const QString &pv, double rate, int value)
{
    m_hasHeartbeat = true;
    m_heartbeatPv = pv;
    m_heartbeatRate = rate;
    m_heartbeatValue = value;
}

QStringList AlhModel::unsupportedFeatures() const
{
    QStringList list;
    foreach(const AlhNode &n, m_nodes) {
        const QString where = QString("%1:%2: %3").arg(QFileInfo(n.sourceFile).fileName()).arg(n.sourceLine).arg(nodePath(n.id));
        foreach(const AlhSevrCommand &c, n.sevrCommands)
            list.append(QString("%1: $SEVRCOMMAND %2 %3 (not executed)").arg(where, c.token, c.command));
        foreach(const AlhStatCommand &c, n.statCommands)
            list.append(QString("%1: $STATCOMMAND %2 %3 (not executed)").arg(where, c.status, c.command));
    }
    return list;
}

// alConfigTreePrint: groups and channels with "+--" / "|  " prefixes, names padded to 28
QString AlhModel::treeDump() const
{
    QString out;
    foreach(int root, m_roots) {
        QString sym;
        dumpGroup(out, root, sym);
    }
    return out;
}

void AlhModel::dumpGroup(QString &out, int id, QString &sym) const
{
    static const QString symMiddle = QStringLiteral("+--");
    static const QString symContinue = QStringLiteral("|  ");
    static const QString symBlank = QStringLiteral("   ");
    const AlhNode &g = m_nodes.at(id);
    const int length = sym.length();

    // next sibling group (alh: sllNext in the parent's subGroupList)
    bool hasNextGroup = false;
    if(g.parentId >= 0) {
        const QVector<int> &sib = m_nodes.at(g.parentId).children;
        bool seen = false;
        foreach(int s, sib) {
            if(s == id) { seen = true; continue; }
            if(seen && m_nodes.at(s).isGroup()) { hasNextGroup = true; break; }
        }
    }
    bool hasSubGroup = false;
    foreach(int c, g.children) if(m_nodes.at(c).isGroup()) { hasSubGroup = true; break; }

    if(length) sym.replace(length - 3, 3, symMiddle);
    out += sym + g.name.leftJustified(28) + QLatin1Char('\n');
    if(length) sym.replace(length - 3, 3, hasNextGroup ? symContinue : symBlank);

    sym += hasSubGroup ? symContinue : symBlank;
    foreach(int c, g.children) {
        if(m_nodes.at(c).isChannel())
            out += sym + QStringLiteral("  ") + m_nodes.at(c).name.leftJustified(28) + QLatin1Char('\n');
    }
    sym.replace(length, 3, symBlank);
    foreach(int c, g.children) {
        if(m_nodes.at(c).isGroup()) dumpGroup(out, c, sym);
    }
    sym.truncate(length);
}

static QJsonObject nodeToJson(const AlhModel &model, const AlhNode &n)
{
    QJsonObject o;
    o["type"] = n.isGroup() ? QStringLiteral("group") : QStringLiteral("channel");
    o["name"] = n.name;
    o["file"] = QFileInfo(n.sourceFile).fileName();
    o["line"] = n.sourceLine;
    if(!n.alias.isEmpty()) o["alias"] = n.alias;
    if(n.isChannel()) o["mask"] = n.mask.toString();
    if(!n.command.isEmpty()) o["command"] = n.command;
    if(!n.guidanceText.isEmpty()) o["guidance"] = QJsonArray::fromStringList(n.guidanceText);
    if(!n.guidanceLocation.isEmpty()) o["guidanceLocation"] = n.guidanceLocation;
    if(n.beepSevr >= 0) o["beepSevr"] = AlhModel::severityName(n.beepSevr);
    if(n.hasForcePv) {
        QJsonObject f;
        f["pv"] = n.forcePv.pv;
        f["mask"] = n.forcePv.mask.toString();
        f["forceValue"] = n.forcePv.forceValue;
        if(n.forcePv.resetNE) f["reset"] = QStringLiteral("NE");
        else f["resetValue"] = n.forcePv.resetValue;
        if(n.forcePv.isCalc) {
            f["calc"] = n.forcePv.calcExpr;
            QJsonArray in;
            for(int i = 0; i < 6; i++) in.append(n.forcePv.calcInput[i]);
            f["inputs"] = in;
        }
        o["forcePv"] = f;
    }
    if(!n.sevrPv.isEmpty()) o["sevrPv"] = n.sevrPv;
    if(!n.ackPv.isEmpty()) {
        QJsonObject a;
        a["pv"] = n.ackPv;
        a["value"] = n.ackValue;
        o["ackPv"] = a;
    }
    if(n.hasCountFilter) {
        QJsonObject c;
        c["count"] = n.countFilter.count;
        c["seconds"] = n.countFilter.seconds;
        o["countFilter"] = c;
    }
    if(!n.sevrCommands.isEmpty()) {
        QJsonArray a;
        foreach(const AlhSevrCommand &c, n.sevrCommands) {
            QJsonObject s;
            s["token"] = c.token;
            s["direction"] = c.up ? QStringLiteral("UP") : QStringLiteral("DOWN");
            s["severity"] = c.anyAlarm ? QStringLiteral("ALARM") : (c.severity >= AlhSevCount ? QStringLiteral("ANY") : AlhModel::severityName(c.severity));
            s["command"] = c.command;
            a.append(s);
        }
        o["sevrCommands"] = a;
    }
    if(!n.statCommands.isEmpty()) {
        QJsonArray a;
        foreach(const AlhStatCommand &c, n.statCommands) {
            QJsonObject s;
            s["status"] = c.status;
            s["command"] = c.command;
            a.append(s);
        }
        o["statCommands"] = a;
    }
    if(!n.children.isEmpty()) {
        QJsonArray ch;
        foreach(int c, n.children) ch.append(nodeToJson(model, model.node(c)));
        o["children"] = ch;
    }
    return o;
}

QJsonDocument AlhModel::toJson() const
{
    QJsonObject root;
    root["file"] = QFileInfo(m_mainFile).fileName();
    root["beepSeverity"] = m_beepSeverity >= 0 ? QJsonValue(severityName(m_beepSeverity)) : QJsonValue();
    if(m_hasHeartbeat) {
        QJsonObject h;
        h["pv"] = m_heartbeatPv;
        h["rate"] = m_heartbeatRate;
        h["value"] = m_heartbeatValue;
        root["heartbeat"] = h;
    }
    QJsonArray inc;
    foreach(const QString &f, m_includedFiles) inc.append(QFileInfo(f).fileName());
    root["includes"] = inc;
    QJsonArray roots;
    foreach(int r, m_roots) roots.append(nodeToJson(*this, m_nodes.at(r)));
    root["roots"] = roots;
    QJsonArray warn;
    foreach(const AlhWarning &w, m_warnings) {
        QJsonObject o;
        o["kind"] = w.kindString();
        o["file"] = QFileInfo(w.file).fileName();
        o["line"] = w.line;
        o["text"] = w.text;
        warn.append(o);
    }
    root["warnings"] = warn;
    return QJsonDocument(root);
}

QString AlhModel::severityName(int sev)
{
    if(sev >= 0 && sev < AlhSevCount) return QString::fromLatin1(alhSeverityNames[sev]);
    return QStringLiteral("UNKNOWN");
}

int AlhModel::severityFromToken(const QString &token)
{
    for(int i = 0; i < AlhSevCount; i++)
        if(token.startsWith(QLatin1String(alhSeverityNames[i]))) return i;
    return -1;
}

const QStringList &AlhModel::statusNames()
{
    static QStringList names;
    if(names.isEmpty())
        for(int i = 0; alhStatusNames[i] != Q_NULLPTR; i++) names.append(QString::fromLatin1(alhStatusNames[i]));
    return names;
}

int AlhModel::statusFromToken(const QString &token)
{
    const QStringList &names = statusNames();
    for(int i = 0; i < names.size(); i++)
        if(token.startsWith(names.at(i))) return i;
    return -1;
}
