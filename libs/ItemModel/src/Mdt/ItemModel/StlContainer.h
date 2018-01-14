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
#ifndef MDT_ITEM_MODEL_STL_CONTAINER_H
#define MDT_ITEM_MODEL_STL_CONTAINER_H

#include "MdtItemModelExport.h"
#include <QtGlobal>
#include <iterator>
#include <algorithm>
#include <type_traits>

namespace Mdt{ namespace ItemModel{

    /*! \brief Get the size of \a container
     */
    template<typename Container>
    int MDT_ITEMMODEL_EXPORT containerSize(const Container & container)
    {
      return std::distance(container.cbegin(), container.cend());
    }

    /*! \brief Get row count of \a table
     */
    template<typename Table>
    int MDT_ITEMMODEL_EXPORT tableRowCount(const Table & table)
    {
      return containerSize(table);
    }

    /*! \brief Get column count for \a row in \a table
     *
     * \pre \a row must be in valid range ( 0 <= \a row < tableRowCount(\a table) ).
     */
    template<typename Table>
    int MDT_ITEMMODEL_EXPORT tableColumnCount(const Table & table, int row)
    {
      Q_ASSERT(row >= 0);
      Q_ASSERT(row < tableRowCount(table));

      const auto recordIt = std::next(table.cbegin(), row);

      return containerSize(*recordIt);
    }

    /*! \brief Check if \a container is empty
     */
    template<typename Container>
    bool MDT_ITEMMODEL_EXPORT containerIsEmpty(const Container & container)
    {
      return ( container.cbegin() == container.cend() );
    }

    /*! \brief Compare size of a container in a stateful way
     *
     * This functor compares the size of the container
     *  with the size of its previous call.
     */
    template<typename RecordAdapter>
    class MDT_ITEMMODEL_EXPORT StateFulCompareContainerSize
    {
    public:

//       /*! \brief Constructor
//        */
//       StateFulCompareContainerSize(const RecordAdapter & recordAdapter)
//        : mRecordAdapter(recordAdapter)
//       {
//       }

      /*! \brief Compare size of \a container with previous size
       *
       * At the first call, previous size will be stored
       *  and true is returned.
       */
      template<typename Container>
      bool operator()(const Container & container)
      {
        if(mPrevisouSize < 0){
          mPrevisouSize = mRecordAdapter.containerSize(container);
          return true;
        }
        const int size = mRecordAdapter.containerSize(container);
        if(size != mPrevisouSize){
          mPrevisouSize = size;
          return false;
        }
        return true;
      }

    private:

      int mPrevisouSize = -1;
      RecordAdapter mRecordAdapter;
    };

    /*! \brief Check that each record of the table has the same column count
     */
    template<typename Table, typename RecordAdapter>
    bool MDT_ITEMMODEL_EXPORT eachRecordHasSameColumnCount(const Table & table, const RecordAdapter &)
    {
      return std::all_of(table.cbegin(), table.cend(), StateFulCompareContainerSize<RecordAdapter>());
    }

    /*! \brief Get a const iterator to the element at \a index
     *
     * \pre \a index must be in valid range ( 0 <= \a index < containerSize(container) )
     */
    template<typename Container>
    auto constIteratorAtIndex(const Container & container, int index)
    {
      Q_ASSERT(index >= 0);
      Q_ASSERT(index < containerSize(container));

      return std::next(container.cbegin(), index);
    }

    /*! \brief Get a iterator to the element at \a index
     *
     * \pre \a index must be in valid range ( 0 <= \a index < containerSize(container) )
     */
    template<typename Container>
    auto iteratorAtIndex(Container & container, int index)
    {
      Q_ASSERT(index >= 0);
      Q_ASSERT(index < containerSize(container));

      return std::next(container.begin(), index);
    }

    /*! \brief Get a const iterator to the element at \a row and \a column
     *
     * \pre \a row must be in valid range ( 0 <= \a row < tableRowCount(table) )
     * \pre \a column must be in valid range ( 0 <= \a column < tableColumnCount(table, row) )
     */
    template<typename Table>
    auto constIteratorAtRowColumn(const Table & table, int row, int column)
    {
      Q_ASSERT(row >= 0);
      Q_ASSERT(row < tableRowCount(table));
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < tableColumnCount(table, row));

      const auto recordIt = constIteratorAtIndex(table, row);

      return constIteratorAtIndex(*recordIt, column);
    }

