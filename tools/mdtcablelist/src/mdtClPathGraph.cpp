/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
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
#include "mdtClPathGraph.h"
#include "mdtClPathGraphicsConnection.h"
#include "mdtClPathGraphicsLink.h"
#include "mdtError.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QString>
#include <QModelIndex>

#include <QDebug>

using namespace mdtClPathGraphPrivate;

/*
 * Visitor class implementation
 */

mdtClPathGraphVisitor::mdtClPathGraphVisitor()
{
  pvEdgeQueue = 0;
}



///mdtClPathGraphVisitor::mdtClPathGraphVisitor(QQueue<QPair<QVariant, QVariant> > *edgeQueue)
mdtClPathGraphVisitor::mdtClPathGraphVisitor(QQueue<mdtClPathGraphEdgeData> *edgeQueue)
{
  Q_ASSERT(edgeQueue != 0);

  pvEdgeQueue = edgeQueue;
}

void mdtClPathGraphVisitor::discover_vertex(vertex_t v, const graph_t & g)
{
  qDebug() << "Discover vertex: " << v;
}

void mdtClPathGraphVisitor::examine_edge(edge_t e, const graph_t &g)
{
  Q_ASSERT(pvEdgeQueue != 0);

  ///QPair<QVariant, QVariant> edgeData = get(boost::edge_bundle, g)[e];
  mdtClPathGraphEdgeData edgeData = get(boost::edge_bundle, g)[e];
  pvEdgeQueue->enqueue(edgeData);
  ///qDebug() << "Visitor:  added " << edgeData;
  boost::default_bfs_visitor::examine_edge(e, g); /// Utile ??
}


/*
 * mdtClPathGraph implementation
 */

mdtClPathGraph::mdtClPathGraph(QSqlDatabase db)
{
  pvDatabase = db;
  pvLinkListModel = new QSqlQueryModel;
  pvGraphicsScene = new QGraphicsScene;
}

mdtClPathGraph::~mdtClPathGraph()
{
  delete pvLinkListModel;
  delete pvGraphicsScene;
}

bool mdtClPathGraph::loadLinkList() 
{
  QString sql;
  QSqlError sqlError;
  int row;
  QModelIndex index;
  QVariant data;
  ///QPair<QVariant, QVariant> edgeData;
  mdtClPathGraphEdgeData edgeData;
  vertex_t startVertex, endVertex;
  edge_t edge;
  int startConnectionId, endConnectionId;

  // Clear previous results
  pvGraph.clear();
  pvGraphVertices.clear();
  // Run query to get start and end connection IDs
  sql = "SELECT UnitConnectionStart_Id_FK, UnitConnectionEnd_Id_FK, LinkDirection_Code_FK FROM LinkList_view";
  pvLinkListModel->setQuery(sql, pvDatabase);
  sqlError = pvLinkListModel->lastError();
  if(sqlError.isValid()){
    mdtError e(MDT_DATABASE_ERROR, "Cannot load link list from database", mdtError::Error);
    e.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(e, "mdtClPathGraph");
    e.commit();
    return false;
  }
  // Build the graph
  for(row = 0; row < pvLinkListModel->rowCount(); ++row){
    qDebug() << "Import row " << row << " ...";
    // Get start connection ID
    index = pvLinkListModel->index(row, 0);
    data = pvLinkListModel->data(index);
    if(data.isNull()){
      return false; /// \todo Error message ...
    }
    startConnectionId = data.toInt();
    // Get start vertex or create one if not allready exists
    qDebug() << "getting start vertex ...";
    if(pvGraphVertices.contains(startConnectionId)){
      startVertex = pvGraphVertices.value(startConnectionId);
    }else{
      startVertex = boost::add_vertex(pvGraph);
      pvGraphVertices.insert(startConnectionId, startVertex);
      qDebug() << "Created new start vertex: << " << startVertex << " , ID: " << startConnectionId;
    }
    // Set start ID to edge data
    ///edgeData.first = data;
    edgeData.startConnectionId = data;
    // Get end connection ID
    index = pvLinkListModel->index(row, 1);
    data = pvLinkListModel->data(index);
    if(data.isNull()){
      return false; /// \todo Error message ...
    }
    endConnectionId = data.toInt();
    // Get end vertex or create one if not allready exists
    if(pvGraphVertices.contains(endConnectionId)){
      endVertex = pvGraphVertices.value(endConnectionId);
    }else{
      endVertex = boost::add_vertex(pvGraph);
      pvGraphVertices.insert(endConnectionId, endVertex);
      qDebug() << "Created new end vertex: << " << endVertex << " , ID: " << endConnectionId;
    }
    // Set end connection ID to edge data
    ///edgeData.second = data;
    edgeData.endConnectionId = data;
    // Add edge
    edgeData.isComplement = false;
    boost::add_edge(startVertex, endVertex, edgeData, pvGraph);
    // Add complement edge if link is bidirectinnal
    index = pvLinkListModel->index(row, 2);
    data = pvLinkListModel->data(index);
    if(data.toString() == "BID"){
      edgeData.isComplement = true;
      boost::add_edge(endVertex, startVertex, edgeData, pvGraph);
    }
    // 
    ///qDebug() << "Edge data: " << edgeData;
  }
  /**
  boost::graph_traits<graph_t>::vertex_iterator vi, vi_end;
  boost::graph_traits<graph_t>::adjacency_iterator ai, ai_end;
  for(boost::tie(vi, vi_end) = vertices(pvGraph); vi != vi_end; ++vi){
    ///qDebug() << "Vertex : " << get(boost::vertex_bundle, pvGraph)[*vi];
    qDebug() << "Vertex : " << *vi;
    boost::tie(ai, ai_end) = adjacent_vertices(*vi, pvGraph);
    while(ai != ai_end){
      qDebug() << "-> Adjacent: " << *ai;
      ai++;
    }
  }
  */


  return true;
}

