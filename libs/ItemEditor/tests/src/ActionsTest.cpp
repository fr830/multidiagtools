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
#include "ActionsTest.h"
#include "Mdt/Application.h"
#include "Mdt/ItemEditor/RowState.h"
#include "Mdt/ItemEditor/AbstractActionContainer.h"
#include "Mdt/ItemEditor/NavigationActions.h"
#include "Mdt/ItemEditor/EditionActions.h"
#include "Mdt/ItemEditor/InsertAction.h"
#include "Mdt/ItemEditor/RemoveAction.h"
#include "Mdt/ItemEditor/ResizeToContentsAction.h"
#include "Mdt/ItemEditor/ControllerState.h"
#include "Mdt/ItemEditor/AbstractControllerStateTable.h"
#include "Mdt/ItemEditor/AbstractControllerStatePermission.h"
#include "Mdt/ItemEditor/ControllerStateMachine.h"
#include <QSignalSpy>
#include <QItemSelectionModel>
#include <QStringListModel>
#include <QTableView>
#include <QPointer>
#include <QAction>
#include <memory>

using namespace Mdt::ItemEditor;

void ActionsTest::initTestCase()
{
}

void ActionsTest::cleanupTestCase()
{
}

/*
 * Test classes
 */

class StateTableTestClass : public AbstractControllerStateTable
{
 public:

  void createTable() override
  {
    addTransition(ControllerState::Visualizing, ControllerEvent::DataEditionStarted, ControllerState::Editing);
    addTransition(ControllerState::Visualizing, ControllerEvent::InsertStarted, ControllerState::Inserting);
    addTransition(ControllerState::Editing, ControllerEvent::SubmitDone, ControllerState::Visualizing);
    addTransition(ControllerState::Editing, ControllerEvent::RevertDone, ControllerState::Visualizing);
    addTransition(ControllerState::Inserting, ControllerEvent::SubmitDone, ControllerState::Visualizing);
    addTransition(ControllerState::Inserting, ControllerEvent::RevertDone, ControllerState::Visualizing);
  }
};

/*
 * ActionContainerTester implementation
 */

ActionContainerTester::ActionContainerTester(QObject* parent)
 : AbstractActionContainer(parent)
{
}

/*
 * Tests
 */

void ActionsTest::abstractActionContainerTest()
{
  RowState rs;
  /*
   * Initial state
   */
  ActionContainerTester act;
  QCOMPARE(act.rowCount(), 0);
  QCOMPARE(act.currentRow(), -1);
  QVERIFY(act.rowStateIsNull());
  QVERIFY(act.controllerState() == ControllerState::Visualizing);
  QCOMPARE(act.stateChangedCount(), 0);
  QVERIFY(!act.disableForced());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(&act);
  act.setControllerStateMachine(stateMachine);
  QCOMPARE(act.controllerState(), ControllerState::Visualizing);
  /*
   * Check setters and getters
   */
  // Check row state validity
  rs.setRowCount(20);
  rs.setCurrentRow(20);
  act.setRowState(rs);
  QVERIFY(!act.rowStateIsValid());
  rs.setCurrentRow(10);
  act.setRowState(rs);
  QVERIFY(act.rowStateIsValid());
  // Check other states
  stateMachine->forceCurrentState(ControllerState::Editing);
  QCOMPARE(act.rowCount(), 20);
  QCOMPARE(act.currentRow(), 10);
  QVERIFY(!act.rowStateIsNull());
  QCOMPARE(act.controllerState(), ControllerState::Editing);
  /*
   * Go back to initial state
   */
  rs.clear();
  act.setRowState(rs);
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  act.resetStateChangedCount();
  QCOMPARE(act.rowCount(), 0);
  QCOMPARE(act.currentRow(), -1);
  QCOMPARE(act.controllerState(), ControllerState::Visualizing);
  QCOMPARE(act.stateChangedCount(), 0);
  QVERIFY(!act.disableForced());
  /*
   * Check changing each property
   */
  rs.clear();
  // Change row count
  rs.setRowCount(1);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 1);
  // Change current row
  rs.setCurrentRow(0);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 2);
  // Change controller state
  stateMachine->forceCurrentState(ControllerState::Editing);
  QCOMPARE(act.stateChangedCount(), 3);
  /*
   * Go back to initial state
   */
  rs.clear();
  act.setRowState(rs);
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  act.resetStateChangedCount();
  QCOMPARE(act.rowCount(), 0);
  QCOMPARE(act.currentRow(), -1);
  QCOMPARE(act.controllerState(), ControllerState::Visualizing);
  QCOMPARE(act.stateChangedCount(), 0);
  QVERIFY(!act.disableForced());
  /*
   * Check changing a property multiple times
   */
  rs.clear();
  // Change row count
  rs.setRowCount(1);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 1);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 1);
  // Change current row
  rs.setCurrentRow(0);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 2);
  act.setRowState(rs);
  QCOMPARE(act.stateChangedCount(), 2);
  // Change controller state
  stateMachine->forceCurrentState(ControllerState::Editing);
  QCOMPARE(act.stateChangedCount(), 3);
  stateMachine->forceCurrentState(ControllerState::Editing);
  QCOMPARE(act.stateChangedCount(), 3);
  /*
   * Go back to initial state
   */
  rs.clear();
  act.setRowState(rs);
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  act.resetStateChangedCount();
  QCOMPARE(act.rowCount(), 0);
  QCOMPARE(act.currentRow(), -1);
  QCOMPARE(act.controllerState(), ControllerState::Visualizing);
  QCOMPARE(act.stateChangedCount(), 0);
  QVERIFY(!act.disableForced());
  /*
   * Force disabled
   */
  // Disable all actions
  act.setActionsDisabled(true);
  QVERIFY(act.disableForced());
  QCOMPARE(act.stateChangedCount(), 0);
  rs.clear();
  // Change row count - must not call updateEnableState()
  rs.setRowCount(1);
  act.setRowState(rs);
  QVERIFY(act.disableForced());
  QCOMPARE(act.stateChangedCount(), 0);
  // Change current row - must not call updateEnableState()
  rs.setCurrentRow(0);
  act.setRowState(rs);
  QVERIFY(act.disableForced());
  QCOMPARE(act.stateChangedCount(), 0);
  // Change controller state - must not call updateEnableState()
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(act.disableForced());
  QCOMPARE(act.stateChangedCount(), 0);
  // Clear forcing disabled - Must call updateEnableState()
  act.setActionsDisabled(false);
  QVERIFY(!act.disableForced());
  QCOMPARE(act.stateChangedCount(), 1);
}

