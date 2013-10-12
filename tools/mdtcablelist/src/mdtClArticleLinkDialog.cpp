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
#include "mdtClArticleLinkDialog.h"
#include "mdtSqlSelectionDialog.h"
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMessageBox>

//#include <QDebug>

mdtClArticleLinkDialog::mdtClArticleLinkDialog(QWidget *parent, QSqlDatabase db, QVariant articleId)
 : QDialog(parent)
{
  pvDatabase = db;
  // Setup UI
  setupUi(this);
  setWindowTitle(tr("Article link edition"));
  connect(pbStartConnection, SIGNAL(clicked()), this, SLOT(selectStartConnection()));
  connect(pbEndConnection, SIGNAL(clicked()), this, SLOT(selectEndConnection()));
  // Setup link type
  pvLinkTypeModel = new QSqlQueryModel(this);
  pvLinkTypeModel->setQuery("SELECT Code_PK, NameEN, ValueUnit FROM LinkType_tbl ORDER BY NameEN", pvDatabase);
  cbLinkType->setModel(pvLinkTypeModel);
  cbLinkType->setModelColumn(1);
  cbLinkType->setCurrentIndex(-1);
  connect(cbLinkType, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbLinkTypeCurrentIndexChanged(int)));
  // Setup link direction
  pvLinkDirectionModel = new QSqlQueryModel(this);
  pvLinkDirectionModel->setQuery("SELECT Code_PK, NameEN FROM LinkDirection_tbl", pvDatabase);
  cbLinkDirection->setModel(pvLinkDirectionModel);
  cbLinkDirection->setModelColumn(1);
  connect(cbLinkDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(onCbLinkDirectionCurrentIndexChanged(int)));
  cbLinkDirection->setCurrentIndex(-1);
  // Setup connections
  pvArticleConnectionModel = new QSqlQueryModel(this);
  pvArticleConnectionModel->setQuery("SELECT Id_PK, ArticleConnectorName, ArticleContactName, IoType, FunctionEN \
                                     FROM ArticleConnection_tbl WHERE Article_Id_FK = " + articleId.toString(), pvDatabase);
  lbStartConnectorName->clear();
  lbStartContactName->clear();
  lbEndConnectorName->clear();
  lbEndContactName->clear();
  pvValue = 0.0;
  lbUnit->clear();
}

mdtClArticleLinkDialog::~mdtClArticleLinkDialog()
{
}

void mdtClArticleLinkDialog::setLinkTypeCode(const QVariant & code)
{
  QModelIndex index;
  int row;
  QVariant data;

  pvLinkTypeCode = code;
  for(row = 0; row < pvLinkTypeModel->rowCount(); ++row){
    index = pvLinkTypeModel->index(row, 0);
    data = pvLinkTypeModel->data(index);
    if(data == code){
      cbLinkType->setCurrentIndex(row);
      return;
    }
  }
}

QVariant mdtClArticleLinkDialog::linkTypeCode() const
{
  return pvLinkTypeCode;
}

void mdtClArticleLinkDialog::setLinkDirectionCode(const QVariant & code)
{
  QModelIndex index;
  int row;
  QVariant data;

  pvLinkDirectionCode = code;
  for(row = 0; row < pvLinkDirectionModel->rowCount(); ++row){
    index = pvLinkDirectionModel->index(row, 0);
    data = pvLinkDirectionModel->data(index);
    if(data == code){
      cbLinkDirection->setCurrentIndex(row);
      return;
    }
  }
}

QVariant mdtClArticleLinkDialog::linkDirectionCode() const
{
  return pvLinkDirectionCode;
}

void mdtClArticleLinkDialog::setValue(const QVariant & value)
{
  pvValue = value.toDouble();
  sbValue->setValue(pvValue);
}

QVariant mdtClArticleLinkDialog::value() const
{
  return pvValue;
}

void mdtClArticleLinkDialog::setStartConnectionId(const QVariant & id)
{
  pvStartConnectionId = id;
  displayCurrentSelectedStartConnection();
}

