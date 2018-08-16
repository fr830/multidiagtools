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
#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include "TestBase.h"

class CacheTableModelTest : public QObject
{
 Q_OBJECT

 private slots:

//   void defaultHeaderTest();
//   void customHeaderTest();

  void initTestCase();

  void readOnlyHorizontalHeaderTest();
  void editableHorizontalHeaderTest();

  void readOnlyNoCacheGetTest();
  void editableNoCacheGetTest();

  void readOnlyFlagsTest();
  void editableFlagsTest();

  void readOnlySetCacheTest();
  void editableSetCacheTest();

  void readOnlySetCacheSignalTest();
  void editableSetCacheSignalTest();

  void setDataTest();
  void setDataSignalTest();

  void setDataFromCacheTest();
  void setDataFromCacheSignalTest();

  void readOnlyInsertRowsFromCacheTest();
  void editableInsertRowsFromCacheTest();
  void readOnlyInsertRowsFromCacheSignalTest();
  void editableInsertRowsFromCacheSignalTest();

  void insertRowsTest();
  void insertRowsSignalTest();

  void insertRowsAndSubmitTest();
  void insertRowsAndSubmitSignalTest();

///   void readOnlyItemModelInsertRowTest();
///   void editableItemModelInsertRowTest();

  void readOnlyQtModelTest();
  void editableQtModelTest();

};

#endif // #ifndef TEST_MAIN_H
