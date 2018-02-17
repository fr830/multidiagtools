/****************************************************************************
 **
 ** Copyright (C) 2011-2018 Philippe Steinmann.
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
#include "TypeTraitsTest.h"
#include "Mdt/Entity/Def.h"
#include "Mdt/Entity/TypeTraits/IsEntityFieldDef.h"
#include "Mdt/Entity/TypeTraits/IsEntityDef.h"

using namespace Mdt::Entity;
using namespace Mdt::Entity::TypeTraits;

void TypeTraitsTest::initTestCase()
{
}

void TypeTraitsTest::cleanupTestCase()
{
}

/*
 * Entities
 */

struct ArticleDataStruct
{
  qlonglong id;
  QString description;
};

MDT_ENTITY_DEF(
  (ArticleDataStruct),
  Article,
  (id, FieldFlag::IsPrimaryKey),
  (description, FieldFlag::IsRequired, FieldMaxLength(250))
)

/*
 * IsEntityDef compile time tests
 */

static_assert(!IsEntityDef<ArticleDataStruct>::value, "" );
static_assert( IsEntityDef<ArticleDef>::value, "" );
static_assert(!IsEntityDef<ArticleDef::idField>::value , "" );
static_assert(!IsEntityDef<ArticleDef::descriptionField>::value , "" );
static_assert(!IsEntityDef<int>::value, "" );
static_assert(!IsEntityDef<QString>::value, "" );

/*
 * IsEntityFieldDef compile time tests
 */

static_assert( IsEntityFieldDef<ArticleDef::idField>::value , "" );
static_assert( IsEntityFieldDef<ArticleDef::descriptionField>::value , "" );
static_assert(!IsEntityFieldDef<ArticleDataStruct>::value , "" );
static_assert(!IsEntityFieldDef<ArticleDef>::value , "" );
static_assert(!IsEntityFieldDef<int>::value , "" );
static_assert(!IsEntityFieldDef<QString>::value , "" );

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::CoreApplication app(argc, argv);
  TypeTraitsTest test;

//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
