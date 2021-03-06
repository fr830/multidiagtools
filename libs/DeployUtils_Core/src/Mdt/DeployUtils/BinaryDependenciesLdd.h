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
#ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_LDD_H
#define MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_LDD_H

#include "BinaryDependenciesImplementationInterface.h"
#include "LibraryInfoList.h"
#include "Mdt/PlainText/StringRecord.h"
#include "Mdt/PlainText/StringRecordList.h"
#include "MdtDeployUtils_CoreExport.h"
#include <QStringList>

namespace Mdt{ namespace DeployUtils{

  /*! \brief Binary dependencies ldd implementation
   */
  class MDT_DEPLOYUTILS_CORE_EXPORT BinaryDependenciesLdd : public BinaryDependenciesImplementationInterface
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    BinaryDependenciesLdd(QObject* parent = nullptr);

    /*! \brief Find dependencies for a executable or a library
     */
    bool findDependencies(const QString & binaryFilePath) override;

    /*! \internal Fill dependencies, made public for unit tests
     */
    void fillAndSetDependencies(PlainText::StringRecordList & data);

    /*! \internal Buid a library info list from a StringRecordList, made public for unit tests
     */
    static LibraryInfoList stringRecordListToLibraryInfoList(const PlainText::StringRecordList & list);

   private:

    static void setLibrariesNameIfMissing(PlainText::StringRecordList & recordList);
    static bool isLibraryNotFound(const PlainText::StringRecord & record);
    static bool isLibraryNotInExcludeList(const PlainText::StringRecord & record);
    static QStringList stringRecordListToStringNameList(const PlainText::StringRecordList & recordList);

    PlainText::StringRecordList mNotFoundDependencies;
  };

}} // namespace Mdt{ namespace DeployUtils{

#endif // #ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_LDD_H
