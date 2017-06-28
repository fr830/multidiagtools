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
#include "mdtCsvTest.h"
#include "mdtCsvStringParser.h"
#include "mdtCsvStringParserIterator.h"
#include "mdtCsvFileParserIterator.h"
#include "mdtCsvFileParserIteratorSharedData.h"
#include "mdtCsvFileParser.h"
#include "mdtCsvFileParserModel.h"
#include "mdtCsvStringGenerator.h"
#include "mdtCsvFileGenerator.h"
#include "mdtCsvSettings.h"
#include "mdtCsvData.h"
#include "mdt/csv/FieldType.h"
#include "mdt/csv/ParserFormatSetupProxyModel.h"
#include "mdt/csv/ParserFormatSetupDelegate.h"
#include "mdt/csv/RecordFormatter.h"
#include "mdtCsvParserSettingsWidget.h"
#include "mdtCsvGeneratorSettingsWidget.h"
#include "mdtCsvFileSettingsWidget.h"
#include "mdtCsvFileParserSettingsDialog.h"
#include "mdtCsvFileGeneratorSettingsDialog.h"
#include "mdtCsvTableViewDataMapper.h"
#include "mdtCsvTableViewExportDialog.h"
#include "mdtComboBoxItemDelegate.h"
#include "mdtApplication.h"
#include <QTemporaryFile>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QtTest>
#include <QTableView>
#include <QTreeView>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSignalSpy>
#include <string>
#include <vector>

#include <QDebug>

/** \todo  CSV "generator" (xy string out, CsvFileSave, see QFileSave).
 *         When a separator or eol is in string,
 *         do automatic data protection. Also double quote, etc...
 */

void mdtCsvTest::fileSettingsWidgetTest()
{
  mdtCsvFileSettingsWidget widget;

  /*
   * Set/get
   */
  // Set file settings
  widget.setFileSettings("/path/to/csvfile.csv", "UTF-16");
  // Check file settings
  QCOMPARE(widget.filePath(), QString("/path/to/csvfile.csv"));
  QCOMPARE(widget.fileEncoding(), QByteArray("UTF-16"));
  /*
   * Play
   */
  widget.setSelectFileMode(mdtCsvFileSettingsWidget::SelectOpen);
  widget.show();
  while(widget.isVisible()){
    QTest::qWait(500);
  }
}

void mdtCsvTest::parserSettingsTest()
{
  mdtCsvParserSettings s;

  /*
   * Initial state
   */
  QCOMPARE(s.fieldSeparator, ',');
  QCOMPARE(s.fieldProtection, '\"');
  QCOMPARE(s.parseExp, true);
  QVERIFY(s.isValid());
  /*
   * Some setup
   */
  // Wrong setup
  s.fieldSeparator = ',';
  s.fieldProtection = ',';
  s.parseExp = false;
  QVERIFY(!s.isValid());
  // Wrong setup
  s.fieldSeparator = ',';
  s.fieldProtection = '~';
  s.parseExp = true;
  QVERIFY(!s.isValid());
  // Good setup
  s.fieldSeparator = ';';
  s.fieldProtection = '\'';
  s.parseExp = false;
  QVERIFY(s.isValid());
  /*
   * Clear
   */
  s.clear();
  QCOMPARE(s.fieldSeparator, ',');
  QCOMPARE(s.fieldProtection, '\"');
  QCOMPARE(s.parseExp, true);
  QVERIFY(s.isValid());
}

void mdtCsvTest::parserSettingsWidgetTest()
{
  mdtCsvParserSettings settings;
  mdtCsvParserSettingsWidget widget;

  /*
   * Initial state
   */
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, ',');
  QCOMPARE(settings.fieldProtection, '\"');
  QCOMPARE(settings.parseExp, true);
  QVERIFY(settings.isValid());
  /*
   * Set/get
   */
  // Set some known settings
  settings.fieldSeparator = ':';
  settings.fieldProtection = '\'';
  settings.parseExp = true;
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, ':');
  QCOMPARE(settings.fieldProtection, '\'');
  QCOMPARE(settings.parseExp, true);
  QVERIFY(settings.isValid());
  // Set some known settings (TAB)
  settings.fieldSeparator = '\t';
  settings.fieldProtection = '\"';
  settings.parseExp = false;
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, '\t');
  QCOMPARE(settings.fieldProtection, '\"');
  QCOMPARE(settings.parseExp, false);
  QVERIFY(settings.isValid());
  // Set some unknown settings
  settings.fieldSeparator = 'a';
  settings.fieldProtection = 'b';
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, 'a');
  QCOMPARE(settings.fieldProtection, 'b');

  /*
   * Play
   */
//   widget.show();
//   while(widget.isVisible()){
//     QTest::qWait(500);
//   }
}

void mdtCsvTest::fileParserSettingsDialogTest()
{
  mdtCsvFileParserSettingsDialog dialog;
  mdtCsvParserSettings settings;
  QTemporaryFile csvFile1;

  /*
   * Prepare CSV source file 1
   */
  QVERIFY(csvFile1.open());
  QVERIFY(csvFile1.write("A,B,C,D\n") > 0);
  QVERIFY(csvFile1.write("A1,B1,C1,D1\n") > 0);
  QVERIFY(csvFile1.write("A2,B2,C2,D2\n") > 0);
  csvFile1.close();
  /*
   * Set/get
   */
  // Set file settings
  dialog.setFileSettings(csvFile1.fileName(), "UTF-8");
  // Set CSV settings
  settings.fieldSeparator = ',';
  settings.fieldProtection = '\"';
  settings.parseExp = true;
  dialog.setCsvSettings(settings);
  // Check file settings
  QCOMPARE(dialog.filePath(), csvFile1.fileName());
  QCOMPARE(dialog.fileEncoding(), QByteArray("UTF-8"));
  // Check CSV setting
  settings = dialog.getCsvSettings();
  QCOMPARE(settings.fieldSeparator, ',');
  QCOMPARE(settings.fieldProtection, '\"');
  QVERIFY(settings.parseExp);

  /*
   * Play
   */
  dialog.exec();
}

void mdtCsvTest::generatorSettingsTest()
{
  mdtCsvGeneratorSettings s;

  /*
   * Initial state
   */
  QCOMPARE(s.fieldSeparator, ',');
  QCOMPARE(s.fieldProtection, '\"');
  QCOMPARE(s.eol.c_str(), MDT_CSV_NATIVE_EOL);
  QVERIFY(!s.allwaysProtectTextFields);
  QVERIFY(s.isValid());
  /*
   * Some setup
   */
  // Good setup
  s.fieldSeparator = ',';
  s.fieldProtection = '\"';
  s.eol = "\n";
  s.allwaysProtectTextFields = true;
  QVERIFY(s.allwaysProtectTextFields);
  QVERIFY(s.isValid());
  // Wrong setup
  s.fieldSeparator = ',';
  s.fieldProtection = ',';
  s.eol = '\n';
  QVERIFY(!s.isValid());
  // Wrong setup
  s.fieldSeparator = ',';
  s.fieldProtection = '\"';
  s.eol = 'a';
  QVERIFY(!s.isValid());
  /*
   * Clear
   */
  s.clear();
  QCOMPARE(s.fieldSeparator, ',');
  QCOMPARE(s.fieldProtection, '\"');
  QCOMPARE(s.eol.c_str(), MDT_CSV_NATIVE_EOL);
  QVERIFY(!s.allwaysProtectTextFields);
  QVERIFY(s.isValid());
}

void mdtCsvTest::generatorSettingsWidgetTest()
{
  mdtCsvGeneratorSettingsWidget widget;
  mdtCsvGeneratorSettings settings;

  /*
   * Set/get
   */
  // Set some known settings
  settings.fieldSeparator = ':';
  settings.fieldProtection = '\'';
  settings.eol = "\r";
  settings.allwaysProtectTextFields = true;
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, ':');
  QCOMPARE(settings.fieldProtection, '\'');
  QCOMPARE(settings.eol.c_str(), "\r");
  QVERIFY(settings.allwaysProtectTextFields);
  // Set some known settings (TAB)
  settings.fieldSeparator = '\t';
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, '\t');
  // Set some unknown settings
  settings.fieldSeparator = 'a';
  settings.fieldProtection = 'b';
  widget.setSettings(settings);
  // Get settings back and check
  settings = widget.getSettings();
  QCOMPARE(settings.fieldSeparator, 'a');
  QCOMPARE(settings.fieldProtection, 'b');

  /*
   * Play
   */
//   widget.show();
//   while(widget.isVisible()){
//     QTest::qWait(500);
//   }
}

void mdtCsvTest::fileGeneratorSettingsDialogTest()
{
  mdtCsvFileGeneratorSettingsDialog dialog;
  mdtCsvGeneratorSettings settings;

  /*
   * Set/get
   */
  // Set file settings
  dialog.setFileSettings("/path/to/csvfile.csv", "UTF-16");
  // Set CSV settings
  settings.fieldSeparator = ':';
  settings.fieldProtection = '\'';
  settings.eol = "\r";
  settings.allwaysProtectTextFields = true;
  dialog.setCsvSettings(settings);
  // Check file settings
  QCOMPARE(dialog.filePath(), QString("/path/to/csvfile.csv"));
  QCOMPARE(dialog.fileEncoding(), QByteArray("UTF-16"));
  // Check CSV setting
  settings = dialog.getCsvSettings();
  QCOMPARE(settings.fieldSeparator, ':');
  QCOMPARE(settings.fieldProtection, '\'');
  QCOMPARE(settings.eol.c_str(), "\r");
  QVERIFY(settings.allwaysProtectTextFields);

  /*
   * Play
   */
  dialog.exec();
}

