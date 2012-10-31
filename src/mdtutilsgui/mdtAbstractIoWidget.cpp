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
#include "mdtAbstractIoWidget.h"
#include "mdtAbstractIo.h"
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

mdtAbstractIoWidget::mdtAbstractIoWidget(QWidget *parent)
 : QWidget(parent)
{
  pvAddress = 0;
  // Create label widgets
  lbLabel = new QLabel;
  lbLabel->setFrameStyle(QFrame::Panel | QFrame::Plain);
  lbLabel->setLineWidth(1);
  lbLabel->setAlignment(Qt::AlignHCenter);
  pbDetails = new QPushButton;
  pbDetails->setText("...");
  pbDetails->setToolTip(tr("Show details"));
  // Infomration box
  pvMessageBox = new QMessageBox;
  pvMessageBox->setText(tr("          I/O informations                           "));
  connect(pbDetails, SIGNAL(clicked()), this, SLOT(showIoInformations()));
}

mdtAbstractIoWidget::~mdtAbstractIoWidget()
{
  delete pvMessageBox;
}

void mdtAbstractIoWidget::setIo(mdtAbstractIo *io)
{
  Q_ASSERT(io != 0);

  // Signals/slots from io to widget
  connect(io, SIGNAL(addressChangedForUi(int)), this, SLOT(setAddress(int)));
  connect(io, SIGNAL(labelShortChangedForUi(const QString&)), this, SLOT(setLabelShort(const QString&)));
  connect(io, SIGNAL(labelChangedForUi(const QString&)), this, SLOT(setLabel(const QString&)));
  connect(io, SIGNAL(detailsChangedForUi(const QString&)), this, SLOT(setDetails(const QString&)));
  // Get initial data
  setAddress(io->address());
  setLabelShort(io->labelShort());
  setLabel(io->label());
  setDetails(io->details());
}

void mdtAbstractIoWidget::showIoInformations()
{
  Q_ASSERT(pvMessageBox != 0);

  QString str;

  // Labels
  str = tr("Short label: ");
  str += lbLabel->text();
  str += "\n";
  str += tr("Label: ");
  str += lbLabel->toolTip();
  str += "\n";
  str += tr("Address: 0x");
  str += QString::number(pvAddress, 16);
  str += " (dec: ";
  str += QString::number(pvAddress, 10);
  str += ")";
  pvMessageBox->setInformativeText(str);
  pvMessageBox->setIcon(QMessageBox::Information);
  pvMessageBox->exec();
}

void mdtAbstractIoWidget::setAddress(int address)
{
  pvAddress = address;
}

void mdtAbstractIoWidget::setLabelShort(const QString &text)
{
  lbLabel->setText(text);
}

void mdtAbstractIoWidget::setLabel(const QString & text)
{
  lbLabel->setToolTip(text);
}

void mdtAbstractIoWidget::setDetails(const QString & text)
{
  pvMessageBox->setDetailedText(text);
}
