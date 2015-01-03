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
#ifndef MDT_USBTMC_TRANSFER_HANDLER_H
#define MDT_USBTMC_TRANSFER_HANDLER_H

#include "mdtError.h"
#include "mdtUsbTransferPool.h"
#include "mdtUsbDeviceDescriptor.h"
#include "mdtUsbInterfaceDescriptor.h"
#include "mdtUsbEndpointDescriptor.h"
#include "mdtUsbTransfer.h"
#include "mdtUsbtmcControlTransfer.h"
#include "mdtUsbtmcBulkTransfer.h"
#include "mdtUsbtmcInterruptTransfer.h"
#include "mdtUsbtmcMessage.h"

#include "mdtBasicStateMachine.h"

///#include "mdtUsbtmcTransferHandlerStateMachine.h"

// #include <vector>
 #include <boost/msm/back/state_machine.hpp>
 #include <boost/msm/front/state_machine_def.hpp>
// #include <boost/msm/front/functor_row.hpp>
// #include <boost/msm/front/euml/common.hpp>
// #include <boost/msm/front/euml/operator.hpp>
#include <boost/shared_ptr.hpp> 

#include <QObject>
#include <QString>
#include <libusb-1.0/libusb.h>
#include <cstdint>
#include <mutex>

#include <memory>

#include <QDebug>

///class mdtUsbtmcControlTransfer;

class mdtUsbtmcPort;

namespace mdtUsbtmcTransferHandlerStateMachine
{
  struct StateMachine;
};

/*! \brief USBTMC transfer handler
 */
class mdtUsbtmcTransferHandler
{
 public:

  /*! \brief USBTMC transfer handler state
   */
  enum class State_t
  {
    Stopped = 0,      /*!< USB port thread is stopped */
    Setup,            /*!< USB port thread is starting and has called setup() */
    Running,          /*!< USB port thread is running (ready for I/O) */
    AbortingBulkIo,   /*!< Currently aborting a Bulk-OU/Bulk-OUT transfer - No bulk transfer possible */
    Stopping,         /*!< USB port thread is stopping (No I/O, submit no more transfer) */
    Disconnected,     /*!< Device is no more present */
    Error             /*!< USB port thread is stopped due to a error (No I/O) */
  };

  /*! \brief Constructor
   */
  mdtUsbtmcTransferHandler(libusb_context *usbContext, mdtUsbtmcPort & port);

  /*! \brief Setup handler
   */
  bool setup(libusb_device_handle *handle, const mdtUsbDeviceDescriptor & descriptor, uint8_t bInterfaceNumber);

  /*! \brief Check if current interface has a Interrupt-IN endpoint
   */
  bool hasInterruptInEndpoint() const
  {
    return !pvInterruptInDescriptor.isEmpty();
  }

  /*! \brief Submit a command
   *
   * \param message USBTMC message that contains data to send.
   *                 If all data are readen from message, EOM flag will be set in USBTMC transfer.
   * \param responseExpected If message is a query that expects a response, like *IDN?, set this flag.
   * \param timeout Timeout [ms]
   */
  bool submitCommand(mdtUsbtmcTxMessage & message, bool responseExpected, unsigned int timeout);

  /*! \brief Get received data
   *
   * \param message USBTMC message in witch data will be appended.
   */
  void getReceivedData(mdtUsbtmcRxMessage & message);

  /*! \brief Will cancel active transfers and change state to Stopping
   */
  void submitStopRequest();

  /*! \brief Will cancel all pending bulk-OUT and bulk-IN transfers and request I/O buffer clear
   *
   * Once all transfers are cancelled, INITIATE_CLEAR and, if needed,
   *  CHECK_CLEAR_STATUS is sent to device.
   */
  void submitBulkIoClear();

  /*! \brief Will cancel all pending bulk-OUT and bulk-IN transfers and request I/O buffer clear
   *
   * Is called by state machine
   */
  void beginBulkIoClear();

