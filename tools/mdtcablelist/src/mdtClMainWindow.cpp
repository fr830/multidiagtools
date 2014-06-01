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
#include "mdtClMainWindow.h"
#include "mdtSqlWindow.h"
#include "mdtSqlForm.h"
#include "mdtSqlDatabaseManager.h"
#include "mdtTtDatabaseSchema.h"
#include "mdtClVehicleTypeEditor.h"
#include "mdtClConnectorEditor.h"
#include "mdtClUnitEditor.h"
#include "mdtClLinkBeamEditor.h"
#include "mdtClArticleEditor.h"
#include "mdtTtTestConnectionCableEditor.h"
#include "mdtTtTestNodeEditor.h"
#include "mdtTtTestModelEditor.h"
#include "mdtTtTestModelItemEditor.h"
#include "mdtTtCableChecker.h"
#include "mdtTtCableCheckerWindow.h"
#include "mdtSqlTableWidget.h"
#include <boost/concept_check.hpp>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QFileInfo>
#include <QMutableListIterator>
#include <QCloseEvent>
#include <QTabWidget>
#include <QSqlTableModel>
#include <QSqlError>

#include <QDebug>

mdtClMainWindow::mdtClMainWindow()
{
  setupUi(this);
  // Database manager
  pvDatabaseManager = new mdtSqlDatabaseManager(this);
  pvDatabaseManager->setForDialogParentWidget(this);
  /// \todo provisoire
  initWorkDirectory();
  ///openDatabaseSqlite();
  // Central widget
  pvTabWidget = 0;
  // Editors - Will be setup on first call
  pvConnectorEditor = 0;
  pvConnectorEditorWindow = 0;
  ///pvArticleEditor = 0;
  ///pvArticleEditorWindow = 0;

  pvUnitEditor = 0;
  pvTestNodeEditor = 0;
  pvTestConnectionCableEditor = 0;
  pvLinkBeamEditor = 0;
  pvLinkBeamEditorWindow = 0;
  pvTestConnectionCableEditorWindow = 0;
  pvTestEditor = 0;
  pvTestEditorWindow = 0;
  pvTestItemEditor = 0;
  pvTestItemEditorWindow = 0;
  pvCableChecker = 0;
  pvCableCheckerWindow = 0;

  createActions();
}

mdtClMainWindow::~mdtClMainWindow()
{
}

void mdtClMainWindow::openDatabase()
{
  openDatabaseSqlite();
}

void mdtClMainWindow::closeDatabase()
{
  /// \todo Check about data saving !!
  /**
  delete pvVehicleTypeEditor;
  pvVehicleTypeEditor = 0;
  */
  delete pvConnectorEditor;
  pvConnectorEditor = 0;
  ///delete pvArticleEditor;
  ///pvArticleEditor = 0;
  delete pvUnitEditor;
  pvUnitEditor = 0;
  
  if(!deleteEditors()){
    return;
  }
  pvDatabaseManager->database().close();
}

void mdtClMainWindow::createNewDatabase()
{
  createDatabaseSqlite();
}

void mdtClMainWindow::importDatabase()
{
  importDatabaseSqlite();
}

void mdtClMainWindow::viewVehicleType()
{
  if(!displayTableView("VehicleType_tbl")){
    createVehicleTypeTableView();
  }
}

