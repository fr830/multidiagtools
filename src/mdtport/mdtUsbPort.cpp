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
#include "mdtUsbPort.h"
#include "mdtError.h"
#include "mdtUsbDeviceDescriptor.h"
#include <QString>
#include <QStringList>

#include <poll.h>

#include <QDebug>

mdtUsbPort::mdtUsbPort(QObject *parent)
 : mdtAbstractPort(parent)
{
  int retVal;

  pvReadTimeout = 0;
  pvWriteTimeout = 0;
  pvHandle = 0;
  pvReadBuffer = 0;
  pvReadBufferSize = 0;
  pvReadenLength = 0;
  pvReadEndpointAddress = 0;
  pvWriteBuffer = 0;
  pvWriteBufferSize = 0;
  pvWrittenLength = 0;
  pvWriteEndpointAddress = 0;
  pvInterruptInBuffer = 0;
  pvInterruptInBufferSize = 0;
  // Init libusb
  retVal = libusb_init(&pvLibusbContext);
  if(retVal != 0){
    pvLibusbContext = 0;
    mdtError e(MDT_USB_IO_ERROR, "libusb_init() failed", mdtError::Error);
    e.setSystemError(retVal, errorText(retVal));
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
  }
  Q_ASSERT(pvLibusbContext != 0);
}

mdtUsbPort::~mdtUsbPort()
{
  close();
  // Free libusb
  libusb_exit(pvLibusbContext);
}

void mdtUsbPort::setReadTimeout(int timeout)
{
  if(timeout < 0){
    pvReadTimeout = 0;  // 0 means infinite on libusb
  }else{
    pvReadTimeout = timeout;
  }
}

void mdtUsbPort::setWriteTimeout(int timeout)
{
  if(timeout < 0){
    pvWriteTimeout = 0;  // 0 means infinite on libusb
  }else{
    pvWriteTimeout = timeout;
  }
}

/// NOTE: \todo : Update timeout states !
mdtAbstractPort::error_t mdtUsbPort::waitForReadyRead()
{
  return NoError;
}

