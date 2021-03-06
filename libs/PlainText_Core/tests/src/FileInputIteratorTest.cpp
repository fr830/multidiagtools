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
#include "FileInputIteratorTest.h"
#include "Mdt/PlainText/FileInputIteratorSharedData.h"
#include "Mdt/PlainText/FileInputIterator.h"
#include <QTemporaryFile>
#include <QTextCodec>
#include <QString>

#include <QDebug>

using namespace Mdt::PlainText;

void FileInputIteratorTest::initTestCase()
{
}

void FileInputIteratorTest::cleanupTestCase()
{
}

/*
 * Tests
 */

void FileInputIteratorTest::sharedDataTest()
{
  FileInputIteratorSharedData sd;
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
  FileInputIteratorSharedData sd2(1);
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

void FileInputIteratorTest::sharedDataReadTest()
{
  QTemporaryFile file;
  QFETCH(QString, fileData);
  QFETCH(QString, fileEncoding);
  QFETCH(int, rawDataBufferSize);
  QString unicodeBuffer;
  QByteArray fileRawData;
  QTextCodec *codec;
  FileInputIteratorSharedData sd(rawDataBufferSize);

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

void FileInputIteratorTest::sharedDataReadTest_data()
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

void FileInputIteratorTest::sharedDataBenchmark()
{
  QTemporaryFile file;
  QFETCH(int, n);
  QFETCH(QString, fileEncoding);
  QFETCH(int, rawDataBufferSize);
  QString fileData;
  QByteArray fileRawData;
  QTextCodec *codec;
  FileInputIteratorSharedData sd(rawDataBufferSize);

  QVERIFY(QTextCodec::codecForName(fileEncoding.toLatin1()) != nullptr);
  /*
   * Generate test data
   */
  fileData.reserve(n+5);
  for(int i = 0; i < n; ++i){
    fileData += QChar( '0' + (i % ('z' - '0')) );
  }
//   qDebug() << "data: " << fileData;
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
  QString unicodeBuffer;
  unicodeBuffer.reserve(n+5);
  // Read data
  QVERIFY(sd.setSource(&file, fileEncoding.toLatin1()));
  QBENCHMARK_ONCE{
    while(!sd.atEnd()){
      unicodeBuffer.append(sd.get());
      QVERIFY(sd.advance());
    }
  }
  // Check
  QCOMPARE(unicodeBuffer, fileData);
}

void FileInputIteratorTest::sharedDataBenchmark_data()
{
  QTest::addColumn<int>("n");
  QTest::addColumn<QString>("fileEncoding");
  QTest::addColumn<int>("rawDataBufferSize");

  QTest::newRow("10C,UTF-8,1024") << 10 << "UTF-8" << 1024;
  QTest::newRow("1'000C,UTF-8,1024") << 1000 << "UTF-8" << 1024;
  QTest::newRow("10'000C,UTF-8,1024") << 10000 << "UTF-8" << 1024;
  QTest::newRow("1'000'000C,UTF-8,1024") << 1000000 << "UTF-8" << 1024;
}

void FileInputIteratorTest::iteratorTest()
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
  FileInputIterator it;
  QVERIFY(!it.errorOccured());
  QVERIFY(it.isEof());
  // Copy
  FileInputIterator it2(it);
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
  FileInputIterator it3(&file, "UTF-8");
  QVERIFY(!it3.errorOccured());
  QVERIFY(!it3.isEof());
  QVERIFY(*it3 == 'A');
  // Construct it4 that is a copy of it3
  FileInputIterator it4(it3);
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
  FileInputIterator first(&file, "UTF-8");
  FileInputIterator last;
  str.clear();
  while(first != last){
    str.append(static_cast<ushort>(*first));
    ++first;
  }
  QCOMPARE(str, QString("ABCDE"));
}

void FileInputIteratorTest::iteratorBenchmark()
{
  QTemporaryFile file;
  QFETCH(int, n);
  QFETCH(QString, fileEncoding);
  QString fileData;
  
  QByteArray fileRawData;
  QTextCodec *codec;

  QVERIFY(QTextCodec::codecForName(fileEncoding.toLatin1()) != nullptr);
  /*
   * Generate test data
   */
  fileData.reserve(n+5);
  for(int i = 0; i < n; ++i){
    fileData += QChar( '0' + (i % ('z' - '0')) );
  }
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
  QString unicodeBuffer;
  unicodeBuffer.reserve(n+5);
  QBENCHMARK_ONCE{
    FileInputIterator first(&file, fileEncoding.toLatin1());
    FileInputIterator last;
    while(first != last){
      unicodeBuffer.append(static_cast<ushort>(*first));
      ++first;
    }
  }
  QCOMPARE(unicodeBuffer, fileData);
}

void FileInputIteratorTest::iteratorBenchmark_data()
{
  QTest::addColumn<int>("n");
  QTest::addColumn<QString>("fileEncoding");

  QTest::newRow("10C,UTF-8") << 10 << "UTF-8";
  QTest::newRow("1'000C,UTF-8") << 1000 << "UTF-8";
  QTest::newRow("10'000C,UTF-8") << 10000 << "UTF-8";
  QTest::newRow("1'000'000C,UTF-8") << 1000000 << "UTF-8";
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::CoreApplication app(argc, argv);
  FileInputIteratorTest test;

  return QTest::qExec(&test, argc, argv);
}
