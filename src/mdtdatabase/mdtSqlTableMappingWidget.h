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
#ifndef MDT_SQL_TABLE_MAPPING_WIDGET_H
#define MDT_SQL_TABLE_MAPPING_WIDGET_H

#include "ui_mdtSqlTableMappingWidget.h"
#include <QWidget>

class mdtSqlTableMappingWidgetItem;
class QVBoxLayout;

/*! \brief Widget that provides edition of mapping between source and destination tables
 */
class mdtSqlTableMappingWidget : public QWidget, Ui::mdtSqlTableMappingWidget
{
 friend class mdtSqlTableMappingWidgetItem;

 Q_OBJECT

 public:

  /*! \brief Constructor
   */
  mdtSqlTableMappingWidget(QWidget *parent = nullptr);

 private slots:

  /*! \brief Edit a field mapping item
   */
  void addFieldMapping();

 private:

  /*! \brief Edit a field mapping
   */
  void editFieldMapping(mdtSqlTableMappingWidgetItem *item);

  /*! \brief Edit a field mapping item
   */
  void removeFieldMapping(mdtSqlTableMappingWidgetItem *item);

  Q_DISABLE_COPY(mdtSqlTableMappingWidget)

  QWidget *pvItemsContainerWidget;
  QVBoxLayout *pvItemsContainerLayout;
};

#endif // #ifndef MDT_SQL_TABLE_MAPPING_WIDGET_H