    /*! \brief Get a iterator to the element at \a row and \a column
     *
     * \pre \a row must be in valid range ( 0 <= \a row < tableRowCount(table) )
     * \pre \a column must be in valid range ( 0 <= \a column < tableColumnCount(table, row) )
     */
    template<typename Table>
    auto iteratorAtRowColumn(Table & table, int row, int column)
    {
      Q_ASSERT(row >= 0);
      Q_ASSERT(row < tableRowCount(table));
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < tableColumnCount(table, row));

      auto recordIt = iteratorAtIndex(table, row);

      return iteratorAtIndex(*recordIt, column);
    }

    /*! \brief Insert \a count copies of \a value to \a container
     */
    template<typename Container, typename T>
    void MDT_ITEMMODEL_EXPORT insertToContainer(Container & container, int pos, int count, const T & value)
    {
      container.insert( std::next(container.begin() , pos), count, value );
    }

    /*! \brief Insert \a count copies of \a value to \a container
     *
     * This is a specialization for QList
     */
    template<typename T>
    void MDT_ITEMMODEL_EXPORT insertToContainer(QList<T> & container, int pos, int count, const T & value)
    {
      container.reserve(container.size() + count);
      for(int i = 0; i < count; ++i){
        container.insert(pos, value);
      }
    }

    /*! \brief Insert \a value to the end of \a container
     */
    template<typename Container, typename T>
    void MDT_ITEMMODEL_EXPORT appendToContainer(Container & container, const T & value)
    {
      insertToContainer(container, containerSize(container), 1, value);
    }

    /*! \brief Insert \a value to the beginning of \a container
     */
    template<typename Container, typename T>
    void MDT_ITEMMODEL_EXPORT prependToContainer(Container & container, const T & value)
    {
      insertToContainer(container, 0, 1, value);
    }

    /*! \brief Implementation for initializeContainer()
     *
     * This version works for containers that provides
     *  a constructor of the form:
     *  \code
     *  Container(size_type count, const T & value);
     *  \endcode
     */
    template<typename Container>
    struct InitializeContainerImpl
    {
      template<typename T>
      static Container initialize(int count, const T & value)
      {
        return Container(count, value);
      }
    };

    /*! \brief Implementation for initializeContainer()
     *
     * This is a specialization for QList
     */
    template<typename T>
    struct InitializeContainerImpl< QList<T> >
    {
      static QList<T> initialize(int count, const T & value)
      {
        QList<T> list;
        insertToContainer(list, 0, count, value);
        return list;
      }
    };

    /*! \brief Initialize \a container with \a count copies of \a value
     *
     * \pre \a count must be >= 1
     */
    template<typename Container, typename T>
    Container MDT_ITEMMODEL_EXPORT initializeContainer(int count, const T & value)
    {
      Q_ASSERT(count >= 1);

      return InitializeContainerImpl<Container>::initialize(count, value);
    }

    /*! \brief Insert \a rowCount records initialized with \a columnCount copies of \a value to \a table before \a row
     */
    template<typename Table, typename T>
    void MDT_ITEMMODEL_EXPORT insertToTable(Table & table, int row, int rowCount, int columnCount, const T & value)
    {
      using record_type = typename Table::value_type;

      insertToContainer( table, row, rowCount, initializeContainer<record_type>(columnCount, value) );
    }

    /*! \brief Remove \a count element starting from \a pos from \a container
     *
     * If \a pos + \a count is grater than upper bound of \a container,
     *  it will be ajusted.
     *
     * \pre \a pos must be >= 0
     * \pre \a count must be >= 1
     * \pre \a pos + \a count must be in valid range ( 1 <= \a pos + \a count <= containerSize( \a container ) )
     */
    template<typename Container>
    void MDT_ITEMMODEL_EXPORT removeFromContainer(Container & container, int pos, int count)
    {
      Q_ASSERT(pos >= 0);
      Q_ASSERT(count >= 0);
      Q_ASSERT( (pos + count) <= containerSize(container) );
      container.erase( std::next(container.begin() , pos), std::next(container.begin() , pos + count) );
    }

    /*! \brief Remove the first element from \a container
     *
     * \pre \a container must not be empty
     */
    template<typename Container>
    void MDT_ITEMMODEL_EXPORT removeFirstFromContainer(Container & container)
    {
      Q_ASSERT(!containerIsEmpty(container));
      removeFromContainer(container, 0, 1);
    }

    /*! \brief Remove the last element from \a container
     *
     * \pre \a container must not be empty
     */
    template<typename Container>
    void MDT_ITEMMODEL_EXPORT removeLastFromContainer(Container & container)
    {
      Q_ASSERT(!containerIsEmpty(container));
      removeFromContainer(container, containerSize(container)-1, 1);
    }

}}

#endif // #ifndef MDT_ITEM_MODEL_STL_CONTAINER_H