void mdtClMainWindow::editVehicleType()
{
  mdtClVehicleTypeEditor *editor;
  mdtSqlWindow *window;

  // Get or create editor
  editor = getVehicleTypeEditor();
  if(editor == 0){
    return;
  }
  // Get window
  window = getEditorWindow(editor);
  Q_ASSERT(window != 0);
  // Select and show
  Q_ASSERT(editor != 0);
  if(!editor->select()){
    displayError(editor->lastError());
    return;
  }
  window->enableNavigation();
  window->show();
  
  /**
  if(pvVehicleTypeEditor == 0){
    pvVehicleTypeEditor = new mdtClVehicleTypeEditor(this, pvDatabaseManager->database());
    if(!pvVehicleTypeEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup vehicle editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvVehicleTypeEditor;
      pvVehicleTypeEditor = 0;
      return;
    }
    pvVehicleTypeEditorWindow = new mdtSqlWindow(this);
    pvVehicleTypeEditorWindow->setSqlForm(pvVehicleTypeEditor);
    pvVehicleTypeEditorWindow->resize(800, 500);
    pvVehicleTypeEditorWindow->enableNavigation();
    pvVehicleTypeEditorWindow->enableEdition();
    pvVehicleTypeEditorWindow->setWindowTitle(tr("Vehicle type edition"));
  }
  Q_ASSERT(pvVehicleTypeEditor != 0);
  Q_ASSERT(pvVehicleTypeEditorWindow != 0);
  pvVehicleTypeEditor->select();
  pvVehicleTypeEditorWindow->show();
  */
}

void mdtClMainWindow::editConnector()
{
  if(pvConnectorEditor == 0){
    pvConnectorEditor = new mdtClConnectorEditor(this, pvDatabaseManager->database());
    if(!pvConnectorEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup connector editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvConnectorEditor;
      pvConnectorEditor = 0;
      return;
    }
    pvConnectorEditorWindow = new mdtSqlWindow(this);
    pvConnectorEditorWindow->setSqlForm(pvConnectorEditor);
    pvConnectorEditorWindow->resize(800, 500);
    pvConnectorEditorWindow->enableNavigation();
    pvConnectorEditorWindow->enableEdition();
    pvConnectorEditorWindow->setWindowTitle(tr("Connector edition"));
  }
  Q_ASSERT(pvConnectorEditor != 0);
  Q_ASSERT(pvConnectorEditorWindow != 0);
  pvConnectorEditor->select();
  pvConnectorEditorWindow->show();
}

void mdtClMainWindow::editUnit()
{
  if(pvUnitEditor == 0){
    pvUnitEditor = new mdtClUnitEditor(this, pvDatabaseManager->database());
    if(!pvUnitEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup unit editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvUnitEditor;
      pvUnitEditor = 0;
      return;
    }
    pvUnitEditorWindow = new mdtSqlWindow(this);
    pvUnitEditorWindow->setSqlForm(pvUnitEditor);
    pvUnitEditorWindow->resize(800, 500);
    pvUnitEditorWindow->enableNavigation();
    pvUnitEditorWindow->enableEdition();
    pvUnitEditorWindow->setWindowTitle(tr("Unit edition"));
  }
  Q_ASSERT(pvUnitEditor != 0);
  Q_ASSERT(pvUnitEditorWindow != 0);
  pvUnitEditor->select();
  pvUnitEditorWindow->show();
}

void mdtClMainWindow::editLinkBeam()
{
  if(pvLinkBeamEditor == 0){
    pvLinkBeamEditor = new mdtClLinkBeamEditor(this, pvDatabaseManager->database());
    if(!pvLinkBeamEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup link beam editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvLinkBeamEditor;
      pvLinkBeamEditor = 0;
      return;
    }
    pvLinkBeamEditorWindow = new mdtSqlWindow(this);
    pvLinkBeamEditorWindow->setSqlForm(pvLinkBeamEditor);
    pvLinkBeamEditorWindow->resize(800, 500);
    pvLinkBeamEditorWindow->enableNavigation();
    pvLinkBeamEditorWindow->enableEdition();
    pvLinkBeamEditorWindow->setWindowTitle(tr("Link beam edition"));
  }
  Q_ASSERT(pvLinkBeamEditor != 0);
  Q_ASSERT(pvLinkBeamEditorWindow != 0);
  pvLinkBeamEditor->select();
  pvLinkBeamEditorWindow->show();
}

void mdtClMainWindow::viewArticle()
{
  if(!displayTableView("Article_tbl")){
    createArticleTableView();
  }
}

