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
#include "FindTranslation.h"
#include "TranslationInfo.h"
#include "QmFileName.h"
#include "Mdt/Error.h"
#include "Mdt/FileSystem/SearchPathList.h"
#include <QDir>
#include <QFileInfo>
#include <QChar>
#include <QStringList>
#include <QLatin1String>
#include <QStringBuilder>
#include <QCoreApplication>

#include <QDebug>

using namespace Mdt::FileSystem;

namespace Mdt{ namespace Translation{

Mdt::Expected< Mdt::Translation::TranslationInfoList > findTranslations(const Mdt::FileSystem::PathList& pathPrefixList)
{
  Q_ASSERT(!pathPrefixList.isEmpty());

  TranslationInfoList translations;

  const auto translationsDirectory = findTranslationsRoot(pathPrefixList);
  if(translationsDirectory.isEmpty()){
    const auto msg = QCoreApplication::translate("findTranslations()","Could not find translations directory. Searched in: '%1'")
                                                 .arg(pathPrefixList.toStringList().join(QChar::fromLatin1('\n')));
    auto error = mdtErrorNew(msg, Mdt::Error::Critical, QString());
    return error;
  }
  QDir dir(translationsDirectory);
  Q_ASSERT(dir.exists());
  const auto fiList = dir.entryInfoList(QDir::Files);
  for(const auto & fi : fiList){
    if( QString::compare(fi.suffix(), QLatin1String("qm"), Qt::CaseInsensitive) == 0 ){
      translations.addTranslation( TranslationInfo::fromQmFilePath(fi.absoluteFilePath()) );
    }
  }

  return translations;

}

QString findTranslationsRoot(const PathList & pathPrefixList)
{
  Q_ASSERT(!pathPrefixList.isEmpty());

  return findDirectoryRoot(QString::fromLatin1("translations"), {QString::fromLatin1(".")}, pathPrefixList);
}

QString findDirectoryRoot(const QString& directory, const QStringList& possibleSubdirectories, const QString& expectedQmFileBaseName, const PathList& pathPrefixList)
{
  QString directoryRoot;
  SearchPathList searchPathList;
  searchPathList.setIncludePathPrefixes(true);
  if(pathPrefixList.isEmpty()){
    searchPathList.setPathPrefixList( PathList::getSystemLibraryPathList() );
  }else{
    searchPathList.setPathPrefixList(pathPrefixList);
  }
  searchPathList.setPathSuffixList(possibleSubdirectories);
  const auto pathList = searchPathList.pathList();

  for(const auto & path : pathList){
    qDebug() << " Searchin in " << path;
    QDir dir( QDir::cleanPath(path % QLatin1String("/") % directory) );
    if(isTranslationDirectory(dir, expectedQmFileBaseName)){
      directoryRoot = dir.absolutePath();
      qDebug() << " - found: " << directoryRoot;
      return directoryRoot;
    }
  }

  return directoryRoot;
}

QString findDirectoryRoot(const QString& directory, const QStringList& possibleSubdirectories, const PathList& pathPrefixList)
{
  return findDirectoryRoot(directory, possibleSubdirectories, QString(), pathPrefixList);
}

bool isTranslationDirectory(const QString & directoryPath, const QString & expectedQmFileBaseName)
{
  return isTranslationDirectory(QDir(directoryPath), expectedQmFileBaseName);
}

bool isTranslationDirectory(const QDir& dir, const QString& expectedQmFileBaseName)
{
  if(!dir.exists()){
    return false;
  }
  const auto fiList = dir.entryInfoList(QDir::Files);
  for(const auto & fi : fiList){
    if(expectedQmFileBaseName.isEmpty()){
      if(QString::compare(fi.suffix(), QLatin1String("qm"), Qt::CaseInsensitive) == 0){
        return true;
      }
    }else{
      QmFileName qmFile(fi.fileName());
      if(qmFile.baseName() == expectedQmFileBaseName){
        return true;
      }
    }
  }
  return false;
}

}} // namespace Mdt{ namespace Translation{