QVariant mdtClArticleLinkDialog::startConnectionId() const
{
  return pvStartConnectionId;
}

void mdtClArticleLinkDialog::setEndConnectionId(const QVariant & id)
{
  pvEndConnectionId = id;
  displayCurrentSelectedEndConnection();
}

QVariant mdtClArticleLinkDialog::endConnectionId() const
{
  return pvEndConnectionId;
}

void mdtClArticleLinkDialog::onCbLinkTypeCurrentIndexChanged(int row)
{
  QModelIndex index;
  QVariant data;

  if(row < 0){
    lbUnit->setText("");
    return;
  }
  // We must update available directions regarding link type
  index = pvLinkTypeModel->index(row, 0);
  data = pvLinkTypeModel->data(index);
  if(data == QVariant("DIODE")){
    pvLinkDirectionModel->setQuery("SELECT Code_PK, NameEN, PictureAscii FROM LinkDirection_tbl WHERE Code_PK <> 'BID'", pvDatabase);
    cbLinkDirection->setEnabled(true);
    sbValue->setValue(0.7);
  }else{
    pvLinkDirectionModel->setQuery("SELECT Code_PK, NameEN, PictureAscii FROM LinkDirection_tbl WHERE Code_PK = 'BID'", pvDatabase);
    cbLinkDirection->setEnabled(false);
    sbValue->setValue(0.0);
  }
  // Update displayed unit (V, Ohm, ...)
  index = pvLinkTypeModel->index(row, 2);
  data = pvLinkTypeModel->data(index);
  lbUnit->setText("[" + data.toString() + "]");
}

void mdtClArticleLinkDialog::onCbLinkDirectionCurrentIndexChanged(int row)
{
  QModelIndex index;
  QVariant data;

  if(row < 0){
    lbLinkDirectionAsciiPicture->setText("");
    return;
  }
  // Update the ASCII picture
  index = pvLinkDirectionModel->index(row, 2);
  data = pvLinkDirectionModel->data(index);
  lbLinkDirectionAsciiPicture->setText(data.toString());
}

void mdtClArticleLinkDialog::selectStartConnection()
{
  mdtSqlSelectionDialog dialog(this);

  // Setup and show dialog
  dialog.setMessage(tr("Please select the start connection"));
  dialog.setModel(pvArticleConnectionModel, false);
  dialog.setColumnHidden("Id_PK", true);
  dialog.setHeaderData("ArticleConnectorName", "Connector");
  dialog.setHeaderData("ArticleContactName", "Contact");
  dialog.setHeaderData("IoType", "I/O type");
  dialog.setHeaderData("FunctionEN", "Function");
  dialog.addSelectionResultColumn("Id_PK");
  dialog.resize(600, 400);
  if(dialog.exec() != QDialog::Accepted){
    return;
  }
  // Store result
  Q_ASSERT(dialog.selectionResult().size() == 1);
  pvStartConnectionId = dialog.selectionResult().at(0);
  // Update GUI
  displayCurrentSelectedStartConnection();
}

void mdtClArticleLinkDialog::selectEndConnection()
{
  mdtSqlSelectionDialog dialog(this);

  // Setup and show dialog
  dialog.setMessage(tr("Please select the end connection"));
  dialog.setModel(pvArticleConnectionModel, false);
  dialog.setColumnHidden("Id_PK", true);
  dialog.setHeaderData("ArticleConnectorName", "Connector");
  dialog.setHeaderData("ArticleContactName", "Contact");
  dialog.setHeaderData("IoType", "I/O type");
  dialog.setHeaderData("FunctionEN", "Function");
  dialog.addSelectionResultColumn("Id_PK");
  dialog.resize(600, 400);
  if(dialog.exec() != QDialog::Accepted){
    return;
  }
  // Store result
  Q_ASSERT(dialog.selectionResult().size() == 1);
  pvEndConnectionId = dialog.selectionResult().at(0);
  // Update GUI
  displayCurrentSelectedEndConnection();
}

