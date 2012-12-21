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
#include "mdtFrameCodecScpi.h"
#include "mdtError.h"
#include <cfloat>
#include <QChar>

#include <QDebug>

mdtFrameCodecScpi::mdtFrameCodecScpi()
{
  /*
   * Values returned by device
   *  when a measure fails
   *  (OL or other)
   * See SCPI-99, chap. 7.2.1
   */
  pvInfinity = 9.9e37;
  pvNinfinity = -9.9e37;
  pvNan = 9.91e37;
}

mdtFrameCodecScpi::~mdtFrameCodecScpi()
{
}

bool mdtFrameCodecScpi::decodeValues(const QByteArray &data, QString sep)
{
  int i;
  QVariant value;
  double fltValue;

  // Clear previous results
  pvValues.clear();
  pvNodes.clear();

  // Store raw data in local QString constainer
  pvAsciiData = data;

  // Remove begin/end spaces and EOF
  if(!clean()){
    return false;
  }

  // Extract nodes
  pvNodes = pvAsciiData.split(sep);
  for(i=0; i<pvNodes.size(); i++){
    // We ignore empty nodes
    if(pvNodes.at(i).isEmpty()){
      continue;
    }
    // Convert node
    value = convertData(pvNodes.at(i));
    // Check limits if type is double
    if(value.type() == QVariant::Double){
      fltValue = value.toDouble();
      // Check about Nan
      if(qAbs(fltValue - pvNan) <= FLT_EPSILON){
        value.clear();
      }else{
        // Check about -OL and OL
        if((fltValue <= (pvNinfinity+FLT_EPSILON))||(fltValue >= (pvInfinity-FLT_EPSILON))){
          value.clear();
        }
      }
    }
    // Add converted value
    pvValues.append(value);
  }

  return true;
}

bool mdtFrameCodecScpi::decodeIdn(const QByteArray &data)
{
  QVariant value;
  int i;

  // Clear previous results
  pvValues.clear();
  pvNodes.clear();

  // Store raw data in local QString constainer
  pvAsciiData = data;

  // Remove begin/end spaces and EOF
  if(!clean()){
    return false;
  }

  // Build the values list
  pvNodes = pvAsciiData.split(",");
  // Store result in data list
  for(i=0; i<pvNodes.size(); i++){
    pvValues << pvNodes.at(i);
  }

  return true;
}

bool mdtFrameCodecScpi::decodeError(QByteArray &data)
{
  QVariant value;
  bool ok = false;

  // Clear previous results
  pvValues.clear();
  pvNodes.clear();

  // Store raw data in local QString constainer
  pvAsciiData = data;

  // Remove begin/end spaces and EOF
  if(!clean()){
    return false;
  }

  // Build the values list
  pvNodes = pvAsciiData.split(",");
  // Store result in data list
  if(pvNodes.size() < 2){
    mdtError e(MDT_FRAME_DECODE_ERROR, "Frame contains not an error" , mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtFrameCodecScpi");
    e.commit();
    return false;
  }
  // Error code
  value = pvNodes.at(0);
  value = value.toInt(&ok);
  if(!ok){
    value.clear();
  }
  pvValues << value;
  // Error text
  pvValues << pvNodes.at(1);

  return true;
}

bool mdtFrameCodecScpi::decodeIEEEblock(QString data)
{
  int i;
  bool ok = false;
  QString dataLen;

  // If string length is < 2: shure: not an IEEE block
  if(data.size() < 2){
    mdtError e(MDT_FRAME_DECODE_ERROR, "Frame is not a IEEE block" , mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtFrameCodecScpi");
    e.commit();
    return false;
  }
  // If first char is '#' , simply remove it
  if(data.at(0) == QChar('#')){
    data.remove(0, 1);
  }
  // We must have a number that tells how many info digits that follows
  i = QString(data.at(0)).toInt(&ok); // Note: we need a QString, not a QChar.
  if(!ok){
    mdtError e(MDT_FRAME_DECODE_ERROR, "Frame contains no block header length" , mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtFrameCodecScpi");
    e.commit();
    return false;
  }
  data.remove(0, 1);
  // Check that we have enouth chars to decode data length
  if(data.size() < i){
    mdtError e(MDT_FRAME_DECODE_ERROR, "Frame contains no block header with data length" , mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtFrameCodecScpi");
    e.commit();
    return false;
  }
  // Get the data length
  dataLen = data.left(i);
  data.remove(0, i);
  i = dataLen.toInt(&ok);
  if(!ok){
    mdtError e(MDT_FRAME_DECODE_ERROR, "Unable to decode data length in block header" , mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtFrameCodecScpi");
    e.commit();
    return false;
  }
  // Ok, let's decode the data
  return decodeIEEEdata(data.toAscii());
}
