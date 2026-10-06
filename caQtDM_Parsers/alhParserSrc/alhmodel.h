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
 *
 *  Data model of an EPICS Alarm Handler (alh) configuration; semantics follow
 *  alConfig.c / alLib.c of https://github.com/epics-extensions/alh
 */

#ifndef ALHMODEL_H
#define ALHMODEL_H

#include <QJsonDocument>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>
#include "alhparserdefs.h"

// severity index as used by alh (ERROR = ALARM_NSEV = not connected)
enum AlhSeverity { AlhSevNoAlarm = 0, AlhSevMinor = 1, AlhSevMajor = 2,
                   AlhSevInvalid = 3, AlhSevError = 4, AlhSevCount = 5 };

enum AlhNodeType { AlhGroupNode, AlhChannelNode };

// channel mask, alSetMask semantics: character based, position independent
struct ALHPARSER_EXPORT AlhMask {
    bool cancel;          // C: not subscribed, does not count
    bool disable;         // D: subscribed, not reported
    bool noAck;           // A: no acknowledge required
    bool noAckTransient;  // T: transient alarms need no acknowledge
    bool noLog;           // L: not logged

    AlhMask() : cancel(false), disable(false), noAck(false), noAckTransient(false), noLog(false) {}
    static AlhMask fromString(const QString &s, QString *unknownChars = Q_NULLPTR);
    QString toString() const;        // "CDATL" positions, '-' for unset
    bool isEmpty() const;
    bool operator==(const AlhMask &o) const;
    bool operator!=(const AlhMask &o) const { return !(*this == o); }
};

struct ALHPARSER_EXPORT AlhForcePv {
    QString pv;              // "CALC" selects the calc variant
    AlhMask mask;
    double forceValue;       // default 1
    double resetValue;       // default 0
    bool resetNE;            // reset when value != forceValue
    bool isCalc;
    QString calcExpr;        // $FORCEPV_CALC
    QString calcInput[6];    // $FORCEPV_CALC_A..F: number != 0 is a constant, else a pv

    AlhForcePv() : forceValue(1.0), resetValue(0.0), resetNE(false), isCalc(false) {}
    static bool inputIsConstant(const QString &input, double *value = Q_NULLPTR);
};

// $SEVRCOMMAND UP_<SEV>|DOWN_<SEV>|UP_ALARM <command> (parsed, never executed)
struct ALHPARSER_EXPORT AlhSevrCommand {
    bool up;
    int severity;            // 0..4, AlhSevCount = no match (alh keeps it silently)
    bool anyAlarm;           // "ALARM" keyword
    QString token;
    QString command;
    AlhSevrCommand() : up(true), severity(AlhSevCount), anyAlarm(false) {}
};

// $STATCOMMAND <STATUS> <command> (parsed, never executed)
struct ALHPARSER_EXPORT AlhStatCommand {
    QString status;
    QString command;
};

struct ALHPARSER_EXPORT AlhCountFilter {
    int count;
    int seconds;
    AlhCountFilter() : count(0), seconds(0) {}
};

struct ALHPARSER_EXPORT AlhNode {
    int id;
    int parentId;                     // -1 = root
    QVector<int> children;
    AlhNodeType type;
    QString name;                     // group name or pv name
    QString alias;                    // $ALIAS (first wins)
    AlhMask mask;                     // channels only
    QString command;                  // $COMMAND (first wins)
    QStringList guidanceText;         // $GUIDANCE ... $END
    QString guidanceLocation;         // $GUIDANCE <location> (first wins)
    int beepSevr;                     // $BEEPSEVR (last wins), -1 = unset
    bool hasForcePv;                  // $FORCEPV (first wins)
    AlhForcePv forcePv;
    QString sevrPv;                   // $SEVRPV (first wins)
    QString ackPv;                    // $ACKPV (last wins), channels only
    QString ackValue;
    bool hasCountFilter;              // $ALARMCOUNTFILTER (first wins), channels only
    AlhCountFilter countFilter;
    QList<AlhSevrCommand> sevrCommands;
    QList<AlhStatCommand> statCommands;   // channels only
    QString sourceFile;
    int sourceLine;

