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
#include "PkFkProxyModelBase.h"

namespace Mdt{ namespace ItemModel{

PkFkProxyModelBase::PkFkProxyModelBase(QObject* parent)
 : QIdentityProxyModel(parent)
{
}

Qt::ItemFlags PkFkProxyModelBase::flags(const QModelIndex & index) const
{
  if(!index.isValid()){
    return QIdentityProxyModel::flags(index);
  }
  if(mIsKeyEditable && mIsKeyItemsEnabled){
    return QIdentityProxyModel::flags(index);
  }
  if(!mKey.contains(index.column())){
    return QIdentityProxyModel::flags(index);
  }
  auto f = QIdentityProxyModel::flags(index);
  if(!mIsKeyEditable){
    f &= Qt::ItemFlags(~Qt::ItemIsEditable);
  }
  if(!mIsKeyItemsEnabled){
    f &= Qt::ItemFlags(~Qt::ItemIsEnabled);
  }
  return f;
}

void PkFkProxyModelBase::setKey(const ColumnList& key)
{
  mKey = key;
}

void PkFkProxyModelBase::setKeyEditable(bool editable)
{
  mIsKeyEditable = editable;
}

void PkFkProxyModelBase::setKeyItemsEnabled(bool enable)
{
  mIsKeyItemsEnabled = enable;
}

}} // namespace Mdt{ namespace ItemModel{