bool mdtClPathGraph::drawPath(const QVariant & fromConnectionId)
{
  ///QPair<QVariant, QVariant> edge;
  mdtClPathGraphEdgeData edge;
  mdtClPathGraphicsConnection *startConnection, *endConnection;
  mdtClPathGraphicsLink *link;
  int startConnectionId, endConnectionId;
  mdtClPathGraphVisitor visitor(&pvEdgeQueue);
  vertex_t vertex;

  qDebug() << "Vertices: " << pvGraphVertices;
  // Check if we have requested connection ID in the graph
  if(!pvGraphVertices.contains(fromConnectionId.toInt())){
    return false;   /// \todo message..
  }
  vertex = pvGraphVertices.value(fromConnectionId.toInt());
  qDebug() << "Id: " << fromConnectionId.toInt() << " , vertex: " << vertex;
  // Clear previous results
  pvEdgeQueue.clear();
  pvDrawnConnections.clear();
  pvGraphicsScene->clear();
  // Proceed BFS
  breadth_first_search(pvGraph, vertex, boost::visitor(visitor));

  // Simulation BFS ...
  /**
  for(int i = 0; i < 10; i++){
    edge.startConnectionId = i;
    edge.endConnectionId = i+1;
    edge.isComplement = false;
    pvEdgeQueue.enqueue(edge);
  }
  */
  /**
  for(int i = 9; i < 11; i++){
    pvEdgeQueue.enqueue(QPair<QVariant, QVariant>(i, i+2));
  }
  for(int i = 9; i < 16; i++){
    pvEdgeQueue.enqueue(QPair<QVariant, QVariant>(i, i+3));
  }
  */

  // draw all
  while(!pvEdgeQueue.isEmpty()){
    edge = pvEdgeQueue.dequeue();
    ///qDebug() << "Edge, start: " << edge.startConnectionId << " , end: " << edge.endConnectionId << " , isComplement: " << edge.isComplement;
    Q_ASSERT(!edge.startConnectionId.isNull());
    Q_ASSERT(!edge.endConnectionId.isNull());
    if(!edge.isComplement){
      // Try to get startConnection
      startConnectionId = edge.startConnectionId.toInt();
      startConnection = pvDrawnConnections.value(startConnectionId, 0);
      // Try to get end connection
      endConnectionId = edge.endConnectionId.toInt();
      endConnection = pvDrawnConnections.value(endConnectionId, 0);
      ///qDebug() << "Adding from ID " << startConnectionId << " to " << endConnectionId;
      // Create missing connections
      if((startConnection == 0)&&(endConnection == 0)){
        startConnection = newConnectionItem(startConnectionId, 0, false);
        endConnection = newConnectionItem(endConnectionId, startConnection, false);
      }else if(startConnection == 0){
        Q_ASSERT(endConnection != 0);
        startConnection = newConnectionItem(startConnectionId, endConnection, false);
      }else{
        Q_ASSERT(startConnection != 0);
        endConnection = newConnectionItem(endConnectionId, startConnection, false);
      }
      Q_ASSERT(startConnection != 0);
      Q_ASSERT(endConnection != 0);
      // Draw link
      link = new mdtClPathGraphicsLink(startConnection, endConnection);
      ///link->setText(QString::number(startConnectionId) + "->" + QString::number(endConnectionId));
      pvGraphicsScene->addItem(link);
      // Set connections and link data
      setGraphicsItemsData(startConnection, endConnection, link, startConnectionId, endConnectionId);
    }
  }

  return true;
}