void mdtCsvTest::recordTest()
{
  mdtCsvRecord record;

  /*
   * Initial state
   */
  QCOMPARE(record.count(), 0);
  /*
   * Set
   */
  record.columnDataList.append("A");
  QCOMPARE(record.count(), 1);
  /*
   * Get
   */
  QCOMPARE(record.value(0), QVariant("A"));
  /*
   * Clear
   */
  record.clear();
  QCOMPARE(record.count(), 0);
}

void mdtCsvTest::dataTest()
{
  mdtCsvData data;
  mdtCsvRecord record;

  /*
   * Initial state
   */
  QCOMPARE(data.recordCount(), 0);
  /*
   * Set
   */
  data.addRecord(record);
  QCOMPARE(data.recordCount(), 1);
  /*
   * Clear
   */
  data.clear();
  QCOMPARE(data.recordCount(), 0);
}

void mdtCsvTest::fieldTypeTest()
{
  using mdt::csv::FieldType;

  /*
   * Type names
   */
  QCOMPARE(FieldType::nameFromType(QMetaType::Bool), QString("Boolean"));
  QCOMPARE(FieldType::nameFromType(QMetaType::Double), QString("Double"));
  /*
   * Get list of avaliable feild types
   */
  auto ftList = FieldType::getAvailableFieldTypeList();
  QCOMPARE((int)ftList.size(), 10);
  QVERIFY(ftList.at(0).type() == QMetaType::Bool);
  QCOMPARE(ftList.at(0).name(), QString("Boolean"));

  /*
   * Object construction
   */
  FieldType ft;
  // Default constrcuted
  QVERIFY(ft.type() == QMetaType::UnknownType);
  QVERIFY(ft.name().isEmpty());
  // Construct
  ft = FieldType(QMetaType::Double);
  QVERIFY(ft.type() == QMetaType::Double);
  QCOMPARE(ft.name(), QString("Double"));
}

void mdtCsvTest::recordFormatTest()
{
  mdt::csv::RecordFormat format;

  /*
   * Initial state
   */
  QCOMPARE(format.fieldCount(), 0);
  
  /*
   * Set field count
   */
  format.setFieldCount(3, QMetaType::QString);
  QCOMPARE(format.fieldCount(), 3);
  QVERIFY(format.fieldType(0) == QMetaType::QString);
  QVERIFY(format.fieldType(1) == QMetaType::QString);
  QVERIFY(format.fieldType(2) == QMetaType::QString);
  /*
   * Set field type
   */
  format.setFieldType(1, QMetaType::Double);
  QCOMPARE(format.fieldCount(), 3);
  QVERIFY(format.fieldType(0) == QMetaType::QString);
  QVERIFY(format.fieldType(1) == QMetaType::Double);
  QVERIFY(format.fieldType(2) == QMetaType::QString);

}

void mdtCsvTest::recordFormatterTest()
{
  mdt::csv::RecordFormatter formatter;
  mdt::csv::RecordFormat format;
  QVariant value;
  mdtCsvRecord record;

  /*
   * Initial state
   */
  QCOMPARE(formatter.fieldCount(), 0);
  /*
   * Set field count
   */
  formatter.setFieldCount(3, QMetaType::QString);
  QCOMPARE(formatter.fieldCount(), 3);
  QVERIFY(formatter.fieldType(0) == QMetaType::QString);
  QVERIFY(formatter.fieldType(1) == QMetaType::QString);
  QVERIFY(formatter.fieldType(2) == QMetaType::QString);
  /*
   * Set field type
   */
  formatter.setFieldType(2, QMetaType::Bool);
  QVERIFY(formatter.fieldType(0) == QMetaType::QString);
  QVERIFY(formatter.fieldType(1) == QMetaType::QString);
  QVERIFY(formatter.fieldType(2) == QMetaType::Bool);
  /*
   * Format values
   */
  value = "A";
  // Field 0 - String
  QVERIFY(formatter.formatValue(0, value));
  QCOMPARE(value, QVariant("A"));
  // Field 1 - String
  QVERIFY(formatter.formatValue(1, value));
  QCOMPARE(value, QVariant("A"));
  // Field 2 - Bool
  ///QVERIFY(!formatter.formatValue(2, value));
  qDebug() << "Value: " << value;
  QCOMPARE(value, QVariant("A"));
  /*
   * Format record
   */
  // Setup format
  formatter.setFieldCount(3, QMetaType::QString);
  formatter.setFieldType(0, QMetaType::Int);
  QCOMPARE(formatter.fieldCount(), 3);
  // Correct record
  record.clear();
  record.columnDataList << "1" << "A" << "B";
  QVERIFY(formatter.formatRecord(record));
  QCOMPARE(record.value(0), QVariant(1));
  QCOMPARE(record.value(1), QVariant("A"));
  QCOMPARE(record.value(2), QVariant("B"));
  // Smaller record
  record.clear();
  record.columnDataList << "2" << "A";
  QVERIFY(formatter.formatRecord(record));
  QCOMPARE(record.value(0), QVariant(2));
  QCOMPARE(record.value(1), QVariant("A"));
  // Record has more fields than format
  record.clear();
  record.columnDataList << "3" << "A" << "B" << "C";
  QVERIFY(formatter.formatRecord(record));
  QCOMPARE(record.value(0), QVariant(3));
  QCOMPARE(record.value(1), QVariant("A"));
  QCOMPARE(record.value(2), QVariant("B"));
  QCOMPARE(record.value(3), QVariant("C"));
}

// void mdtCsvTest::recordFormatSetupWidgetItemTest()
// {
//   using mdt::csv::RecordFormatSetupWidgetItem;
// 
//   RecordFormatSetupWidgetItem item;
//   QSignalSpy spy(&item, &RecordFormatSetupWidgetItem::fieldTypeChanged);
// 
//   item.setFieldName("Name");
//   item.setFieldType(QMetaType::QString);
//   QCOMPARE(spy.count(), 1);
//   QVERIFY(item.fieldType() == QMetaType::QString);
//   
//   qDebug() << "Signals: " << spy.count();
// 
//   /*
//    * Play
//    */
//   item.show();
//   while(item.isVisible()){
//     QTest::qWait(500);
//   }
// }

// void mdtCsvTest::recordFormatSetupWidgetTest()
// {
//   using mdt::csv::RecordFormatSetupWidget;
// 
//   RecordFormatSetupWidget widget;
//   mdtCsvRecord header;
//   QSignalSpy spy(&widget, &RecordFormatSetupWidget::fieldTypeChanged);
//   QList<QVariant> signalArguments;
// 
//   QVERIFY(spy.isValid());
//   /*
//    * Set 3 columns header
//    */
//   header.clear();
//   header.columnDataList << "A" << "B" << "C";
//   widget.setHeader(header);
//   QCOMPARE(spy.count(), 3);
//   // Check current type for column A
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 0); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column B
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 1); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column C
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 2); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   QCOMPARE(spy.count(), 0);
//   /*
//    * Set 2 columns header
//    */
//   header.clear();
//   header.columnDataList << "A" << "B";
//   widget.setHeader(header);
//   QCOMPARE(spy.count(), 2);
//   // Check current type for column A
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 0); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column B
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 1); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   QCOMPARE(spy.count(), 0);
//   /*
//    * Set 3 columns header
//    */
//   header.clear();
//   header.columnDataList << "A" << "B" << "C";
//   widget.setHeader(header);
//   QCOMPARE(spy.count(), 3);
//   // Check current type for column A
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 0); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column B
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 1); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column C
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 2); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   QCOMPARE(spy.count(), 0);
//   /*
//    * Set 3 columns header
//    */
//   header.clear();
//   header.columnDataList << "A" << "B" << "C";
//   widget.setHeader(header);
//   QCOMPARE(spy.count(), 3);
//   // Check current type for column A
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 0); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column B
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 1); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   // Check current type for column C
//   signalArguments = spy.takeFirst();
//   QCOMPARE(signalArguments.size(), 2);
//   QCOMPARE(signalArguments.at(0).toInt(), 2); // Field index
//   QCOMPARE(signalArguments.at(1).toInt(), static_cast<int>(QMetaType::QString)); // Field type
//   QCOMPARE(spy.count(), 0);
// 
//   /*
//    * Play
//    */
//   widget.show();
//   while(widget.isVisible()){
//     QTest::qWait(500);
//   }
// }

