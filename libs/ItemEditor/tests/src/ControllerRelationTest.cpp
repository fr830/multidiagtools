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
#include "ControllerRelationTest.h"
#include "AbstractControllerRelationTestClass.h"
#include "ItemModelControllerTester.h"
#include "Mdt/ItemModel/VariantTableModel.h"
#include "Mdt/ItemModel/RelationFilterProxyModel.h"
#include "Mdt/ItemModel/RelationKey.h"
#include "Mdt/ItemModel/PrimaryKey.h"
#include "Mdt/ItemModel/ForeignKey.h"
#include "Mdt/ItemModel/FormatProxyModel.h"
#include "Mdt/ItemEditor/TableViewController.h"
#include "Mdt/ItemEditor/WidgetMapperController.h"
#include "Mdt/ItemEditor/ControllerRelationList.h"
#include "Mdt/ItemEditor/ControllerStateMachine.h"
#include "Mdt/ItemEditor/ControllerEvent.h"
#include <QSignalSpy>
#include <QStringListModel>
#include <QTableView>
#include <QSpinBox>
#include <QComboBox>
#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaMethod>

using namespace Mdt::ItemModel;
using namespace Mdt::ItemEditor;

void ControllerRelationTest::initTestCase()
{
}

void ControllerRelationTest::cleanupTestCase()
{
}

/*
 * Tests
 */

void ControllerRelationTest::setControllersTest()
{
  /*
   * Initial state
   */
  AbstractControllerRelationTestClass relation;
  QVERIFY(relation.storedOldParentController() == nullptr);
  QVERIFY(relation.storedParentController() == nullptr);
  QVERIFY(relation.storedOldChildController() == nullptr);
  QVERIFY(relation.storedChildController() == nullptr);
  /*
   * Set parent controller
   */
  ItemModelControllerTester parentController;
  relation.setParentController(&parentController);
  QVERIFY(relation.storedOldParentController() == nullptr);
  QCOMPARE(relation.storedParentController(), &parentController);
  QVERIFY(relation.storedOldChildController() == nullptr);
  QVERIFY(relation.storedChildController() == nullptr);
  /*
   * Set child controller
   */
  ItemModelControllerTester childController;
  relation.setChildController(&childController);
  QVERIFY(relation.storedOldParentController() == nullptr);
  QCOMPARE(relation.storedParentController(), &parentController);
  QVERIFY(relation.storedOldChildController() == nullptr);
  QCOMPARE(relation.storedChildController(), &childController);
  /*
   * Change parent controller
   */
  ItemModelControllerTester parentController2;
  relation.setParentController(&parentController2);
  QCOMPARE(relation.storedOldParentController(), &parentController);
  QCOMPARE(relation.storedParentController(), &parentController2);
  QVERIFY(relation.storedOldChildController() == nullptr);
  QCOMPARE(relation.storedChildController(), &childController);
  /*
   * Change child controller
   */
  ItemModelControllerTester childController2;
  relation.setChildController(&childController2);
  QCOMPARE(relation.storedOldParentController(), &parentController);
  QCOMPARE(relation.storedParentController(), &parentController2);
  QCOMPARE(relation.storedOldChildController(), &childController);
  QCOMPARE(relation.storedChildController(), &childController2);
}

