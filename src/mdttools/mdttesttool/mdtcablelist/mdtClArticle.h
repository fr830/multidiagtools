/****************************************************************************
 **
 ** Copyright (C) 2011-2015 Philippe Steinmann.
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
#ifndef MDT_CL_ARTICLE_H
#define MDT_CL_ARTICLE_H

#include "mdtTtBase.h"
#include "mdtSqlRecord.h"
#include "mdtClArticleConnectionData.h"
#include "mdtClArticleConnectorData.h"
#include "mdtSqlTableSelection.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QModelIndex>
#include <QList>

/*! \brief Helper class for Article and related tables edition
 */
class mdtClArticle : public mdtTtBase
{
 public:

  /*! \brief Constructor
   */
  mdtClArticle(QObject *parent, QSqlDatabase db);

  /*! \brief Destructor
   */
  ~mdtClArticle();

  /*! \brief Get SQL statement for article component selection
   */
  QString sqlForArticleComponentSelection(const QVariant &articleId) const;

  /*! \brief Add a article as component
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool addComponent(const QVariant &articleId, const QVariant &componentId, const QVariant &qty, const QVariant &qtyUnit);

  /*! \brief Edit a component
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool editComponent(const QVariant &articleId, const QVariant &currentComponentId, const QVariant &newComponentId, const QVariant &qty, const QVariant &qtyUnit);

  /*! \brief Remove a single component
   */
  bool removeComponent(const QVariant &articleId, const QVariant &componentId);

  /*! \brief Remove a list of components
   */
  bool removeComponents(const QVariant &articleId, const QList<QVariant> &componentIdList);

  /*! \brief Remove each component that is contained in selection
   *
   * This is usefull used together with mdtSqlTableWidget .
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  ///bool removeComponents(const QVariant &articleId, const QModelIndexList & indexListOfSelectedRows);
  bool removeComponents(const QVariant &articleId, const mdtSqlTableSelection & s);

  /*! \brief Get a list of connector contact data for given connector contact ID list
   *
   * Note: result will contain data from ConnectorContact_tbl
   */
  QList<QSqlRecord> getConnectorContactDataList(const QList<QVariant> & connectorContactIdList, bool *ok);

  /*! \brief Get article connection data for given article connection ID
   *
   * \pre ok must be a valid pointer.
   */
  mdtClArticleConnectionData getConnectionData(const QVariant & articleConnectionId, bool *ok);

  /*! \brief Get a list af article connection data from given article connection ID list
   */
  QList<mdtClArticleConnectionData> getConnectionDataListFromConnectionIdList(const QList<QVariant> & connectionIdList, bool *ok);

  /*! \brief Get article connector data for given article connector ID
   *
   * \param includeConnectionData If true, and SQL statement points to a article connector
   *            that contains article connections, result will be populated with these article connection data.
   * \param includeBaseConnectorData If true, and SQL statement points to a article connector
   *            that is based on a connector (from Connector_tbl), result will be populated with these connector data.
   * \pre ok must be valid
   */
  mdtClArticleConnectorData getConnectorData(const QVariant & articleConnectorId, bool *ok, bool includeConnectionData, bool includeBaseConnectorData);

  /*! \brief Add article connections into article connector data based on a list of given connector contact ID list
   *
   * Will also get some values from data (ArticleConnector_tbl) and set it to created connections (ArticleConnection_tbl):
   *  - ArticleConnector_tbl.Id_PK -> ArticleConnection_tbl.ArticleConnector_Id_FK
   *  - ArticleConnector_tbl.Article_Id_FK -> ArticleConnection_tbl.Article_Id_FK
   *
   * Note: will only update given data, nothing is written to database.
   */
  bool addConnectionDataListFromConnectorContactIdList(mdtClArticleConnectorData & data, const QList<QVariant> & connectorContactIdList);

