/****************************************************************************
 **
 ** Copyright (C) 2011-2016 Philippe Steinmann.
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
#ifndef MDT_SQL_SCHEMA_TEST_H
#define MDT_SQL_SCHEMA_TEST_H

#include <QObject>
#include <QtTest/QtTest>

class SchemaTest : public QObject
{
 Q_OBJECT

 private slots:

  void initTestCase();
  void cleanupTestCase();

  void fieldTypeListTest();
  void fieldTypeNameTest();
  void fiedTypeListModelTest();

  void collationTest();

  void fieldTest();
  void fieldListTest();

  void autoIncrementPrimaryKeyTest();
  void singleFieldPrimaryKeyTest();
  void primaryKeyTest();
  void primaryKeyContainerTest();

  void indexTest();

  void foreignKeyTest();

  void tablePrimaryKeyTest();
  void tablePrimaryKeyAicBenchmark();
  void tablePrimaryKeyMcBenchmark();
  void tableTest();
  void tableModelTest();

};

#endif // #ifndef MDT_SQL_SCHEMA_TEST_H