qint64 mdtUsbPort::read(char *data, qint64 maxSize)
{
  qDebug() << "mdtUsbPort::read() ...";
  int err;
  libusb_transfer *transfer;
  ///unsigned char *data = (unsigned char*)pvReadBuffer;
  int endpointConcernedEvent;
  int len;

  // Ajust size
  if(maxSize > (qint64)pvReadBufferSize){
    len = pvReadBufferSize;
  }else{
    len = maxSize;
  }
  // Alloc transfert
  transfer = libusb_alloc_transfer(0);
  if(transfer == 0){
    mdtError e(MDT_USB_IO_ERROR, "libusb_alloc_transfert() failed", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return -1;
  }
  // Fill transfert
  if(pvReadTransfertType == LIBUSB_TRANSFER_TYPE_BULK){
    libusb_fill_bulk_transfer(transfer, pvHandle, pvReadEndpointAddress, (unsigned char*)pvReadBuffer, len, transferCallback, 0, pvReadTimeout);
  }else if(pvReadTransfertType == LIBUSB_TRANSFER_TYPE_INTERRUPT){
    libusb_fill_interrupt_transfer(transfer, pvHandle, pvReadEndpointAddress, (unsigned char*)pvReadBuffer, len, transferCallback, 0, pvReadTimeout);
  }else{
    libusb_free_transfer(transfer);
    mdtError e(MDT_USB_IO_ERROR, "Unknown transfert type", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return -1;
  }
  // Submit
  /// \todo Handle retval
  err = libusb_submit_transfer(transfer);
  if(err != 0){
    libusb_free_transfer(transfer);
    mdtError e(MDT_USB_IO_ERROR, "libusb_submit_transfer() failed", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return -1;
  }
  // Test with poll() wait
  bool retry;
  int i, numfds;
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(pvLibusbContext);
  i=0;
  while(poll_fds[i] != 0){
    qDebug() << "RD Found fd at " << i;
    i++;
  }
  numfds = i;
  qDebug() << "RD Alloc for " << numfds << " fds";
  struct pollfd *fds = new struct pollfd[numfds];
  // Copy
  for(i=0; i<numfds; i++){
    fds[i].fd = poll_fds[i]->fd;
    fds[i].events = poll_fds[i]->events;
    fds[i].revents = 0;
    ///free(poll_fds[i]);
  }
  free(poll_fds);
  // Wait until a event is for us
  retry = true;
  endpointConcernedEvent = -1;
  unlockMutex();
  usleep(50000);
  while(retry){
    qDebug() << "RD Entering event try...";
    if(libusb_try_lock_events(pvLibusbContext) == 0){
      // Have the lock, do event handling
      qDebug() << "RD Have the lock, do events...";
      while(endpointConcernedEvent != (int)pvReadEndpointAddress){
        // Check if current thread can do event handling
        qDebug() << "RD can do event handling now ?";
        if(!libusb_event_handling_ok(pvLibusbContext)){
          // Not OK now, retry later
          qDebug() << "RD No, cannot do event handling now!";
          libusb_unlock_events(pvLibusbContext);
          continue;
        }
        // Ok, current thread can do event handling now
        qDebug() << "RD Yes, can do event handling now";
        retry = false;
        ///unlockMutex();
        ///usleep(50000); /// NOTE: provisoire
        err = poll(fds, numfds, -1);
        ///lockMutex();
        if(err < 0){
          qDebug() << "Poll fail !";
        }
        if(err > 0){
          qDebug() << "RD Going handle events";
          struct timeval tv = {10, 0};
          err = libusb_handle_events_locked(pvLibusbContext, &tv);
          qDebug() << "RD Handle events done";
        }
        if(err != 0){
          ///libusb_unlock_events(pvLibusbContext);
          break;
        }
        endpointConcernedEvent = transfer->endpoint;
      }
      qDebug() << "RD: unlock event...";
      libusb_unlock_events(pvLibusbContext);
    }else{
      // Another thread is doing event handling. Wait until it signal us it has completed
      qDebug() << "RD: another thread does events, waiting...";
      libusb_lock_event_waiters(pvLibusbContext);
      // Have the event waiters lock
      while(endpointConcernedEvent != (int)pvReadEndpointAddress){
        // Check if other thread still do event handling
        qDebug() << "RD: checking if a thread is doing events (has the event lock)";
        if(!libusb_event_handler_active(pvLibusbContext)){
          // Other thread done, retry now
          qDebug() << "WR: none active, retry now";
          ///libusb_unlock_event_waiters(pvLibusbContext);
          break;
        }
        // Wait...
        libusb_wait_for_event(pvLibusbContext, 0);
      }
      libusb_unlock_event_waiters(pvLibusbContext);
    }
  }
  lockMutex();
  qDebug() << "RD: deleting fds..";
  delete[] fds;

/*
  endpointConcernedEvent = -1;
  while(endpointConcernedEvent != (int)pvReadEndpointAddress){
    unlockMutex();
    usleep(50000); /// NOTE: provisoire
    err = poll(fds, numfds, -1);
    lockMutex();
    if(err < 0){
      qDebug() << "Poll fail !";
    }
    if(err > 0){
      err = libusb_handle_events(pvLibusbContext);
    }
    if(err != 0){
      break;
    }
    endpointConcernedEvent = transfer->endpoint;
  }
*/
  // Wait until a event is for us
  /*
  endpointConcernedEvent = -1;
  while(endpointConcernedEvent != (int)pvReadEndpointAddress){
    qDebug() << "Read, going wait ...";
    unlockMutex();
    usleep(50000); /// NOTE: provisoire
    err = libusb_handle_events(pvLibusbContext);
    lockMutex();
    qDebug() << "Read, wait DONE";
    if(err != 0){
      // Errors are handled just after this loop
      break;
    }
    endpointConcernedEvent = transfer->endpoint;
  }
  */
/**
  qDebug() << "Original:";
  qDebug() << "-> Endpoint: " << hex << transfer->endpoint;
  qDebug() << "-> actual_length: " << hex << transfer->actual_length;
  qDebug() << "-> short frame stransfert NOK state: " << (transfer->flags & LIBUSB_TRANSFER_SHORT_NOT_OK);
  qDebug() << "-> short frame stransfert NOK state: " << (transfer->flags & LIBUSB_TRANSFER_SHORT_NOT_OK);
  qDebug() << "-> transfert complete: " << (transfer->status & LIBUSB_TRANSFER_COMPLETED);
  qDebug() << "-> transfert failed: " << (transfer->status & LIBUSB_TRANSFER_ERROR);
  qDebug() << "-> transfert timeout: " << (transfer->status & LIBUSB_TRANSFER_TIMED_OUT);
  qDebug() << "-> transfert cancelled: " << (transfer->status & LIBUSB_TRANSFER_CANCELLED);
  qDebug() << "-> transfert stall: " << (transfer->status & LIBUSB_TRANSFER_STALL);
  qDebug() << "-> device disconnected: " << (transfer->status & LIBUSB_TRANSFER_NO_DEVICE);
  qDebug() << "-> device sendt to much (overflow): " << (transfer->status & LIBUSB_TRANSFER_OVERFLOW);
*/

  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_INTERRUPTED:  /// \todo can be because device was deconnected, handle this..
      pvReadenLength = 0;
      libusb_free_transfer(transfer);
      return -1;
    default:
      pvReadenLength = 0;
      libusb_free_transfer(transfer);
      mdtError e(MDT_USB_IO_ERROR, "libusb_handle_events() failed", mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return -1;
  }
  len = transfer->actual_length;
  memcpy(data, pvReadBuffer, len);
  libusb_free_transfer(transfer);

  qDebug() << "Readen: " << len;
  return len;
}

void mdtUsbPort::flushIn()
{
  lockMutex();
  mdtAbstractPort::flushIn();
}

/// NOTE: \todo : Update timeout states !
mdtAbstractPort::error_t mdtUsbPort::waitEventWriteReady()
{
  return NoError;
}

qint64 mdtUsbPort::write(const char *data, qint64 maxSize)
{
  int err;
  libusb_transfer *transfer;
  int endpointConcernedEvent;
  int len;

  // Ajust size
  if(maxSize > (qint64)pvWriteBufferSize){
    len = pvWriteBufferSize;
  }else{
    len = maxSize;
  }
  memcpy(pvWriteBuffer, data, len);
  // Alloc transfert
  transfer = libusb_alloc_transfer(0);
  if(transfer == 0){
    mdtError e(MDT_USB_IO_ERROR, "libusb_alloc_transfert() failed", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return UnknownError;
  }
  // Fill transfert
  if(pvWriteTransfertType == LIBUSB_TRANSFER_TYPE_BULK){
    libusb_fill_bulk_transfer(transfer, pvHandle, pvWriteEndpointAddress, (unsigned char*)pvWriteBuffer, len, transferCallback, 0, pvWriteTimeout);
  }else if(pvWriteTransfertType == LIBUSB_TRANSFER_TYPE_INTERRUPT){
    libusb_fill_interrupt_transfer(transfer, pvHandle, pvWriteEndpointAddress, (unsigned char*)pvWriteBuffer, len, transferCallback, 0, pvWriteTimeout);
  }else{
    libusb_free_transfer(transfer);
    mdtError e(MDT_USB_IO_ERROR, "Unknown transfert type", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return UnknownError;
  }
  // Submit
  /// \todo Handle retval
  err = libusb_submit_transfer(transfer);
  if(err != 0){
    libusb_free_transfer(transfer);
    mdtError e(MDT_USB_IO_ERROR, "libusb_submit_transfer() failed", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return UnknownError;
  }

  // Test with poll() wait
  bool retry;
  int i, numfds;
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(pvLibusbContext);
  i=0;
  while(poll_fds[i] != 0){
    qDebug() << "WR Found fd at " << i;
    i++;
  }
  numfds = i;
  qDebug() << "WR Alloc for " << numfds << " fds";
  struct pollfd *fds = new struct pollfd[numfds];
  // Copy
  for(i=0; i<numfds; i++){
    fds[i].fd = poll_fds[i]->fd;
    fds[i].events = poll_fds[i]->events;
    fds[i].revents = 0;
    ///free(poll_fds[i]);
  }
  free(poll_fds);
  // Wait until a event is for us
  retry = true;
  endpointConcernedEvent = -1;
  unlockMutex();
  usleep(50000);
  while(retry){
    qDebug() << "WR Entering event try...";
    if(libusb_try_lock_events(pvLibusbContext) == 0){
      // Have the lock, do event handling
      qDebug() << "WR Have the lock, do events...";
      while(endpointConcernedEvent != (int)pvWriteEndpointAddress){
        // Check if current thread can do event handling
        qDebug() << "WR can do event handling now ?";
        if(!libusb_event_handling_ok(pvLibusbContext)){
          // Not OK now, retry later
          qDebug() << "WR No, cannot do event handling now!";
          libusb_unlock_events(pvLibusbContext);
          continue;
        }
        // Ok, current thread can do event handling now
        qDebug() << "WR Yes, can do event handling now";
        retry = false;
        ///unlockMutex();
        ///usleep(50000); /// NOTE: provisoire
        err = poll(fds, numfds, -1);
        ///lockMutex();
        if(err < 0){
          qDebug() << "WR Poll fail !";
        }
        if(err > 0){
          qDebug() << "WR Going handle events";
          struct timeval tv = {10, 0};
          err = libusb_handle_events_locked(pvLibusbContext, &tv);
          qDebug() << "WR Handle events done";
        }
        if(err != 0){
          break;
        }
        endpointConcernedEvent = transfer->endpoint;
      }
      qDebug() << "WR: unlock event...";
      libusb_unlock_events(pvLibusbContext);
    }else{
      // Another thread is doing event handling. Wait until it signal us it has completed
      qDebug() << "WR: another thread does events, waiting...";
      libusb_lock_event_waiters(pvLibusbContext);
      // Have the event waiters lock
      while(endpointConcernedEvent != (int)pvWriteEndpointAddress){
        qDebug() << "WR: checking if a thread is doing events (has the event lock)";
        // Check if other thread still do event handling
        if(!libusb_event_handler_active(pvLibusbContext)){
          // Other thread done, retry now
          qDebug() << "WR: none active, retry now";
          ///libusb_unlock_event_waiters(pvLibusbContext);
          break;
        }
        // Wait...
        qDebug() << "WR: wait ...";
        libusb_wait_for_event(pvLibusbContext, 0);
      }
      libusb_unlock_event_waiters(pvLibusbContext);
    }
  }
  lockMutex();
  qDebug() << "WR: deleting fds..";
  delete[] fds;

  // Test with poll() wait
  /*
  int i, numfds;
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(pvLibusbContext);
  i=0;
  while(poll_fds[i] != 0){
    qDebug() << "Found fd at " << i;
    i++;
  }
  numfds = i;
  qDebug() << "Alloc for " << numfds << " fds";
  struct pollfd *fds = new struct pollfd[numfds];
  // Copy
  for(i=0; i<numfds; i++){
    fds[i].fd = poll_fds[i]->fd;
    fds[i].events = poll_fds[i]->events;
    fds[i].revents = 0;
    ///free(poll_fds[i]);
  }
  free(poll_fds);
  // Wait until a event is for us
  endpointConcernedEvent = -1;
  while(endpointConcernedEvent != (int)pvWriteEndpointAddress){
    unlockMutex();
    usleep(50000); /// NOTE: provisoire
    err = poll(fds, numfds, -1);
    lockMutex();
    if(err < 0){
      qDebug() << "Poll fail !";
    }
    if(err > 0){
      err = libusb_handle_events(pvLibusbContext);
    }
    if(err != 0){
      break;
    }
    endpointConcernedEvent = transfer->endpoint;
  }
  */

  // Wait until a event is for us
  /*
  endpointConcernedEvent = -1;
  while(endpointConcernedEvent != (int)pvWriteEndpointAddress){
    unlockMutex();
    usleep(500000); /// NOTE: provisoire
    err = libusb_handle_events(pvLibusbContext);
    lockMutex();
    if(err != 0){
      break;
    }
    endpointConcernedEvent = transfer->endpoint;
  }
  */
  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_INTERRUPTED:  /// \todo can be because device was deconnected, handle this..
      pvWrittenLength = 0;
      libusb_free_transfer(transfer);
      return WaitingCanceled;
    default:
      pvWrittenLength = 0;
      libusb_free_transfer(transfer);
      mdtError e(MDT_USB_IO_ERROR, "libusb_handle_events() failed", mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return UnknownError;
  }
  pvWrittenLength = transfer->actual_length;
  ///memcpy(data, pvWriteBuffer, transfer->actual_length);
  libusb_free_transfer(transfer);
  qDebug() << "mdtUsbPort::write() , written: " << pvWrittenLength;

  return pvWrittenLength;
}

void mdtUsbPort::flushOut()
{
  lockMutex();
  mdtAbstractPort::flushOut();
}

void mdtUsbPort::transferCallback(struct libusb_transfer *transfer)
{
  Q_ASSERT(transfer != 0);

  qDebug() << "Tranfert callback ...";
  qDebug() << "-> Endpoint: " << hex << transfer->endpoint;
  qDebug() << "-> actual_length: " << hex << transfer->actual_length;
  qDebug() << "-> short frame stransfert NOK state: " << (transfer->flags & LIBUSB_TRANSFER_SHORT_NOT_OK);
  qDebug() << "-> transfert complete: " << (transfer->status & LIBUSB_TRANSFER_COMPLETED);
  qDebug() << "-> transfert failed: " << (transfer->status & LIBUSB_TRANSFER_ERROR);
  qDebug() << "-> transfert timeout: " << (transfer->status & LIBUSB_TRANSFER_TIMED_OUT);
  qDebug() << "-> transfert cancelled: " << (transfer->status & LIBUSB_TRANSFER_CANCELLED);
  qDebug() << "-> transfert stall: " << (transfer->status & LIBUSB_TRANSFER_STALL);
  qDebug() << "-> device disconnected: " << (transfer->status & LIBUSB_TRANSFER_NO_DEVICE);
  qDebug() << "-> device sendt to much (overflow): " << (transfer->status & LIBUSB_TRANSFER_OVERFLOW);
  
  /**
  mdt_usb_port_transfer_data *data = (mdt_usb_port_transfer_data*)transfer->user_data;
  Q_ASSERT(data != 0);
  data->flag1 = transfer->status;
  */
  
  ///int *completed = (int*)transfer->user_data;
  ///*completed = 1;
}

mdtAbstractPort::error_t mdtUsbPort::pvOpen()
{
  Q_ASSERT(!isOpen());

  quint16 vid;
  quint16 pid;
  QString str;
  QStringList lst;
  bool ok = false;

  ///qDebug() << "mdtUsbPort::pvOpen() ... (Not implemented yet)";

  // Extract vendor ID and product ID
  lst = pvPortName.split(":");
  if(lst.size() < 2){
    mdtError e(MDT_USB_IO_ERROR, "Error in port format for " + pvPortName + " (must be idVendor:idProduct)", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return SetupError;
  }
  // Check vendor ID format and convert
  str = lst.at(0);
  if(str.left(2) == "0x"){
    vid = str.toUInt(&ok, 16);
  }else{
    vid = str.toUInt(&ok, 10);
  }
  if(!ok){
    mdtError e(MDT_USB_IO_ERROR, "Cannot extract idVendor in " + pvPortName, mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return SetupError;
  }
  // Check product ID format and convert
  str = lst.at(1);
  if(str.left(2) == "0x"){
    pid = str.toUInt(&ok, 16);
  }else{
    pid = str.toUInt(&ok, 10);
  }
  if(!ok){
    mdtError e(MDT_USB_IO_ERROR, "Cannot extract idProduct in " + pvPortName, mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return SetupError;
  }
  // Open port
  pvHandle = libusb_open_device_with_vid_pid(pvLibusbContext, vid, pid);
  if(pvHandle == 0){
    mdtError e(MDT_USB_IO_ERROR, "Cannot open device " + pvPortName, mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return PortNotFound;
  }

  return NoError;
}

void mdtUsbPort::pvClose()
{
  Q_ASSERT(isOpen());
  Q_ASSERT(pvHandle != 0);

  int err;

  // Release port
  err = libusb_release_interface(pvHandle, 0);  /// \todo interface nunber hardcoded, BAD
  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_NOT_FOUND:
      break;
    case LIBUSB_ERROR_NO_DEVICE:
      break;
    default:
      mdtError e(MDT_USB_IO_ERROR, "libubs_release_interface() failed", mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
  }
  // Re-attach possibly detached driver in pvSetup()
#ifdef Q_OS_LINUX
  err = libusb_attach_kernel_driver(pvHandle, 0);  /// \todo interface nunber hardcoded, BAD
  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_NOT_FOUND:  // No driver was detached
      break;
    case LIBUSB_ERROR_INVALID_PARAM:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot re-attach kernel driver (interface not found) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      break;
    }
    case LIBUSB_ERROR_NO_DEVICE:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot re-attach kernel driver (device not found/disconnected) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      break;
    }
    case LIBUSB_ERROR_NOT_SUPPORTED:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot re-attach kernel driver (unload is not supported on current platform) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      break;
    }
    default:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot re-attach kernel driver (unhandled error) on device " + pvPortName, mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      break;
    }
  }
#endif
  // Close port
  libusb_close(pvHandle);
  // Free memory
  delete[] pvReadBuffer;
  pvReadBuffer = 0;
  delete[] pvWriteBuffer;
  pvWriteBuffer = 0;
}

mdtAbstractPort::error_t mdtUsbPort::pvSetup()
{
  Q_ASSERT(isOpen());
  ///Q_ASSERT(pvConfig != 0);
  Q_ASSERT(pvHandle != 0);

  // Essais
  libusb_device *device;
  mdtUsbDeviceDescriptor deviceDescriptor;
  mdtUsbEndpointDescriptor *bulkEndpointDescriptor;
  mdtUsbEndpointDescriptor *interruptEndpointDescriptor;
  int err;

  // Search devices endpoints
  device = libusb_get_device(pvHandle);
  Q_ASSERT(device != 0);
  err = deviceDescriptor.fetchAttributes(device, true);
  if(err != 0){
    switch(err){
      case LIBUSB_ERROR_NOT_FOUND:
        return PortNotFound;
      default:
        return UnknownError;
    }
  }
  // Find device's output endpoints
  bulkEndpointDescriptor = deviceDescriptor.firstBulkOutEndpoint(0, 0, true);
  interruptEndpointDescriptor = deviceDescriptor.firstInterruptOutEndpoint(0, 0, true);
  if((bulkEndpointDescriptor == 0)&&(interruptEndpointDescriptor == 0)){
    mdtError e(MDT_USB_IO_ERROR, "Found no output endpoint for data in device " + pvPortName, mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return SetupError;
  }
  if((bulkEndpointDescriptor != 0)&&(interruptEndpointDescriptor != 0)){
    mdtError e(MDT_USB_IO_ERROR, "Found output bulk and interrupt, choose bulk out endpoint for data in device " + pvPortName, mdtError::Warning);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    // Choose bulk out
    interruptEndpointDescriptor = 0;
  }
  if(bulkEndpointDescriptor != 0){
    ///qDebug() << "Bulk OUT address: " << bulkEndpointDescriptor->address();
    ///qDebug() << " Max packet size: " << bulkEndpointDescriptor->maxPacketSize();
    pvWriteBufferSize = bulkEndpointDescriptor->transactionsCountPerMicroFrame();
    if(pvWriteBufferSize < 1){
      pvWriteBufferSize = 1;
    }
    pvWriteBufferSize *= bulkEndpointDescriptor->maxPacketSize();
    pvWriteEndpointAddress = bulkEndpointDescriptor->address() | LIBUSB_ENDPOINT_OUT;
    pvWriteBuffer = new char[pvWriteBufferSize];
    pvWriteTransfertType = LIBUSB_TRANSFER_TYPE_BULK;
    qDebug() << "Bulk OUT address: 0x" << hex << pvWriteEndpointAddress;
    qDebug() << " Max packet size: " << pvWriteBufferSize;
  }
  if(interruptEndpointDescriptor != 0){
    ///qDebug() << "Interrupt OUT address: " << interruptEndpointDescriptor->address();
    ///qDebug() << " Max packet size: " << interruptEndpointDescriptor->maxPacketSize();
    pvWriteBufferSize = interruptEndpointDescriptor->transactionsCountPerMicroFrame();
    if(pvWriteBufferSize < 1){
      pvWriteBufferSize = 1;
    }
    pvWriteBufferSize *= interruptEndpointDescriptor->maxPacketSize();
    pvWriteEndpointAddress = interruptEndpointDescriptor->address() | LIBUSB_ENDPOINT_OUT;
    pvWriteBuffer = new char[pvWriteBufferSize];
    pvWriteTransfertType = LIBUSB_TRANSFER_TYPE_INTERRUPT;
    qDebug() << "Interrupt OUT address: 0x" << hex << pvWriteEndpointAddress;
    qDebug() << " Max packet size: " << pvWriteBufferSize;
  }
  Q_ASSERT(pvWriteBuffer != 0);
  // Find device's input endpoints
  bulkEndpointDescriptor = deviceDescriptor.firstBulkInEndpoint(0, 0, true);
  interruptEndpointDescriptor = deviceDescriptor.firstInterruptInEndpoint(0, 0, true);
  if((bulkEndpointDescriptor == 0)&&(interruptEndpointDescriptor == 0)){
    mdtError e(MDT_USB_IO_ERROR, "Found no input endpoint for data in device " + pvPortName, mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtUsbPort");
    e.commit();
    return SetupError;
  }
  if(bulkEndpointDescriptor != 0){
    pvReadBufferSize = bulkEndpointDescriptor->transactionsCountPerMicroFrame();
    if(pvReadBufferSize < 1){
      pvReadBufferSize = 1;
    }
    pvReadBufferSize *= bulkEndpointDescriptor->maxPacketSize();
    pvReadEndpointAddress = bulkEndpointDescriptor->address() | LIBUSB_ENDPOINT_IN;
    pvReadBuffer = new char[pvReadBufferSize];
    pvReadTransfertType = LIBUSB_TRANSFER_TYPE_BULK;
    qDebug() << "Bulk IN address: 0x" << hex << pvReadEndpointAddress;
    qDebug() << " Max packet size: " << pvReadBufferSize;
  }
  if(interruptEndpointDescriptor != 0){
    ///qDebug() << "Interrupt IN address: " << interruptEndpointDescriptor->address();
    ///qDebug() << " Max packet size: " << interruptEndpointDescriptor->maxPacketSize();
    // Some device have a Bulk IN + Interrupt IN
    if(bulkEndpointDescriptor != 0){
      qDebug() << "Additionnal interrupt IN , currently not supported..";
    }else{
      pvReadBufferSize = interruptEndpointDescriptor->transactionsCountPerMicroFrame();
      if(pvReadBufferSize < 1){
        pvReadBufferSize = 1;
      }
      pvReadBufferSize *= interruptEndpointDescriptor->maxPacketSize();
      pvReadEndpointAddress = interruptEndpointDescriptor->address() | LIBUSB_ENDPOINT_IN;
      pvReadBuffer = new char[pvReadBufferSize];
      pvReadTransfertType = LIBUSB_TRANSFER_TYPE_INTERRUPT;
      qDebug() << "Interrupt IN address: 0x" << hex << pvReadEndpointAddress;
      qDebug() << " Max packet size: " << pvReadBufferSize;
    }
  }
  Q_ASSERT(pvReadBuffer != 0);
  // Unload possibly loaded driver that uses the device
#ifdef Q_OS_LINUX
  err = libusb_detach_kernel_driver(pvHandle, 0);  /// \todo interface nunber hardcoded, BAD
  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_NOT_FOUND:  // No driver loaded
      break;
    case LIBUSB_ERROR_INVALID_PARAM:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot detach kernel driver (interface not found) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return PortNotFound;
    }
    case LIBUSB_ERROR_NO_DEVICE:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot detach kernel driver (device not found/disconnected) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return PortNotFound;
    }
    case LIBUSB_ERROR_NOT_SUPPORTED:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot detach kernel driver (unload is not supported on current platform) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return SetupError;
    }
    default:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot detach kernel driver (unhandled error) on device " + pvPortName, mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return SetupError;
    }
  }
#endif
  // Claim the interface
  err = libusb_claim_interface(pvHandle, 0);  /// \todo interface nunber hardcoded, BAD
  switch(err){
    case 0:
      break;
    case LIBUSB_ERROR_NOT_FOUND:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot claim interface (interface not found) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return PortNotFound;
    }
    case LIBUSB_ERROR_BUSY:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot claim interface (interface busy) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return PortLocked;
    }
    case LIBUSB_ERROR_NO_DEVICE:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot claim interface (device not found/disconnected) on device " + pvPortName, mdtError::Error);
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return PortNotFound;
    }
    default:
    {
      mdtError e(MDT_USB_IO_ERROR, "Cannot claim interface (unhandled error) on device " + pvPortName, mdtError::Error);
      e.setSystemError(err, errorText(err));
      MDT_ERROR_SET_SRC(e, "mdtUsbPort");
      e.commit();
      return SetupError;
    }
  }
  
  /// \todo Setup functions ......

  /// \todo Set timeouts ...
  
  return NoError;
}

