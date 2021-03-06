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
#ifndef MDT_ITEM_MODEL_RELATION_FILTER_PROXY_MODEL_H
#define MDT_ITEM_MODEL_RELATION_FILTER_PROXY_MODEL_H

#include "SortFilterProxyModel.h"
#include "RelationFilterExpression.h"
#include "ParentModelColumn.h"
#include "ChildModelColumn.h"
#include "LikeExpression.h"
#include "RowList.h"
#include "RelationKey.h"
#include "PrimaryKey.h"
#include "ForeignKey.h"
#include "MdtItemModelExport.h"
#include <QPointer>
#include <QModelIndex>
#include <QVector>
#include <memory>

namespace Mdt{ namespace ItemModel{

  class RelationKeyCopier;

  /*! \brief Provides support for filtering data between another model and a view
   *
   * Example of usage:
   * \code
   * #include <Mdt/ItemModel/RelationFilterProxyModel.h>
   * #include <QTableView>
   * #include "ClientTableModel.h"
   * #include "AddressTableModel.h"
   *
   * using namepsace Mdt::ItemModel;
   *
   * auto *view = new QTableView;
   * auto *clientModel = new ClientTableModel;
   * auto *addressModel = new AddressTableModel(view);
   * auto *proxyModel = new RelationFilterProxyModel(view);
   *
   * // Setup view acting on addressModel
   * proxyModel->setParentModel(clientModel);
   * proxyModel->setSourceModel(addressModel);
   * view->setModel(proxyModel);
   *
   * // Setup a filter and apply it
   * ChildModelColumn addressClientId(1);
   * ParentModelColumn clientId(0);
   * proxyModel->setFilter( addressClientId == cliendId );
   * proxyModel->setParentModelMatchRow(0);
   * \endcode
   */
  class MDT_ITEMMODEL_EXPORT RelationFilterProxyModel : public SortFilterProxyModel
  {
   Q_OBJECT

   public:

    /*! \brief Construct a model
     */
    explicit RelationFilterProxyModel(QObject* parent = nullptr);

    // Destructor - unique_ptr needs to know how to destruct RelationKeyCopier
    ~RelationFilterProxyModel();
    // Disable copy
    RelationFilterProxyModel(const RelationFilterProxyModel &) = delete;
    RelationFilterProxyModel & operator=(const RelationFilterProxyModel &) = delete;
    // Disable move
    RelationFilterProxyModel(RelationFilterProxyModel &&) = delete;
    RelationFilterProxyModel & operator=(RelationFilterProxyModel &&) = delete;

    /*! \brief Set parent model
     *
     * \pre \a model must be a valid pointer
     */
    void setParentModel(QAbstractItemModel *model);

    /*! \brief Get parent model
     */
    QAbstractItemModel *parentModel() const
    {
      return mParentModel;
    }

    /*! \brief Set relation filter
     *
     * \param expression Expression to apply as filter.
     * \pre \a expression must not be null
     *
     * \sa setParentModelMatchRow()
     */
    void setFilter(const RelationFilterExpression & expression);

    /*! \brief Set relation filter
     *
     * For each pairs of parent/child model column in \a relationKey,
     *  a equality criteria will be created.
     *  Each pairs are also chained with a AND constraint.
     *
     * \pre \a relationKey must not be null
     * \pre \a relationKey must contain max 4 columns pairs
     */
    void setFilter(const RelationKey & relationKey);

    /*! \brief Set relation filter
     *
     * This is the same as:
     * \code
     * RelationFilterProxyModel proxyModel;
     * RelationKey key;
     * PrimaryKey pk({1,2});
     * ForeignKey fk({2,3});
     * key.setKey(pk, fk);
     * proxyModel.setFilter(key);
     * \endcode
     *
     * \pre \a parentModelPk must not be null
     * \pre \a childModelFk must not be null
     * \pre Both \a parentModelPk and \a childModelFk must have the same count of columns and max 4 columns
     */
    void setFilter(const PrimaryKey & parentModelPk, const ForeignKey & childModelFk);

    /*! \brief Get relation key for equality
     */
    RelationKey relationKeyForEquality() const;

    /*! \brief Get row of parent model for which filter must match
     */
    int parentModelMatchRow() const
    {
      return mParentModelRow;
    }

    /*! \brief Re-implemented from SortFilterProxyModel
     */
    void setDynamicSortFilter(bool enable);

    /*! \brief Get a list of rows in source model that are currently avaiable in this proxy model
     */
    RowList getCurrentSourceModelRowList() const;

    /*! \brief Reimplemented from SortFilterProxyModel
     */
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

   public slots:

    /*! \brief Set row of parent model for which filter must match
     *
     * If row is out of bound, filter will allways evaluate false,
     *  and this proxy model will present no data.
     *
     * \pre \a row must be >= -1
     * \sa setFilter()
     */
    void setParentModelMatchRow(int row);

   private slots:

    /*! \brief Actions to perform when source model changed
     */
    void onSourceModelChanged();

    /*! \brief Actions to perform when rows have been inserted into this proxy model
     */
    void onRowsInserted(const QModelIndex & parent, int first, int last);

    /*! \brief Actions to perform when parent model data changed
     */
    void onParentModelDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles = QVector<int>());

   private:

    /*! \brief Return true if filter expression was set and evaluates true
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    int mParentModelRow = -1;
    bool mInserting = false;
    QPointer<QAbstractItemModel> mParentModel;
    RelationFilterExpression mFilterExpression;
    std::unique_ptr<RelationKeyCopier> mKeyCopier;
    QMetaObject::Connection mRowsInsertedConnection;
    QMetaObject::Connection mParentModelDataChangedConnection;
  };

}} // namespace Mdt{ namespace ItemModel{

#endif // #ifndef MDT_ITEM_MODEL_RELATION_FILTER_PROXY_MODEL_H