  /*! \brief Update unit connections with data from given article connection
   *
   * For each unit connection that is based on given article connection,
   *  data will be updated with given fields values.
   *
   * \param fields Article field names. Matching is done internally.
   *               Supported fields are: ArticleContactName, FunctionEN,
   *               FunctionFR, FunctionDE, FunctionIT and Resistance.
   * \param connectionId Article connection ID.
   */
  bool updateUnitConnections(const QStringList & fields, const QVariant & connectionId);

  /*! \brief Update unit connections with data from given article connection
   *
   * See bool updateUnitConnections(const QStringList &, const QVariant &);
   * 
   * \todo Dangerous function, check it better (in unit tests)
   */
  bool updateUnitConnections(const QStringList & fields, const mdtSqlTableSelection & s);

  /*! \brief Edit a record in ArticleLink_tbl
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool editLink(const QVariant & articleConnectionStartId, const QVariant & articleConnectionEndId, const mdtSqlRecord &data);

  /*! \brief Update links in Link_tbl that are based in given article link
   *
   * \param articleConnectionStartId Article link start connection ID
   * \param articleConnectionEndId Article link end connection ID
   * \param articleLinkFields Article link fields for witch update must be done in Link_tbl
   *                    ArticleConnectionStart_Id_FK and ArticleConnectionEnd_Id_FK are not supported.
   *                    For other fields, mapping is done internally between ArticleLink_tbl to Link_tbl.
   * \pre articleConnectionStartId and articleConnectionEndId must not be null
   * \pre articleLinkFields must not contain ArticleConnectionStart_Id_FK or ArticleConnectionEnd_Id_FK
   */
  bool updateRelatedLinks(const QVariant & articleConnectionStartId, const QVariant & articleConnectionEndId, const QStringList & articleLinkFields);

  /*! \brief Update links in Link_tbl that are based in given article link
   *
   * \param articleConnectionStartId Article link start connection ID
   * \param articleConnectionEndId Article link end connection ID
   * \param articleLinkField Article link field for witch update must be done in Link_tbl
   *                    ArticleConnectionStart_Id_FK and ArticleConnectionEnd_Id_FK are not supported.
   *                    For other field name, mapping is done internally between ArticleLink_tbl to Link_tbl.
   * \pre articleConnectionStartId and articleConnectionEndId must not be null
   * \pre articleLinkField must not be ArticleConnectionStart_Id_FK or ArticleConnectionEnd_Id_FK
   */
  bool updateRelatedLinks(const QVariant & articleConnectionStartId, const QVariant & articleConnectionEndId, const QString & articleLinkField)
  {
    QStringList fields;
    fields.append(articleLinkField);
    return updateRelatedLinks(articleConnectionStartId, articleConnectionEndId, fields);
  }

  /*! \brief Remove a signle link from ArticleLink_tbl
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool removeLink(const QVariant & articleConnectionStartId, const QVariant & articleConnectionEndId);

  /*! \brief Remove each link that is contained in selection from ArticleLink_tbl
   */
  bool removeLinks(const mdtSqlTableSelection & s);

 private:

  /*! \brief Get article connection data
   *
   * \pre Given SQL statement must return 1 article connection row
   * \pre ok must be valid
   */
  mdtClArticleConnectionData getConnectionDataPv(const QString & sql, bool *ok);

  /*! \brief Get article connector data
   *
   * \param includeConnectionData If true, and SQL statement points to a article connector
   *            that contains article connections, result will be populated with these article connection data.
   * \param includeBaseConnectorData If true, and SQL statement points to a article connector
   *            that is based on a connector (from Connector_tbl), result will be populated with these connector data.
   * \pre Given SQL statement must return 1 article connector row
   * \pre ok must be valid
   */
  mdtClArticleConnectorData getConnectorDataPv(const QString & sql, bool *ok, bool includeConnectionData, bool includeBaseConnectorData);
};

#endif  // #ifndef MDT_CL_ARTICLE_H