void mdtCsvTest::csvStringParserIteratorTest()
{
  QString str;

  /*
   * Constructs and assignements
   */
  str = "ABCDEF";
  // Direct assignement
  mdtCsvStringParserIterator it(str.cbegin());
  QCOMPARE(*it, wchar_t('A'));
  // Default constructed
  mdtCsvStringParserIterator first, last;
  // Assignement
  first = str.cbegin();
  QCOMPARE(*first, wchar_t('A'));
  last = first;
  QCOMPARE(*last, wchar_t('A'));
  /*
   * Increment
   */
  it = str.cbegin();
  QCOMPARE(*it, wchar_t('A'));
  // Pre-increment
  ++it;
  QCOMPARE(*it, wchar_t('B'));
  // Post-increment
  QCOMPARE(*it++, wchar_t('B'));
  QCOMPARE(*it, wchar_t('C'));
  /*
   * Decrement
   */
  it = str.cbegin();
  QCOMPARE(*it, wchar_t('A'));
  ++it;
  ++it;
  QCOMPARE(*it, wchar_t('C'));
  // Pre-decrement
  --it;
  QCOMPARE(*it, wchar_t('B'));
  // Post-decrement
  QCOMPARE(*it--, wchar_t('B'));
  QCOMPARE(*it, wchar_t('A'));
  /*
   * Increment and decrement by n
   */
  it = str.cbegin();
  QCOMPARE(*it, wchar_t('A'));
  it += 2;
  QCOMPARE(*it, wchar_t('C'));
  it -= 2;
  QCOMPARE(*it, wchar_t('A'));
  first = it;
  it = first + 2;
  QCOMPARE(*it, wchar_t('C'));
  first = it - 1;
  QCOMPARE(*first, wchar_t('B'));
  /*
   * Index based access
   */
  it = str.cbegin();
  QCOMPARE(it[0], wchar_t('A'));
  QCOMPARE(it[1], wchar_t('B'));
  QCOMPARE(it[2], wchar_t('C'));
  /*
   * Comparisons
   */
  first = str.cbegin();
  it = first;
  QVERIFY(it == first);
  QVERIFY(!(it != first));
  QVERIFY(!(it < first));
  QVERIFY(it <= first);
  QVERIFY(!(it > first));
  QVERIFY(it >= first);
  // Increment it and check
  ++it;
  QVERIFY(!(it == first));
  QVERIFY(it != first);
  QVERIFY(!(it < first));
  QVERIFY(!(it <= first));
  QVERIFY(it > first);
  QVERIFY(it >= first);
}

void mdtCsvTest::csvStringParserIteratorBenchmark()
{
  QString source = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
  std::vector<wchar_t> destination;

  destination.reserve(source.size());
  QBENCHMARK
  {
    destination.clear();
    mdtCsvStringParserIterator first(source.cbegin());
    mdtCsvStringParserIterator last(source.cend());
    while(first != last){
      destination.push_back(*first);
      ++first;
    }
  }
  QCOMPARE((int)destination.size(), source.size());
}

void mdtCsvTest::stringParserReadLineTest()
{
  QFETCH(QString, sourceData);
  QFETCH(mdtCsvData, expectedData);
  QFETCH(bool, expectedOk);
  QFETCH(mdtCsvParserSettings, csvSettings);
  mdtCsvStringParser parser;
  mdtExpected<mdtCsvRecord> record;
  mdtCsvData data;

  /*
   * Initial state
   */
  QVERIFY(parser.atEnd());
  // Setup CSV source string and parser
  parser.setCsvSettings(csvSettings);
  parser.setSource(sourceData);
  // Parse line by line
  while(!parser.atEnd()){
    record = parser.readLine();
    if(expectedOk){
      QVERIFY(record.hasValue());
    }else{
      QVERIFY(record.hasError());
    }
    data.addRecord(record.value());
  }
  // Check
  QCOMPARE(data.recordCount(), expectedData.recordCount());
  for(int row = 0; row < data.recordCount(); ++row){
    auto rec = data.recordList.at(row);
    auto expectedRecord = expectedData.recordList.at(row);
    QCOMPARE(rec.count(), expectedRecord.count());
    for(int col = 0; col < rec.count(); ++col){
      auto colData = rec.columnDataList.at(col);
      auto expectedColData = expectedRecord.columnDataList.at(col);
      QCOMPARE(colData, expectedColData);
    }
  }
}

void mdtCsvTest::stringParserReadLineTest_data()
{
  buildCsvParserTestData();
}

void mdtCsvTest::stringParserReadAllTest()
{
  QFETCH(QString, sourceData);
  QFETCH(mdtCsvData, expectedData);
  QFETCH(bool, expectedOk);
  QFETCH(mdtCsvParserSettings, csvSettings);
  mdtCsvStringParser parser(csvSettings);
  mdtCsvRecord record;
  mdtExpected<mdtCsvData> data;

  /*
   * Initial state
   */
  QVERIFY(parser.atEnd());
  // Setup CSV source string and parser
  parser.setSource(sourceData);
  // Parse the entires string
  data = parser.readAll();
  if(expectedOk){
    QVERIFY(data.hasValue());
  }else{
    QVERIFY(data.hasError());
    return;
  }
  QVERIFY(data.hasValue());
  // Check
  QCOMPARE(data.value().recordCount(), expectedData.recordCount());
  for(int row = 0; row < data.value().recordCount(); ++row){
    record = data.value().recordList.at(row);
    auto expectedRecord = expectedData.recordList.at(row);
    QCOMPARE(record.count(), expectedRecord.count());
    for(int col = 0; col < record.count(); ++col){
      auto colData = record.columnDataList.at(col);
      auto expectedColData = expectedRecord.columnDataList.at(col);
      QCOMPARE(colData, expectedColData);
    }
  }
}

void mdtCsvTest::stringParserReadAllTest_data()
{
  buildCsvParserTestData();
}

void mdtCsvTest::csvFileParserIteratorSharedDataTest()
{
  mdtCsvFileParserIteratorSharedData sd;
  QTemporaryFile file;

  QVERIFY(QTextCodec::codecForName("UTF-8") != nullptr);
  /*
   * Initial state
   */
  QVERIFY(sd.atEnd());
  /*
   * set source to a empty file
   */
  // Pass file that is no allready open + existing encoding
  QVERIFY(!file.isOpen());
  QVERIFY(!sd.setSource(&file, "UTF-8"));
  QVERIFY(!file.isOpen());
  QVERIFY(sd.atEnd());
  // Pass file that is allready open + existing encoding
  QVERIFY(file.open());
  QVERIFY(sd.setSource(&file, "UTF-8"));
  QVERIFY(file.isOpen());
  QVERIFY(sd.atEnd());
  // Pass a file that is allready open + NON existing encoding
  QVERIFY(!sd.setSource(&file, "SomeUnknownEncoding"));
  QVERIFY(sd.atEnd());
  QVERIFY(file.isOpen()); // file must be keeped untouched
  /*
   * Prepare file with some data
   */
  file.close();
  QVERIFY(file.open());
  QVERIFY(file.write("ABCDE") > 0);
  file.close();
  /*
   * Some flags checks
   */
  QVERIFY(file.open());
  QVERIFY(sd.setSource(&file, "UTF-8"));
  QVERIFY(!sd.atEnd());
  QVERIFY(!sd.atEnd());
  // Close file and check that sd is at end
  file.close();
  ///QVERIFY(sd.atEnd());
  /*
   * Check getting data
   */
  QVERIFY(file.open());
  QVERIFY(sd.setSource(&file, "UTF-8"));
  QVERIFY(file.isOpen());
  // Get first char
  QVERIFY(!sd.atEnd());
  QCOMPARE(sd.get(), QChar('A'));
  QCOMPARE(sd.get(), QChar('A'));
  // Step and get
  QVERIFY(!sd.atEnd());
  QVERIFY(sd.advance());
  QCOMPARE(sd.get(), QChar('B'));
  // Step and get
  QVERIFY(!sd.atEnd());
  QVERIFY(sd.advance());
  QCOMPARE(sd.get(), QChar('C'));
  // Step and get
  QVERIFY(!sd.atEnd());
  QVERIFY(sd.advance());
  QCOMPARE(sd.get(), QChar('D'));
  // Step and get
  QVERIFY(!sd.atEnd());
  QVERIFY(sd.advance());
  QCOMPARE(sd.get(), QChar('E'));
  // Step again - Now we reached EOF
  QVERIFY(sd.advance());
  QVERIFY(sd.atEnd());
  /*
   * Check error handling
   */
  // Re-open file
  file.close();
  QVERIFY(file.open());
  mdtCsvFileParserIteratorSharedData sd2(1);
  QVERIFY(sd2.setSource(&file, "UTF-8"));
  // Get first char
  QVERIFY(!sd2.atEnd());
  QCOMPARE(sd2.get(), QChar('A'));
  // Step and get
  QVERIFY(sd2.advance());
  QVERIFY(!sd2.atEnd());
  QCOMPARE(sd2.get(), QChar('B'));
  // Close file
  file.close();
  QVERIFY(!sd2.advance());
  QVERIFY(sd2.atEnd());
}

