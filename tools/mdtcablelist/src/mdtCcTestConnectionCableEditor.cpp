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
#include "mdtCcTestConnectionCableEditor.h"
#include "mdtCcTestConnectionCable.h"
#include "mdtSqlSelectionDialog.h"
#include <QSqlQueryModel>
#include <QSqlError>
#include <QString>

#include <QDebug>

mdtCcTestConnectionCableEditor::mdtCcTestConnectionCableEditor(QObject *parent, QSqlDatabase db)
 : QObject(parent)
{
  pvDatabase = db;
}

void mdtCcTestConnectionCableEditor::createCable()
{
  mdtCcTestConnectionCable tcc(pvDatabase);
  QVariant dutUnitId;
  QVariant testNodeId;
  QVariant dutStartConnectorId;
  QList<QVariant> dutStartConnectionIdList;
  QList<QVariant> dutLinkedConnectorIdList;
  QList<QVariant> dutEndConnectorIdList;
  QList<QVariant> idList;
  int i, k;

  // Let user choose DUT unit and test node
  dutUnitId = selectDutUnitId();
  if(dutUnitId.isNull()){
    return;
  }
  testNodeId = selectTestNode();
  if(testNodeId.isNull()){
    return;
  }
  // Select a start connector
  dutStartConnectorId = selectStartConnectorId(dutUnitId);
  if(dutStartConnectorId.isNull()){
    return;
  }
  qDebug() << "Selected DUT: " << dutUnitId << " , Test node: " << testNodeId;
  // Get start unit connections for selected connector
  dutStartConnectionIdList = tcc.getToUnitConnectorRelatedUnitConnectionIdList(dutStartConnectorId);
  qDebug() << "Start connections: " << dutStartConnectionIdList;
  for(i = 0; i < dutStartConnectionIdList.size(); ++i){
    // Get list of unit connectors that is linked to selected start unit connection
    idList = tcc.getToUnitConnectionLinkedUnitConnectorIdList(dutStartConnectionIdList.at(i));
    for(k = 0; k < idList.size(); ++k){
      if(!dutLinkedConnectorIdList.contains(idList.at(k))){
        dutLinkedConnectorIdList.append(idList.at(k));
      }
    }
  }
  // Select end connector(s)
  dutEndConnectorIdList = selectEndConnectorIdList(dutLinkedConnectorIdList);
  qDebug() << "End connectors: " << dutEndConnectorIdList;
}

