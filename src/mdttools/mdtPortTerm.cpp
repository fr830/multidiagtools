/****************************************************************************
 **
 ** Copyright (C) 2011-2012 Philippe Steinmann.
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
#include "mdtPortTerm.h"
#include "mdtApplication.h"
#include <QHBoxLayout>
#include <QAction>

#include <QDebug>

mdtPortTerm::mdtPortTerm(QWidget *parent)
 : QMainWindow(parent)
{
  setupUi(this);
  // Serial port members
  pvSerialPortManager = 0;
  pvSerialPortCtlWidget = 0;
  // Current port manager
  pvCurrentPortManager = 0;
  
  connect(pbSendCmd, SIGNAL(clicked()), this, SLOT(sendCmd()));
  
  attachToSerialPort();
  
  // Actions
  connect(action_Setup, SIGNAL(triggered()), this, SLOT(serialPortSetup()));
  pvLanguageActionGroup = 0;
}

mdtPortTerm::~mdtPortTerm()
{
  detachFromSerialPort();
}

void mdtPortTerm::setAvailableTranslations(QMap<QString, QString> &avaliableTranslations, const QString &currentTranslationKey)
{
  QMap<QString, QString>::const_iterator it = avaliableTranslations.constBegin();

  // Create a action group
  if(pvLanguageActionGroup == 0){
    pvLanguageActionGroup = new QActionGroup(this);
    connect(pvLanguageActionGroup, SIGNAL(triggered(QAction*)), mdtApp, SLOT(changeLanguage(QAction*)));
  }
  // Travel available translation and add actions to menu + group
  while(it != avaliableTranslations.constEnd()){
    QAction *action = new QAction(it.value(), this);
    action->setCheckable(true);
    action->setData(it.key());
    if(it.key() == currentTranslationKey){
      action->setChecked(true);
    }
    menu_Language->addAction(action);
    pvLanguageActionGroup->addAction(action);
    it++;
  }
}

void mdtPortTerm::appendReadenData()
{
  if(pvCurrentPortManager == 0){
    qDebug() << "TERM: err, pvCurrentPortManager == 0";
    return;
  }
  teTerm->append(pvCurrentPortManager->lastReadenFrame());
}

void mdtPortTerm::sendCmd()
{
  QString cmd;

  if(pvCurrentPortManager == 0){
    qDebug() << "TERM: err, pvCurrentPortManager == 0";
    return;
  }
  if(teCmd->toPlainText().size() < 1){
    return;
  }
  /// NOTE: essais
  cmd = teCmd->toPlainText();
  //cmd.remove('\n');
  cmd.append((char)0x0D);
  //cmd.append((char)0x04);
  pvCurrentPortManager->writeData(cmd.toAscii());
  teCmd->clear();
}

void mdtPortTerm::on_pbSendCmdAbort_clicked()
{
  if(pvSerialPortManager != 0){
    pvSerialPortManager->port().flushOut();
    pvSerialPortManager->port().flushIn();
  }
  teCmd->clear();
}

void mdtPortTerm::on_pbClearTerm_clicked()
{
  teTerm->clear();
}

void mdtPortTerm::retranslate()
{
  retranslateUi(this);
}

void mdtPortTerm::attachToSerialPort()
{
  // Create objects
  pvSerialPortManager = new mdtSerialPortManager;
  pvSerialPortCtlWidget = new mdtSerialPortCtlWidget;
  pvCurrentPortManager = pvSerialPortManager;
  // Ctl widget
  bottomHLayout->insertWidget(0, pvSerialPortCtlWidget);
  pvSerialPortCtlWidget->makeConnections(pvSerialPortManager);

  connect(pvSerialPortManager, SIGNAL(newDataReaden()), this, SLOT(appendReadenData()));
}

void mdtPortTerm::detachFromSerialPort()
{
  if(pvSerialPortManager != 0){
    delete pvSerialPortManager;
    pvSerialPortManager = 0;
  }
  /// NOTE: à compléter
}

void mdtPortTerm::serialPortSetup()
{
  // Show setup dialog
  if(pvSerialPortManager != 0){
    mdtSerialPortSetupDialog d(this);
    d.setPortManager(pvSerialPortManager);
    d.exec();
  }
}