void mdtCsvTest::csvFileParserIteratorSharedDataReadTest()
{
  QTemporaryFile file;
  QFETCH(QString, fileData);
  QFETCH(QString, fileEncoding);
  QFETCH(int, rawDataBufferSize);
  QString unicodeBuffer;
  QByteArray fileRawData;
  QTextCodec *codec;
  mdtCsvFileParserIteratorSharedData sd(rawDataBufferSize);

  QVERIFY(QTextCodec::codecForName(fileEncoding.toLatin1()) != nullptr);
  /*
   * Write our test data
   */
  // Encode test data
  codec = QTextCodec::codecForName(fileEncoding.toLatin1());
  QVERIFY(codec != nullptr);
  fileRawData = codec->fromUnicode(fileData);
  // Write test data
  QVERIFY(file.open());
  QVERIFY(file.write(fileRawData) == fileRawData.size());
  // Close file (to flush data) and re-open it
  file.close();
  QVERIFY(file.open());
  /*
   * Check
   */
  QVERIFY(unicodeBuffer.isEmpty());
  // Read data
  QVERIFY(sd.setSource(&file, fileEncoding.toLatin1()));
//   sd.setInternalRawDataBufferSize(rawDataBufferSize);
  while(!sd.atEnd()){
    unicodeBuffer.append(sd.get());
    QVERIFY(sd.advance());
  }
  // Check
  QCOMPARE(unicodeBuffer, fileData);
}

void mdtCsvTest::csvFileParserIteratorSharedDataReadTest_data()
{
  QTest::addColumn<QString>("fileData");
  QTest::addColumn<QString>("fileEncoding");
  QTest::addColumn<int>("rawDataBufferSize");

  // Note: only edit this part with a UTF-8 editor
  QTest::newRow("Empty file,UTF-8,1") << "" << "UTF-8" << 1;
  QTest::newRow("Empty file,UTF-8,2") << "" << "UTF-8" << 2;
  QTest::newRow("Empty file,UTF-8,3") << "" << "UTF-8" << 3;
  QTest::newRow("Empty file,UTF-8,1024") << "" << "UTF-8" << 1024;
  QTest::newRow("A,UTF-8,1") << "A" << "UTF-8" << 1;
  QTest::newRow("A,UTF-8,2") << "A" << "UTF-8" << 2;
  QTest::newRow("A,UTF-8,3") << "A" << "UTF-8" << 3;
  QTest::newRow("A,UTF-8,4") << "A" << "UTF-8" << 4;
  QTest::newRow("A,UTF-8,1024") << "A" << "UTF-8" << 1024;
  QTest::newRow("ö,UTF-8,1") << "ö" << "UTF-8" << 1;
  QTest::newRow("ö,UTF-8,2") << "ö" << "UTF-8" << 2;
  QTest::newRow("ö,UTF-8,3") << "ö" << "UTF-8" << 3;
  QTest::newRow("ö,UTF-8,4") << "ö" << "UTF-8" << 4;
  QTest::newRow("ö,UTF-8,1024") << "ö" << "UTF-8" << 1024;
  QTest::newRow("AB,UTF-8,1") << "AB" << "UTF-8" << 1;
  QTest::newRow("AB,UTF-8,2") << "AB" << "UTF-8" << 2;
  QTest::newRow("AB,UTF-8,3") << "AB" << "UTF-8" << 3;
  QTest::newRow("AB,UTF-8,4") << "AB" << "UTF-8" << 4;
  QTest::newRow("AB,UTF-8,1024") << "AB" << "UTF-8" << 1024;
  QTest::newRow("Aö,UTF-8,1") << "Aö" << "UTF-8" << 1;
  QTest::newRow("Aö,UTF-8,2") << "Aö" << "UTF-8" << 2;
  QTest::newRow("Aö,UTF-8,3") << "Aö" << "UTF-8" << 3;
  QTest::newRow("Aö,UTF-8,4") << "Aö" << "UTF-8" << 4;
  QTest::newRow("Aö,UTF-8,1024") << "Aö" << "UTF-8" << 1024;
  QTest::newRow("AöB,UTF-8,1") << "AöB" << "UTF-8" << 1;
  QTest::newRow("AöB,UTF-8,2") << "AöB" << "UTF-8" << 2;
  QTest::newRow("AöB,UTF-8,3") << "AöB" << "UTF-8" << 3;
  QTest::newRow("AöB,UTF-8,4") << "AöB" << "UTF-8" << 4;
  QTest::newRow("AöB,UTF-8,5") << "AöB" << "UTF-8" << 5;
  QTest::newRow("AöB,UTF-8,6") << "AöB" << "UTF-8" << 6;
  QTest::newRow("AöB,UTF-8,1024") << "AöB" << "UTF-8" << 1024;
  QTest::newRow("Medium ASCII string,UTF-8,3") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789\nabcdefghijklmnopqrstuvwxyz" << "UTF-8" << 3;
  QTest::newRow("Medium ASCII string,UTF-8,1024") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789\nabcdefghijklmnopqrstuvwxyz" << "UTF-8" << 1024;
  QTest::newRow("Medium string,UTF-8,3") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-8" << 3;
  QTest::newRow("Medium string,UTF-8,1024") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-8" << 1024;
  QTest::newRow("Medium string,UTF-16,3") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-16" << 3;
  QTest::newRow("Medium string,UTF-16,1024") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-16" << 1024;
  QTest::newRow("Medium string,UTF-32,3") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-32" << 3;
  QTest::newRow("Medium string,UTF-32,1024") << "ABCDEFGHIJKLMNOPQRSTUVWXYZ@01€23456789aäbcdeéfghijklmnoöpqrstuvwxyz" << "UTF-32" << 1024;
}

void mdtCsvTest::csvFileParserIteratorTest()
{
  QTemporaryFile file;
  QString str;

  QVERIFY(QTextCodec::codecForName("UTF-8") != nullptr);
  /*
   * Prepare file
   */
  QVERIFY(file.open());
  QVERIFY(file.write("ABCDE") > 0);
  file.close();
  /*
   * Constructs
   */
  // Default (end-of-stream iterator)
  mdtCsvFileParserIterator it;
  QVERIFY(!it.errorOccured());
  QVERIFY(it.isEof());
  // Copy
  mdtCsvFileParserIterator it2(it);
  QVERIFY(!it2.errorOccured());
  QVERIFY(it2.isEof());
  QVERIFY(it == it2);
  QVERIFY(!(it != it2));
  // Assign file to it
  QVERIFY(file.open());
  QVERIFY(it.setSource(&file, "UTF-8"));
  QVERIFY(!it.errorOccured());
  QVERIFY(!it.isEof());
  file.close();
  // Assign again
  QVERIFY(file.open());
  QVERIFY(it.setSource(&file, "UTF-8"));
  QVERIFY(!it.errorOccured());
  QVERIFY(!it.isEof());
  // Clear
  it.clear();
  QVERIFY(it.isEof());
  file.close();
  // Assign again
  QVERIFY(file.open());
  QVERIFY(it.setSource(&file, "UTF-8"));
  QVERIFY(!it.errorOccured());
  QVERIFY(!it.isEof());
  file.close();
  // Construct it3 that acts on file
  QVERIFY(file.open());
  mdtCsvFileParserIterator it3(&file, "UTF-8");
  QVERIFY(!it3.errorOccured());
  QVERIFY(!it3.isEof());
  QVERIFY(*it3 == 'A');
  // Construct it4 that is a copy of it3
  mdtCsvFileParserIterator it4(it3);
  QVERIFY(!it4.errorOccured());
  QVERIFY(!it4.isEof());
  QVERIFY(*it4 == 'A');
  QVERIFY(it4 == it3);
  // Assign it4 to it
  it = it4;
  QVERIFY(!it.errorOccured());
  QVERIFY(!it.isEof());
  QVERIFY(*it == 'A');
  QVERIFY(it == it4);
  // Step it and check
  QVERIFY(!it.isEof());
  ++it;
  QVERIFY(*it == 'B');
  // Step it and check
  QVERIFY(!it.isEof());
  ++it;
  QVERIFY(*it == 'C');
  // Step it and check
  QVERIFY(!it.isEof());
  ++it;
  QVERIFY(*it == 'D');
  // Step it and check
  QVERIFY(!it.isEof());
  ++it;
  QVERIFY(*it == 'E');
  // Step - Now we reached EOF
  ++it;
  QVERIFY(it.isEof());
  /*
   * Some error handling
   */
  // Close the file and check that we end up with end-of-stream iterators
  file.close();
  QVERIFY(it.isEof());
  QVERIFY(it3.isEof());
  QVERIFY(it4.isEof());
  /*
   * Check reading the whole file
   */
  file.close();
  QVERIFY(file.open());
  mdtCsvFileParserIterator first(&file, "UTF-8");
  mdtCsvFileParserIterator last;
  str.clear();
  while(first != last){
    str.append(static_cast<ushort>(*first));
    ++first;
  }
  QCOMPARE(str, QString("ABCDE"));
}