    AlhNode() : id(-1), parentId(-1), type(AlhGroupNode), beepSevr(-1),
                hasForcePv(false), hasCountFilter(false), sourceLine(0) {}
    bool isGroup() const { return type == AlhGroupNode; }
    bool isChannel() const { return type == AlhChannelNode; }
    QString displayName() const { return alias.isEmpty() ? name : alias; }
};

struct ALHPARSER_EXPORT AlhWarning {
    enum Kind { InvalidInput, InvalidOptional, InvalidEnd, LogicNoContext, ParentNotFound,
                IncludeMissing, IncludeRecursion, Truncated, UnknownMaskChar,
                UnsupportedFeature, DuplicateIgnored };
    Kind kind;
    QString file;
    int line;
    QString text;

    AlhWarning() : kind(InvalidInput), line(0) {}
    AlhWarning(Kind k, const QString &f, int l, const QString &t) : kind(k), file(f), line(l), text(t) {}
    QString kindString() const;
    QString toString() const;        // "file:line: kind: text"
};

class ALHPARSER_EXPORT AlhModel
{
public:
    AlhModel();

    void clear();

    // tree
    int addNode(const AlhNode &node);          // sets id, links to parentId; returns id
    int nodeCount() const { return m_nodes.size(); }
    int groupCount() const;
    int channelCount() const;
    const QVector<AlhNode> &nodes() const { return m_nodes; }
    AlhNode &node(int id) { return m_nodes[id]; }
    const AlhNode &node(int id) const { return m_nodes.at(id); }
    const QVector<int> &roots() const { return m_roots; }
    bool isAncestorOrSelf(int ancestorId, int id) const;
    QString nodePath(int id) const;            // names from root, '/' separated
    int findByPath(const QString &path) const; // -1 if not found

    // globals
    int beepSeverity() const { return m_beepSeverity; }            // -1 = unset
    int effectiveBeepSeverity() const;                             // default MINOR
    void setBeepSeverity(int sev) { m_beepSeverity = sev; }
    bool hasHeartbeat() const { return m_hasHeartbeat; }
    QString heartbeatPv() const { return m_heartbeatPv; }
    double heartbeatRate() const { return m_heartbeatRate; }
    int heartbeatValue() const { return m_heartbeatValue; }
    void setHeartbeat(const QString &pv, double rate, int value);

    // files / diagnostics
    QString mainFile() const { return m_mainFile; }
    void setMainFile(const QString &f) { m_mainFile = f; }
    QStringList includedFiles() const { return m_includedFiles; }
    void addIncludedFile(const QString &f) { m_includedFiles.append(f); }
    const QList<AlhWarning> &warnings() const { return m_warnings; }
    void addWarning(const AlhWarning &w) { m_warnings.append(w); }
    QStringList unsupportedFeatures() const;   // $SEVRCOMMAND/$STATCOMMAND occurrences

    // output
    QString treeDump() const;                  // format of alConfigTreePrint
    QJsonDocument toJson() const;

    // names
    static QString severityName(int sev);
    static int severityFromToken(const QString &token);     // prefix match like alh, -1 if none
    static const QStringList &statusNames();
    static int statusFromToken(const QString &token);       // prefix match, -1 if none

private:
    void dumpGroup(QString &out, int id, QString &sym) const;
    QVector<AlhNode> m_nodes;
    QVector<int> m_roots;
    int m_beepSeverity;
    bool m_hasHeartbeat;
    QString m_heartbeatPv;
    double m_heartbeatRate;
    int m_heartbeatValue;
    QString m_mainFile;
    QStringList m_includedFiles;
    QList<AlhWarning> m_warnings;
};

#endif // ALHMODEL_H
