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
#ifndef TST_ALHPARSER_H
#define TST_ALHPARSER_H

#include <QObject>
#include <QTest>

#include "alhconfigparser.h"
#include "alhuigenerator.h"

// parser, model and ui generator of caQtDM_Parsers/alhParserSrc (fixtures in tst_alh_fixtures.qrc)
class TestAlhParser : public QObject
{
    Q_OBJECT
public:
    TestAlhParser() = default;

private slots:
    void maskRoundTrip();
    void forcePvInputs();
    void upstreamExample();
    void treeDumpFormat();
    void optionalLines();
    void toleranceContinuesParsing();
    void includeFiles();
    void alhDefaults();
    void macros();
    void sevrCommandTokens();
    void jsonContainsTree();
    void uiGenerator();

private:
    bool parseFixture(const QString &name, AlhModel &model, const AlhConfigParser::Options &options = AlhConfigParser::Options());
    int countWarnings(const AlhModel &model, AlhWarning::Kind kind) const;
};

#endif // TST_ALHPARSER_H