void mdtCsvTest::csvFileParserTest()
{
  mdtCsvFileParser parser;
  QTemporaryFile file;
  QString str;
  mdtExpected<mdtCsvRecord> record;

  QVERIFY(QTextCodec::codecForName("UTF-8") != nullptr);
  /*
   * Prepare file
   */
  QVERIFY(file.open());
  QVERIFY(file.write(u8"ABCDE\n1234\n") > 0);
  file.close();
  /*
   * Initial state
   */
  QVERIFY(!parser.isValid());
  QVERIFY(!parser.isOpen());
  parser.setCsvSettings(mdtCsvParserSettings());
  QVERIFY(parser.isValid());
  QVERIFY(parser.atEnd());
  QVERIFY(!parser.isOpen());
  /*
   * Open/close test
   */
  QVERIFY(parser.openFile(file.fileName(), "UTF-8"));
  QVERIFY(parser.isOpen());
  QVERIFY(!parser.atEnd());
  parser.closeFile();
  QVERIFY(!parser.isOpen());
  QVERIFY(parser.atEnd());
  QVERIFY(parser.openFile(file.fileName(), "UTF-8"));
  QVERIFY(parser.isOpen());
  QVERIFY(!parser.atEnd());
  /*
   * Read line by line
   */
  // Read first line
  QVERIFY(!parser.atEnd());
  record = parser.readLine();
  QVERIFY(record.hasValue());
  QCOMPARE(record.value().count(), 1);
  QCOMPARE(record.value().columnDataList.at(0), QVariant("ABCDE"));
  // Read second line
  QVERIFY(!parser.atEnd());
  record = parser.readLine();
  QVERIFY(record.hasValue());
  QCOMPARE(record.value().count(), 1);
  QCOMPARE(record.value().columnDataList.at(0), QVariant("1234"));
  QVERIFY(parser.atEnd());
  parser.closeFile();
  /*
   * Update CSV settings test
   */
  parser.setCsvSettings(mdtCsvParserSettings());
  QVERIFY(parser.openFile(file.fileName(), "UTF-8"));
  // Read first line
  QVERIFY(!parser.atEnd());
  record = parser.readLine();
  QVERIFY(record.hasValue());
  QCOMPARE(record.value().count(), 1);
  QCOMPARE(record.value().columnDataList.at(0), QVariant("ABCDE"));
  // Read second line
  QVERIFY(!parser.atEnd());
  record = parser.readLine();
  QVERIFY(record.hasValue());
  QCOMPARE(record.value().count(), 1);
  QCOMPARE(record.value().columnDataList.at(0), QVariant("1234"));
  QVERIFY(parser.atEnd());
}

void mdtCsvTest::csvFileParserReadLineTest()
{
  QFETCH(QString, sourceData);
  QFETCH(mdtCsvData, expectedData);
  QFETCH(bool, expectedOk);
  QFETCH(mdtCsvParserSettings, csvSettings);
  mdtCsvFileParser parser;
  mdtCsvRecord record;
  mdtCsvData data;
  QTemporaryFile file;
  QTextCodec *codec;
  QByteArray rawFileData;

  /*
   * Prepare file
   */
  codec = QTextCodec::codecForName("UTF-8");
  QVERIFY(codec != nullptr);
  rawFileData = codec->fromUnicode(sourceData);
  QVERIFY(file.open());
  QVERIFY(file.write(rawFileData) >= rawFileData.size());
  file.close();
  /*
   * Initial state
   */
  QVERIFY(parser.atEnd());
  // Setup CSV parser
  parser.setCsvSettings(csvSettings);
  QVERIFY(parser.openFile(file.fileName(), "UTF-8"));
  // Parse line by line
  while(!parser.atEnd()){
    auto rec = parser.readLine();
    if(expectedOk){
      QVERIFY(rec.hasValue());
      data.addRecord(rec.value());
    }else{
      QVERIFY(rec.hasError());
    }
  }
  // Check
  QCOMPARE(data.recordCount(), expectedData.recordCount());
  for(int row = 0; row < data.recordCount(); ++row){
    record = data.recordList.at(row);
    auto expectedRecord = expectedData.recordList.at(row);
    QCOMPARE(record.count(), expectedRecord.count());
    for(int col = 0; col < record.count(); ++col){
      auto colData = record.columnDataList.at(col);
      auto expectedColData = expectedRecord.columnDataList.at(col);
      QCOMPARE(colData, expectedColData);
    }
  }
  parser.closeFile();
}

void mdtCsvTest::csvFileParserReadLineTest_data()
{
  buildCsvParserTestData();
}

void mdtCsvTest::csvFileParserReadAllTest()
{
  QFETCH(QString, sourceData);
  QFETCH(mdtCsvData, expectedData);
  QFETCH(bool, expectedOk);
  QFETCH(mdtCsvParserSettings, csvSettings);
  mdtCsvFileParser parser;
  mdtCsvRecord record;
  mdtCsvData data;
  QTemporaryFile file;
  QTextCodec *codec;
  QByteArray rawFileData;

  /*
   * Prepare file
   */
  codec = QTextCodec::codecForName("UTF-8");
  QVERIFY(codec != nullptr);
  rawFileData = codec->fromUnicode(sourceData);
  QVERIFY(file.open());
  QVERIFY(file.write(rawFileData) >= rawFileData.size());
  file.close();
  /*
   * Initial state
   */
  QVERIFY(parser.atEnd());
  // Setup CSV parser
  parser.setCsvSettings(csvSettings);
  QVERIFY(parser.openFile(file.fileName(), "UTF-8"));
  // Parse file
  auto exData = parser.readAll();
  if(expectedOk){
    QVERIFY(exData.hasValue());
    data = exData.value();
  }else{
    QVERIFY(exData.hasError());
  }
  // Check
  QCOMPARE(data.recordCount(), expectedData.recordCount());
  for(int row = 0; row < data.recordCount(); ++row){
    record = data.recordList.at(row);
    auto expectedRecord = expectedData.recordList.at(row);
    QCOMPARE(record.count(), expectedRecord.count());
    for(int col = 0; col < record.count(); ++col){
      auto colData = record.columnDataList.at(col);
      auto expectedColData = expectedRecord.columnDataList.at(col);
      QCOMPARE(colData, expectedColData);
    }
  }
  parser.closeFile();
}

void mdtCsvTest::csvFileParserReadAllTest_data()
{
  buildCsvParserTestData();
}

void mdtCsvTest::fileParserModelTest()
{
  mdtCsvFileParserModel model;
  QModelIndex index;
  QTableView tableView;
  QTreeView treeView;
  mdtCsvParserSettings csvSettings;
  QTemporaryFile csvFile1, csvFile2;

  /*
   * Initial state
   */
  QCOMPARE(model.rowCount(), 0);
  QCOMPARE(model.columnCount(), 0);
  /*
   * Prepare CSV source file 1
   *
   * Simple file
   */
  QVERIFY(csvFile1.open());
  QVERIFY(csvFile1.write("A,B,C\n") > 0);
  QVERIFY(csvFile1.write("A1,B1,C1\n") > 0);
  QVERIFY(csvFile1.write("A2,B2,C2\n") > 0);
  QVERIFY(csvFile1.write("A3,B3,C3\n") > 0);
  csvFile1.close();
  /*
   * Set CSV settings
   */
  QVERIFY(model.setCsvSettings(csvSettings));
  // Because we never set a file, model must be empty
  QCOMPARE(model.rowCount(), 0);
  QCOMPARE(model.columnCount(), 0);
  /*
   * Open CSV file 1
   */
  QVERIFY(model.setFile(csvFile1.fileName(), "UTF-8"));
  // Check header
  QCOMPARE(model.columnCount(), 3);
  QCOMPARE(model.headerData(0, Qt::Horizontal), QVariant("A"));
  QCOMPARE(model.headerData(1, Qt::Horizontal), QVariant("B"));
  QCOMPARE(model.headerData(2, Qt::Horizontal), QVariant("C"));
  // Check data
  QCOMPARE(model.rowCount(), 0);
  /*
   * Fetch data
   */
  // Fetch row 0
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 1);
  // Fetch row 1
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 2);
  // Fetch row 3
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 3);
  // End
  QVERIFY(!model.canFetchMore(QModelIndex()));
  /*
   * Check data
   */
  // Check row 0
  index = model.index(0, 0);
  QVERIFY(index.isValid());
  QCOMPARE(model.data(index), QVariant("A1"));
  index = model.index(0, 1);
  QVERIFY(index.isValid());
  QCOMPARE(model.data(index), QVariant("B1"));
  index = model.index(0, 2);
  QVERIFY(index.isValid());
  QCOMPARE(model.data(index), QVariant("C1"));

  /*
   * Prepare CSV source file 2
   *
   * File with variable columns count
   */
  QVERIFY(csvFile2.open());
  QVERIFY(csvFile2.write("A,B,C\n") > 0);
  QVERIFY(csvFile2.write("A1,B1,C1\n") > 0);
  QVERIFY(csvFile2.write("A2,B2\n") > 0);
  QVERIFY(csvFile2.write("A2,B2,C2\n") > 0);
  QVERIFY(csvFile2.write("A2,B2,C2,D2\n") > 0);
  csvFile2.close();
  /*
   * Open CSV file 2
   */
  QVERIFY(model.setFile(csvFile2.fileName(), "UTF-8"));
  // Check header
  QCOMPARE(model.columnCount(), 3);
  QCOMPARE(model.headerData(0, Qt::Horizontal), QVariant("A"));
  QCOMPARE(model.headerData(1, Qt::Horizontal), QVariant("B"));
  QCOMPARE(model.headerData(2, Qt::Horizontal), QVariant("C"));
  // Check data
  QCOMPARE(model.rowCount(), 0);
  /*
   * Fetch data
   */
  // Fetch row 0
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(model.columnCount(), 3);
  // Fetch row 1
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 2);
  QCOMPARE(model.columnCount(), 3);
  // Fetch row 2
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 3);
  QCOMPARE(model.columnCount(), 3);
  // Fetch row 3
  QVERIFY(model.canFetchMore(QModelIndex()));
  model.fetchMore(QModelIndex());
  QCOMPARE(model.rowCount(), 4);
  QCOMPARE(model.columnCount(), 4);

  /*
   * Setup views
   */
  // Setup table view
  tableView.setModel(&model);
  tableView.resize(600, 200);
  tableView.show();
  // Setup tree view
  treeView.setModel(&model);
  treeView.show();

  /*
   * Play
   */
  tableView.resizeColumnsToContents();
  while(tableView.isVisible()){
    QTest::qWait(500);
  }
  
  QFAIL("Test not complete now");
}

