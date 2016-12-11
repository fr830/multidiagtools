/****************************************************************************
 **
 ** Copyright (C) 2011-2016 Philippe Steinmann.
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
#include "VariantTableModel.h"
#include <QChar>
#include <algorithm>

namespace Mdt{ namespace ItemModel{

VariantTableModel::VariantTableModel(VariantTableModelStorageRule storageRule, QObject* parent)
 : QAbstractTableModel(parent),
   mStorageRule(storageRule),
   mColumnCount(0)
{
}

int VariantTableModel::rowCount(const QModelIndex& parent) const
{
   if(parent.isValid()){
     return 0;
   }
   return mData.size();
}

int VariantTableModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return mColumnCount;
}

QVariant VariantTableModel::data(const QModelIndex& index, int role) const
{
  Q_ASSERT(index.row() >= 0);
  Q_ASSERT(index.row() < (int)mData.size());
  Q_ASSERT(index.column() >= 0);
  Q_ASSERT(index.column() < mColumnCount);

  if( (role == Qt::DisplayRole) || (role == Qt::EditRole) ){
    return mData[index.row()].data(index.column(), role);
  }
  return QVariant();
}

void VariantTableModel::setItemEnabled(const QModelIndex& index, bool enable)
{
  Q_ASSERT(index.row() >= 0);
  Q_ASSERT(index.row() < (int)mData.size());
  Q_ASSERT(index.column() >= 0);
  Q_ASSERT(index.column() < mColumnCount);

  mData[index.row()].setItemEnabled(index.column(), enable);
}

void VariantTableModel::setItemEditable(const QModelIndex& index, bool editable)
{
  Q_ASSERT(index.row() >= 0);
  Q_ASSERT(index.row() < (int)mData.size());
  Q_ASSERT(index.column() >= 0);
  Q_ASSERT(index.column() < mColumnCount);

  mData[index.row()].setItemEditable(index.column(), editable);
}

Qt::ItemFlags VariantTableModel::flags(const QModelIndex& index) const
{
  Q_ASSERT(index.row() >= 0);
  Q_ASSERT(index.row() < (int)mData.size());
  Q_ASSERT(index.column() >= 0);
  Q_ASSERT(index.column() < mColumnCount);

  return mData[index.row()].flags(index.column(), QAbstractTableModel::flags(index));
}

bool VariantTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  Q_ASSERT(index.row() >= 0);
  Q_ASSERT(index.row() < (int)mData.size());
  Q_ASSERT(index.column() >= 0);
  Q_ASSERT(index.column() < mColumnCount);

  if( (role == Qt::DisplayRole) || (role == Qt::EditRole) ){
    mData[index.row()].setData(index.column(), value, role);
    emit dataChanged(index, index);
    return true;
  }
  return false;
}

void VariantTableModel::resize(int rows, int columns)
{
  Q_ASSERT(rows >= 0);
  Q_ASSERT(columns >= 0);

  beginResetModel();
  resizeRowCount(rows);
  resizeColumnCount(columns);
  endResetModel();
}

void VariantTableModel::populateColumnWithInt(int column, int firstValue, int increment)
{
  Q_ASSERT(column >= 0);
  Q_ASSERT(column < mColumnCount);

  const int n = rowCount();
  for(int row = 0; row < n; ++row){
    auto idx = index(row, column);
    setData(idx, firstValue);
    firstValue += increment;
  }
}

void VariantTableModel::populateColumnWithAscii(int column, char firstValue, int increment)
{
  Q_ASSERT(column >= 0);
  Q_ASSERT(column < mColumnCount);

  const int n = rowCount();
  for(int row = 0; row < n; ++row){
    auto idx = index(row, column);
    setData(idx, QChar(firstValue));
    firstValue += increment;
  }
}

void VariantTableModel::populateColumn(int column, const std::vector<QVariant> & data)
{
  Q_ASSERT((int)data.size() <= rowCount());
  Q_ASSERT(column >= 0);
  Q_ASSERT(column < columnCount());

  const int n = data.size();
  for(int row = 0; row < n; ++row){
    auto idx = index(row, column);
    Q_ASSERT(idx.isValid());
    setData(idx, data[row]);
  }
}

void VariantTableModel::populate(int rows, int columns)
{
  beginResetModel();
  mColumnCount = columns;
  mData.clear();
  mData.reserve(rows);
  for(int i = 0; i < rows; ++i){
    mData.push_back(generateRowData(i));
  }
  endResetModel();
}

void VariantTableModel::clear()
{
  beginResetModel();
  mData.clear();
  mColumnCount = 0;
  endResetModel();
}

void VariantTableModel::resizeRowCount(int rows)
{
  mData.resize( rows, VariantTableModelRow(mStorageRule, mColumnCount) );
}

void VariantTableModel::resizeColumnCount(int columns)
{
  for(auto & row : mData){
    row.resize(columns, mStorageRule);
  }
  mColumnCount = columns;
}

VariantTableModelRow VariantTableModel::generateRowData(int currentRow) const
{
  VariantTableModelRow rowData(mStorageRule, mColumnCount);

  for(int i = 0; i < mColumnCount; ++i){
    rowData.setData( i, QString("%1%2").arg(currentRow).arg( QChar('A'+i) ) , Qt::DisplayRole );
  }

  return rowData;
}

}} // namespace Mdt{ namespace ItemModel{