void ActionsTest::navigationActionsTest()
{
  std::unique_ptr<NavigationActions> actions(new NavigationActions(nullptr));
  QPointer<QAction> toFirst = actions->toFirst();
  QPointer<QAction> toPrevious = actions->toPrevious();
  QPointer<QAction> toNext = actions->toNext();
  QPointer<QAction> toLast = actions->toLast();
  RowState rs;
  /*
   * Initial state
   */
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(actions.get());
  actions->setControllerStateMachine(stateMachine);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  /*
   * Check row changes
   */
  // N: 0 , current row: -1
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  actions->setRowState(rs);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  // N: 1 , current row: 0
  rs.setRowCount(1);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  // N: 2 , current row: 0
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  // N: 2 , current row: 1
  rs.setRowCount(2);
  rs.setCurrentRow(1);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  // N: 3 , current row: 0
  rs.setRowCount(3);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  // N: 3 , current row: 1
  rs.setRowCount(3);
  rs.setCurrentRow(1);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  // N: 3 , current row: 2
  rs.setRowCount(3);
  rs.setCurrentRow(2);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  /*
   * Check with some invalid row states
   */
  // N: 1 , current row: -1
  rs.setRowCount(1);
  rs.setCurrentRow(-1);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  // N: 2 , current row: -1
  rs.setRowCount(2);
  rs.setCurrentRow(-1);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  /*
   * Check in editing state
   */
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  // Set a valid row state
  rs.setRowCount(4);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(!toNext->isEnabled());
  QVERIFY(!toLast->isEnabled());
  /*
   * Go to visualizing state
   */
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  QVERIFY(!toFirst->isEnabled());
  QVERIFY(!toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  // Set a valid row state
  rs.setRowCount(4);
  rs.setCurrentRow(1);
  actions->setRowState(rs);
  QVERIFY(toFirst->isEnabled());
  QVERIFY(toPrevious->isEnabled());
  QVERIFY(toNext->isEnabled());
  QVERIFY(toLast->isEnabled());
  /*
   * Clear
   */
  actions.reset();
  QVERIFY(toFirst.isNull());
  QVERIFY(toPrevious.isNull());
  QVERIFY(toNext.isNull());
  QVERIFY(toLast.isNull());
}

void ActionsTest::editionActionsTest()
{
  std::unique_ptr<EditionActions> actions(new EditionActions(nullptr));
  QPointer<QAction> submitAction = actions->submitAction();
  QPointer<QAction> revertAction = actions->revertAction();
  RowState rs;
  /*
   * Initial state
   */
  QVERIFY(!submitAction->isEnabled());
  QVERIFY(!revertAction->isEnabled());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(actions.get());
  actions->setControllerStateMachine(stateMachine);
  /*
   * Check controller state
   */
  // Set a valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  // Visualizing state
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  QVERIFY(!submitAction->isEnabled());
  QVERIFY(!revertAction->isEnabled());
  // Editing state
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(submitAction->isEnabled());
  QVERIFY(revertAction->isEnabled());
  /*
   * Check setting row sate in Editing state
   */
  stateMachine->forceCurrentState(ControllerState::Editing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(submitAction->isEnabled());
  QVERIFY(revertAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  actions->setRowState(rs);
  QVERIFY(!submitAction->isEnabled());
  QVERIFY(!revertAction->isEnabled());
  /*
   * Check setting row sate in Visualizing state
   */
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  actions->setRowState(rs);
  QVERIFY(!submitAction->isEnabled());
  QVERIFY(!revertAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  actions->setRowState(rs);
  QVERIFY(!submitAction->isEnabled());
  QVERIFY(!revertAction->isEnabled());
  /*
   * Clear
   */
  actions.reset();
  QVERIFY(submitAction.isNull());
  QVERIFY(revertAction.isNull());
}

void ActionsTest::insertActionTest()
{
  std::unique_ptr<InsertAction> action(new InsertAction(nullptr));
  QPointer<QAction> insertAction = action->insertAction();
  /*
   * Initial state
   */
  /// \todo Define a initial state (also in controller) and enable
//   QVERIFY(!insertAction->isEnabled());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(action.get());
  action->setControllerStateMachine(stateMachine);
  /*
   * Change controller state
   */
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  QVERIFY(insertAction->isEnabled());
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(!insertAction->isEnabled());
  /*
   * Clear
   */
  action.reset();
  QVERIFY(insertAction.isNull());

  QFAIL("Initial state not clear");
}

void ActionsTest::removeActionTest()
{
  RowState rs;
  std::unique_ptr<RemoveAction> action(new RemoveAction(nullptr));
  QPointer<QAction> removeAction = action->removeAction();
  /*
   * Initial state
   */
  QVERIFY(!removeAction->isEnabled());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(action.get());
  action->setControllerStateMachine(stateMachine);
  /*
   * Check controller state
   */
  // Set a valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  // Visualizing state
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  QVERIFY(removeAction->isEnabled());
  // Editing state
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(!removeAction->isEnabled());
  /*
   * Check setting row sate in Editing state
   */
  stateMachine->forceCurrentState(ControllerState::Editing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  QVERIFY(!removeAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  action->setRowState(rs);
  QVERIFY(!removeAction->isEnabled());
  /*
   * Check setting row sate in Visualizing state
   */
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  QVERIFY(removeAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  action->setRowState(rs);
  QVERIFY(!removeAction->isEnabled());
  /*
   * Clear
   */
  action.reset();
  QVERIFY(removeAction.isNull());
}

void ActionsTest::resizeToContentsTest()
{
  RowState rs;
  std::unique_ptr<ResizeToContentsAction> action(new ResizeToContentsAction(nullptr));
  QPointer<QAction> resizeToContentsAction = action->resizeToContentsAction();
  /*
   * Initial state
   */
  QVERIFY(resizeToContentsAction != nullptr);
  QVERIFY(!resizeToContentsAction->isEnabled());
  /*
   * Set controller state machine
   */
  auto *stateMachine = ControllerStateMachine::makeNew<StateTableTestClass, AbstractControllerStatePermission>(action.get());
  action->setControllerStateMachine(stateMachine);
  /*
   * Check controller state
   */
  // Set a valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  // Visualizing state
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  QVERIFY(resizeToContentsAction->isEnabled());
  // Editing state
  stateMachine->forceCurrentState(ControllerState::Editing);
  QVERIFY(!resizeToContentsAction->isEnabled());
  /*
   * Check setting row sate in Editing state
   */
  stateMachine->forceCurrentState(ControllerState::Editing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  QVERIFY(!resizeToContentsAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  action->setRowState(rs);
  QVERIFY(!resizeToContentsAction->isEnabled());
  /*
   * Check setting row sate in Visualizing state
   */
  stateMachine->forceCurrentState(ControllerState::Visualizing);
  // Set valid row state
  rs.setRowCount(2);
  rs.setCurrentRow(0);
  action->setRowState(rs);
  QVERIFY(resizeToContentsAction->isEnabled());
  // Null row state
  rs.setRowCount(0);
  rs.setCurrentRow(-1);
  action->setRowState(rs);
  QVERIFY(!resizeToContentsAction->isEnabled());
  /*
   * Clear
   */
  action.reset();
  QVERIFY(resizeToContentsAction.isNull());
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  ActionsTest test;

  if(!app.init()){
    return 1;
  }
//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
