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
#include "mdtSqlDatabaseSchemaThread.h"
#include "mdtSqlDatabaseSchemaModel.h"
#include "mdtAlgorithms.h"
#include "mdtSqlError.h"
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>

mdtSqlDatabaseSchemaThread::mdtSqlDatabaseSchemaThread(QObject* parent)
 : QThread(parent),
   pvAbort(false)
{
}

void mdtSqlDatabaseSchemaThread::createSchema(const mdtSqlDatabaseSchema & s, const QSqlDatabase & db)
{
  Q_ASSERT(!isRunning());

  pvSchema = s;
  Q_ASSERT(&pvSchema != &s);
  pvDatabaseInfo = db;
  start();
}

bool mdtSqlDatabaseSchemaThread::createSchemaBlocking(const mdtSqlDatabaseSchema & s, const QSqlDatabase & db)
{
  Q_ASSERT(db.isValid());
  Q_ASSERT(db.isOpen());

  auto tables = s.tableList();
  auto tablePopulations = s.tablePopulationSchemaList();
  auto views = s.viewList();

  // Create table
  for(const auto & ts : tables){
    if(!createTable(ts, db)){
      return false;
    }
  }
  // Populate tables
  for(const auto & tps : tablePopulations){
    if(!populateTable(tps, db)){
      return false;
    }
  }
  // Create views
  for(const auto & vs : views){
    if(!createView(vs, db)){
      return false;
    }
  }

  return true;
}

void mdtSqlDatabaseSchemaThread::abort()
{
  pvAbort = true;
}

void mdtSqlDatabaseSchemaThread::run()
{
  QString connectionName;
  {
    auto db = createConnection();
    connectionName = db.connectionName();
    auto tables = pvSchema.tableList();
    auto tablePopulations = pvSchema.tablePopulationSchemaList();
    auto views = pvSchema.viewList();
    auto triggers = pvSchema.triggerList();
    double globalProgess = 0.0;
    double globalProgessStep = 0.0;
    double elemetsCount;

    /*
     * Init
     */
    pvAbort = false;
    // Global progress
    elemetsCount = tables.size() + tablePopulations.size() + views.size() + triggers.size();
    if(elemetsCount > 0.0){
      globalProgessStep = 100.0 / elemetsCount;
    }
    emit globalProgressChanged(0);
    qDebug() << "THD started...";

    // Check that we are successfully connected to database
    if(!db.isOpen()){
      return;
    }
    /*
     * Create tables
     */
    createTables(tables, db, globalProgess, globalProgessStep);
    // Populate tables
    populateTables(tablePopulations, db, globalProgess, globalProgessStep);
    // Create views
    createViews(views, db, globalProgess, globalProgessStep);
    // Create triggers
    createTriggers(triggers, db, globalProgess, globalProgessStep);

    qDebug() << "THD END";
    
    // Close database
    db.close();
  }
  QSqlDatabase::removeDatabase(connectionName);
}

QSqlDatabase mdtSqlDatabaseSchemaThread::createConnection()
{
  QString cnn = mdtAlgorithms::generateString(6, QSqlDatabase::connectionNames());
  QSqlDatabase db = QSqlDatabase::addDatabase(pvDatabaseInfo.driverName(), cnn);

  qDebug() << "Driver: " << pvDatabaseInfo.driverName();
  qDebug() << "DB: " << pvDatabaseInfo.databaseName();
  
  db.setDatabaseName(pvDatabaseInfo.databaseName());
  db.setHostName(pvDatabaseInfo.hostName());
  db.setPort(pvDatabaseInfo.port());
  db.setUserName(pvDatabaseInfo.userName());
  db.setPassword(pvDatabaseInfo.password());
  db.setConnectOptions(pvDatabaseInfo.connectOptions());
  if(!db.open()){
    auto error = mdtErrorNewQ(tr("Connection to database") + " '" + db.databaseName() + "' " + tr("failed."), mdtError::Error, this);
    error.stackError(mdtSqlError::fromQSqlError(db.lastError()));
    error.commit();
    emit globalErrorOccured(error);
  }

  return db;
}

