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
#ifndef MDT_MODBUS_IO_TOOL_H
#define MDT_MODBUS_IO_TOOL_H

#include "ui_mdtModbusIoTool.h"
#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QActionGroup>

class mdtDeviceModbusWago;
class mdtDeviceIos;
class mdtDeviceIosWidget;
class mdtPortStatusWidget;

/*! \brief Minimal tool for MODBUS I/O
 */
class mdtModbusIoTool : public QMainWindow, public Ui::mdtModbusIoTool
{
 Q_OBJECT

 public:

  /*! \brief Construct a mdtModbusIoTool object
   */
  mdtModbusIoTool(QWidget *parent = 0, Qt::WindowFlags flags = 0);

  /*! \brief destruct a mdtModbusIoTool object
   */
  ~mdtModbusIoTool();

  /*! \brief Build the translations menu
   */
  void setAvailableTranslations(const QMap<QString, QString> &avaliableTranslations, const QString &currentTranslationKey);

 public slots:

  /*! \brief Retranslate
   *
   * This slot is called by mdtApplication
   *  when language has changed
   */
  void retranslate();

 private slots:

  /*! \brief Update current state
   */
  void setState(int state);

  /*! \brief Call setup dialog
   */
  void setup();

  /*! \brief Connect to a device with given hardware node ID
   *
   * Node ID will be readen from sbHwNodeId spin box.
   */
  void connectToNode();

  /*! \brief Disconnect from device
   */
  void disconnectFromNode();

  /*! \brief Used to show a message in status bar
   *
   * \param message Message to show
   * \param timeout If > 0, message will be cleared after timeout [ms]
   */
  void showStatusMessage(const QString &message, int timeout = 0);

  /*! \brief Used to show a message in status bar
   *
   * \param message Message to show
   * \param details Details to show
   * \param timeout If > 0, message will be cleared after timeout [ms]
   */
  void showStatusMessage(const QString &message, const QString &details, int timeout = 0);

 private:

  /*! \brief Set the disconnected state
   */
  void setStateDisconnected();

  /*! \brief Set the connecting state
   */
  void setStateConnecting();

  /*! \brief Set the ready state
   */
  void setStateReady();

  /*! \brief Set the busy state
   */
  void setStateBusy();

  /*! \brief Set the warning state
   */
  void setStateWarning();

  /*! \brief Set the error state
   */
  void setStateError();

  bool pvReady;
  mdtDeviceModbusWago *pvDeviceModbusWago;
  mdtDeviceIos *pvDeviceIos;
  mdtDeviceIosWidget *pvDeviceIosWidget;
  mdtPortStatusWidget *pvStatusWidget;
  // Translations menu
  QActionGroup *pvLanguageActionGroup;
  // Diseable copy
  Q_DISABLE_COPY(mdtModbusIoTool);
};

#endif  // #ifndef MDT_MODBUS_IO_TOOL_H
