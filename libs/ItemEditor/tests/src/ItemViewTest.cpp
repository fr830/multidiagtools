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
#include "ItemViewTest.h"
#include "ItemViewTestEdit.h"
#include "Mdt/Application.h"
#include "TestTableModel.h"
#include "Mdt/ItemEditor/ItemSelectionModel.h"
#include <QSignalSpy>
// #include <QItemSelectionModel>
// #include <QStringListModel>
#include <QTableView>
#include <QTreeView>
#include <QListView>
#include <QItemSelection>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>

#include <QDebug>


/*
 * Init and cleanup
 */

void ItemViewTest::initTestCase()
{
}

void ItemViewTest::cleanupTestCase()
{
}

/*
 * Check that editing helper functions
 * works with a standard views and delegates
 */

void ItemViewTest::tableViewEditBeginEndTest()
{
  QTableView view;
  TestTableModel model;
  QModelIndex index;
  QFETCH(int, rows);
  QFETCH(int, columns);
  QFETCH(BeginEditTrigger, beginEditTrigger);
  QFETCH(EndEditTrigger, endEditTrigger);

  /*
   * Setup model and view
   */
  model.populate(rows, columns);
  view.setModel(&model);
  view.show();
  /*
   * Check
   */
  // Check at row 0 and column 0
  index = model.index(0, 0);
  QVERIFY(index.isValid());
  QVERIFY(model.data(index) != QVariant("TEST"));
  edit(view, index, "TEST", beginEditTrigger, endEditTrigger);
  QCOMPARE(model.data(index), QVariant("TEST"));
  // Check at row 1 if possible
  if(model.rowCount() > 1){
    index = model.index(1, 0);
    QVERIFY(index.isValid());
    QVERIFY(model.data(index) != QVariant("TEST"));
    edit(view, index, "TEST", beginEditTrigger, endEditTrigger);
    QCOMPARE(model.data(index), QVariant("TEST"));
  }
  // Check at column 1 if possible
  if(model.columnCount() > 1){
    index = model.index(0, 1);
    QVERIFY(index.isValid());
    QVERIFY(model.data(index) != QVariant("TEST"));
    edit(view, index, "TEST", beginEditTrigger, endEditTrigger);
    QCOMPARE(model.data(index), QVariant("TEST"));
  }
  /*
   * Play
   */
//   view.resize(300, 200);
//   while(view.isVisible()){
//     QTest::qWait(500);
//   }
}

void ItemViewTest::tableViewEditBeginEndTest_data()
{
  QTest::addColumn<int>("rows");
  QTest::addColumn<int>("columns");
  QTest::addColumn<BeginEditTrigger>("beginEditTrigger");
  QTest::addColumn<EndEditTrigger>("endEditTrigger");

  QTest::newRow("S:DClick/E:Click/(1,2)") << 1 << 2 << BeginEditTrigger::DoubleClick << EndEditTrigger::IndexChange;
  QTest::newRow("S:DClick/E:Click/(2,1)") << 2 << 1 << BeginEditTrigger::DoubleClick << EndEditTrigger::IndexChange;
  QTest::newRow("S:DClick/E:<Enter>/(2,1)") << 2 << 1 << BeginEditTrigger::DoubleClick << EndEditTrigger::EnterKeyClick;
  QTest::newRow("S:F2/E:Click/(2,1)") << 2 << 1 << BeginEditTrigger::F2KeyClick << EndEditTrigger::IndexChange;
  QTest::newRow("S:F2/E:<Enter>/(2,1)") << 2 << 1 << BeginEditTrigger::F2KeyClick << EndEditTrigger::EnterKeyClick;
}

/*
 * Tests
 */

void ItemViewTest::tableViewSetCurrentIndexTest()
{
  using Mdt::ItemEditor::ItemSelectionModel;

  QTableView view;
  TestTableModel model;
  QModelIndex index;

  /*
   * Initial state
   */
  ItemSelectionModel selectionModel(&model);
  QVERIFY(selectionModel.isCurrentRowChangeAllowed());
  /*
   * Setup model and view
   */
  model.populate(3, 2);
  view.setModel(&model);
  view.setSelectionModel(&selectionModel);
  view.show();
  /*
   * Check using setCurrentIndex()
   */
  index = model.index(1, 0);
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 1);
  QCOMPARE(view.currentIndex().column(), 0);
  // Inhibit row change
  selectionModel.setCurrentRowChangeAllowed(false);
  QVERIFY(!selectionModel.isCurrentRowChangeAllowed());
  index = model.index(0, 0);
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 1);
  QCOMPARE(view.currentIndex().column(), 0);
  // Changing column must still work
  index = model.index(1, 1);
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 1);
  QCOMPARE(view.currentIndex().column(), 1);
  /*
   * Check using select(QModelIndex) before setCurrentIndex()
   */
  // Allow row change and go back to row 0
  selectionModel.setCurrentRowChangeAllowed(true);
  index = model.index(0, 0);
  selectionModel.select(index, QItemSelectionModel::ClearAndSelect);
  QVERIFY(selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 0);
  // Inhibit row change
  selectionModel.setCurrentRowChangeAllowed(false);
  index = model.index(1, 0);
  selectionModel.select(index, QItemSelectionModel::ClearAndSelect);
  QVERIFY(!selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 0);
  // Changing column must still work
  index = model.index(0, 1);
  selectionModel.select(index, QItemSelectionModel::ClearAndSelect);
  QVERIFY(selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 1);
  /*
   * Check using select(QItemSelection) before setCurrentIndex()
   */
  // Allow row change and go back to row 0
  selectionModel.setCurrentRowChangeAllowed(true);
  index = model.index(0, 0);
  selectionModel.select(QItemSelection(index, index), QItemSelectionModel::ClearAndSelect);
  QVERIFY(selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 0);
  // Inhibit row change
  selectionModel.setCurrentRowChangeAllowed(false);
  index = model.index(1, 0);
  selectionModel.select(QItemSelection(index, index), QItemSelectionModel::ClearAndSelect);
  QVERIFY(!selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 0);
  // Changing column must still work
  index = model.index(0, 1);
  selectionModel.select(QItemSelection(index, index), QItemSelectionModel::ClearAndSelect);
  QVERIFY(selectionModel.isSelected(index));
  view.setCurrentIndex(index);
  QCOMPARE(view.currentIndex().row(), 0);
  QCOMPARE(view.currentIndex().column(), 1);

  /*
   * Play
   */
  view.resize(300, 200);
  while(view.isVisible()){
    QTest::qWait(500);
  }
}


/*
 * Helper functions for editing item views
 */

void ItemViewTest::beginEditing(QAbstractItemView & view, const QModelIndex & index, BeginEditTrigger beginEditTrigger)
{
  ItemViewTestEdit::beginEditing(view, index, beginEditTrigger);
}

void ItemViewTest::endEditing(QAbstractItemView & view, const QModelIndex & editingIndex, EndEditTrigger endEditTrigger)
{
  ItemViewTestEdit::endEditing(view, editingIndex, endEditTrigger);
}

void ItemViewTest::edit(QAbstractItemView& view, const QModelIndex& index, const QString& str,
                        BeginEditTrigger beginEditTrigger, EndEditTrigger endEditTrigger)
{
  ItemViewTestEdit::edit(view, index, str, beginEditTrigger, endEditTrigger);
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  ItemViewTest test;

  if(!app.init()){
    return 1;
  }
//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
