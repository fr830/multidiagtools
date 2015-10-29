/****************************************************************************
 **
 ** Copyright (C) 2011-2015 Philippe Steinmann.
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
#ifndef MDT_SQL_DATABASE_COPIER_MAPPING_H
#define MDT_SQL_DATABASE_COPIER_MAPPING_H

#include "mdtSqlDatabaseCopierTableMapping.h"
#include <QSqlDatabase>
#include <QVector>
#include <QStringList>

/*! \brief Store SQL database copy mapping
 */
class mdtSqlDatabaseCopierMapping
{
 public:

  /*! \brief Set source database
   *
   * Will also reset table mapping.
   *
   * \sa resetTableMapping()
   */
  bool setSourceDatabase(const QSqlDatabase & db);

  /*! \brief Get source database
   */
  QSqlDatabase sourceDatabase() const
  {
    return pvSourceDatabase;
  }

  /*! \brief Set destination database
   *
   * Will also reset table mapping.
   *
   * \sa resetTableMapping()
   */
  bool setDestinationDatabase(const QSqlDatabase & db);

  /*! \brief Get destination database
   */
  QSqlDatabase destinationDatabase() const
  {
    return pvDestinationDatabase;
  }

  /*! \brief Reset table mapping
   *
   * Will clear table mapping,
   *  then, generate it for each available table in source database.
   *  After that, no valid table mapping is set to destination database.
   */
  bool resetTableMapping();

  /*! \brief Clear table mapping
   */
  void clearTableMapping();

  /*! \brief Generate table mapping by name
   *
   * Will first reset table mapping.
   *  Then, for each table in source database,
   *  destination table is defined by source table name.
   *  For each mapped table, field mapping is alos generated by field names.
   *
   * \todo Checks + get mapping state of each item
   */
  bool generateTableMappingByName();

  /*! \brief Get count of table mapping
   */
  int tableMappingCount() const
  {
    return pvTableMappingList.size();
  }

  /*! \brief Get source table name for given mapping index
   *
   * \pre index must be in a valid range
   */
  QString sourceTableName(int index) const
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < pvTableMappingList.size());
    return pvTableMappingList.at(index).sourceTableName();
  }

  /*! \brief Get destination table name for given mapping index
   *
   * \pre index must be in a valid range
   */
  QString destinationTableName(int index) const
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < pvTableMappingList.size());
    return pvTableMappingList.at(index).destinationTableName();
  }

  /*! \brief Get table mapping state for given mapping index
   *
   * \pre index must be in a valid range
   */
  mdtSqlDatabaseCopierTableMapping::MappingState tableMappingState(int index) const
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < pvTableMappingList.size());
    return pvTableMappingList.at(index).mappingState();
  }

  /*! \brief Get table mapping at given index
   *
   * \pre index must be in a valid range
   */
  mdtSqlDatabaseCopierTableMapping tableMapping(int index) const
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < pvTableMappingList.size());
    return pvTableMappingList.at(index);
  }

  /*! \brief Set table mapping for given index
   *
   * \pre index must be in a valid range
   */
  void setTableMapping(int index, const mdtSqlDatabaseCopierTableMapping & tm)
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < pvTableMappingList.size());
    pvTableMappingList[index] = tm;
  }

  /*! \brief Get the list of table mapping
   *
   * Returned list will only contain table mapping
   *  witch has the MappingComplete state.
   */
  QVector<mdtSqlDatabaseCopierTableMapping> getCompletedTableMappingList() const;

  /*! \brief Get last error
   */
  mdtError lastError() const
  {
    return pvLastError;
  }

 private:

  /*! \brief Get a list of tables for given database
   *
   * Will also remove some system tables from the list.
   *
   * Note: QSqlDatabase::tables(QSql::Tables) returns
   *       no system tables, but, with QSQLite,
   *       tables like sqlite_sequence are returned.
   *       This function will remove them for SQLite database.
   *
   */
  QStringList getTables(const QSqlDatabase & db);

  /*! \brief Get list of table (SQLite implementation)
   */
  QStringList getTablesSqlite(const QSqlDatabase & db);

  QSqlDatabase pvSourceDatabase;
  QSqlDatabase pvDestinationDatabase;
  QVector<mdtSqlDatabaseCopierTableMapping> pvTableMappingList;
  mdtError pvLastError;
};

#endif // #ifndef MDT_SQL_DATABASE_COPIER_MAPPING_H
