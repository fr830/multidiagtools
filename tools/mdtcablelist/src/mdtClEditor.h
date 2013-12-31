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
#ifndef MDT_CL_EDITOR_H
#define MDT_CL_EDITOR_H

#include "mdtError.h"
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class mdtSqlWindowOld;
class mdtSqlFormWindow;
class mdtSqlFormDialog;
class mdtSqlFormOld;
class QWidget;

/*! \brief Base class for editors
 *
 * This class is base for several editors.
 *  Goal is to provide a common way for callers
 *  when using a editor (such as show, setup, ...) .
 */
class mdtClEditor : public QObject
{
 Q_OBJECT

 public:

  /*! \brief Constructor
   */
  mdtClEditor(QObject *parent, QSqlDatabase db);

  /*! \brief Destructor
   */
  virtual ~mdtClEditor();

  /*! \brief Setup editor as window
   *
   * The widgets are layed out in a QMainWindow .
   *
   * Will also call setupTables() .
   *
   * \pre Setup must be done only once .
   */
  bool setupAsWindow();

  /*! \brief Setup editor as dialog
   *
   * The widgets are layed out in a dialog .
   *
   * Will also call setupTables() .
   *
   * \param parent Used the same way than in QDialog .
   * \pre Setup must be done only once .
   */
  bool setupAsDialog(QWidget *parent = 0);

  /*! \brief Get database instance
   */
  QSqlDatabase &database();

  /*! \brief Show editor
   */
  void show();

  /*! \brief Show editor and block caller
   *
   * Works the same way than a QDialog .
   *
   * \pre Setup must be done as dialog (see setupAsDialog() ) .
   */
  int exec();

  /*! \brief Select given tableName
   */
  bool select(const QString & tableName);

  /*! \brief Get last error
   */
  mdtError lastError() const;

 protected:

  /*! \brief Access form object
   *
   * Will return a valid pointer only after setup is done .
   *
   * \sa setupAsWindow()
   * \sa setupAsDialog()
   */
  mdtSqlFormOld *form();

  /*! \brief Access SQL window object
   *
   * Will return a valid pointer only after setupAsWindow() was called .
   */
  mdtSqlWindowOld *sqlWindow();

  /*! \brief Setup tables
   *
   * Must be re-implemented in subclass .
   */
  virtual bool setupTables() = 0;

  /*! \brief Display last error in a message box
   */
  void displayLastError();

  mdtError pvLastError;

 private:

  Q_DISABLE_COPY(mdtClEditor);

  mdtSqlFormOld *pvForm;
  mdtSqlFormWindow *pvFormWindow;
  mdtSqlFormDialog *pvFormDialog;
  QSqlDatabase pvDatabase;
};

#endif // #ifndef MDT_CL_EDITOR_H
