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
#ifndef ABSTRACT_TABLE_MODEL_TEST_H
#define ABSTRACT_TABLE_MODEL_TEST_H

#include "TestBase.h"

class AbstractTableModelTest : public QObject
{
 Q_OBJECT

 private slots:

  void initTestCase();

  void readOnlyConstructTest();
  void editableConstructTest();

  void readOnlyRowCountTest();
  void editableRowCountTest();

  void readOnlyColumnCountTest();
  void editableColumnCountTest();

  void readOnlyFlagsTest();
  void editableFlagsTest();

  void defaultHeaderTest();
  void customHeaderTest();

  void readOnlyDataTest();
  void editableDataTest();

  void emitDataChangedTest();

  void emitVerticalHeaderDataChangedTest();

  void readOnlySetDataTest();
  void editableSetDataTest();

  void readOnlyPrependAppendRowsTest();
  void editablePrependAppendRowsTest();
  void itemModelInsertRowTest();

  void readOnlyRemoveFirstLastRowTest();
  void editableRemoveFirstLastRowTest();
  void itemModelRemoveRowTest();

  void readOnlyQtModelTest();
  void editableQtModelTest();

//   void insertRowsTest();
//   void insertRowsSignalTest();
//   void itemModelInsertRowTest();
//   void setDataTest();
  
};

#endif // #ifndef ABSTRACT_TABLE_MODEL_TEST_H
