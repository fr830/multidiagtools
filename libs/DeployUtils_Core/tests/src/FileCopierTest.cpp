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
#include "FileCopierTest.h"
#include "Mdt/DeployUtils/FileCopier.h"
#include "Mdt/DeployUtils/LibraryInfo.h"
#include "Mdt/DeployUtils/QtPluginInfo.h"
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <QDebug>

using namespace Mdt::DeployUtils;

void FileCopierTest::initTestCase()
{
}

void FileCopierTest::cleanupTestCase()
{
}

/*
 * Tests
 */

void FileCopierTest::createDirectoryTest()
{
  FileCopier fc;
  QTemporaryDir root;
  QString path;

  QVERIFY(root.isValid());
  /*
   * Create a directory
   * by giving a full path
   */
  path = QDir::cleanPath( root.path() + "/dirA" );
  QVERIFY(fc.createDirectory(path));
  QDir dirA(path);
  QCOMPARE(dirA.absolutePath(), path);
  QVERIFY(dirA.exists());
  /*
   * Try to create a directory
   * by giving a path to a existing file
   */
  // Create a file
  path = QDir::cleanPath( root.path() + "/fileB" );
  QFile fileB(path);
  QVERIFY( fileB.open(QFile::WriteOnly) );
  QVERIFY( fileB.write("B") > 0 );
  fileB.close();
  // Try to create the directory
  QVERIFY(!fc.createDirectory(path));
  // Check that file was untouched
  QVERIFY( fileB.open(QFile::ReadOnly) );
  QCOMPARE( fileB.readAll(), QByteArray("B") );
}

void FileCopierTest::copyLibrariesTest()
{
  FileCopier fc;
  QTemporaryDir sourceRoot;
  QTemporaryDir destinationRoot;

  QVERIFY(sourceRoot.isValid());
  QVERIFY(destinationRoot.isValid());

  /*
   * Create library info list
   * and create realted source files
   */
  const auto libraryInfoList = createLibraryInfoList(QStringList{"libA.so", "libB.so"}, sourceRoot.path());
  for(const auto & li : libraryInfoList){
    QVERIFY( createFile(li.absoluteFilePath()) );
    QFileInfo fi(li.absoluteFilePath());
    QVERIFY(fi.exists());
  }
  /*
   * Run copy and check
   */
  const QString destinationDirectoryPath = QDir::cleanPath( destinationRoot.path() + "usr/lib/" );
  QVERIFY(fc.copyLibraries(libraryInfoList, destinationDirectoryPath));
  for(const auto & li : libraryInfoList){
    const QString filePath = QDir::cleanPath( destinationDirectoryPath + "/" + li.libraryName().toFullNameLinux() );
    QFileInfo fi(filePath);
    QVERIFY(fi.exists());
  }
  // Run the copy a second time (copier must not fail if destination file exists)
  QVERIFY(fc.copyLibraries(libraryInfoList, destinationDirectoryPath));
}

void FileCopierTest::copyQtPluginsTest()
{
  FileCopier fc;
  QTemporaryDir sourceRoot;
  QTemporaryDir destinationRoot;

  QVERIFY(sourceRoot.isValid());
  QVERIFY(destinationRoot.isValid());

  /*
   * Create Qt plugin info list
   * and create realted source files
   */
  const auto qtPluginInfoList = createQtPluginInfoList({{"libqxcb.so","platforms"},{"libqtaudio.so","audio"}}, sourceRoot.path());
  for(const auto & qpi : qtPluginInfoList){
    QVERIFY( createFile(qpi.absoluteFilePath()) );
    QFileInfo fi(qpi.absoluteFilePath());
    QVERIFY(fi.exists());
  }
  /*
   * Run copy and check
   */
  const QString destinationDirectoryPath = QDir::cleanPath( destinationRoot.path() + "usr/lib/" );
  QVERIFY(fc.copyPlugins(qtPluginInfoList, destinationDirectoryPath));
  for(const auto & qpi : qtPluginInfoList){
    const QString filePath = QDir::cleanPath( destinationDirectoryPath + "/" + qpi.directoryName() + "/" + qpi.libraryName().fullName() );
    QFileInfo fi(filePath);
    QVERIFY(fi.exists());
  }
  // Run the copy a second time (copier must not fail if destination file exists)
  QVERIFY(fc.copyPlugins(qtPluginInfoList, destinationDirectoryPath));
}

LibraryInfoList FileCopierTest::createLibraryInfoList(const QStringList& libNameList, const QString& pathPrefix)
{
  LibraryInfoList list;

  for(const auto & libName : libNameList){
    LibraryInfo li;
    li.setLibraryPlatformName(libName);
    li.setAbsoluteFilePath( QDir::cleanPath(pathPrefix + "/" + libName) );
    list.addLibrary(li);
  }

  return list;
}

QtPluginInfoList FileCopierTest::createQtPluginInfoList(const std::vector<FileCopierTest::QtPluginNameAndDir> & qtPluginsDef, const QString& pathPrefix)
{
  QtPluginInfoList plugins;

  for(const auto & pluginDef : qtPluginsDef){
    QtPluginInfo qpi;
    qpi.setLibraryPlatformName(pluginDef.first);
    qpi.setAbsoluteFilePath( QDir::cleanPath(pathPrefix + "/" + pluginDef.first) );
    qpi.setDirectoryName(pluginDef.second);
    plugins.addPlugin(qpi);
  }

  return plugins;
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::CoreApplication app(argc, argv);
  FileCopierTest test;

//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
