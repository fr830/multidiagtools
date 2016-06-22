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
#ifndef MDT_SQL_SCHEMA_FOREIGN_KEY_LIST_H
#define MDT_SQL_SCHEMA_FOREIGN_KEY_LIST_H

#include "ForeignKey.h"
#include <QVector>

namespace Mdt{ namespace Sql{ namespace Schema{

  /*! \brief List of ForeignKey
   */
  class ForeignKeyList
  {
   public:

    /*! \brief STL-style const iterator
     */
    typedef QVector<ForeignKey>::const_iterator const_iterator;

    /*! \brief Append a foreign key
     */
    void append(const ForeignKey & fk)
    {
      pvForeignKeyList.append(fk);
    }

    /*! \brief Update child table name for each foreign key in this list
     */
    void updateChildTableName(const QString & tableName)
    {
      for(auto & fk : pvForeignKeyList){
        fk.setChildTableName(tableName);
      }
    }

    /*! \brief Get count of elements
     */
    int size() const
    {
      return pvForeignKeyList.size();
    }

    /*! \brief Get foreign key at index
     *
     * \pre index must be valid
     */
    const ForeignKey & at(int index) const
    {
      Q_ASSERT(index >= 0);
      Q_ASSERT(index < size());

      return pvForeignKeyList.at(index);
    }

    /*! \brief Get foreign key that references table designed by tableName
     */
    ForeignKey foreignKeyReferencing(const QString & tableName) const
    {
      ForeignKey fk;
      for(const auto & _fk : pvForeignKeyList){
        if( QString::compare(_fk.parentTableName(), tableName, Qt::CaseInsensitive) == 0){
          fk = _fk;
          break;
        }
      }
      return fk;
    }

    /*! \brief Get begin const iterator
     */
    const_iterator begin() const
    {
      return pvForeignKeyList.cbegin();
    }

    /*! \brief Get end const iterator
     */
    const_iterator end() const
    {
      return pvForeignKeyList.cend();
    }

    /*! \brief Clear
     */
    void clear()
    {
      pvForeignKeyList.clear();
    }

   private:

    QVector<ForeignKey> pvForeignKeyList;
  };

}}} // namespace Mdt{ namespace Sql{ namespace Schema{

#endif // #ifndef MDT_SQL_SCHEMA_FOREIGN_KEY_LIST_H
