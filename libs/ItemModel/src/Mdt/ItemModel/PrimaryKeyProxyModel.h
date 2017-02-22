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
#ifndef MDT_ITEM_MODEL_PRIMARY_KEY_PROXY_MODEL_H
#define MDT_ITEM_MODEL_PRIMARY_KEY_PROXY_MODEL_H

#include "PrimaryKey.h"
#include "PkFkProxyModelBase.h"
#include <initializer_list>

namespace Mdt{ namespace ItemModel{

  /*! \brief Hold informations about a primary key of a item model
   *
   * Example of usage:
   * \code
   * QTableView view;
   * auto *model = new MyModel(&view);
   * auto *proxyModel = new PrimaryKeyProxyModel(&view);
   *
   * proxyModel->setSourceModel(model);
   * proxyModel->setPrimaryKey({1,2});
   * proxyModel->setPrimaryKeyEditable(false);
   *
   * view.setModel(proxyModel);
   * \endcode
   */
  class PrimaryKeyProxyModel : public PkFkProxyModelBase
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    explicit PrimaryKeyProxyModel(QObject *parent = nullptr);

    /*! \brief Set primary key
     *
     * \pre \a pk must not be null
     */
    void setPrimaryKey(const PrimaryKey & pk);

    /*! \brief Set primary key
     *
     * \pre Each column in \a pk must be >= 0
     * \pre Each column in \a pk must be unique
     */
    void setPrimaryKey(std::initializer_list<int> pk);

    /*! \brief Set primary key editable
     *
     * By default, primary key is editable
     */
    void setPrimaryKeyEditable(bool editable);

    /*! \brief Check if primary is editable
     */
    bool isPrimaryKeyEditable() const
    {
      return isKeyEditable();
    }

    /*! \brief Set primary key items enabled
     *
     * By default, primary key items are enabled.
     */
    void setPrimaryKeyItemsEnabled(bool enable);

    /*! \brief Check if primary key items are enabled
     */
    bool isPrimaryKeyItemsEnabled() const
    {
      return isKeyItemsEnabled();
    }
  };

}} // namespace Mdt{ namespace ItemModel{

#endif // #ifndef MDT_ITEM_MODEL_PRIMARY_KEY_PROXY_MODEL_H