  /*! \brief Submit a custom transfer
   *
   * Used for debug and testing.
   * \pre transferBufferLength must be <= Internal frame capacity
   */
  void dbgSubmitCustomBulkOutTransfer(uint8_t msgID, uint8_t bTag, uint8_t bTagInverse, uint32_t transferSize, uint8_t bmTransferAttributes, uint8_t termChar,
                                      bool responseExpected, int transferBufferLength, unsigned int timeout);

  /*! \brief Force to abort a pending Bulk-OUT transfer
   *
   * Will get a pending Bulk-OUT transfer and proceed abort it.
   *  Used for debug and testing.
   */
  void dbgSubmitAbortOnePendingBulkOutTransfer();

  /*! \brief Force to abort a pending Bulk-IN transfer
   *
   * Will get a pending Bulk-IN transfer and proceed abort it.
   *  Used for debug and testing.
   */
  void dbgSubmitAbortOnePendingBulkInTransfer();

  /*! \brief Submit a CLEAR_FEATURE control request with wValue = ENDPOINT_HALT for given endpoint
   */
  void submitClearEndpointHalt(uint8_t endpointNumber, unsigned int timeout);

  /*! \brief Submit a CLEAR_FEATURE control request with wValue = ENDPOINT_HALT for Bulk-OUT endpoint
   */
  void submitClearBulkOutEndpointHalt(unsigned int timeout)
  {
    submitClearEndpointHalt(pvBulkOutDescriptor.number(), timeout);
  }

  /*! \brief Submit a CLEAR_FEATURE control request with wValue = ENDPOINT_HALT for Bulk-IN endpoint
   */
  void submitClearBulkInEndpointHalt(unsigned int timeout)
  {
    submitClearEndpointHalt(pvBulkInDescriptor.number(), timeout);
  }

  /*! \brief Submit control transfers cancellation
   */
  void submitControlTransfersCancel();

  /*! \brief Control transfer callback
   */
  static void controlTransferCallback(libusb_transfer *transfer);

  /*! \brief Bulk-OUT transfer callback
   */
  static void bulkOutTransferCallback(libusb_transfer *transfer);

  /*! \brief Bulk-IN transfer callback
   */
  static void bulkInTransferCallback(libusb_transfer *transfer);

  /*! \brief Interrupt-IN transfer callback
   */
  static void interruptInTransferCallback(libusb_transfer *transfer);

  /*! \brief Get last error
   */
  mdtError lastError() const
  {
    return pvLastError;
  }

  /*! \brief Set current state
   */
  void setCurrentState(State_t s){
    pvStateMachine.setCurrentState(s);
  }

  /*! \brief Get current state
   */
  inline State_t currentState() const
  {
    return pvStateMachine.currentState();
  }

  /*! \brief Check if handler must be stopped
   *
   * Is called by port thread to know if it must exit.
   */
  bool mustBeStopped() const;
//   {
//     switch(currentState()){
//       case State_t::Running:
//       case State_t::AbortingBulkIo:
//       case State_t::Stopping:
//         return false;
//       default:
//         return true;
//     }
//   }

  /*! \brief Notify error
   *
   * Must be called only by state machine
   */
  void notifyError(const mdtError & error);

  /*! \brief Send a event to state machine
   *
   * Used by internal state machine to send events.
   *
   * This is needed, because only mdtUsbtmcTransferHandler instance
   *  contains a StateMachine object, witch locks the event mutex
   *  when calling process_event().
   */
  template<typename EventType>
  void sendEvent(EventType const & event);

  /*! \brief Check if there are pending transfers
   *
   * Is called by state machine
   */
  bool hasPendingTransfers();

  /*! \brief Submit cancellation of all pending transfers
   *
   * Is called by state machine
   */
  void submitCancelAllTransfers();

  /*! \brief Restore all pending transfers to their pool
   *
   * Is called by state machine
   */
  void restoreAllTransfers();

