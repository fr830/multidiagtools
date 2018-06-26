/****************************************************************************
 **
 ** Copyright (C) 2011-2018 Philippe Steinmann.
 **
 ** This file is part of multiDiagTools library.
 **
 ** multiDiagTools is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** multiDiagTools is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with multiDiagTools.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#include "LikeExpressionBenchmark.h"
#include "Mdt/QueryExpression/LikeExpression.h"
#include "Mdt/QueryExpression/LikeExpressionRegexTransform.h"

using namespace Mdt::QueryExpression;

/*
 * Tests
 */

void LikeExpressionBenchmark::likeExpressionRegexTransformEscapeBenchmark()
{
  QString str;
  QBENCHMARK{
    str = "*A?B{0102}??C*D\\?E";
    LikeExpressionRegexTransform::escapeRegexMetacharacters(str);
  }
  QCOMPARE(str, QString("*A?B\\{0102\\}??C*D\\?E"));
}

void LikeExpressionBenchmark::likeExpressionRegexTransformBenchmark()
{
  QString result;
  QString likeExpr("*A?B{0102}??C*D\\?E");
  QBENCHMARK{
    result = LikeExpressionRegexTransform::getRegexPattern(likeExpr);
  }
  QCOMPARE(result, QString("^.*A.B\\{0102\\}..C.*D\\?E$"));
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::CoreApplication app(argc, argv);
  LikeExpressionBenchmark test;

  return QTest::qExec(&test, argc, argv);
}
