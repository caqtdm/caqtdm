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
 *  Reader for alh configuration files (.alhConfig), tolerant like alConfig.c:
 *  errors are reported as warnings, parsing never stops.
 */

#ifndef ALHCONFIGPARSER_H
#define ALHCONFIGPARSER_H

#include <QMap>
#include <QString>
#include <QStringList>
#include "alhparserdefs.h"
#include "alhmodel.h"

class QTextStream;

class ALHPARSER_EXPORT AlhConfigParser
{
public:
    struct Options {
        QString includeDir;               // alh configDir; default: directory of the main file
        QMap<QString, QString> macros;    // $(NAME) substitution before parsing (caQtDM extension)
        int maxIncludeDepth;
        int pvNameSize;                   // alh PVNAME_SIZE = PVNAME_STRINGSZ + FLDNAME_SZ
        Options() : maxIncludeDepth(32), pvNameSize(65) {}
    };

    // alh buffer sizes (sscanf widths / fgets buffer)
    enum { GroupNameSize = 64, ChannelTokenSize = 127, LineBufferSize = 500,
           ForcePvNameSize = 64, ForcePvMaskSize = 6, AckPvNameSize = 30, AckPvValueSize = 32 };

    explicit AlhConfigParser(const Options &options = Options());

    bool parseFile(const QString &path, AlhModel *model);     // false only if the main file is unreadable
    bool parseText(const QString &text, const QString &virtualPath, AlhModel *model);

    static QString expandMacros(const QString &text, const QMap<QString, QString> &macros);

private:
    bool readFile(const QString &path, int depth, int includeParent);
    void parseStream(QTextStream &in, const QString &file, int depth, int includeParent);
    void handleGroup(const QStringList &tok, const QString &file, int lineNo, int includeParent);
    void handleChannel(const QStringList &tok, const QString &file, int lineNo);
    void handleInclude(const QStringList &tok, const QString &file, int lineNo, int depth);
    void handleOptional(const QString &content, QTextStream &in, const QString &file, int &lineNo);
    void readGuidanceBlock(QTextStream &in, const QString &file, int &lineNo, AlhNode &node);
    int resolveParent(const QString &parentName) const;
    QString truncated(const QString &s, int size, const QString &what, const QString &file, int lineNo);
    void warn(AlhWarning::Kind kind, const QString &file, int line, const QString &text);
    QString decode(const QByteArray &raw) const;

    Options m_opt;
    QString m_includeDir;
    AlhModel *m_model;
    int m_curGroup;        // group context for parent resolution
    int m_curNode;         // target of $-lines (last GROUP or CHANNEL)
    QStringList m_includeStack;
};

#endif // ALHCONFIGPARSER_H
