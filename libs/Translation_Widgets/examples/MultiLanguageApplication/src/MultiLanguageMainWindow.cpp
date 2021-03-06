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
#include "MultiLanguageMainWindow.h"
#include <QEvent>

using namespace Mdt::Translation;

MultiLanguageMainWindow::MultiLanguageMainWindow(QWidget* parent)
 : QMainWindow(parent)
{
  mUi.setupUi(this);
  if(mLanguageSelection.findTranslations()){
    mLanguageSelection.addAvailableLanguagesToMenu(mUi.menu_Language);
    mLanguageSelection.addAvailableLanguagesToToolBar(mUi.toolBar_Language);
  } // Maybe display/log a error if findTranslations() fails
}

void MultiLanguageMainWindow::changeEvent(QEvent* event)
{
  if(event != nullptr){
    mLanguageSelection.handleChangeEvent(event);
    if(event->type() == QEvent::LanguageChange){
      mUi.retranslateUi(this);
    }
  }
  QMainWindow::changeEvent(event);
}