void mdtClMainWindow::editArticle()
{
  mdtClArticleEditor *editor;
  mdtSqlWindow *window;

  // Get or create editor
  editor = getArticleEditor();
  if(editor == 0){
    return;
  }
  // Get window
  window = getEditorWindow(editor);
  Q_ASSERT(window != 0);
  // Select and show
  Q_ASSERT(editor != 0);
  if(!editor->select()){
    displayError(editor->lastError());
    return;
  }
  window->enableNavigation();
  window->show();

  
  /**
  if(pvArticleEditor == 0){
    pvArticleEditor = new mdtClArticleEditor(this, pvDatabaseManager->database());
    if(!pvArticleEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup article editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvArticleEditor;
      pvArticleEditor = 0;
      return;
    }
    pvArticleEditorWindow = new mdtSqlWindow(this);
    pvArticleEditorWindow->setSqlForm(pvArticleEditor);
    pvArticleEditorWindow->resize(800, 500);
    pvArticleEditorWindow->enableNavigation();
    pvArticleEditorWindow->enableEdition();
    pvArticleEditorWindow->setWindowTitle(tr("Article edition"));
  }
  Q_ASSERT(pvArticleEditor != 0);
  Q_ASSERT(pvArticleEditorWindow != 0);
  pvArticleEditor->select();
  pvArticleEditorWindow->show();
  */
}

void mdtClMainWindow::editTestConnectionCable()
{
  if(pvTestConnectionCableEditor == 0){
    pvTestConnectionCableEditor = new mdtTtTestConnectionCableEditor(this, pvDatabaseManager->database());
    if(!pvTestConnectionCableEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup test connection cable editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvTestConnectionCableEditor;
      pvTestConnectionCableEditor = 0;
      return;
    }
    pvTestConnectionCableEditorWindow = new mdtSqlWindow(this);
    pvTestConnectionCableEditorWindow->setSqlForm(pvTestConnectionCableEditor);
    pvTestConnectionCableEditorWindow->resize(800, 500);
    pvTestConnectionCableEditorWindow->enableNavigation();
    pvTestConnectionCableEditorWindow->enableEdition();
    pvTestConnectionCableEditorWindow->setWindowTitle(tr("Test cable edition"));
  }
  Q_ASSERT(pvTestConnectionCableEditor != 0);
  Q_ASSERT(pvTestConnectionCableEditorWindow != 0);
  pvTestConnectionCableEditor->select();
  pvTestConnectionCableEditorWindow->show();
}

void mdtClMainWindow::editTestNode()
{
  if(pvTestNodeEditor == 0){
    pvTestNodeEditor = new mdtTtTestNodeEditor(this, pvDatabaseManager->database());
    if(!pvTestNodeEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup test node editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvTestNodeEditor;
      pvTestNodeEditor = 0;
      return;
    }
    pvTestNodeEditorWindow = new mdtSqlWindow(this);
    pvTestNodeEditorWindow->setSqlForm(pvTestNodeEditor);
    pvTestNodeEditorWindow->resize(800, 500);
    pvTestNodeEditorWindow->enableNavigation();
    pvTestNodeEditorWindow->enableEdition();
    pvTestNodeEditorWindow->setWindowTitle(tr("Test node edition"));
  }
  Q_ASSERT(pvTestNodeEditor != 0);
  Q_ASSERT(pvTestNodeEditorWindow != 0);
  pvTestNodeEditor->select();
  pvTestNodeEditorWindow->show();
}

void mdtClMainWindow::editTest()
{
  if(pvTestEditor == 0){
    pvTestEditor = new mdtTtTestModelEditor(this, pvDatabaseManager->database());
    if(!pvTestEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup test editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvTestEditor;
      pvTestEditor = 0;
      return;
    }
    pvTestEditorWindow = new mdtSqlWindow(this);
    pvTestEditorWindow->setSqlForm(pvTestEditor);
    pvTestEditorWindow->resize(800, 500);
    pvTestEditorWindow->enableNavigation();
    pvTestEditorWindow->enableEdition();
  }
  Q_ASSERT(pvTestEditor != 0);
  Q_ASSERT(pvTestEditorWindow != 0);
  pvTestEditor->select();
  pvTestEditorWindow->show();
}

