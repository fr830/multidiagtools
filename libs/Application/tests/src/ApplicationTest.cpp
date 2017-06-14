/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
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
#include "ApplicationTest.h"
#include "Mdt/Application.h"
#include "Mdt/Error.h"

void ApplicationTest::initTestCase()
{
}

void ApplicationTest::cleanupTestCase()
{
}

/*
 * Tests
 */

void ApplicationTest::coreDumpAfterErrorCommitBugTest()
{
  /*
   * Bug was in Mdt::Application,
   * which create a Logger, but never called Logger::cleanup().
   * Now fixed.
   */
  auto error = mdtErrorNew("test", Mdt::Error::Critical, "MyClass");
  error.commit();
}


/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  ApplicationTest test;

  if(!app.init()){
    return 1;
  }
  app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
