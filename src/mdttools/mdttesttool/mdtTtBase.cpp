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
#include "mdtTtBase.h"
#include <QSqlError>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>

#include <QDebug>

/// \todo Add tr() in errors

mdtTtBase::mdtTtBase(QObject *parent, QSqlDatabase db)
 : QObject(parent)
{
  pvDatabase = db;
}

mdtTtBase::~mdtTtBase() 
{
}

QSqlDatabase mdtTtBase::database() 
{
  return pvDatabase;
}

mdtError mdtTtBase::lastError() 
{
  return pvLastError;
}

bool mdtTtBase::addRecord(const mdtSqlRecord & record, const QString & tableName)
{
  QString sql;
  QSqlQuery query(database());
  ///int i, index, lastIndex;
  QVector<int> indexList;
  int i;

  // Get last field index that has a value
  /**
  lastIndex = -1;
  for(i = 0; i < record.count(); ++i){
    if(record.hasValue(i)){
      lastIndex = i;
    }
  }
  // Prepare query for insertion
  sql = "INSERT INTO " + tableName +" (";
  for(i = 0; i <= lastIndex; ++i){
    if(record.hasValue(i)){
      sql += record.fieldName(i);
      if(i < (lastIndex-1)){
        sql += ",";
      }
    }
  }
  sql += ") VALUES (";
  for(i = 0; i <= lastIndex; ++i){
    if(record.hasValue(i)){
      sql += "?";
      if(i < (lastIndex-1)){
        sql += ",";
      }
    }
  }
  sql += ")";
  */
  indexList = record.fieldIndexesWithValue();
  sql = record.sqlForInsert(tableName);
  qDebug() << "SQL: " << sql;
  // Prepare query
  if(!query.prepare(sql)){
    QSqlError sqlError = query.lastError();
    pvLastError.setError(tr("Cannot prepare query for inertion in table '") + tableName + tr("'"), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  // Add values and execute query
  for(i = 0; i < indexList.size(); ++i){
    query.bindValue(i, record.value(indexList.at(i)));
  }
  /**
  index = 0;
  for(i = 0; i <= lastIndex; ++i){
    if(record.hasValue(i)){
      query.bindValue(index, record.value(i));
      ++index;
    }
  }
  */
  if(!query.exec()){
    QSqlError sqlError = query.lastError();
    pvLastError.setError(tr("Cannot exec query for inertion in table '") + tableName + tr("'"), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }

  return true;
}

bool mdtTtBase::addRecordList(const QList<mdtSqlRecord> & recordList, const QString & tableName, bool singleTransaction )
{
  int i;

  if(singleTransaction){
    if(!beginTransaction()){
      return false;
    }
  }
  for(i = 0; i < recordList.size(); ++i){
    if(!addRecord(recordList.at(i), tableName)){
      if(singleTransaction){
        rollbackTransaction();
      }
      return false;
    }
  }
  if(singleTransaction){
    if(!commitTransaction()){
      return false;
    }
  }

  return true;  
}

QList<QSqlRecord> mdtTtBase::getData(const QString & sql, bool *ok, const QStringList & expectedFields)
{
  QSqlQuery query(database());
  QList<QSqlRecord> dataList;
  int i;

  // Execute query
  if(!query.exec(sql)){
    QSqlError sqlError = query.lastError();
    pvLastError.setError(tr("Cannot exec query: '") + sql + tr("'"), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    if(ok != 0){
      *ok = false;
    }
    return dataList;
  }
  // If requested, check that expected fields exists in result
  if(expectedFields.size() > 0){
    QStringList missingFields;
    QSqlRecord record = query.record();
    for(i = 0; i < expectedFields.size(); ++i){
      if(record.indexOf(expectedFields.at(i)) < 0){
        missingFields.append(expectedFields.at(i));
      }
    }
    if(missingFields.size() > 0){
      QString text = tr("A query returned not all expected fields. Missing fields:\n");
      for(i = 0; i < missingFields.size(); ++i){
        text += " - " + missingFields.at(i) + "\n";
      }
      text += tr("SQL statement: '") + sql + tr("'");
      pvLastError.setError(text, mdtError::Error);
      MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
      pvLastError.commit();
      if(ok != 0){
        *ok = false;
      }
      return dataList;

    }
  }
  // Get data
  while(query.next()){
    dataList.append(query.record());
  }
  if(ok != 0){
    *ok = true;
  }

  return dataList;
}

bool mdtTtBase::updateRecord(const QString & tableName, const mdtSqlRecord & record, const QSqlRecord & matchData)
{
  QSqlQuery query(database());
  QString sql;
  QVector<int> indexList;
  int i;

  // Set base SQL statement and prepare query
  indexList = record.fieldIndexesWithValue();
  sql = record.sqlForUpdate(tableName, matchData);
  if(!query.prepare(sql)){
    QSqlError sqlError = query.lastError();
    pvLastError.setError(tr("Cannot prepare query to update table '") + tableName + tr("'"), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  // Add values and execute query
  for(i = 0; i < indexList.size(); ++i){
    query.bindValue(i, record.value(indexList.at(i)));
  }
  if(!query.exec()){
    QSqlError sqlError = query.lastError();
    pvLastError.setError(tr("Cannot exec query to update table '") + tableName + tr("'"), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }

  return true;
}

bool mdtTtBase::updateRecord(const QString & tableName, const mdtSqlRecord & record, const QString & matchField, const QVariant & matchData)
{
  mdtSqlRecord matchRecord;

  if(!matchRecord.addField(matchField, tableName, database())){
    pvLastError = matchRecord.lastError();
    return false;
  }
  matchRecord.setValue(matchField, matchData);

  return updateRecord(tableName, record, matchRecord);
}

bool mdtTtBase::removeData(const QString & tableName, const QStringList & fields, const QModelIndexList & indexes)
{
  int i;
  int col;
  int max;
  QVariant data;
  QString sql;
  QString delimiter;

  // If no index was given, we simply do nothing
  if(indexes.isEmpty()){
    return true;
  }
  // Check that indexes contains not more columns than fields
  max = 0;
  for(i = 0; i < indexes.size(); ++i){
    col = indexes.at(i).column();
    if(col > max){
      max = col;
    }
  }
  ///qDebug() << "indexes max column : " << max;
  if(max > fields.size()){
    pvLastError.setError("Cannot remove rows in table " + tableName + ": indexes contains more columns than fields list.", mdtError::Error);
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  // Generate SQL statement
  sql = "DELETE FROM " + tableName;
  for(i = 0; i < indexes.size(); ++i){
    if(i == 0){
      sql += " WHERE (";
    }else{
      sql += " OR ";
    }
    col = indexes.at(i).column();
    data = indexes.at(i).data();
    // Set the delimiter
    delimiter = sqlDataDelimiter(data.type());
    Q_ASSERT(col < fields.size());
    sql += fields.at(col) + " = " + delimiter + data.toString() + delimiter;
  }
  sql += ")";
  ///qDebug() << "SQL: " << sql;
  // Submit query
  QSqlQuery query(database());
  if(!query.exec(sql)){
    QSqlError sqlError;
    sqlError = query.lastError();
    pvLastError.setError("Cannot remove rows in table " + tableName, mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }

  return true;
}

bool mdtTtBase::removeData(const QString & tableName, const QString & fieldName, const QModelIndexList & indexes)
{
  return removeData(tableName, QStringList(fieldName), indexes);
}

bool mdtTtBase::removeData(const QString & tableName, const QString & fieldName, const QVariant & matchData)
{
  QString sql;
  QString delimiter;

  // Generate SQL statement
  delimiter = sqlDataDelimiter(matchData.type());
  sql = "DELETE FROM " + tableName + " WHERE " + fieldName + " = " + delimiter + matchData.toString() + delimiter;
  ///qDebug() << "SQL: " << sql;
  // Submit query
  QSqlQuery query(database());
  if(!query.exec(sql)){
    QSqlError sqlError;
    sqlError = query.lastError();
    pvLastError.setError("Cannot remove row in table " + tableName, mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }

  return true;
}

bool mdtTtBase::beginTransaction() 
{
  if(!pvDatabase.transaction()){
    QSqlError sqlError = pvDatabase.lastError();
    pvLastError.setError("Cannot beginn transaction (database: " + pvDatabase.databaseName() + ")", mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  return true;
}

bool mdtTtBase::rollbackTransaction() 
{
  if(!pvDatabase.rollback()){
    QSqlError sqlError = pvDatabase.lastError();
    pvLastError.setError("Cannot beginn rollback (database: " + pvDatabase.databaseName() + ")", mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  return true;
}

bool mdtTtBase::commitTransaction() 
{
  if(!pvDatabase.commit()){
    QSqlError sqlError = pvDatabase.lastError();
    pvLastError.setError("Cannot beginn commit (database: " + pvDatabase.databaseName() + ")", mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtTtBase");
    pvLastError.commit();
    return false;
  }
  return true;
}

QString mdtTtBase::sqlDataDelimiter(QVariant::Type type)
{
  return mdtSqlRecord::sqlDataDelimiter(type);
}