void mdtClMainWindow::editTestItem()
{
  if(pvTestItemEditor == 0){
    pvTestItemEditor = new mdtTtTestModelItemEditor(0, pvDatabaseManager->database());
    if(!pvTestItemEditor->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup test item editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvTestItemEditor;
      pvTestItemEditor = 0;
      return;
    }
    pvTestItemEditorWindow = new mdtSqlWindow(this);
    pvTestItemEditorWindow->setSqlForm(pvTestItemEditor);
    pvTestItemEditorWindow->resize(800, 500);
    pvTestItemEditorWindow->enableNavigation();
    pvTestItemEditorWindow->enableEdition();
  }
  Q_ASSERT(pvTestItemEditor != 0);
  Q_ASSERT(pvTestItemEditorWindow != 0);
  pvTestItemEditor->select();
  pvTestItemEditorWindow->show();
}

void mdtClMainWindow::runCableChecker()
{
  if(pvCableChecker == 0){
    pvCableChecker = new mdtTtCableChecker(0, pvDatabaseManager->database());
    if(!pvCableChecker->setupTables()){
      QMessageBox msgBox(this);
      msgBox.setText(tr("Cannot setup test item editor."));
      msgBox.setInformativeText(tr("This can happen if selected database has wrong format (is also not a database made for ")\
                                + qApp->applicationName() + tr(")"));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      delete pvCableChecker;
      pvCableChecker = 0;
      return;
    }
    pvCableCheckerWindow = new mdtTtCableCheckerWindow(this);
    pvCableCheckerWindow->setCableChecker(pvCableChecker);
    pvCableCheckerWindow->resize(800, 500);
  }
  Q_ASSERT(pvCableChecker != 0);
  Q_ASSERT(pvCableCheckerWindow != 0);
  pvCableChecker->select();
  pvCableCheckerWindow->show();
}

/**
void mdtClMainWindow::createTestConnectionCable()
{
  mdtTtTestConnectionCableEditor editor(this, pvDatabaseManager->database());

  editor.createCable();
}

void mdtClMainWindow::connectTestCable()
{
  mdtTtTestConnectionCableEditor editor(this, pvDatabaseManager->database());

  editor.connectTestCable();
}

void mdtClMainWindow::disconnectTestCable()
{
  mdtTtTestConnectionCableEditor editor(this, pvDatabaseManager->database());

  editor.disconnectTestCable();
}
*/

bool mdtClMainWindow::createVehicleTypeTableView()
{
  mdtSqlTableWidget *tableWidget;

  tableWidget = createTableView("VehicleType_tbl", tr("Vehicle types"));
  if(tableWidget == 0){
    return false;
  }

  return true;
}

mdtClVehicleTypeEditor* mdtClMainWindow::getVehicleTypeEditor()
{
  mdtClVehicleTypeEditor *editor;

  editor = getEditor<mdtClVehicleTypeEditor>();
  if(editor != 0){
    return editor;
  }else{
    return createVehicleTypeEditor();
  }
}

mdtClVehicleTypeEditor *mdtClMainWindow::createVehicleTypeEditor()
{
  mdtClVehicleTypeEditor *editor;
  mdtSqlWindow *window;

  editor = new mdtClVehicleTypeEditor(0, pvDatabaseManager->database());
  window = setupEditor(editor);
  if(window == 0){
    return 0;
  }
  window->setWindowTitle(tr("Vehicle type edition"));
  window->resize(800, 600);

  return editor;
}

bool mdtClMainWindow::createArticleTableView()
{
  mdtSqlTableWidget *tableWidget;

  tableWidget = createTableView("Article_tbl", tr("Articles"));
  if(tableWidget == 0){
    return false;
  }

  return true;
}

mdtClArticleEditor *mdtClMainWindow::getArticleEditor()
{
  mdtClArticleEditor *editor;

  editor = getEditor<mdtClArticleEditor>();
  if(editor != 0){
    return editor;
  }else{
    return createArticleEditor();
  }
}