void mdtCsvTest::parserFormatSetupProxyModelTest()
{
  mdtCsvFileParserModel model;
  mdt::csv::ParserFormatSetupProxyModel proxyModel;
  mdt::csv::RecordFormat format;
  QModelIndex index;
  QTableView tableView;
  QTreeView treeView;
  auto *formatDelegate = new mdt::csv::ParserFormatSetupDelegate(&tableView);
  mdtCsvParserSettings csvSettings;
  QTemporaryFile csvFile1, csvFile2;

  /*
   * Setup views
   */
  // Setup table view
  tableView.setModel(&proxyModel);
  tableView.setItemDelegateForRow(0, formatDelegate);
  tableView.resize(600, 200);
  tableView.show();
  // Setup tree view
  treeView.setModel(&proxyModel);
  treeView.show();
  /*
   * Initial state
   */
  QCOMPARE(proxyModel.rowCount(), 1);
  QCOMPARE(proxyModel.columnCount(), 0);
  /// \todo Check header
  /*
   * Setup model
   */
  proxyModel.setSourceModel(&model);
  QCOMPARE(proxyModel.rowCount(), 1);
  QCOMPARE(proxyModel.columnCount(), 0);
  /// \todo Check header
  /*
   * Prepare CSV source file 1
   *
   * Simple file
   */
  QVERIFY(csvFile1.open());
  QVERIFY(csvFile1.write("A,B,C\n") > 0);
  QVERIFY(csvFile1.write("A1,B1,C1\n") > 0);
  QVERIFY(csvFile1.write("A2,B2,C2\n") > 0);
  QVERIFY(csvFile1.write("A3,B3,C3\n") > 0);
  csvFile1.close();
  /*
   * Set CSV settings
   */
  QVERIFY(model.setCsvSettings(csvSettings));
  // Because we never set a file, model has only row 0 (format selection)
  QCOMPARE(proxyModel.rowCount(), 1);
  QCOMPARE(proxyModel.columnCount(), 0);
  /*
   * Open CSV file 1
   */
  QVERIFY(model.setFile(csvFile1.fileName(), "UTF-8"));
  // Check header
  QCOMPARE(proxyModel.columnCount(), 3);
  QCOMPARE(proxyModel.headerData(0, Qt::Horizontal), QVariant("A"));
  QCOMPARE(proxyModel.headerData(1, Qt::Horizontal), QVariant("B"));
  QCOMPARE(proxyModel.headerData(2, Qt::Horizontal), QVariant("C"));
  // Check data
  QCOMPARE(proxyModel.rowCount(), 1);
  /*
   * Fetch data
   */
  // Fetch row 0
  QVERIFY(proxyModel.canFetchMore(QModelIndex()));
  proxyModel.fetchMore(QModelIndex());
  QCOMPARE(proxyModel.rowCount(), 2);
  // Fetch row 1
  QVERIFY(proxyModel.canFetchMore(QModelIndex()));
  proxyModel.fetchMore(QModelIndex());
  QCOMPARE(proxyModel.rowCount(), 3);
  // Fetch row 3
  QVERIFY(proxyModel.canFetchMore(QModelIndex()));
  proxyModel.fetchMore(QModelIndex());
  QCOMPARE(proxyModel.rowCount(), 4);
  // End
  QVERIFY(!proxyModel.canFetchMore(QModelIndex()));
  /*
   * Check data
   */
  QCOMPARE(proxyModel.rowCount(), 4);
  // Check row 0 (formats)
  index = proxyModel.index(0, 0);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("String"));
  index = proxyModel.index(0, 1);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("String"));
  index = proxyModel.index(0, 2);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("String"));
  // Check row 1
  index = proxyModel.index(1, 0);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("A1"));
  index = proxyModel.index(1, 1);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("B1"));
  index = proxyModel.index(1, 2);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("C1"));
  // Check row 2
  index = proxyModel.index(2, 0);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("A2"));
  index = proxyModel.index(2, 1);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("B2"));
  index = proxyModel.index(2, 2);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("C2"));
  // Check row 3
  index = proxyModel.index(3, 0);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("A3"));
  index = proxyModel.index(3, 1);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("B3"));
  index = proxyModel.index(3, 2);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant("C3"));
  /*
   * Check record format
   */
  format = proxyModel.recordFormat();
  QCOMPARE(format.fieldCount(), 3);
  QVERIFY(format.fieldType(0) == QMetaType::QString);
  QVERIFY(format.fieldType(1) == QMetaType::QString);
  QVERIFY(format.fieldType(2) == QMetaType::QString);
  /*
   * Update format
   */
  format.setFieldType(0, QMetaType::Int);
  proxyModel.setRecordFormat(format);
  // Check column 0
  index = proxyModel.index(0, 0);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant(mdt::csv::FieldType::nameFromType(QMetaType::Int)));
  // Check column 1
  index = proxyModel.index(0, 1);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant(mdt::csv::FieldType::nameFromType(QMetaType::QString)));
  // Check column 2
  index = proxyModel.index(0, 2);
  QVERIFY(index.isValid());
  QCOMPARE(proxyModel.data(index), QVariant(mdt::csv::FieldType::nameFromType(QMetaType::QString)));

  /*
   * Play
   */
  tableView.resizeColumnsToContents();
  while(tableView.isVisible()){
    QTest::qWait(500);
  }
}

void mdtCsvTest::csvStringGeneratorFromRecordTest()
{
  QFETCH(mdtCsvData, sourceData);
  QFETCH(QString, expectedCsvString);
  QFETCH(mdtCsvGeneratorSettings, csvSettings);
  QString csvString;
  mdtCsvStringGenerator generator(csvSettings);

  /*
   * Initial state
   */
  
  /*
   * Write each line
   */
  for(const auto & record : sourceData.recordList){
    csvString += generator.getCsvString(record);
  }
  /*
   * Check
   */
  QCOMPARE(csvString, expectedCsvString);
}

void mdtCsvTest::csvStringGeneratorFromRecordTest_data()
{
  buildCsvGeneratorTestData();
}

void mdtCsvTest::csvStringGeneratorFromDataTest()
{
  QFETCH(mdtCsvData, sourceData);
  QFETCH(QString, expectedCsvString);
  QFETCH(mdtCsvGeneratorSettings, csvSettings);
  QString csvString;
  mdtCsvStringGenerator generator(csvSettings);

  /*
   * Initial state
   */
  
  /*
   * Convert
   */
  csvString = generator.getCsvString(sourceData);
  /*
   * Check
   */
  QCOMPARE(csvString, expectedCsvString);
}

void mdtCsvTest::csvStringGeneratorFromDataTest_data()
{
  buildCsvGeneratorTestData();
}

void mdtCsvTest::csvFileGeneratorWriteLineTest()
{
  QFETCH(mdtCsvData, sourceData);
  QFETCH(QString, expectedCsvString);
  QFETCH(mdtCsvGeneratorSettings, csvSettings);
  QString csvString;
  mdtCsvFileGenerator generator(csvSettings);
  QTemporaryFile file;
  QTextCodec *codec;

  codec = QTextCodec::codecForName("UTF-8");
  QVERIFY(codec != nullptr);
  /*
   * Initial state
   */

  /*
   * Create test file
   */
  QVERIFY(file.open());
  file.close();
  /*
   * Write CSV file
   */
  QVERIFY(generator.openFile(file.fileName(), "UTF-8"));
  for(const auto & record : sourceData.recordList){
    QVERIFY(generator.writeLine(record));
  }
  QVERIFY(generator.closeFile());
  /*
   * Check
   */
  QVERIFY(codec != nullptr);
  QVERIFY(file.open());
  csvString = codec->toUnicode(file.readAll());
  QCOMPARE(csvString, expectedCsvString);
}

void mdtCsvTest::csvFileGeneratorWriteLineTest_data()
{
  buildCsvGeneratorTestData();
}