QVariant mdtClPathGraph::getLinkedConnectionIds(const QVariant & fromConnectionId) 
{
}

void mdtClPathGraph::attachView(QGraphicsView *view) 
{
  Q_ASSERT(view != 0);

  view->setScene(pvGraphicsScene);
}

mdtClPathGraphicsConnection *mdtClPathGraph::newConnectionItem(int id, mdtClPathGraphicsConnection *itemForNextPos, bool reverse)
{
  mdtClPathGraphicsConnection *item;

  item = new mdtClPathGraphicsConnection;
  if(itemForNextPos != 0){
    item->setPos(itemForNextPos->nextPosition(reverse));
  }else{
    item->setPos(0.0, 0.0);
  }
  pvGraphicsScene->addItem(item);
  pvDrawnConnections.insert(id, item);

  return item;
}

bool mdtClPathGraph::setGraphicsItemsData(mdtClPathGraphicsConnection *startConnection, mdtClPathGraphicsConnection *endConnection, mdtClPathGraphicsLink *link, int startConnectionId, int endConnectionId)
{
  Q_ASSERT(startConnection != 0);
  Q_ASSERT(endConnection != 0);
  Q_ASSERT(link != 0);

  QString sql;
  QSqlError sqlError; 
  QVariant data;
  QSqlQuery query(pvDatabase);
  QString str;

  sql = "SELECT StartCabinet, StartSchemaPosition, StartUnitConnectorName, StartUnitContactName, "\
        "EndCabinet, EndSchemaPosition, EndUnitConnectorName, EndUnitContactName, Identification "\
        "FROM LinkList_view ";
  sql += " WHERE UnitConnectionStart_Id_FK = " + QString::number(startConnectionId);
  sql += " AND UnitConnectionEnd_Id_FK = " + QString::number(endConnectionId);
  if(!query.exec(sql)){
    sqlError = query.lastError();
    mdtError e(MDT_DATABASE_ERROR, "Cannot get connections informations from database", mdtError::Error);
    e.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(e, "mdtClPathGraph");
    e.commit();
    startConnection->setText("??");
    endConnection->setText("??");
    return false;
  }
  query.next();
  // Set start data
  str = query.value(0).toString() + "\n";
  str += query.value(1).toString() + "\n";
  str += query.value(2).toString() + ";";
  str += query.value(3).toString();
  startConnection->setText(str);
  // Set end data
  str = query.value(4).toString() + "\n";
  str += query.value(5).toString() + "\n";
  str += query.value(6).toString() + ";";
  str += query.value(7).toString();
  endConnection->setText(str);
  // Set link data
  str = query.value(8).toString();
  link->setText(str);

  return true;
}

mdtClUnitConnectionData mdtClPathGraph::getUnitConnectionStartDataFromModel(int row) 
{
}

mdtClUnitConnectionData mdtClPathGraph::getUnitConnectionEndDataFromModel(int row) 
{
}

mdtClLinkData mdtClPathGraph::getLinkDataFromModel(int row) 
{
}