QVariant mdtCcTestConnectionCableEditor::selectDutUnitId()
{
  mdtSqlSelectionDialog selectionDialog;
  QSqlError sqlError;
  QSqlQueryModel model;
  QString sql;

  // Setup model to show available connectors
  sql = "SELECT * FROM VehicleType_Unit_view ";
  model.setQuery(sql, pvDatabase);
  sqlError = model.lastError();
  if(sqlError.isValid()){
    pvLastError.setError(tr("Unable to get unit list."), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtCcTestConnectionCableEditor");
    pvLastError.commit();
    ///displayLastError();
    return QVariant();
  }
  // Setup and show dialog
  selectionDialog.setMessage("Please select unit to test.");
  selectionDialog.setModel(&model, false);
  selectionDialog.setColumnHidden("Unit_Id_FK", true);
  selectionDialog.setColumnHidden("VehicleType_Id_FK", true);
  ///selectionDialog.setHeaderData("SubType", tr("Variant"));
  ///selectionDialog.setHeaderData("SeriesNumber", tr("Serie"));
  selectionDialog.addSelectionResultColumn("Unit_Id_FK");
  selectionDialog.resize(750, 300);
  if(selectionDialog.exec() != QDialog::Accepted){
    return QVariant();
  }
  Q_ASSERT(selectionDialog.selectionResult().size() == 1);

  return selectionDialog.selectionResult().at(0);
}

QVariant mdtCcTestConnectionCableEditor::selectTestNode()
{
  mdtCcTestConnectionCable tcc(pvDatabase);
  mdtSqlSelectionDialog selectionDialog;
  QSqlError sqlError;
  QSqlQueryModel model;
  QString sql;

  // Setup model to show available test nodes
  ///sql = "SELECT * FROM TestNode_tbl ";
  sql = tcc.sqlForTestNodeSelection();
  model.setQuery(sql, pvDatabase);
  sqlError = model.lastError();
  if(sqlError.isValid()){
    pvLastError.setError(tr("Unable to get test node list."), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtCcTestConnectionCableEditor");
    pvLastError.commit();
    ///displayLastError();
    return QVariant();
  }
  // Setup and show dialog
  selectionDialog.setMessage("Please select test node to use.");
  selectionDialog.setModel(&model, false);
  selectionDialog.setColumnHidden("VehicleType_Id_FK_PK", true);
  selectionDialog.setColumnHidden("Id_PK", true);
  selectionDialog.setHeaderData("SubType", tr("Variant"));
  selectionDialog.setHeaderData("SeriesNumber", tr("Serie"));
  selectionDialog.addSelectionResultColumn("VehicleType_Id_FK_PK");
  selectionDialog.resize(600, 300);
  if(selectionDialog.exec() != QDialog::Accepted){
    return QVariant();
  }
  Q_ASSERT(selectionDialog.selectionResult().size() == 1);

  return selectionDialog.selectionResult().at(0);
}

QVariant mdtCcTestConnectionCableEditor::selectStartConnectorId(const QVariant & dutUnitId) 
{
  mdtCcTestConnectionCable tcc(pvDatabase);
  mdtSqlSelectionDialog selectionDialog;
  QSqlError sqlError;
  QSqlQueryModel model;
  QString sql;

  // Setup model to show available connectors
  sql = tcc.sqlForStartConnectorSelection(dutUnitId);
  model.setQuery(sql, pvDatabase);
  sqlError = model.lastError();
  if(sqlError.isValid()){
    pvLastError.setError(tr("Unable to get unit connector list."), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtCcTestConnectionCableEditor");
    pvLastError.commit();
    ///displayLastError();
    return QVariant();
  }
  // Setup and show dialog
  selectionDialog.setMessage("Please select a connector to test.");
  selectionDialog.setModel(&model, false);
  selectionDialog.setColumnHidden("Id_PK", true);
  selectionDialog.setColumnHidden("Unit_Id_FK", true);
  selectionDialog.setColumnHidden("Connector_Id_FK", true);
  selectionDialog.setColumnHidden("ArticleConnector_Id_FK", true);
  selectionDialog.addSelectionResultColumn("Id_PK");
  selectionDialog.resize(500, 300);
  if(selectionDialog.exec() != QDialog::Accepted){
    return QVariant();
  }
  Q_ASSERT(selectionDialog.selectionResult().size() == 1);

  return selectionDialog.selectionResult().at(0);
}

QList<QVariant> mdtCcTestConnectionCableEditor::selectEndConnectorIdList(const QList<QVariant> & unitConnectorIdList) 
{
  mdtCcTestConnectionCable tcc(pvDatabase);
  mdtSqlSelectionDialog selectionDialog;
  QModelIndexList selectedItems;
  QList<QVariant> idList;
  QSqlError sqlError;
  QSqlQueryModel model;
  QString sql;
  int i;

  // Setup model to show available connectors
  sql = tcc.sqlForUnitConnectorSelectionFromUnitConnectorIdList(unitConnectorIdList);
  model.setQuery(sql, pvDatabase);
  sqlError = model.lastError();
  if(sqlError.isValid()){
    pvLastError.setError(tr("Unable to get unit connector list."), mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtCcTestConnectionCableEditor");
    pvLastError.commit();
    ///displayLastError();
    return idList;
  }
  // Setup and show dialog
  selectionDialog.setMessage("Please select end connector(s) to test.");
  selectionDialog.setModel(&model, true);
  selectionDialog.setColumnHidden("Id_PK", true);
  selectionDialog.setColumnHidden("Unit_Id_FK", true);
  selectionDialog.setColumnHidden("Connector_Id_FK", true);
  selectionDialog.setColumnHidden("ArticleConnector_Id_FK", true);
  selectionDialog.addSelectionResultColumn("Id_PK");
  selectionDialog.resize(500, 300);
  if(selectionDialog.exec() != QDialog::Accepted){
    return idList;
  }
  selectedItems = selectionDialog.selectionResults();
  for(i = 0; i < selectedItems.size(); ++i){
    idList.append(selectedItems.at(i).data());
  }

  return idList;  
}

