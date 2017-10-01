/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
 **
 ** This file is part of Mdt library.
 **
 ** Mdt is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** Mdt is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with Mdt. If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#ifndef MDT_ITEM_MODEL_HEADER_PROXY_MODEL_TEST_H
#define MDT_ITEM_MODEL_HEADER_PROXY_MODEL_TEST_H

#include "TestBase.h"

class HeaderProxyModelTest : public TestBase
{
 Q_OBJECT

 private slots:

  void initTestCase();
  void cleanupTestCase();

  void itemTest();
  void itemListTest();
  void dimensionTest();
  void horizontalLabelsTest();
  void qtModelTest();
};

#endif // #ifndef MDT_ITEM_MODEL_HEADER_PROXY_MODEL_TEST_H