void ControllerRelationTest::setModelToControllersFirstTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  parentModel.resize(0, 1);
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  QCOMPARE(parentController.sourceModel(), &parentModel);
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(0, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  QCOMPARE(childController.sourceModel(), &childModel);
  QCOMPARE(childController.modelForView(), &childModel);
  /*
   * Set parent controller
   */
  AbstractControllerRelationTestClass relation;
  relation.setParentController(&parentController);
  QCOMPARE(relation.parentController(), &parentController);
  QVERIFY(relation.childController() == nullptr);
  QCOMPARE(relation.parentControllerModel(), &parentModel);
  QVERIFY(relation.childControllerModel() == nullptr);
  // Check that parent model was signaled to subclass
  QCOMPARE(relation.parentControllerStoredModel(), &parentModel);
  QVERIFY(relation.childControllerStoredModel() == nullptr);
  // Check that relation did no mismatch with controllers
  QCOMPARE(parentController.sourceModel(), &parentModel);
  QCOMPARE(parentController.modelForView(), &parentModel);
  /*
   * Set child controller
   */
  relation.setChildController(&childController);
  QCOMPARE(relation.parentController(), &parentController);
  QCOMPARE(relation.childController(), &childController);
  QCOMPARE(relation.parentControllerModel(), &parentModel);
  QCOMPARE(relation.childControllerModel(), &childModel);
  // Check that child model was signaled to subclass
  QCOMPARE(relation.parentControllerStoredModel(), &parentModel);
  QCOMPARE(relation.childControllerStoredModel(), &childModel);
  // Check that relation did no mismatch with controllers
  QCOMPARE(parentController.sourceModel(), &parentModel);
  QCOMPARE(parentController.modelForView(), &parentModel);
  QCOMPARE(childController.sourceModel(), &childModel);
  QCOMPARE(childController.modelForView(), &childModel);
}

void ControllerRelationTest::setModelToControllersAfterTest()
{
  /*
   * Setup parent and child controllers
   */
  ItemModelControllerTester parentController;
  parentController.setObjectName("parentController");
  ItemModelControllerTester childController;
  childController.setObjectName("childController");
  /*
   * Set parent controller
   */
  AbstractControllerRelationTestClass relation;
  relation.setParentController(&parentController);
  QCOMPARE(relation.parentController(), &parentController);
  QVERIFY(relation.childController() == nullptr);
  QVERIFY(relation.parentControllerModel() == nullptr);
  QVERIFY(relation.childControllerModel() == nullptr);
  // Subclass has not yet recieved any model changed signal
  QVERIFY(relation.parentControllerStoredModel() == nullptr);
  QVERIFY(relation.childControllerStoredModel() == nullptr);
  // Check that relation did no mismatch with controllers
  QVERIFY(parentController.sourceModel() == nullptr);
  QVERIFY(parentController.modelForView() == nullptr);
  /*
   * Set child controller
   */
  relation.setChildController(&childController);
  QCOMPARE(relation.parentController(), &parentController);
  QCOMPARE(relation.childController(), &childController);
  QVERIFY(relation.parentControllerModel() == nullptr);
  QVERIFY(relation.childControllerModel() == nullptr);
  // Subclass has not yet recieved any model changed signal
  QVERIFY(relation.parentControllerStoredModel() == nullptr);
  QVERIFY(relation.childControllerStoredModel() == nullptr);
  // Check that relation did no mismatch with controllers
  QVERIFY(parentController.sourceModel() == nullptr);
  QVERIFY(parentController.modelForView() == nullptr);
  QVERIFY(childController.sourceModel() == nullptr);
  QVERIFY(childController.modelForView() == nullptr);
  /*
   * Setup parent model
   */
  VariantTableModel parentModel;
  parentModel.resize(0, 1);
  parentController.setModel(&parentModel);
  QCOMPARE(relation.parentControllerModel(), &parentModel);
  QVERIFY(relation.childControllerModel() == nullptr);
  // Check that parent model was signaled to subclass
  QCOMPARE(relation.parentControllerStoredModel(), &parentModel);
  QVERIFY(relation.childControllerStoredModel() == nullptr);
  // Check that relation did no mismatch with controllers
  QCOMPARE(parentController.sourceModel(), &parentModel);
  QCOMPARE(parentController.modelForView(), &parentModel);
  /*
   * Setup child model
   */
  VariantTableModel childModel;
  childModel.resize(0, 1);
  childController.setModel(&childModel);
  QCOMPARE(relation.parentControllerModel(), &parentModel);
  QCOMPARE(relation.childControllerModel(), &childModel);
  // Check that child model was signaled to subclass
  QCOMPARE(relation.parentControllerStoredModel(), &parentModel);
  QCOMPARE(relation.childControllerStoredModel(), &childModel);
  // Check that relation did no mismatch with controllers
  QCOMPARE(parentController.sourceModel(), &parentModel);
  QCOMPARE(parentController.modelForView(), &parentModel);
  QCOMPARE(childController.sourceModel(), &childModel);
  QCOMPARE(childController.modelForView(), &childModel);
}

