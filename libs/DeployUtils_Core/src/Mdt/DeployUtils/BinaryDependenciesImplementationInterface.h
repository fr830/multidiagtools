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
#ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_IMPLEMENTATION_INTERFACE_H
#define MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_IMPLEMENTATION_INTERFACE_H

#include "LibraryInfoList.h"
#include "PathList.h"
#include "Mdt/Error.h"
#include <QObject>

namespace Mdt{ namespace DeployUtils{

  /*! \brief
   */
  class BinaryDependenciesImplementationInterface : public QObject
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    BinaryDependenciesImplementationInterface(QObject* parent = nullptr);

    /*! \brief Set a list of paths where to search dependencies first
     */
    void setLibrarySearchFirstPathList(const PathList & pathList);

    /*! \brief Get the list of paths where to search dependencies first
     */
    PathList librarySearchFirstPathList() const
    {
      return mLibrarySearchFirstPathList;
    }

    /*! \brief Find dependencies for a executable or a library
     */
    virtual bool findDependencies(const QString & binaryFilePath) = 0;

    /*! \brief Get dependencies
     */
    LibraryInfoList dependencies() const
    {
      return mDependencies;
    }

    /*! \brief Get last error
     */
    Mdt::Error lastError() const
    {
      return mLastError;
    }

   protected:

    /*! \brief Set dependencies
     */
    void setDependencies(const LibraryInfoList & dependencies);

    /*! \brief Set last error
     */
    void setLastError(const Mdt::Error & error);

   private:

    LibraryInfoList mDependencies;
    PathList mLibrarySearchFirstPathList;
    Mdt::Error mLastError;
  };

}} // namespace Mdt{ namespace DeployUtils{

#endif // #ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_IMPLEMENTATION_INTERFACE_H
