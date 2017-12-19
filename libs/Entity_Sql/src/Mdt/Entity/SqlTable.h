/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
 **
 ** This file is part of Mdt library.
 **
 ** Mdt is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** Mdt is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with Mdt.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#ifndef MDT_ENTITY_SQL_TABLE_H
#define MDT_ENTITY_SQL_TABLE_H

#include "Mdt/Sql/Schema/Table.h"
#include "MdtEntity_SqlExport.h"

namespace Mdt{ namespace Entity{

  /*! \brief Helper to reflect a entity to a SQL table
   */
  class MDT_ENTITY_SQL_EXPORT SqlTable
  {
   public:

    /*! \brief Create a SQL table from \a entity
     */
    static Mdt::Sql::Schema::Table fromEntity();
  };

}} // namespace Mdt{ namespace Entity{

#endif // #ifndef MDT_ENTITY_SQL_TABLE_H