void mdtSqlDatabaseSchemaThread::createTables(QList<mdtSqlTableSchema> & tables, const QSqlDatabase & db,
                                              double & globalProgress, double globalProgressStep)
{
  bool errorOccured = false;
  double progress = 0.0;
  double progressStep;

  if(tables.isEmpty()){
    return;
  }
  progressStep = 100.0 / static_cast<double>(tables.size());
  for(auto & ts : tables){
    if(pvAbort){
      return;
    }
    // Create table
    if(!createTable(ts, db)){
      errorOccured = true;
    }
    // Update progresses
    if(!errorOccured){
      progress += progressStep;
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Table, "", progress);
      globalProgress += globalProgressStep;
      emit globalProgressChanged(globalProgress);
    }
  }
  // Update tables creation status
  if(errorOccured){
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Table, "", mdtSqlDatabaseSchemaModel::StatusError);
  }else{
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Table, "", mdtSqlDatabaseSchemaModel::StatusOk);
  }
}

bool mdtSqlDatabaseSchemaThread::createTable(const mdtSqlTableSchema & ts, const QSqlDatabase & db)
{
  QSqlQuery query(db);
  QString tableName = ts.tableName();

  emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Table, tableName, -1);
  for(const auto & sql : ts.getSqlForCreateTable(db)){
    if(!query.exec(sql)){
      auto error = mdtErrorNewQ(tr("Cannot create table '") + tableName + tr("'"), mdtError::Error, this);
      error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
      error.commit();
      emit objectErrorOccured(mdtSqlDatabaseSchemaModel::Table, tableName, error);
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Table, tableName, 0);
      return false;
    }
  }
  emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Table, tableName, 100);
  emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Table, tableName, mdtSqlDatabaseSchemaModel::StatusOk);

  return true;
}

void mdtSqlDatabaseSchemaThread::populateTables(const QList<mdtSqlTablePopulationSchema> & tablePopulations, const QSqlDatabase & db,
                                                double & globalProgress, double globalProgressStep)
{
  bool errorOccured = false;
  double progress = 0.0;
  double progressStep;

  if(tablePopulations.isEmpty()){
    return;
  }
  progressStep = 100.0 / static_cast<double>(tablePopulations.size());
  for(const auto & tps : tablePopulations){
    if(pvAbort){
      return;
    }
    if(!populateTable(tps, db)){
      errorOccured = true;
    }
    // Update progresses
    if(!errorOccured){
      progress += progressStep;
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::TablePopulation, "", progress);
      globalProgress += globalProgressStep;
      emit globalProgressChanged(globalProgress);
    }
  }
  // Update tables population status
  if(errorOccured){
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::TablePopulation, "", mdtSqlDatabaseSchemaModel::StatusError);
  }else{
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::TablePopulation, "", mdtSqlDatabaseSchemaModel::StatusOk);
  }
}

bool mdtSqlDatabaseSchemaThread::populateTable(const mdtSqlTablePopulationSchema & tps, const QSqlDatabase & db)
{
  Q_ASSERT(db.driver() != nullptr);

  QSqlQuery query(db);
  QString name = tps.name();
  QString sql = tps.sqlForInsert(db.driver());

  for(int row = 0; row < tps.rowDataCount(); ++row){
    // Build prepare statement
    if(!query.prepare(sql)){
      auto error = mdtErrorNewQ(tr("Cannot populate table '") + tps.tableName() + tr("'"), mdtError::Error, this);
      error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
      error.commit();
      emit objectErrorOccured(mdtSqlDatabaseSchemaModel::TablePopulation, name, error);
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::TablePopulation, name, 0);
      return false;
    }
    // Bind row data values
    for(const auto data : tps.rowData(row)){
      query.addBindValue(data);
    }
    // Exec query
    if(!query.exec()){
      auto error = mdtErrorNewQ(tr("Cannot populate table '") + tps.tableName() + tr("'"), mdtError::Error, this);
      error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
      error.commit();
      emit objectErrorOccured(mdtSqlDatabaseSchemaModel::TablePopulation, name, error);
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::TablePopulation, name, 0);
      return false;
    }
  }
  emit objectProgressChanged(mdtSqlDatabaseSchemaModel::TablePopulation, name, 100);
  emit objectStatusChanged(mdtSqlDatabaseSchemaModel::TablePopulation, name, mdtSqlDatabaseSchemaModel::StatusOk);

  return true;
}

