/****************************************************************************
 **
 ** Copyright (C) 2011-2016 Philippe Steinmann.
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
#ifndef ADDRESS_WIDGET_H
#define ADDRESS_WIDGET_H

#include "Mdt/ItemEditor/TableViewWidget.h"

/*! \brief Table view based widget to edit addresses
 */
class AddressWidget : public Mdt::ItemEditor::TableViewWidget
{
 Q_OBJECT

 public:

  explicit AddressWidget(QWidget* parent = nullptr);
};

#endif // #ifndef ADDRESS_WIDGET_H