mdtClArticleEditor *mdtClMainWindow::createArticleEditor()
{
  mdtClArticleEditor *editor;
  mdtSqlWindow *window;

  editor = new mdtClArticleEditor(0, pvDatabaseManager->database());
  window = setupEditor(editor);
  if(window == 0){
    return 0;
  }
  window->setWindowTitle(tr("Article edition"));
  window->resize(800, 600);

  return editor;
}

mdtSqlTableWidget *mdtClMainWindow::createTableView(const QString & tableName, const QString & userFriendlyTableName)
{
  mdtSqlTableWidget *tableWidget;
  QSqlTableModel *model;
  QString uftn;

  // Check that we have currently a database open
  if(!pvDatabaseManager->database().isOpen()){
    displayWarning(tr("No database is open."), tr("Please open a database and try again."));
    return 0;
  }
  // Set user friendly table name
  if(userFriendlyTableName.isEmpty()){
    uftn = tableName;
  }else{
    uftn = userFriendlyTableName;
  }
  // Setup model
  model = new QSqlTableModel(this, pvDatabaseManager->database());
  model->setTable(tableName);
  if(!model->select()){
    QSqlError sqlError = model->lastError();
    mdtError e(tr("Unable to select data in table '") + tableName + tr("'"), mdtError::Error);
    e.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(e, "mdtClMainWindow");
    e.commit();
    displayError(e);
    delete model;
    return 0;
  }
  // Setup widget
  tableWidget = new mdtSqlTableWidget;
  tableWidget->setModel(model);
  tableWidget->setUserFriendlyTableName(uftn);
  // Setup tab widget (if needed)
  if(pvTabWidget == 0){
    pvTabWidget = new QTabWidget;
    pvTabWidget->setTabsClosable(true);
    setCentralWidget(pvTabWidget);  /// \todo Check if current central widget deleted ..
  }
  // Add view
  pvTabWidget->addTab(tableWidget, uftn);
  pvOpenViews.append(tableWidget);

  return tableWidget;
}

bool mdtClMainWindow::displayTableView(const QString& tableName)
{
  mdtSqlTableWidget *tableWidget;
  QSqlTableModel *model;
  int i;

  // Search requested table widget
  for(i = 0; i < pvOpenViews.size(); ++i){
    tableWidget = pvOpenViews.at(i);
    Q_ASSERT(tableWidget != 0);
    model = tableWidget->model();
    Q_ASSERT(model != 0);
    if(model->tableName() == tableName){
      // We have the requested widget, refresh data
      if(!model->select()){
        QSqlError sqlError = model->lastError();
        mdtError e(tr("Unable to select data in table '") + tableName + tr("'"), mdtError::Error);
        e.setSystemError(sqlError.number(), sqlError.text());
        MDT_ERROR_SET_SRC(e, "mdtClMainWindow");
        e.commit();
        return false;
      }
      // Show the widget - If a view exists in pvOpenViews, tab widget was allready created
      Q_ASSERT(pvTabWidget != 0);
      pvTabWidget->setCurrentWidget(tableWidget);
      return true;
    }
  }
  // Requested view was not found
  return false;
}

mdtSqlWindow *mdtClMainWindow::setupEditor(mdtSqlForm *editor)
{
  Q_ASSERT(editor != 0);

  mdtSqlWindow *window;

  // Setup editor
  if(!editor->setupTables()){
    displayError(editor->lastError());
    delete editor;
    return 0;
  }
  // Setup window
  window = new mdtSqlWindow(this);
  window->setSqlForm(editor);
  window->enableEdition();
  pvOpenEditors.append(window);

  return window;
}

template <class T> T *mdtClMainWindow::getEditor()
{
  int i;
  mdtSqlForm *form;
  T *editor;

  for(i = 0; i < pvOpenEditors.size(); ++i){
    form = pvOpenEditors.at(i)->sqlForm();
    if(form != 0){
      editor = dynamic_cast<T*>(form);
      if(editor != 0){
        return editor;
      }
    }
  }

  return 0;
}