void mdtCsvTest::csvFileGeneratorWriteAllTest()
{
  QFETCH(mdtCsvData, sourceData);
  QFETCH(QString, expectedCsvString);
  QFETCH(mdtCsvGeneratorSettings, csvSettings);
  QString csvString;
  mdtCsvFileGenerator generator(csvSettings);
  QTemporaryFile file;
  QTextCodec *codec;

  codec = QTextCodec::codecForName("UTF-8");
  QVERIFY(codec != nullptr);
  /*
   * Initial state
   */

  /*
   * Create test file
   */
  QVERIFY(file.open());
  file.close();
  /*
   * Write CSV file
   */
  QVERIFY(generator.openFile(file.fileName(), "UTF-8"));
  QVERIFY(generator.writeAll(sourceData));
  QVERIFY(generator.closeFile());
  /*
   * Check
   */
  QVERIFY(codec != nullptr);
  QVERIFY(file.open());
  csvString = codec->toUnicode(file.readAll());
  QCOMPARE(csvString, expectedCsvString);
}

void mdtCsvTest::csvFileGeneratorWriteAllTest_data()
{
  buildCsvGeneratorTestData();
}

void mdtCsvTest::csvTableViewDataMapperTest()
{
  const int rowCount = 8;
  const int colCount = 5;
  QStandardItemModel model(rowCount, colCount);
  QTableView view;
  mdtCsvTableViewDataMapper csvDataMapper;
  mdtCsvRecord csvRecord;

  /*
   * Initial state
   */
  QCOMPARE(csvDataMapper.currentRow(), -1);

  /*
   * Build test data
   */
  for(int row = 0; row < model.rowCount(); ++row){
    for(int col = 0; col < model.columnCount(); ++col){
      auto *item = new QStandardItem(QString("%0%1").arg(QChar('A' + row)).arg(col+1));
      model.setItem(row, col, item);
    }
  }
  /*
   * Setup view:
   *  - Cross column "2" and "3" (index 1 and 2)
   *  - Hide column "4"
   *  - Cross row "3" (C) and "4" (D)
   *  - Hide row "6" (F)
   */
  view.setModel(&model);
  // Cross columns
  QVERIFY(view.horizontalHeader() != nullptr);
  view.horizontalHeader()->moveSection(1, 2); // Columns "2" and "3"
  // Hide columns
  view.horizontalHeader()->hideSection(3);    // Column "4"
  // Cross rows
  QVERIFY(view.verticalHeader() != nullptr);
  view.verticalHeader()->moveSection(2, 3);   // Rows "3" and "4"
  // Hide rows
  view.verticalHeader()->hideSection(5);      // Row "6"
  /*
   * Setup mapper
   */
  csvDataMapper.setView(&view);
  QCOMPARE(csvDataMapper.currentRow(), -1);
  /*
   * Check
   */
  // Get header record
  csvRecord = csvDataMapper.getHeaderRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("5"));
  // Go to first row
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 0);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("A1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("A3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("A2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("A5"));
  // Got to next row (B)
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 1);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("B1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("B3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("B2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("B5"));
  // Got to next row (D)
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 2);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("D1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("D3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("D2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("D5"));
  // Got to next row (C)
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 3);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("C1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("C3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("C2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("C5"));
  // Got to next row (E)
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 4);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("E1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("E3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("E2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("E5"));
  // Got to next row: row "6" (F) is hidden, it must be skipped, and we must be at row G after next()
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 6);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("G1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("G3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("G2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("G5"));
  // Got to next row (H)
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 7);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("H1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("H3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("H2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("H5"));
  // Now, next() must fail
  QVERIFY(!csvDataMapper.next());
  // Reset and check that we can restart
  csvDataMapper.reset();
  QVERIFY(csvDataMapper.next());
  QCOMPARE(csvDataMapper.currentRow(), 0);
  csvRecord = csvDataMapper.getCurrentRecord();
  QCOMPARE(csvRecord.columnDataList.size(), 4);
  QCOMPARE(csvRecord.columnDataList.at(0), QVariant("A1"));
  QCOMPARE(csvRecord.columnDataList.at(1), QVariant("A3"));
  QCOMPARE(csvRecord.columnDataList.at(2), QVariant("A2"));
  QCOMPARE(csvRecord.columnDataList.at(3), QVariant("A5"));

  /*
   * Play
   */
//   view.resize(400, 400);
//   view.resizeColumnsToContents();
//   view.resizeRowsToContents();
//   view.show();
//   while(view.isVisible()){
//     QTest::qWait(500);
//   }
}

void mdtCsvTest::csvTableViewExportThreadTest()
{

}

void mdtCsvTest::csvTableViewExportDialogTest()
{
  mdtCsvTableViewExportDialog dialog;
  const int rowCount = 1000;
  const int colCount = 5;
  QStandardItemModel model(rowCount, colCount);
  QTableView view;
  mdtCsvTableViewDataMapper csvDataMapper;
  mdtCsvRecord csvRecord;

  /*
   * Initial state
   */
  QCOMPARE(csvDataMapper.currentRow(), -1);

  /*
   * Build test data
   */
  for(int row = 0; row < model.rowCount(); ++row){
    for(int col = 0; col < model.columnCount(); ++col){
      auto *item = new QStandardItem(QString("%0%1").arg(QChar('A' + (row%256))).arg(col+1));
      model.setItem(row, col, item);
    }
  }
  // Setup view
  view.setModel(&model);
  // Setup dialog
  dialog.setView(&view);
  
  /*
   * Play
   */
  dialog.exec();
}

void mdtCsvTest::buildCsvParserTestData()
{
  QTest::addColumn<QString>("sourceData");
  QTest::addColumn<mdtCsvData>("expectedData");
  QTest::addColumn<bool>("expectedOk");
  QTest::addColumn<mdtCsvParserSettings>("csvSettings");

  QString sourceData;
  mdtCsvRecord expectedRecord;
  mdtCsvData expectedData;
  mdtCsvParserSettings csvSettings;
  const bool Ok = true;
  const bool Nok = false;

  /*
   * Tests are generally based on CSV-1203 standard.
   * Some exceptions also comes from RFC 4180
   */

  /*
   * Simple CSV tests:
   *  - Tests witout field protection
   *  - Test with EOL: \n and \r\n
   */
  csvSettings.fieldSeparator = ',';
  csvSettings.fieldProtection = '"';
  csvSettings.parseExp = false;
  // Empty CSV
  sourceData = "";
  expectedData.clear();
  QTest::newRow("Empty(,|FP:None|EOL:None") << sourceData << expectedData << Nok << csvSettings;
  // Single char CSV - No EOL (No EOL is allowed by RFC 4180)
  sourceData = "A";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("1 char(,|FP:None|EOL:None") << sourceData << expectedData << Ok << csvSettings;
  // Single char CSV - \n EOL
  sourceData = "A\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("1 char(,|FP:None|EOL:\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single char CSV - \n EOL
  sourceData = "A\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("1 char(,|FP:None|EOL:\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV - No EOL (No EOL is allowed by RFC 4180)
  sourceData = "A,B,C,D";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,B,C,D") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV - \n EOL
  sourceData = "A,B,C,D\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,B,C,D\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV (more than 1 char per field) - \n EOL
  sourceData = "ABCD\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("ABCD\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV (more than 1 char per field) - \n EOL
  sourceData = "ABCD,EFGH\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("ABCD,EFGH\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV - \r\n EOL
  sourceData = "A,B,C,D\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,B,C,D\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with empty parts
  sourceData = "A,,B,C\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "" << "B" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,,B,C\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with empty parts (at end)
  sourceData = "A,,B,\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "" << "B" << "";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,,B,\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with more than 1 char per field and empty parts
  sourceData = "AB,,C,D\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "AB" << "" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("AB,,C,D\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  // 2 line CSV - \n EOL
  sourceData = "A,B,C,D\n1,2,3,4\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  expectedRecord.clear();
  expectedRecord.columnDataList << "1" << "2" << "3" << "4";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,B,C,D\\n1,2,3,4\\n") << sourceData << expectedData << Ok << csvSettings;
  // 2 line CSV - \r\n EOL
  sourceData = "A,B,C,D\r\n1,2,3,4\r\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  expectedRecord.clear();
  expectedRecord.columnDataList << "1" << "2" << "3" << "4";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A,B,C,D\\r\\n1,2,3,4\\r\\n") << sourceData << expectedData << Ok << csvSettings;
  /*
   * Non quoted with spaces
   */
  // 1 line of single char
  /// \todo check if error should be reported
  sourceData = "A ";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A ";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A ") << sourceData << expectedData << Ok << csvSettings;
  // 1 line of many chars
  /// \todo check if error should be reported
  sourceData = "AB ";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "AB ";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("AB ") << sourceData << expectedData << Ok << csvSettings;
  // 1 line of many chars
  /// \todo check if error should be reported
  sourceData = "A B";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A B") << sourceData << expectedData << Ok << csvSettings;
  // 2 lines of single char
  /// \todo check if error should be reported
  sourceData = "A \nB";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A ";
  expectedData.addRecord(expectedRecord);
  expectedRecord.clear();
  expectedRecord.columnDataList << "B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("A \\nB") << sourceData << expectedData << Ok << csvSettings;
  /*
   * Quoted CSV tests:
   *  - Test with EOL: \n and \r\n
   */
  csvSettings.fieldSeparator = ',';
  csvSettings.fieldProtection = '"';
  csvSettings.parseExp = false;
  // Single char CSV
  sourceData = "\"A\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single string CSV
  sourceData = "\"ABCD\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"ABCD\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single string CSV with quoted parts
  sourceData = "\"A\"\"BC\"\"D\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A\"BC\"D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\"\"BC\"\"D\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV
  sourceData = "\"A\",\"B\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\",\"B\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with empty parts
  sourceData = "\"A\",\"\",\"B\",\"C\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "" << "B" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\",\"\",\"B\",\"C\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with empty parts (at end)
  sourceData = "\"A\",\"\",\"B\",\"\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "" << "B" << "";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\",\"\",\"B\",\"\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV (with many charachter strings)
  sourceData = "\"ABCD\",\"EFGH\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"ABCD\",\"EFGH\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV with EOL in quoted field
  sourceData = "\"A\nB\",\"C\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A\nB" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\\nB\",\"C\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV with EOL in quoted field
  sourceData = "\"A\r\nB\",\"C\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A\r\nB" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\\r\\nB\",\"C\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV with , in quoted field
  sourceData = "\"A,B\",\"C\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A,B" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A,B\",\"C\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line CSV with double quoted string in quoted field
  sourceData = "\"A\"\"B\"\"\",\"C\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A\"B\"" << "C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\"\"B\"\"\",\"C\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single line with more than 1 char per field and empty parts
  sourceData = "\"AB\",\"\",\"C\",\"D\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "AB" << "" << "C" << "D";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"AB\",\"\",\"C\",\"D\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // RFC 4180 escape quote. We are not conform, but check that this does not crasch
  /// \todo Determine if this should generate a error or not
  sourceData = "\"A\"\"B\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A\"B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A\"\"B\"\\n") << sourceData << expectedData << Nok << csvSettings;
  /*
   * Non quoted with spaces
   */
  // 1 line of single char
  sourceData = "\"A \"";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A ";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A \"") << sourceData << expectedData << Ok << csvSettings;
  // 1 line of many chars
  sourceData = "\"AB \"";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "AB ";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"AB \"") << sourceData << expectedData << Ok << csvSettings;
  // 1 line of many chars
  sourceData = "\"A B\"";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A B\"") << sourceData << expectedData << Ok << csvSettings;
  // 2 lines of single char
  sourceData = "\"A \"\n\"B\"";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A ";
  expectedData.addRecord(expectedRecord);
  expectedRecord.clear();
  expectedRecord.columnDataList << "B";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"A \"\\n\"B\"") << sourceData << expectedData << Ok << csvSettings;
  /*
   * Excel protection marker CSV tests
   */
  csvSettings.fieldSeparator = ',';
  csvSettings.fieldProtection = '\"';
  csvSettings.parseExp = true;
  // Single char CSV
  sourceData = "~A\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("~A\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single string CSV
  sourceData = "~ABCD\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("~ABCD\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single CSV line
  sourceData = "~ABCD,EFGH\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("~ABCD,EFGH\\n") << sourceData << expectedData << Ok << csvSettings;
  // Single quoted CSV line
  sourceData = "\"~ABCD\",\"EFGH\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"~ABCD\",\"EFGH\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Do not parse EXP
  csvSettings.parseExp = false;
  // Single CSV line
  sourceData = "~ABCD,EFGH\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "~ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("~ABCD,EFGH\\n(EXP ignored)") << sourceData << expectedData << Ok << csvSettings;
  // Single quoted CSV line
  sourceData = "\"~ABCD\",\"EFGH\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "~ABCD" << "EFGH";
  expectedData.addRecord(expectedRecord);
  QTest::newRow("\"~ABCD\",\"EFGH\"\\n(EXP ignored)") << sourceData << expectedData << Ok << csvSettings;

  /*
   * Some tests witch non ASCII strings:
   * Note: only edit this part with a UTF-8 editor !
   */
  csvSettings.fieldSeparator = ',';
  csvSettings.fieldProtection = '\"';
  csvSettings.parseExp = true;
  // Non quoted
  sourceData = u8"A,é,à,B,è,ü,ö,ä,\u03B1,\u012C\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "é" << "à" << "B" << "è" << "ü" << "ö" << "ä" << u8"\u03B1" << u8"\u012C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow(u8"A,é,à,B,è,ü,ö,ä,\u03B1,\u012C\\n") << sourceData << expectedData << Ok << csvSettings;
  // Non quoted
  sourceData = u8"A,éà,Bèü,öä\u03B1\u012C\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << u8"éà" << u8"Bèü" << u8"öä\u03B1\u012C";
  expectedData.addRecord(expectedRecord);
  QTest::newRow(u8"A,éà,Bèü,öä\u03B1\u012C\\n") << sourceData << expectedData << Ok << csvSettings;
  // Quoted
  sourceData = u8"\"A\",\"é\",\"à\",\"B\",\"è\",\"ü\",\"ö\",\"ä\",\"\u03B1\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << "A" << "é" << "à" << "B" << "è" << "ü" << "ö" << "ä" << u8"\u03B1";
  expectedData.addRecord(expectedRecord);
  QTest::newRow(u8"\"A\",\"é\",\"à\",\"B\",\"è\",\"ü\",\"ö\",\"ä\",\"\u03B1\"\\n") << sourceData << expectedData << Ok << csvSettings;
  // Quoted
  sourceData = u8"\"A,é\",\"à,B\",\"è,ü\",\"ö,ä,\u03B1\"\n";
  expectedData.clear();
  expectedRecord.clear();
  expectedRecord.columnDataList << u8"A,é" << u8"à,B" << u8"è,ü" << u8"ö,ä,\u03B1";
  expectedData.addRecord(expectedRecord);
  QTest::newRow(u8"\"A,é\",\"à,B\",\"è,ü\",\"ö,ä,\u03B1\"\\n") << sourceData << expectedData << Ok << csvSettings;

  /// \todo add casese from: http://stackoverflow.com/questions/7436481/how-to-make-my-split-work-only-on-one-real-line-and-be-capable-to-skip-quoted-pa/7462539#7462539

  /// \todo Add some from ~/programmation/c_cpp/libreoffiche/git/core/sc/qa/unit/data/contentCSV/
  
  /// \todo Check if A B,C\n should be valid (space between A and B, unquoted)
}

void mdtCsvTest::buildCsvGeneratorTestData()
{
  QTest::addColumn<mdtCsvData>("sourceData");
  QTest::addColumn<QString>("expectedCsvString");
  QTest::addColumn<mdtCsvGeneratorSettings>("csvSettings");

  mdtCsvRecord record;
  mdtCsvData sourceData;
  QString expectedCsvString;
  mdtCsvGeneratorSettings csvSettings;

  /*
   * Tests are based on CSV-1203 standard.
   */
  csvSettings.eol = '\n';
  csvSettings.fieldSeparator = ',';
  csvSettings.fieldProtection = '"';
  csvSettings.allwaysProtectTextFields = false;
  // Empty data
  record.clear();
  sourceData.clear();
  expectedCsvString = "";
  QTest::newRow("") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 field short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "A";
  sourceData.addRecord(record);
  expectedCsvString = "A\n";
  QTest::newRow("[A]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 field short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "AB";
  sourceData.addRecord(record);
  expectedCsvString = "AB\n";
  QTest::newRow("[AB]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 field short data
  sourceData.clear();
  record.clear();
  record.columnDataList << 1;
  sourceData.addRecord(record);
  expectedCsvString = "1\n";
  QTest::newRow("[1]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 field short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "A,B";
  sourceData.addRecord(record);
  expectedCsvString = "\"A,B\"\n";
  QTest::newRow("[A,B]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 field short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "A\"B\"C";
  sourceData.addRecord(record);
  expectedCsvString = "\"A\"\"B\"\"C\"\n";
  QTest::newRow("[A\"B\"C]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 line short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "A" << "B";
  sourceData.addRecord(record);
  expectedCsvString = "A,B\n";
  QTest::newRow("[A|B]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 line short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "AB" << "C";
  sourceData.addRecord(record);
  expectedCsvString = "AB,C\n";
  QTest::newRow("[AB|C]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted 1 line short data
  sourceData.clear();
  record.clear();
  record.columnDataList << "A" << 1;
  sourceData.addRecord(record);
  expectedCsvString = "A,1\n";
  QTest::newRow("[A|1]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted line with null data
  sourceData.clear();
  record.clear();
  record.columnDataList << "" << 1 << QVariant();
  sourceData.addRecord(record);
  expectedCsvString = ",1,\n";
  QTest::newRow("[|1|null]") << sourceData << expectedCsvString << csvSettings;
  // Auto quoted simple 2 Lines
  sourceData.clear();
  record.clear();
  record.columnDataList << "A" << "B" << "C";
  sourceData.addRecord(record);
  record.clear();
  record.columnDataList << "1" << "2" << "3";
  sourceData.addRecord(record);
  expectedCsvString = "A,B,C\n1,2,3\n";
  QTest::newRow("[A|B|C][1|2|3]") << sourceData << expectedCsvString << csvSettings;
}


/*
 * Main
 */
int main(int argc, char **argv)
{
  mdtApplication app(argc, argv);
  mdtCsvTest csvTest;

  if(!app.init()){
    return 1;
  }

  return QTest::qExec(&csvTest, argc, argv);
}