void mdtClArticleLinkDialog::accept()
{
  QStringList errorList;
  QString infoText;
  int i;

  // Store and check link type
  storeCurrentSelectedLinkType();
  if(pvLinkTypeCode.isNull()){
    errorList << tr("Link type is not set");
  }
  // Store and check link direction
  storeCurrentSelectedLinkDirection();
  if(pvLinkDirectionCode.isNull()){
    errorList << tr("Direction is not set");
  }
  // Store value
  pvValue = sbValue->value();
  // Start and end connections are allready set afetr selection by user
  if(pvStartConnectionId.isNull()){
    errorList << tr("Start connection is not set");
  }
  if(pvEndConnectionId.isNull()){
    errorList << tr("End connection is not set");
  }
  // If something is missing, display a message to the user
  if(!errorList.isEmpty()){
    infoText = tr("Please check following points and try again:\n");
    for(i = 0; i < errorList.size(); ++i){
      infoText += "- " + errorList.at(i) + "\n";
    }
    QMessageBox msgBox;
    msgBox.setText(tr("Some informations are missing"));
    msgBox.setInformativeText(infoText);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Article link warning"));
    msgBox.exec();
    return;
  }
  // Here, all is ok, accept dialog
  QDialog::accept();
}

void mdtClArticleLinkDialog::reject()
{
  pvLinkTypeCode.clear();
  pvLinkDirectionCode.clear();
  pvValue = 0.0;
  pvStartConnectionId.clear();
  pvEndConnectionId.clear();
  QDialog::reject();
}

void mdtClArticleLinkDialog::storeCurrentSelectedLinkType()
{
  QModelIndex index;
  int row;

  row = cbLinkType->currentIndex();
  if(row < 0){
    pvLinkTypeCode.clear();
    return;
  }
  index = pvLinkTypeModel->index(row, 0);
  pvLinkTypeCode = pvLinkTypeModel->data(index);
}

void mdtClArticleLinkDialog::storeCurrentSelectedLinkDirection()
{
  QModelIndex index;
  int row;

  row = cbLinkDirection->currentIndex();
  if(row < 0){
    pvLinkDirectionCode.clear();
    return;
  }
  index = pvLinkDirectionModel->index(row, 0);
  pvLinkDirectionCode = pvLinkDirectionModel->data(index);
}

void mdtClArticleLinkDialog::displayCurrentSelectedStartConnection()
{
  QModelIndex index;
  QVariant data;
  int row;

  if(pvStartConnectionId.isNull()){
    lbStartConnectorName->clear();
    lbStartContactName->clear();
    return;
  }
  for(row = 0; row < pvArticleConnectionModel->rowCount(); ++row){
    index = pvArticleConnectionModel->index(row, 0);
    data = pvArticleConnectionModel->data(index);
    if(data == pvStartConnectionId){
      // Set connector name
      index = pvArticleConnectionModel->index(row, 1);
      data = pvArticleConnectionModel->data(index);
      lbStartConnectorName->setText(data.toString());
      // Set contact name
      index = pvArticleConnectionModel->index(row, 2);
      data = pvArticleConnectionModel->data(index);
      lbStartContactName->setText(data.toString());
      break;
    }
  }
}

void mdtClArticleLinkDialog::displayCurrentSelectedEndConnection()
{
  QModelIndex index;
  QVariant data;
  int row;

  if(pvEndConnectionId.isNull()){
    lbEndConnectorName->clear();
    lbEndContactName->clear();
    return;
  }
  for(row = 0; row < pvArticleConnectionModel->rowCount(); ++row){
    index = pvArticleConnectionModel->index(row, 0);
    data = pvArticleConnectionModel->data(index);
    if(data == pvEndConnectionId){
      // Set connector name
      index = pvArticleConnectionModel->index(row, 1);
      data = pvArticleConnectionModel->data(index);
      lbEndConnectorName->setText(data.toString());
      // Set contact name
      index = pvArticleConnectionModel->index(row, 2);
      data = pvArticleConnectionModel->data(index);
      lbEndContactName->setText(data.toString());
      break;
    }
  }
}