mdtSqlWindow *mdtClMainWindow::getEditorWindow(mdtSqlForm *editor)
{
  int i;

  for(i = 0; i < pvOpenEditors.size(); ++i){
    if(pvOpenEditors.at(i)->sqlForm() == editor){
      return pvOpenEditors.at(i);
    }
  }

  return 0;
}

bool mdtClMainWindow::deleteEditors()
{
  QMutableListIterator<mdtSqlWindow*> it(pvOpenEditors);
  mdtSqlWindow *window;
  mdtSqlForm *form;

  while(it.hasNext()){
    window = it.next();
    Q_ASSERT(window != 0);
    form = window->sqlForm();
    if(form != 0){
      if(form->allDataAreSaved()){
        it.remove();
        delete window;
      }else{
        break;
      }
    }
  }

  return pvOpenEditors.isEmpty();
}

void mdtClMainWindow::closeEvent(QCloseEvent* event)
{
  Q_ASSERT(event != 0);

  if(!deleteEditors()){
    event->ignore();
  }else{
    event->accept();
  }
}

void mdtClMainWindow::createActions()
{
  // Open database
  connect(actOpenDatabase, SIGNAL(triggered()), this, SLOT(openDatabase()));
  // Close database
  connect(actCloseDatabase, SIGNAL(triggered()), this, SLOT(closeDatabase()));
  // Create new database
  connect(actCreateNewDatabase, SIGNAL(triggered()), this, SLOT(createNewDatabase()));
  // Import database
  connect(actImportDatabase, SIGNAL(triggered()), this, SLOT(importDatabase()));

  // Article edition
  connect(actViewArticle, SIGNAL(triggered()), this, SLOT(viewArticle()));
  connect(actEditArticle, SIGNAL(triggered()), this, SLOT(editArticle()));
  ///connect(pbEditArticle, SIGNAL(clicked()), this, SLOT(editArticle()));
  // Vehicle type edition
  connect(actViewVehicleType, SIGNAL(triggered()), this, SLOT(viewVehicleType()));
  connect(actEditVehicleType, SIGNAL(triggered()), this, SLOT(editVehicleType()));
  connect(pbEditVehicleType, SIGNAL(clicked()), this, SLOT(editVehicleType()));
  // Connector editor
  connect(actEditConnector, SIGNAL(triggered()), this, SLOT(editConnector()));
  // Unit edition
  connect(actEditUnit, SIGNAL(triggered()), this, SLOT(editUnit()));
  connect(pbEditUnit, SIGNAL(clicked()), this, SLOT(editUnit()));

  // Link beam edition
  connect(actEditLinkBeam, SIGNAL(triggered()), this, SLOT(editLinkBeam()));

  // Test connection cable
  connect(actEditTestConnectionCable, SIGNAL(triggered()), this, SLOT(editTestConnectionCable()));
  /**
  connect(actCreateTestConnectionCable, SIGNAL(triggered()), this, SLOT(createTestConnectionCable()));
  connect(actConnectTestCable, SIGNAL(triggered()), this, SLOT(connectTestCable()));
  connect(actDisconnectTestCable, SIGNAL(triggered()), this, SLOT(disconnectTestCable()));
  */
  // Test node edition
  connect(actEditTestNode, SIGNAL(triggered()), this, SLOT(editTestNode()));
  // Test edition
  connect(actEditTest, SIGNAL(triggered()), this, SLOT(editTest()));
  // Test item edition
  connect(actEditTestItem, SIGNAL(triggered()), this, SLOT(editTestItem()));
  // Cable checker
  connect(actCableChecker, SIGNAL(triggered()), this, SLOT(runCableChecker()));

}

