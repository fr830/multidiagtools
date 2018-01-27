/****************************************************************************
 **
 ** Copyright (C) 2011-2018 Philippe Steinmann.
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
#ifndef MDT_ENTITY_DEBUG_H
#define MDT_ENTITY_DEBUG_H

#include "MdtEntity_CoreExport.h"
#include "FieldAttributes.h"
#include <QString>
#include <QLatin1String>
#include <QVariant>
#include <boost/fusion/include/sequence.hpp>

namespace Mdt{ namespace Entity{

  MDT_ENTITY_CORE_EXPORT void printStringToConsole(const QString & str);

  MDT_ENTITY_CORE_EXPORT const QString stringFromBool(bool b);

  template<typename Data>
  struct PrintFieldToConsole
  {
    PrintFieldToConsole(const Data & data)
    : mData(data)
    {
    }

    template<typename Key>
    void operator()(const Key &) const
    {
      printStringToConsole(QLatin1String(" field ") + Key::fieldName() + QLatin1String(":"));
      static const auto attributes = Key::fieldAttributes();
      printStringToConsole(QLatin1String("  required: ") + stringFromBool(attributes.isRequired()));
      printStringToConsole(QLatin1String("  unique: ") + stringFromBool(attributes.isUnique()));
      printStringToConsole(QLatin1String("  max length: ") + QString::number(attributes.maxLength()));
      printStringToConsole(QLatin1String("  value: ") + QVariant(boost::fusion::at_key<Key>(mData)).toString());
    }

    const Data & mData;
  };

  /*! \brief Print entity data to the console
   *
   * \a data must have the following methods:
   * \code
   * const EntityDef def() const;
   * const DataStruct & constDataStruct() const;
   * \endcode
   * EntityDef is a struct generated by MDT_ENTITY_DEF() ,
   * and DataStruct is the struct used to generate EntityDef .
   */
  template<typename T>
  void printEntityDataToConsole(const T & data)
  {
    printStringToConsole(QLatin1String("Entity: ") + data.def().entityName());
    boost::fusion::for_each(data.def(), PrintFieldToConsole<decltype(data.constDataStruct())>(data.constDataStruct()));
  }

  

}} // namespace Mdt{ namespace Entity{

#endif // #ifndef MDT_ENTITY_DEBUG_H