void ControllerRelationTest::controllerTesterStateTableTest()
{
  /*
   * Setup model and controller
   */
  VariantTableModel model;
  model.resize(1, 1);
  ItemModelControllerTester controller;
  controller.setModel(&model);
  auto *stateMachine = controller.controllerStateMachine();
  QVERIFY(stateMachine != nullptr);

  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  // Visualizing - Editing
  stateMachine->dataEditionStarted();
  QCOMPARE(stateMachine->currentState(), ControllerState::Editing);
  stateMachine->submitDone();
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  stateMachine->dataEditionStarted();
  QCOMPARE(stateMachine->currentState(), ControllerState::Editing);
  stateMachine->revertDone();
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  // Visualizing - Inserting
  stateMachine->insertStarted();
  QCOMPARE(stateMachine->currentState(), ControllerState::Inserting);
  stateMachine->dataEditionStarted();
  QCOMPARE(stateMachine->currentState(), ControllerState::Inserting);
  stateMachine->dataEditionDone();
  QCOMPARE(stateMachine->currentState(), ControllerState::Inserting);
  stateMachine->submitDone();
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  stateMachine->insertStarted();
  QCOMPARE(stateMachine->currentState(), ControllerState::Inserting);
  stateMachine->revertDone();
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  // Visualizing - ParentEditing
  stateMachine->setEvent(ControllerEvent::EditionStartedFromParent);
  QCOMPARE(stateMachine->currentState(), ControllerState::ParentEditing);
  stateMachine->setEvent(ControllerEvent::EditionDoneFromParent);
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
  // Visualizing - ChildEditing
  stateMachine->setEvent(ControllerEvent::EditionStartedFromChild);
  QCOMPARE(stateMachine->currentState(), ControllerState::ChildEditing);
  stateMachine->setEvent(ControllerEvent::EditionDoneFromChild);
  QCOMPARE(stateMachine->currentState(), ControllerState::Visualizing);
}

