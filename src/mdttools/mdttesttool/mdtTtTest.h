/****************************************************************************
 **
 ** Copyright (C) 2011-2014 Philippe Steinmann.
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
#ifndef MDT_TT_TEST_H
#define MDT_TT_TEST_H

#include "mdtTtBase.h"
#include "mdtTtTestData.h"
#include "mdtValue.h"
#include "mdtTtTestNodeUnitSetupData.h"
#include "mdtTtTestItemNodeSetupData.h"
#include <QSqlDatabase>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSqlRecord>
#include <memory>

class QSqlTableModel;

/*! \brief Helper class to manipulate test data
 *
 * 
 */
class mdtTtTest : public mdtTtBase
{
 public:

  /*! \brief Constructor
   */
  mdtTtTest(QObject *parent, QSqlDatabase db);

  /*! \brief Setup test item model
   *
   * This is only needed to run test items.
   */
  bool setupTestItemModel();

  /*! \brief Get test data for given test ID
   */
  mdtTtTestData getTestData(const QVariant & testId, bool includeModelData, bool *ok);

  /*! \brief Add a test
   *
   * \param data Test data. New test will be based on TestModel_Id_FK.
   *              A new Test item will be created for each item
   *              contained in TestModelItem_tbl that is part of given TestModel_Id_FK.
   *
   * \return Test ID from freshly created test, or a Null value on error
   */
  QVariant addTest(const mdtTtTestData & data);

  /*! \brief Update data for given test data
   *
   * Note: in data, if TestModel_Id_FK is different from
   *        those that is currently stored,
   *        related test items will be removed, and created again, based on given test model.
   */
  bool updateTest(const QVariant & testId, const mdtTtTestData & data);

  /*! \brief Check if more test item is available
   */
  bool hasMoreTestItem() const;

  /*! \brief Reset the test item cursor
   */
  void resetTestItemCursor();

  /*! \brief Go to next test item
   *
   * \return The test item ID
   */
  QVariant nextTestItem();

  /*! \brief Get setup data for given test item ID
   */
  mdtTtTestItemNodeSetupData getSetupData(const QVariant & testItemId, bool & ok);

  /*! \brief Convert a mdtValue to a double
   *
   * If value has -OL flag set, -1e300 is returned.
   * If value has +OL flag set, 1e300 is returned.
   */
  static QVariant valueToDouble(const mdtValue & value);

  /*! \brief Convert a double to a value
   *
   * If dblVal is <= -1e300, -OL flag will be set.
   * If dblVal is >= 1e300, +OL flag will be set.
   */
  static mdtValue doubleToValue(const QVariant & dblVal);

  /*! \brief Set test item model
   *
   * If a model that contains test item data
   *  is allready used, for example in the GUI,
   *  it's possible to re-use it here.
   *  When do so, test results (measured values, states, ...)
   *  will allways be reflected to GUI.
   *
   * Other goal to use a model is to get/set data
   *  in a efficient way, without instanciating QSqlQuery
   *  objects for each, possibly single data, query.
   *
   * The given model will not be deleted by this class .
   *
   * \deprecated
   */
  ///bool setTestItemSqlModel(QSqlTableModel *model);

  /*! \brief Get a list of test item IDs for given test ID
   *
   * Items are sorted by sequence number, ascending
   */
  QList<QVariant> getTestItemIdListForTestId(const QVariant & testId);

  /*! \brief Get list of hardware node ID used by given test ID
   */
  QList<QVariant> getHardwareNodeIdListForTestId(const QVariant & testId);

  /*! \brief Get list of hardware node ID used by given test item ID
   */
  QList<QVariant> getHardwareNodeIdListForTestItemId(const QVariant & testItemId);

  /*! \brief Get test node unit setups for a given test item ID and hardwareNodeId
   *
   * \param testItemId Primary key of TestItem_tbl
   * \param hardwareNodeId NodeId from TestNode_tbl
   *
   * \deprecated
   */
  ///QList<mdtTtTestNodeUnitSetupData> getNodeUnitSetupList(const QVariant & testItemId, const QVariant & hardwareNodeId);
  QList<QSqlRecord> getNodeUnitSetupList(const QVariant & testItemId, const QVariant & hardwareNodeId);

  /*! \brief Set test model
   *
   * Will also add a test item for each test item .
   */
  bool setTestModel(const QVariant & testResultId, const QVariant & testId);

  /*! \brief Add a test item
   */
  bool addItem(const QVariant & testResultId, const QVariant & testItemId);

  /*! \brief Add a test items based on given test ID
   *
   * \deprecated Use addTestItems()
   */
  bool addItemsByTestId(const QVariant & testResultId, const QVariant & testId);

  /*! \brief Edit a test item
   */
  bool editItem(const QVariant & testItemId, const QString & fieldName, const QVariant & data);

  /*! \brief Set measured value
   *
   * Note: value will not be stored to database immediatly,
   *  call submitTestItemSqlModelData() to do it .
   */
  bool setMeasuredValue(const QVariant & testItemId, const mdtValue & value);

  /*! \brief Submit test items SQL model data to database
   *
   * If a error occurs, it will be available with lastError() .
   */
  bool submitTestItemSqlModelData();

 ///private slots:

  /*! \brief Set sql model pointers tu Null whenn they are destroyed
   */
  ///void onSqlModelDestroyed(QObject *obj);

 private:

  /*! \brief Add test items for given test ID and test model ID
   */
  bool addTestItems(const QVariant & testId, const QVariant & testModelId);

  /*! \brief Remove test items from given test ID
   */
  bool removeTestItems(const QVariant & testId);

  /*! \brief Apply filter of pvTestItemTableModel to current test ID and reset pvCurrentTestItemRow to first item
   */
  void resetTestItemTableModel(const QVariant & testId);

  
  
  /*! \brief Check if test item sql model was set
   *
   * Will store a error if not ok .
   */
  bool testItemSqlModelOk();

  /*! \brief Get index for given testItemId and column
   *
   * Will store a error if a invalid index is returned
   */
  QModelIndex indexOfTestItem(const QVariant & testItemId, int column);

  /*! \brief Helper method to store data into test item SQL model
   *
   * If a error occurs, it will be stored .
   */
  bool setTestItemSqlModelData(const QModelIndex & index, const QVariant & data);

  /*! \brief Helper method to get data from test item SQL model
   *
   * If a error occurs, it will be stored .
   */
  QVariant getTestItemSqlModelData(const QModelIndex & index);

  /*! \brief Helper method to get data from test item SQL model
   *
   * \overload getTestItemSqlModelData(const QModelIndex &)
   */
  QVariant getTestItemSqlModelData(int row, int column);

  Q_DISABLE_COPY(mdtTtTest);

  // Test item data models
  std::shared_ptr<QSqlTableModel> pvTestItemTableModel;
  int pvCurrentTestItemRow;

  /// \todo Obselete..
  int pvColIdxOfTestItemId;
  int pvColIdxOfExpectedValue;
  int pvColIdxOfMeasuredValue;
  int pvColIdxOfResult;
};

#endif // #ifndef MDT_TT_TEST_H
