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
#include "mdtClUnit.h"
#include <QSqlQuery>

#include <QDebug>

mdtClUnit::mdtClUnit(QSqlDatabase db)
{
  pvDatabase = db;
  pvToUnitConnectionRelatedRangesModel = new QSqlQueryModel;
  pvUnitModel = new QSqlQueryModel;
  pvArticleConnectionModel = new QSqlQueryModel;
}

mdtClUnit::~mdtClUnit()
{
  delete pvToUnitConnectionRelatedRangesModel;
  delete pvUnitModel;
  delete pvArticleConnectionModel;
}

const QSqlError &mdtClUnit::lastError()
{
  return pvLastError;
}

QSqlQueryModel *mdtClUnit::unitModelForComponentSelection(const QVariant &unitId)
{
  QString sql;

  sql =  "SELECT Id_PK, SchemaPosition, Cabinet, Coordinate "\
         "FROM Unit_tbl "\
         "WHERE ( Id_PK <> " + unitId.toString() + " "\
         "AND Composite_Id_FK IS NULL )";
  pvUnitModel->setQuery(sql, pvDatabase);

  return pvUnitModel;
}

bool mdtClUnit::addComponent(const QVariant &unitId, const QVariant &componentId)
{
  QString sql;
  QSqlQuery query(pvDatabase);

  // Prepare query for edition
  sql = "UPDATE Unit_tbl "\
        "SET Composite_Id_FK = :Composite_Id_FK "\
        "WHERE Id_PK = " + componentId.toString();
  if(!query.prepare(sql)){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot prepare query for component assignation", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return false;
  }
  // Add values and execute query
  query.bindValue(":Composite_Id_FK", unitId);
  if(!query.exec()){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot execute query for component assignation", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return false;
  }

  return true;
}

bool mdtClUnit::removeComponent(const QVariant &componentId)
{
  QList<QVariant> idList;

  idList.append(componentId);

  return removeComponents(idList);
}

bool mdtClUnit::removeComponents(const QList<QVariant> &componentIdList)
{
  int i;
  QString sql;

  if(componentIdList.size() < 1){
    return true;
  }
  // Generate SQL
  sql = "UPDATE Unit_tbl SET Composite_Id_FK = NULL ";
  for(i = 0; i < componentIdList.size(); ++i){
    if(i == 0){
      sql += " WHERE ( ";
    }else{
      sql += " OR ";
    }
    sql += " Id_PK = " + componentIdList.at(i).toString();
  }
  sql += " ) ";
  // Submit query
  QSqlQuery query(pvDatabase);
  if(!query.exec(sql)){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot execute query for component assignation remove", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return false;
  }

  return true;
}

bool mdtClUnit::removeComponents(const QModelIndexList & indexListOfSelectedRows)
{
  int i;
  QList<QVariant> idList;

  for(i = 0; i < indexListOfSelectedRows.size(); ++i){
    idList.append(indexListOfSelectedRows.at(i).data());
  }

  return removeComponents(idList);
}

QSqlQueryModel *mdtClUnit::modelForArticleConnectionSelection(const QVariant & unitId, const QVariant &articleId)
{
  QString sql;

  sql = "SELECT "\
        " ArticleConnection_tbl.Id_PK , "\
        " ArticleConnection_tbl.Article_Id_FK , "\
        " ArticleConnection_tbl.ArticleConnectorName , "\
        " ArticleConnection_tbl.ArticleContactName , "\
        " ArticleConnection_tbl.IoType , "\
        " ArticleConnection_tbl.FunctionEN "\
        "FROM ArticleConnection_tbl "\
        "LEFT JOIN UnitConnection_tbl "\
        " ON ArticleConnection_tbl.Id_PK = UnitConnection_tbl.ArticleConnection_Id_FK "\
        "WHERE Article_Id_FK = " + articleId.toString();
  qDebug() << "SEL ART CNN SQL: " << sql;
  pvArticleConnectionModel->setQuery(sql, pvDatabase);
  pvLastError = pvArticleConnectionModel->lastError();
  if(pvLastError.isValid()){
    mdtError e(MDT_DATABASE_ERROR, "Cannot execute query for article connection selection", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
  }

  return pvArticleConnectionModel;
}

int mdtClUnit::toUnitRelatedArticleConnectionCount(const QVariant & unitId)
{
  QString sql;
  QSqlQuery query(pvDatabase);

  // Build and run query
  sql = "SELECT COUNT(*) FROM UnitConnection_tbl \
         JOIN ArticleConnection_tbl \
         ON UnitConnection_tbl.ArticleConnection_Id_FK = ArticleConnection_tbl.Id_PK \
         WHERE UnitConnection_tbl.Unit_Id_FK = " + unitId.toString();
  if(!query.exec(sql)){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Query execution failed" , mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return -1;
  }
  // Get value
  if(query.next()){
    Q_ASSERT(query.value(0).isValid());
    return query.value(0).toInt();
  }

  // Should not happen
  return -1;
}

QList<QVariant> mdtClUnit::toUnitRelatedArticleConnectionIds(const QVariant & unitId) {
}

bool mdtClUnit::unitConnectionIsRelatedToArticleConnection(const QVariant & unitConnectionId) {
}

QVariant mdtClUnit::toUnitConnectionRelatedArticleConnectionData(const QVariant & unitConnectionId, const QString & field) {
}

QSqlQueryModel *mdtClUnit::toUnitConnectionRelatedRangesModel(const QVariant & unitConnectionId)
{
  
  
  return pvToUnitConnectionRelatedRangesModel;
}

bool mdtClUnit::addUnitConnection(const mdtClUnitConnectionData & data)
{
  QString sql;
  QSqlQuery query(pvDatabase);

  // Prepare query for insertion
  sql = "INSERT INTO UnitConnection_tbl (Unit_Id_FK, ArticleConnection_Id_FK, IsATestPoint, SchemaPage, FunctionEN, SignalName, SwAddress, UnitConnectorName, UnitContactName) "\
        "VALUES (:Unit_Id_FK, :ArticleConnection_Id_FK, :IsATestPoint, :SchemaPage, :FunctionEN, :SignalName, :SwAddress, :UnitConnectorName, :UnitContactName)";
  if(!query.prepare(sql)){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot prepare query for component inertion", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return false;
  }
  // Add values and execute query
  query.bindValue(":Unit_Id_FK", data.unitId());
  query.bindValue(":ArticleConnection_Id_FK", data.articleConnectionId());
  query.bindValue(":IsATestPoint", QVariant());   /// \todo Implement
  query.bindValue(":SchemaPage", data.schemaPage());
  query.bindValue(":FunctionEN", data.functionEN());
  query.bindValue(":SignalName", data.signalName());
  query.bindValue(":SwAddress", data.swAddress());
  query.bindValue(":UnitConnectorName", data.unitConnectorName());
  query.bindValue(":UnitContactName", data.unitContactName());
  if(!query.exec()){
    pvLastError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot execute query for connection inertion", mdtError::Error);
    e.setSystemError(pvLastError.number(), pvLastError.text());
    MDT_ERROR_SET_SRC(e, "mdtClUnit");
    e.commit();
    return false;
  }

  return true;
}

bool mdtClUnit::removeUnitConnection(const QVariant & unitConnectionId) {
}

bool mdtClUnit::editUnitConnection(const mdtClUnitConnectionData & data) {
}

mdtClUnitConnectionData mdtClUnit::getUnitConnectionData(const QVariant & unitConnectionId) {
}



bool mdtClUnit::addRange(const QVariant & baseUnitConnectionId, const mdtClUnitConnectionData & rangeData) {
}