void ControllerRelationTest::editSubmitRevertParentChildStateTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  parentModel.resize(1, 1);
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  parentController.setPrimaryKey({0});
  QVERIFY(parentController.controllerStateMachine() != nullptr);
  QSignalSpy parentStateChangedSpy(parentController.controllerStateMachine(), &ControllerStateMachine::currentStateChanged);
  QVERIFY(parentStateChangedSpy.isValid());
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(1, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  childController.setForeignKey({0});
  QVERIFY(childController.controllerStateMachine() != nullptr);
  QSignalSpy childStateChangedSpy(childController.controllerStateMachine(), &ControllerStateMachine::currentStateChanged);
  QVERIFY(childStateChangedSpy.isValid());
  /*
   * Setup relation
   *
   * parentController
   *       |
   *       ---> childController
   */
  parentController.addChildController(&childController);
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  parentStateChangedSpy.clear();
  childStateChangedSpy.clear();
  /*
   * Beginn editing from parent controller
   */
  // begin editing
  parentController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::Editing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  QCOMPARE(parentStateChangedSpy.count(), 1);
  QCOMPARE(childStateChangedSpy.count(), 1);
  // Submit
  QVERIFY(childController.submit());
  QVERIFY(parentController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(parentStateChangedSpy.count(), 2);
  QCOMPARE(childStateChangedSpy.count(), 2);
  // begin editing
  parentController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::Editing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  QCOMPARE(parentStateChangedSpy.count(), 3);
  QCOMPARE(childStateChangedSpy.count(), 3);
  // Revert
  parentController.revert();
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(parentStateChangedSpy.count(), 4);
  QCOMPARE(childStateChangedSpy.count(), 4);
  parentStateChangedSpy.clear();
  childStateChangedSpy.clear();
  /*
   * Beginn editing from child controller
   */
  // Begin editing
  childController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(childController.controllerState(), ControllerState::Editing);
  QCOMPARE(parentStateChangedSpy.count(), 1);
  QCOMPARE(childStateChangedSpy.count(), 1);
  // Submit
  QVERIFY(childController.submit());
  QVERIFY(parentController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(parentStateChangedSpy.count(), 2);
  QCOMPARE(childStateChangedSpy.count(), 2);
  // Begin editing
  childController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(childController.controllerState(), ControllerState::Editing);
  QCOMPARE(parentStateChangedSpy.count(), 3);
  QCOMPARE(childStateChangedSpy.count(), 3);
  // Revert
  childController.revert();
  parentController.revert();
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(parentStateChangedSpy.count(), 4);
  QCOMPARE(childStateChangedSpy.count(), 4);
}

void ControllerRelationTest::editSubmitRevertTopMiddleChildStateTest()
{
  /*
   * Setup top parent model and controller
   */
  VariantTableModel topParentModel;
  topParentModel.resize(1, 1);
  ItemModelControllerTester topParentController;
  topParentController.setModel(&topParentModel);
  topParentController.setPrimaryKey({0});
  /*
   * Setup middle model and controller
   */
  VariantTableModel middleModel;
  middleModel.resize(1, 1);
  ItemModelControllerTester middleController;
  middleController.setModel(&middleModel);
  middleController.setPrimaryKey({0});
  middleController.setForeignKey({0});
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(1, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  childController.setForeignKey({0});
  /*
   * Setup relations
   *
   * topParentController
   *       |
   *       ---> middleController
   *                 |
   *                 ---> childController
   */
  topParentController.addChildController(&middleController);
  middleController.addChildController(&childController);
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Beginn editing from top parent controller
   */
  // Begin editing
  topParentController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::Editing);
  QCOMPARE(middleController.controllerState(), ControllerState::ParentEditing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  // Submit
  QVERIFY(topParentController.submit());
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  // Begin editing
  topParentController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::Editing);
  QCOMPARE(middleController.controllerState(), ControllerState::ParentEditing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  // Revert
  topParentController.revert();
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Beginn editing from middle controller
   */
  // Begin editing
  middleController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(middleController.controllerState(), ControllerState::Editing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  // Submit from middle controller
  QVERIFY(middleController.submit());
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  // Begin editing
  middleController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(middleController.controllerState(), ControllerState::Editing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  // Revert from middle controller
  middleController.revert();
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Beginn editing child controller
   */
  // Begin editing
  childController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(middleController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(childController.controllerState(), ControllerState::Editing);
  // Submit from child controller
  QVERIFY(childController.submit());
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  // Begin editing
  childController.startEditing();
  QCOMPARE(topParentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(middleController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(childController.controllerState(), ControllerState::Editing);
  // Revert from child controller
  childController.revert();
  QCOMPARE(topParentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(middleController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
}


void ControllerRelationTest::changeParentControllerStateTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  parentModel.resize(1, 1);
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(1, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  /*
   * Initial states
   */
  AbstractControllerRelationTestClass relation;
  relation.setParentController(&parentController);
  relation.setChildController(&childController);
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Start editing from parent controller
   */
  parentController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::Editing);
  QCOMPARE(childController.controllerState(), ControllerState::ParentEditing);
  /*
   * Stop editing
   */
  QVERIFY(parentController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);

  QFAIL("Not complete");
}

void ControllerRelationTest::changeChildControllerStateTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  parentModel.resize(1, 1);
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(1, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  /*
   * Setup relation
   */
  AbstractControllerRelationTestClass relation;
  relation.setParentController(&parentController);
  relation.setChildController(&childController);
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Start editing from child controller
   */
  childController.startEditing();
  QCOMPARE(parentController.controllerState(), ControllerState::ChildEditing);
  QCOMPARE(childController.controllerState(), ControllerState::Editing);
  /*
   * Stop editing
   */
  QVERIFY(childController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);

  QFAIL("Not complete");
}

void ControllerRelationTest::submitRevertStateTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  parentModel.resize(1, 1);
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  childModel.resize(1, 1);
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  /*
   * Setup relation
   */
  AbstractControllerRelationTestClass relation;
  relation.setParentController(&parentController);
  relation.setChildController(&childController);
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit then submit in parent controller
   */
  parentController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  QVERIFY(parentController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit then revert in parent controller
   */
  parentController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  parentController.revert();
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit then submit in child controller
   */
  childController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit then revert in child controller
   */
  childController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  childController.revert();
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit in child controller and submit from partent controller
   */
  childController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  QVERIFY(parentController.submit());
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);
  /*
   * Edit in child controller and revert from partent controller
   */
  childController.startEditing();
  QVERIFY(parentController.controllerState() != ControllerState::Visualizing);
  QVERIFY(childController.controllerState() != ControllerState::Visualizing);
  parentController.revert();
  QCOMPARE(parentController.controllerState(), ControllerState::Visualizing);
  QCOMPARE(childController.controllerState(), ControllerState::Visualizing);

  QFAIL("Not complete");
}

void ControllerRelationTest::submitRevertDataTest()
{
//   /*
//    * Setup parent model and controller
//    */
//   VariantTableModel parentModel;
//   parentModel.resize(1, 1);
//   ItemModelControllerTester parentController;
//   parentController.setModel(&parentModel);
//   /*
//    * Setup child model and controller
//    */
//   VariantTableModel childModel;
//   childModel.resize(1, 1);
//   ItemModelControllerTester childController;
//   childController.setModel(&childModel);
//   /*
//    * Setup relation
//    */
//   AbstractControllerRelationTestClass relation;
//   relation.setParentController(&parentController);
//   relation.setChildController(&childController);
//   /*
//    * Edit in child controller and submit from partent controller
//    */
//   childController.startEditing();
//   childController.clearDataSubmitToModelCount();
//   QVERIFY(parentController.submit());
//   QCOMPARE(childController.dataSubmitToModelCount(), 1);
//   /*
//    * Edit in child controller and revert from partent controller
//    */
//   childController.startEditing();
//   childController.clearDataRevertFromModelCount();
//   parentController.revert();
//   QCOMPARE(childController.dataRevertFromModelCount(), 1);
//   /*
//    * Edit then revert in parent controller
//    */
// 
//   /*
//    * Edit then submit in child controller
//    */
// 
//   /*
//    * Edit then revert in child controller
//    */

  QFAIL("Not complete");
}

void ControllerRelationTest::controllerAddChildControllerTest()
{
  /*
   * Setup parent model and controller
   */
  VariantTableModel parentModel;
  ItemModelControllerTester parentController;
  parentController.setModel(&parentModel);
  /*
   * Setup child model and controller
   */
  VariantTableModel childModel;
  ItemModelControllerTester childController;
  childController.setModel(&childModel);
  /*
   * Add child controller
   */
  ParentModelColumn pmc(0);
  FilterColumn fc(0);
  parentController.addChildController(&childController, fc == pmc);
//   QVERIFY(childController.isRelationFilterEnabled());
//   QCOMPARE(childController.relationFilterModel()->parentModel(), &parentModel);
//   QCOMPARE(childController.relationFilterModel()->sourceModel(), &childModel);

  QFAIL("Not complete (missing check of correct model mappings)");
}

void ControllerRelationTest::controllerRelationFilterTest()
{

  QFAIL("Not complete");
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  ControllerRelationTest test;

  if(!app.init()){
    return 1;
  }
//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