QString mdtUsbPort::errorText(int errorCode) const
{
  switch(errorCode){
    case LIBUSB_ERROR_IO:
      return tr("Input/output error");
    case LIBUSB_ERROR_INVALID_PARAM:
      return tr("Invalid parameter");
    case LIBUSB_ERROR_ACCESS:
      return tr("Access denied (insufficient permissions)");
    case LIBUSB_ERROR_NO_DEVICE:
      return tr("No such device (it may have been disconnected)");
    case LIBUSB_ERROR_NOT_FOUND:
      return tr("Entity not found");
    case LIBUSB_ERROR_BUSY:
      return tr("Resource busy");
    case LIBUSB_ERROR_TIMEOUT:
      return tr("Operation timed out");
    case LIBUSB_ERROR_OVERFLOW:
      return tr("Overflow");
    case LIBUSB_ERROR_PIPE:
      return tr("Pipe error");
    case LIBUSB_ERROR_INTERRUPTED:
      return tr("System call interrupted (perhaps due to signal)");
    case LIBUSB_ERROR_NO_MEM:
      return tr("Insufficient memory");
    case LIBUSB_ERROR_NOT_SUPPORTED:
      return tr("Operation not supported or unimplemented on this platform");
    default:
      return tr("Unknown error");
  }
}