  /*! \brief Handle control transfer complete
   *
   * Is called by state machine
   */
  void handleControlTransferComplete(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Restore control transfer to pool
   *
   * Is called by state machine
   */
  void restoreControlTransferToPool(mdtUsbtmcControlTransfer *transfer, bool lockMutex);

  /*! \brief Handle Bulk-OUT transfer complete
   *
   * Is called by state machine
   */
  void handleBulkOutTransferComplete(mdtUsbtmcBulkTransfer *transfer);

  /*! \brief Restore bulk-OUT transfer to pool
   *
   * Is called by state machine
   */
  void restoreBulkOutTransferToPool(mdtUsbtmcBulkTransfer *transfer, bool lockMutex);

  /*! \brief Handle Bulk-IN transfer complete
   *
   * Is called by state machine
   */
  void handleBulkInTransferComplete(mdtUsbtmcBulkTransfer *transfer);

  /*! \brief Restore bulk-IN transfer to pool
   */
  void restoreBulkInTransferToPool(mdtUsbtmcBulkTransfer *transfer, bool lockMutex);

  /*! \brief Submit a Interrupt-IN transfer
   */
  void submitInterruptInTransfer(unsigned int timeout, mdtUsbtmcInterruptTransfer *transfer = 0);

  /*! \brief Handle Interrupt-IN transfer complete
   *
   * Is called by state machine
   */
  void handleInterruptInTransferComplete(mdtUsbtmcInterruptTransfer *transfer);

  /*! \brief Restore interrupt-IN transfer to pool
   *
   * Is called by state machine
   */
  void restoreInterruptInTransferToPool(mdtUsbtmcInterruptTransfer *transfer, bool lockMutex);

 private:

  /*! \brief Begin to abort a bulk-OUT transfer
   *
   * Will submit the first INITIATE_ABORT_BULK_OUT request and return.
   *  Later on, handleControlTransferComplete() will receive a response,
   *  then calls handleInitiateAbortBulkOutResponse().
   */
  void beginAbortBulkOutTransfer(uint8_t bTag);

  /*! \brief Handle INITIATE_ABORT_BULK_OUT response
   *
   * This function is called by handleControlTransferComplete()
   *  when a INITIATE_ABORT_BULK_OUT response was received.
   *
   * Regarding USBTMC status:
   *  - INITIATE_ABORT_BULK_OUT can be resubmitted
   *  - CHECK_ABORT_BULK_OUT_STATUS request can be sent
   *  - If done, current state will be updated back to Running
   */
  void handleInitiateAbortBulkOutResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Handle CHECK_ABORT_BULK_OUT_STATUS resonse
   *
   * This function is called by handleControlTransferComplete()
   *  when a CHECK_ABORT_BULK_OUT_STATUS response was received.
   *
   * Regarding USBTMC status:
   * 
   */
  void handleCheckAbortBulkOutStatusResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Submit a REQUEST_DEV_DEP_MSG_IN
   */
  void submitRequestDevDepMsgIn(mdtUsbtmcBulkTransfer *transfer);

  /*! \brief Get next bTag for bulk-OUT endpoint
   */
  uint8_t nextBulkOutbTag()
  {
    ++pvBulkOutbTag;
    if(pvBulkOutbTag == 0){
      ++pvBulkOutbTag;
    }
    return pvBulkOutbTag;
  }

  /*! \brief Retire IRPs for given transfer
   */
  void submitRetireBulkIRPs(mdtUsbtmcBulkTransfer *transfer);

  /*! \brief Submit cancel for all Bulk-OUT transfers
   *
   * If a action is given as argument, it will be set to last transfer cancel request.
   */
  void submitCancelAllBulkOutTransfers(bool lockMutex, mdtUsbtmcBulkTransfer::SplitAction_t action = mdtUsbtmcBulkTransfer::SplitAction_t::NoAction);

  /*! \brief Submit cancel for all Bulk-IN transfers
   *
   * If a action is given as argument, it will be set to last transfer cancel request.
   */
  void submitCancelAllBulkInTransfers(bool lockMutex, mdtUsbtmcBulkTransfer::SplitAction_t action = mdtUsbtmcBulkTransfer::SplitAction_t::NoAction);

  /*! \brief Submit a Bulk-IN transfer
   */
  void submitBulkInTransfer(unsigned int timeout, mdtUsbtmcBulkTransfer::SplitAction_t action = mdtUsbtmcBulkTransfer::SplitAction_t::NoAction);

  /*! \brief Begin to abort a bulk-IN transfer
   *
   * At first, current state is set to AbortingBulkIo,
   *  preventing new bulk request to be accepted.
   *
   * Will retire IRPs for all pending bulk-OUT transfers
   *  that expect a response and submit the first INITIATE_ABORT_BULK_IN request.
   *
   *  Later on, handleControlTransferComplete() will receive a response,
   *  then calls handleInitiateAbortBulkInResponse().
   */
  void beginAbortBulkInTransfer(uint8_t bTag);

  /*! \brief Handle INITIATE_ABORT_BULK_IN response
   *
   * This function is called by handleControlTransferComplete()
   *  when a INITIATE_ABORT_BULK_IN response was received.
   *
   * If device returned STATUS_SUCCESS, split action will be set to CHECK_ABORT_STATUS for a pending bulk-IN transfer.
   *  This way, handleInitiateAbortBulkInResponse() knows that it must call submitCheckAbortBulkInStatusRequest().
   *
   * Regarding other USBTMC status:
   *  - INITIATE_ABORT_BULK_IN can be resubmitted
   *  - If done, retires bulk-IN IRPs and current state will be updated back to Running
   */
  void handleInitiateAbortBulkInResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Submit CHECK_ABORT_BULK_IN_STATUS query
   *
   * Called by handleBulkInTransferComplete()
   *  if transfer split action was set to CHECK_ABORT_STATUS.
   */
  void submitCheckAbortBulkInStatusRequest(mdtUsbtmcControlTransfer *controlTransfer = 0);

  /*! \brief Handle CHECK_ABORT_BULK_IN_STATUS resonse
   *
   * This function is called by handleControlTransferComplete()
   *  when a CHECK_ABORT_BULK_IN_STATUS response was received.
   *
   * If device returned STATUS_PENDING with bmAbortBulkIn.D0=1 :
   *  - If a pending bulk-IN transfer exists, its action will be set to CHECK_ABORT_STATUS
   *  - Else, a new bulk-IN transfer is initiated with action set to CHECK_ABORT_STATUS
   *
   * If device returned STATUS_PENDING with bmAbortBulkIn.D0=0 :
   *  - submitCheckAbortBulkInStatusRequest() is called after some time.
   *
   * If device returned a other status, current state will be updated back to Running.
   */
  void handleCheckAbortBulkInStatusResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Submit INITIATE_CLEAR request
   *
   * This function is called by handleBulkOutTransferComplete() and/or handleBulkInTransferComplete() or directly by submitBulkTransfersCancellation()
   *  when completed transfer has split action INITIATE_CLEAR.
   *
   * This function will really submit the request if there is not pending
   *  bulk transfer, that to prevent sending INITIATE_CLEAR more than once.
   */
  void submitInitiateClearRequest();

  /*! \brief Handle INITIATE_CLEAR response
   *
   * This function is called by handleControlTransferComplete()
   *  when a INITIATE_CLEAR response was received.
   */
  void handleInitiateClearResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Submit CHECK_CLEAR_STATUS query
   *
   * Can be called by handleBulkInTransferComplete()
   *  if transfer split action was set to CHECK_CLEAR_STATUS
   */
  void submitCheckClearStatusRequest(mdtUsbtmcControlTransfer *controlTransfer = 0);

  /*! \brief Handle CHECK_CLEAR_STATUS response
   *
   * This function is called by handleControlTransferComplete()
   *  when a CHECK_CLEAR_STATUS response was received.
   */
  void handleCheckClearStatusResponse(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Submit cancel for all Interrupt-IN transfers
   */
  void submitCancelAllInterruptInTransfers(bool lockMutex);

  /*! \brief Submit a GET_CAPABILITIES request and \todo Comment..
   *
   * This function does synchronous transfer, be care about.
   *
   * \sa See handleSyncEvents()
   */
  bool getCapabilities(uint8_t interfaceNumber, unsigned int timeout);

  /*! \brief Process synchronous control transfer
   *
   * Note: must be called only from port thread, but not from a callback.
   *  See handleSyncEvents() for some explaination.
   *
   * This function handles no lock and no transfer pool,
   *  caller is responsible of this.
   */
  bool processSyncControlTransfer(mdtUsbtmcControlTransfer *transfer);

  /*! \brief Get device identification string
   *
   * Helper function for error messages generation.
   *  Will simply get pvDeviceDescriptor::idString() with some "decoration text", will not query device.
   */
  QString deviceIdString() const;

  /*! \brief Get libubs_transfer_status text
   */
  QString libusbTransferStatusText(libusb_transfer_status status) const;

  /*! \brief Set last error regarding bulk-OUT transfer status code
   */
  void setLastErrorFromBulkOutTransferStatus(mdtUsbtmcBulkTransfer *transfer, mdtError::level_t level, bool lockMutex);

  /*! \brief Set last error regarding bulk-IN transfer status code
   */
  void setLastErrorFromBulkInTransferStatus(mdtUsbtmcBulkTransfer *transfer, mdtError::level_t level, bool lockMutex);

  /*! \brief Set last error regarding interrupt-IN transfer status code
   */
  void setLastErrorFromInterruptInTransferStatus(mdtUsbtmcInterruptTransfer *transfer, mdtError::level_t level, bool lockMutex);

  /*! \brief Set last error regarding transfer status code
   *
   * Note: will only set the systemError part
   */
  void setLastErrorFromTransferStatus(libusb_transfer_status status, bool lockMutex);

  /*! \brief Set state regarding libusb_error
   */
  void setCurrentStateFromLibusbError(libusb_error err);

  Q_DISABLE_COPY(mdtUsbtmcTransferHandler);

  libusb_context *pvUsbContext;
  libusb_device_handle *pvDeviceHandle;
  mdtBasicStateMachine<State_t> pvStateMachine;
  
  ///std::unique_ptr<mdtUsbtmcTransferHandlerStateMachine::StateMachine> pvStateMachineNew;
  ///std::shared_ptr<StateMachine> pvStateMachineNew;
  boost::shared_ptr<mdtUsbtmcTransferHandlerStateMachine::StateMachine> pvStateMachineNew;
  ///smWrapper pvStateMachineNew;
  
  mdtUsbtmcPort & pvPort;
  // Descriptors
  mdtUsbDeviceDescriptor pvDeviceDescriptor;
  mdtUsbInterfaceDescriptor pvInterfaceDescriptor;
  mdtUsbEndpointDescriptor pvBulkOutDescriptor;
  mdtUsbEndpointDescriptor pvBulkInDescriptor;
  mdtUsbEndpointDescriptor pvInterruptInDescriptor;
  // Bulk-OUT transfer members
  mdtUsbTransferPool<mdtUsbtmcBulkTransfer> pvBulkOutTransferPool;
  uint8_t pvBulkOutbTag;
  std::mutex pvBulkOutTransferMutex;
  // Bulk-IN transfer members
  mdtUsbTransferPool<mdtUsbtmcBulkTransfer> pvBulkInTransferPool;
  std::mutex pvBulkInTransferMutex;
  // Interrupt-IN transfer members
  mdtUsbTransferPool<mdtUsbtmcInterruptTransfer> pvInterruptInTransferPool;
  std::mutex pvInterruptInTransferMutex;
  // Control transfer members
  mdtUsbTransferPool<mdtUsbtmcControlTransfer> pvControlTransferPool;
  std::mutex pvControlTransferMutex;
  // Error members
  mdtError pvLastError;
  std::mutex pvLastErrorMutex;
};

#endif  // #ifndef MDT_USBTMC_TRANSFER_HANDLER_H
