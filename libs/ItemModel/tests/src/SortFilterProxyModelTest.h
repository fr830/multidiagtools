/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
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
#ifndef MDT_ITEM_MODEL_SORT_FILTER_PROXY_MODEL_TEST_H
#define MDT_ITEM_MODEL_SORT_FILTER_PROXY_MODEL_TEST_H

#include "TestBase.h"

class SortFilterProxyModelTest : public TestBase
{
 Q_OBJECT

 private slots:

  void initTestCase();
  void cleanupTestCase();

  void QSortFilterProxyModelFilterInsertRowTest();
  void QSortFilterProxyModelFilterInsertRowTest_data();

  void filterInsertRowTest();
  void filterInsertRowTest_data();

  void qtModelTest();

 private:

  void setupInsertRowTestData();
};

#endif // #ifndef MDT_ITEM_MODEL_SORT_FILTER_PROXY_MODEL_TEST_H