bool mdtClMainWindow::initWorkDirectory()
{
  pvWorkDirectory = QDir::home();
  if(!pvWorkDirectory.cd("mdtcablelist")){
    if(!pvWorkDirectory.mkdir("mdtcablelist")){
      QMessageBox msgBox(this);
      mdtError e(tr("Cannot create 'mdtcablelist' directory."), mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtClMainWindow");
      e.commit();
      msgBox.setText(e.text());
      /// \todo Let the user a chance to choose another place to create this directory !
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      return false;
    }
    if(!pvWorkDirectory.cd("mdtcablelist")){
      ///QMessageBox msgBox(this);
      mdtError e(tr("Cannot create 'mdtcablelist' directory."), mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtClMainWindow");
      e.commit();
      displayWarning(e.text());
      /**
      msgBox.setText(e.text());
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      */
      return false;
    }
  }

  return true;
}

bool mdtClMainWindow::openDatabaseSqlite()
{
  QFileInfo fileInfo;

  // Check that no database is allready open
  if(pvDatabaseManager->database().isOpen()){
    displayWarning(tr("A database is allready open."), tr("Close current database and try again."));
    return false;
  }
  // Let the user choose a file
  fileInfo = pvDatabaseManager->chooseDatabaseSqlite(pvWorkDirectory);
  if(fileInfo.fileName().isEmpty()){
    return false;
  }
  /// \todo Mange, let user choose, etc...
  ///QFileInfo fileInfo(pvWorkDirectory, "test01.sqlite");
  // Open database
  if(!pvDatabaseManager->openDatabaseSqlite(fileInfo)){
    displayWarning(pvDatabaseManager->lastError().text());
    /// \todo Dangerous !!
    /**
    mdtTtDatabaseSchema dbSchema(pvDatabaseManager);
    if(!dbSchema.createSchemaSqlite(fileInfo)){
      QMessageBox msgBox(this);
      mdtError e(tr("Cannot create database schema."), mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtClMainWindow");
      e.commit();
      msgBox.setText(e.text());
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      return false;
    }
    */
    return false;
  }
  // Check that we have open a cable list schema
  mdtTtDatabaseSchema dbSchema(pvDatabaseManager);
  if(!dbSchema.checkSchema()){
    displayWarning(tr("Choosen file does not contain a valid cable list database."));
    closeDatabase();
    return false;
  }

  return true;
}

bool mdtClMainWindow::createDatabaseSqlite()
{
  // Check that no database is allready open
  if(pvDatabaseManager->database().isOpen()){
    displayWarning(tr("A database is allready open."), tr("Close current database and try again."));
    return false;
  }
  // Create database
  mdtTtDatabaseSchema dbSchema(pvDatabaseManager);
  if(!dbSchema.createSchemaSqlite(pvWorkDirectory)){
    QMessageBox msgBox(this);
    msgBox.setText(tr("Database creation failed.") + "                ");
    msgBox.setDetailedText(dbSchema.lastError().text());
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return false;
  }

  return true;
}

bool mdtClMainWindow::importDatabaseSqlite()
{
  // Check that we have currently a database open
  if(!pvDatabaseManager->database().isOpen()){
    displayWarning(tr("Cannot import a database."), tr("Please open a database and try again."));
    return false;
  }
  /// \todo Save current edition !
  /// \todo Create a backup from current database !
  // Let the user a chance to cancel import
  QMessageBox msgBox(this);
  msgBox.setText(tr("You are about to import a database into current database."));
  msgBox.setInformativeText(tr("Do you want to continue ?"));
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if(msgBox.exec() != QMessageBox::Yes){
    return false;
  }
  // Import ...
  mdtTtDatabaseSchema dbSchema(pvDatabaseManager);

  return dbSchema.importDatabase(pvWorkDirectory);
}

void mdtClMainWindow::displayWarning(const QString & text , const QString & informativeText)
{
  QMessageBox msgBox(this);
  msgBox.setText(text);
  msgBox.setInformativeText(informativeText);
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.exec();
}

void mdtClMainWindow::displayError(const mdtError & error)
{
  QMessageBox msgBox;

  msgBox.setText(error.text());
  msgBox.setDetailedText(error.systemText());
  msgBox.setIcon(error.levelIcon());
  msgBox.exec();
}