void mdtSqlDatabaseSchemaThread::createViews(const QList<mdtSqlViewSchema::Schema> & views, const QSqlDatabase & db,
                                             double& globalProgress, double globalProgressStep)
{
  bool errorOccured = false;
  double progress = 0.0;
  double progressStep;

  if(views.isEmpty()){
    return;
  }
  progressStep = 100.0 / static_cast<double>(views.size());
  for(const auto & vs : views){
    if(pvAbort){
      return;
    }
    if(!createView(vs, db)){
      errorOccured = true;
    }
    // Update progresses
    if(!errorOccured){
      progress += progressStep;
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::View, "", progress);
      globalProgress += globalProgressStep;
      emit globalProgressChanged(globalProgress);
    }
  }
  // Update views creation status
  if(errorOccured){
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::View, "", mdtSqlDatabaseSchemaModel::StatusError);
  }else{
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::View, "", mdtSqlDatabaseSchemaModel::StatusOk);
  }
}

bool mdtSqlDatabaseSchemaThread::createView(const mdtSqlViewSchema::Schema & vs, const QSqlDatabase& db)
{
  Q_ASSERT(db.driver() != nullptr);

  QSqlQuery query(db);
  QString name = vs.name();
  QString sql;

  // Drop view
  sql = vs.getSqlForDrop(db.driver());
  if(!query.exec(sql)){
    auto error = mdtErrorNewQ(tr("Cannot drop view '") + name + tr("'"), mdtError::Error, this);
    error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
    error.commit();
    emit objectErrorOccured(mdtSqlDatabaseSchemaModel::View, name, error);
    emit objectProgressChanged(mdtSqlDatabaseSchemaModel::View, name, 0);
    return false;
  }
  // Create view
  sql = vs.getSqlForCreate(db.driver());
  if(!query.exec(sql)){
    auto error = mdtErrorNewQ(tr("Cannot create view '") + name + tr("'"), mdtError::Error, this);
    error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
    error.commit();
    emit objectErrorOccured(mdtSqlDatabaseSchemaModel::View, name, error);
    emit objectProgressChanged(mdtSqlDatabaseSchemaModel::View, name, 0);
    return false;
  }
  emit objectProgressChanged(mdtSqlDatabaseSchemaModel::View, name, 100);
  emit objectStatusChanged(mdtSqlDatabaseSchemaModel::View, name, mdtSqlDatabaseSchemaModel::StatusOk);

  return true;
}

void mdtSqlDatabaseSchemaThread::createTriggers(const QList<mdtSqlTriggerSchema> & triggers, const QSqlDatabase & db,
                                                double & globalProgress, double globalProgressStep)
{
  bool errorOccured = false;
  double progress = 0.0;
  double progressStep;

  if(triggers.isEmpty()){
    return;
  }
  progressStep = 100.0 / static_cast<double>(triggers.size());
  for(const auto & trigger : triggers){
    if(pvAbort){
      return;
    }
    if(!createTrigger(trigger, db)){
      errorOccured = true;
    }
    // Update progresses
    if(!errorOccured){
      progress += progressStep;
      emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Trigger, "", progress);
      globalProgress += globalProgressStep;
      emit globalProgressChanged(globalProgress);
    }
  }
  // Update views creation status
  if(errorOccured){
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Trigger, "", mdtSqlDatabaseSchemaModel::StatusError);
  }else{
    emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Trigger, "", mdtSqlDatabaseSchemaModel::StatusOk);
  }
}

bool mdtSqlDatabaseSchemaThread::createTrigger(const mdtSqlTriggerSchema& trigger, const QSqlDatabase& db)
{
  Q_ASSERT(db.isValid());

  QSqlQuery query(db);
  QString name = trigger.name();
  QString sql;

  // Drop trigger
  sql = trigger.getSqlForDropTrigger(db);
  if(!query.exec(sql)){
    auto error = mdtErrorNewQ(tr("Cannot drop trigger '") + name + tr("'"), mdtError::Error, this);
    error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
    error.commit();
    emit objectErrorOccured(mdtSqlDatabaseSchemaModel::Trigger, name, error);
    emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Trigger, name, 0);
    return false;
  }
  // Create trigger
  sql = trigger.getSqlForCreateTrigger(db);
  if(!query.exec(sql)){
    auto error = mdtErrorNewQ(tr("Cannot create trigger '") + name + tr("'"), mdtError::Error, this);
    error.stackError(mdtSqlError::fromQSqlError(query.lastError()));
    error.commit();
    emit objectErrorOccured(mdtSqlDatabaseSchemaModel::Trigger, name, error);
    emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Trigger, name, 0);
    return false;
  }
  emit objectProgressChanged(mdtSqlDatabaseSchemaModel::Trigger, name, 100);
  emit objectStatusChanged(mdtSqlDatabaseSchemaModel::Trigger, name, mdtSqlDatabaseSchemaModel::StatusOk);

  return true;
}
