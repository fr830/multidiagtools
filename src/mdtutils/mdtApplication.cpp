/****************************************************************************
 **
 ** Copyright (C) 2011-2012 Philippe Steinmann.
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
#include "mdtApplication.h"
#include "mdtErrorOut.h"
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>
#include <iostream>

#include <QDebug>

mdtApplication *mdtApplication::pvInstance = 0;

mdtApplication::mdtApplication(int &argc, char **argv, bool GUIenabled)
 : QtSingleApplication(argc, argv, GUIenabled)
{
  pvInstance = this;
}

mdtApplication::~mdtApplication()
{
  // Free the error system
  mdtErrorOut::destroy();
}

bool mdtApplication::init(bool allowMultipleInstances)
{
  QFileInfo fi;
  QString logFileName;

  // Check if another instance of program is running
  if(!allowMultipleInstances){
    if(sendMessage("")){
      std::cerr << "mdtApplication::init(): another instance is running , abort" << std::endl;
      return false;
    }
  }

#ifdef Q_OS_UNIX
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

  // System data directory path
  if(!searchSystemDataDir()){
    return false;
  }
  // Init home directory structure
  if(!initHomeDir()){
    return false;
  }
  qDebug() << "mdtApplication , data dir: " << pvSystemDataDirPath;
  qDebug() << "mdtApplication , log dir: " << pvLogDirPath;
  qDebug() << "mdtApplication , lib version: " << mdtLibVersion();

  // Build log file name
  fi.setFile(arguments().at(0));
  logFileName = fi.baseName();
  if(logFileName.isEmpty()){
    logFileName = "mdtapplication";
  }
  logFileName += "-";
  logFileName += QString::number(applicationPid());
  logFileName += ".log";
  qDebug() << "mdtApplication::init(): log file: " << logFileName;
  // Init error system
  if(!mdtErrorOut::init(pvLogDirPath + "/" + logFileName)){
    std::cerr << "mdtApplication::init(): unable to init the error system" << std::endl;
    return false;
  }

  return true;
}

mdtApplication *mdtApplication::instance()
{
  Q_ASSERT(pvInstance != 0);

  return pvInstance;
}

QString mdtApplication::systemDataDirPath()
{
  return instance()->pvSystemDataDirPath;
}

QString mdtApplication::mdtLibVersion()
{
  return QString(QString::number(MDTLIB_VERSION_MAJOR) + "." + QString::number(MDTLIB_VERSION_MINOR) + "." + QString::number(MDTLIB_VERSION_MICRO));
}

bool mdtApplication::searchSystemDataDir()
{
  QDir dir;

  // At first, we look in application directory
  dir.setPath(applicationDirPath());
  if(!dir.exists()){
    std::cerr << "mdtApplication::searchSystemDataDir(): cannot find application directory" << std::endl;
    return false;
  }
  if(dir.cd("data")){
    // Ok, found.
    pvSystemDataDirPath = dir.absolutePath();
    return true;
  }
  // Check in application's parent directory
  if(dir.cd("../data")){
    // Ok, found.
    pvSystemDataDirPath = dir.absolutePath();
    return true;
  }
  // On Linux/Unix, data is probably installed in /usr/share/mdt or /usr/local/share
#ifdef Q_OS_UNIX
  if(dir.cd("/usr/share/mdt/data")){
    // Ok, found.
    pvSystemDataDirPath = dir.absolutePath();
    return true;
  }
  if(dir.cd("/usr/local/share/mdt/data")){
    // Ok, found.
    pvSystemDataDirPath = dir.absolutePath();
    return true;
  }
#endif  // #ifdef Q_OS_UNIX

  // Data directory not found :-(
  std::cerr << "mdtApplication::searchSystemDataDir(): cannot find system data directory" << std::endl;

  return false;
}

bool mdtApplication::initHomeDir()
{
  QDir dir(QDir::home());

  // Home directory must exists first..
  if(!dir.exists()){
    std::cerr << "mdtApplication::initHomeDir(): cannot find home directory" << std::endl;
    return false;
  }
  // Look about the .mdt directory
  if(!dir.cd(".mdt")){
    // Not found, try to create it
    if(!dir.mkdir(".mdt")){
      std::cerr << "mdtApplication::initHomeDir(): cannot create .mdt in home directory" << std::endl;
      return false;
    }
    if(!dir.cd(".mdt")){
      std::cerr << "mdtApplication::initHomeDir(): cannot enter $home/.mdt directory" << std::endl;
      return false;
    }
  }
  // Log directory
  if(!dir.cd("log")){
    // Not found, try to create it
    if(!dir.mkdir("log")){
      std::cerr << "mdtApplication::initHomeDir(): cannot create log in home directory" << std::endl;
      return false;
    }
    if(!dir.cd("log")){
      std::cerr << "mdtApplication::initHomeDir(): cannot enter $home/.mdt/log directory" << std::endl;
      return false;
    }
  }
  pvLogDirPath = dir.absolutePath();

  return true;
}
