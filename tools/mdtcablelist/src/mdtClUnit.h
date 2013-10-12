/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
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
#ifndef MDT_CL_UNIT_H
#define MDT_CL_UNIT_H

#include "mdtClUnitConnectionData.h"
#include "mdtError.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QList>

/*! \brief Helper class to mana unit and related data
 */
class mdtClUnit
{
 public:

  /*! \brief Constructor
   */
  mdtClUnit(QSqlDatabase db);

  /*! \brief Destructor
   */
  ~mdtClUnit();

  /*! \brief Get last error occured
   */
  const QSqlError &lastError();

  /*! \brief Get a model with units for compononent selection
   *
   * Will setup a query model witch contains all units except unitId
   *  and units that allready are component of unitId .
   */
  QSqlQueryModel *unitModelForComponentSelection(const QVariant &unitId);

  /*! \brief Add a unit as component
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool addComponent(const QVariant &unitId, const QVariant &componentId);

  /*! \brief Edit a component
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool editComponent(const QVariant &unitId, const QVariant &currentComponentId, const QVariant &newComponentId);

  /*! \brief Remove a single component
   */
  bool removeComponent(const QVariant &unitId, const QVariant &componentId);

  /*! \brief Remove a list of components
   */
  bool removeComponents(const QVariant &unitId, const QList<QVariant> &componentIdList);

  /*! \brief Remove each link that is contained in selection
   *
   * This is usefull used together with mdtSqlTableWidget .
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool removeComponents(const QVariant &unitId, const QModelIndexList & indexListOfSelectedRows);

  /*! \brief Return the number unit connections that are related to article connections
   *
   * \return A number >= 0 on success, a value < 0 on error.
   *         On error, use lastError() to check what goes wrong.
   */
  int toUnitRelatedArticleConnectionCount(const QVariant & unitId);

  /*! \brief
   */
  QList<QVariant> toUnitRelatedArticleConnectionIds(const QVariant & unitId);

  /*! \brief
   */
  bool unitConnectionIsRelatedToArticleConnection(const QVariant & unitConnectionId);

  /*! \brief
   */
  QVariant toUnitConnectionRelatedArticleConnectionData(const QVariant & unitConnectionId, const QString & field);

  /*! \brief
   */
  QSqlQueryModel *toUnitConnectionRelatedRangesModel(const QVariant & unitConnectionId);

  /*! \brief
   */
  bool addUnitConnection(const mdtClUnitConnectionData & data);

  /*! \brief
   */
  bool removeUnitConnection(const QVariant & unitConnectionId);

  /*! \brief
   */
  bool editUnitConnection(const mdtClUnitConnectionData & data);

  /*! \brief
   */
  mdtClUnitConnectionData getUnitConnectionData(const QVariant & unitConnectionId);

  /*! \brief
   */
  bool addRange(const QVariant & baseUnitConnectionId, const mdtClUnitConnectionData & rangeData);

 private:

  Q_DISABLE_COPY(mdtClUnit);

  QSqlDatabase pvDatabase;
  QSqlQueryModel *pvToUnitConnectionRelatedRangesModel;
  QSqlQueryModel *pvUnitModel;
  QSqlError pvLastError;
};

#endif  // #ifndef MDT_CL_UNIT_H
