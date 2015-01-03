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
#ifndef MDT_USBTMC_TRANSFER_HANDLER_STATE_MACHINE_H
#define MDT_USBTMC_TRANSFER_HANDLER_STATE_MACHINE_H

#include "mdtUsbtmcTransferHandler.h"
#include "mdtUsbtmcControlTransfer.h"
#include "mdtUsbtmcBulkTransfer.h"
#include "mdtUsbtmcInterruptTransfer.h"
#include "mdtError.h"
#include <vector>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <mutex>
#include <QtGlobal>

///#include <memory>

#include <QDebug>

/*! \brief USBTMC transfer handler state machine namespace
 *
 * State machine specific stuff is defined in this namespace,
 *  such as events, states and state machine itself.
 *
 * Some rules:
 *  - We cannot call process_event() from a on_entry or on_exit,
 *    because given FSM is not a StateMachine instance,
 *    but a StateMachine_ one, witch will not lock the mutex.
 */
namespace mdtUsbtmcTransferHandlerStateMachine
{
  using namespace boost::msm::front;

  /*
   * Events
   */

  /*! \brief Error occured event
   */
  struct ErrorOccuredEvent
  {
    /*! \brief Construct error event from mdtError object
     *
     * Note: it is assumed that given error has a libusb_error
     *  as system error.
     */
    ErrorOccuredEvent(const mdtError & e) : error(e){}
    mdtError error;
  };

  /*! \brief Device disconnected event
   */
  struct DeviceDisconnectedEvent {};

  /*! \brief Stop requested event
   */
  struct StopRequestedEvent {};

  /*! \brief Control transfer completed event
   */
  struct ControlTransferCompletedEvent
  {
    ControlTransferCompletedEvent(mdtUsbtmcControlTransfer *t)
     : transfer(t)
    {
      Q_ASSERT(transfer != 0);
    }
    mdtUsbtmcControlTransfer *transfer;
  };

  /*! \brief Bulk-OUT transfer completed event
   */
  struct BulkOutTransferCompletedEvent
  {
    BulkOutTransferCompletedEvent(mdtUsbtmcBulkTransfer *t)
     : transfer(t)
    {
      Q_ASSERT(transfer != 0);
    }
    mdtUsbtmcBulkTransfer *transfer;
  };

  /*! \brief Bulk-IN transfer completed event
   */
  struct BulkInTransferCompletedEvent
  {
    BulkInTransferCompletedEvent(mdtUsbtmcBulkTransfer *t)
     : transfer(t)
    {
      Q_ASSERT(transfer != 0);
    }
    mdtUsbtmcBulkTransfer *transfer;
  };

  /*! \brief Interrupt-IN transfer completed event
   */
  struct InterruptInTransferCompletedEvent
  {
    InterruptInTransferCompletedEvent(mdtUsbtmcInterruptTransfer *t)
     : transfer(t)
    {
      Q_ASSERT(transfer != 0);
    }
    mdtUsbtmcInterruptTransfer *transfer;
  };

  /*! \brief No more Control transfer pending event
   */
  struct NoMoreControlTransferPendingEvent {};

  /*! \brief No more Bulk-OUTtransfer pending event
   */
  struct NoMoreBulkOutTransferPendingEvent {};

  /*! \brief No more Bulk-INtransfer pending event
   */
  struct NoMoreBulkInTransferPendingEvent {};

  /*! \brief No more Interrupt-IN transfer pending event
   */
  struct NoMoreInterruptInTransferPendingEvent {};

  /*! \brief No more transfer pending event
   */
  struct NoMoreTransferPendingEvent {};

  /*! \brief Clear Bulk I/O requested event
   */
  struct ClearBulkIoRequestedEvent {};

  /*! \brief Split action done event
   */
  struct SplitActionDoneEvent {};

  /*
   * Flags
   */

  /*! \brief Tell usb port thread that it must not handle events anymore
   */
  struct MustBeStoppedFlag {};

  /*! \brief Tell USBTMC port that we can accept requests
   */
  struct IdleFlag {};

  /// \todo Remove !
  struct fakeRequest {};

  /*
   * Running_ submachine
   */

  /*! \brief Running submachine
   */
  struct Running_ : public state_machine_def<Running_>
  {
    /*! \brief Default constructor
     */
    Running_()
     : transferHandler(0)
    {
      qDebug() << "Running_() - transferHandler: 0x" << hex << transferHandler;
    }
    /*! \brief Concrete constructor
     */
    Running_(mdtUsbtmcTransferHandler *th)
     : transferHandler(th)
    {
      qDebug() << "Running_() - transferHandler: 0x" << hex << transferHandler;
      Q_ASSERT(transferHandler != 0);
    }
    /*! \brief Running submachine entry/
     */
    template<typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
      qDebug() << "Entering Running ... - transferHandler: 0x" << hex << transferHandler;
      Q_ASSERT(fsm.transferHandler != 0);
      // Submit first Interrupt-IN transfer (if interface has Interrupt-IN endpoint)
      if(fsm.transferHandler->hasInterruptInEndpoint()){
        fsm.transferHandler->submitInterruptInTransfer(50000);
      }
    }

    /*! \brief Running submachine exit/
     */
    template<typename Event, typename FSM>
    void on_exit(Event const &, FSM &)
    {
      qDebug() << "Leaving Running ...";
    }

    /*
     * Running_ submachine - actions
     */

    /*
     * Running_ submachine - guards
     */

    /*
     * Running_ submachine - states
     */

    /*! \brief Idle state
     */
    struct Idle : public boost::msm::front::state<>
    {
      typedef boost::mpl::vector1<IdleFlag> flag_list;
      /*! \brief Idle entry/
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM & fsm)
      {
        qDebug() << "Entering Idle ...";
      }
      /*! \brief Idle exit/
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Idle ...";
      }
    };

    /*! \brief Processing state
     */
    struct Processing : public boost::msm::front::state<>
    {
      /*! \brief Processing entry/
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM &)
      {
        qDebug() << "Entering Processing ...";
      }
      /*! \brief Processing exit/
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Processing ...";
      }
    };

    /*! \brief ClearingBulkIo state
     */
    struct ClearingBulkIo : public boost::msm::front::state<>
    {
      /*! \brief ClearingBulkIo entry/
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM & fsm)
      {
        qDebug() << "Entering ClearingBulkIo ...";
        Q_ASSERT(fsm.transferHandler != 0);
        fsm.transferHandler->beginBulkIoClear();
      }
      /*! \brief ClearingBulkIo exit/
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving ClearingBulkIo ...";
      }
    };

    /*! \brief Initial state of Running submachine
     */
    typedef Idle initial_state;

    // Make transition table a bit more readable
    typedef Running_ rsm;

    /*! \brief Running_ submachine transition table
     */
    struct transition_table : boost::mpl::vector<
      //      Start state     Event                                   Next state       Action                                   Guard
      // +----------------+-----------------------------------------+-----------------+----------------------------------------+-------------------------------+----
      _row <   Idle       , ClearBulkIoRequestedEvent               , ClearingBulkIo                                                                             >
      // +----------------+-----------------------------------------+-----------------+----------------------------------------+-------------------------------+----
    >{};

    // Running_ members
    mdtUsbtmcTransferHandler *transferHandler;
  };

  /*! \brief Running submachine
    */
  typedef boost::msm::back::state_machine<Running_> Running;

  /*
   * StateMachine_ definition
   */

  /*! \brief Main state machine definition class
   */
  struct StateMachine_ : public boost::msm::front::state_machine_def<StateMachine_>
  {
    /*! \brief State machine def Constructor
     */
    StateMachine_()
     : transferHandler(0)
    {
    }

    /*! \brief Set transfer handler
     *
     * Must be called before starting the state machine
     */
    void setTh(mdtUsbtmcTransferHandler *th)
    {
      transferHandler = th;
    }

    /*
     * StateMachine_ actions
     */

    /*! \brief Restore all transfers to pools action
     */
    template<typename EventType>
    void restoreAllTransfers(EventType const &)
    {
      Q_ASSERT(transferHandler != 0);
      qDebug() << "** restoreAllTransfers ..";
      transferHandler->restoreAllTransfers();
    }

    /*! \brief Handle Control transfer completed action
     */
    void handleControlTransferCompleted(ControlTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->handleControlTransferComplete(event.transfer);
    }

    /*! \brief Restore Control transfer to pool action
     *
     * This version will send NoMoreTransferPendingEvent if transfer handler has no more pending transfers.
     * (NNMTP: Notify NoMoreTransferPending)
     */
    void restoreControlTransferNNMTP(ControlTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->restoreControlTransferToPool(event.transfer, true);
      if(!transferHandler->hasPendingTransfers()){
        transferHandler->sendEvent(NoMoreTransferPendingEvent());
      }
    }

    /*! \brief Handle Bulk-OUT transfer completed action
     */
    void handleBulkOutTransferCompleted(BulkOutTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->handleBulkOutTransferComplete(event.transfer);
    }

    /*! \brief Restore Bulk-OUT transfer to pool action
     *
     * This version will send NoMoreTransferPendingEvent if transfer handler has no more pending transfers.
     * (NNMTP: Notify NoMoreTransferPending)
     */
    void restoreBulkOutTransferNNMTP(BulkOutTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->restoreBulkOutTransferToPool(event.transfer, true);
      if(!transferHandler->hasPendingTransfers()){
        transferHandler->sendEvent(NoMoreTransferPendingEvent());
      }
    }

    /*! \brief Handle Bulk-IN transfer completed action
     */
    void handleBulkInTransferCompleted(BulkInTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->handleBulkInTransferComplete(event.transfer);
    }

    /*! \brief Restore Bulk-IN transfer to pool action
     *
     * This version will send NoMoreTransferPendingEvent if transfer handler has no more pending transfers.
     * (NNMTP: Notify NoMoreTransferPending)
     */
    void restoreBulkInTransferNNMTP(BulkInTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->restoreBulkInTransferToPool(event.transfer, true);
      if(!transferHandler->hasPendingTransfers()){
        transferHandler->sendEvent(NoMoreTransferPendingEvent());
      }
    }

    /*! \brief Handle Interrupt-IN transfer completed action
     */
    void handleInterruptInTransferCompleted(InterruptInTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->handleInterruptInTransferComplete(event.transfer);
    }

    /*! \brief Restore Interrupt-IN transfer to pool action
     *
     * This version will send NoMoreTransferPendingEvent if transfer handler has no more pending transfers.
     * (NNMTP: Notify NoMoreTransferPending)
     */
    void restoreInterruptInTransferNNMTP(InterruptInTransferCompletedEvent const & event)
    {
      Q_ASSERT(transferHandler != 0);
      transferHandler->restoreInterruptInTransferToPool(event.transfer, true);
      if(!transferHandler->hasPendingTransfers()){
        transferHandler->sendEvent(NoMoreTransferPendingEvent());
      }
    }

    /*! \brief Notify that Bulk I/O was aborted
     */
    template<typename EventType>
    void notifyBulkIoAborted(EventType const &)
    {
      Q_ASSERT(transferHandler != 0);
      qDebug() << "** notifyBulkIoAborted ..";
    }

    /*
     * StateMachine_ guards
     */

    /*! \brief Check if ErrorOccuredEvent contains a unhandled error
     */
    bool isUnhandledError(ErrorOccuredEvent const & evt)
    {
      libusb_error err = static_cast<libusb_error>(evt.error.systemNumber());
      switch(err){
        case LIBUSB_ERROR_NO_DEVICE:
          return false;
        case LIBUSB_SUCCESS:
          return true;
        case LIBUSB_ERROR_IO:
          return true;
        case LIBUSB_ERROR_INVALID_PARAM:
          return true;
        case LIBUSB_ERROR_ACCESS:
          return true;
        case LIBUSB_ERROR_NOT_FOUND:
          return true;
        case LIBUSB_ERROR_BUSY:
          return true;
        case LIBUSB_ERROR_TIMEOUT:
          return true;
        case LIBUSB_ERROR_OVERFLOW:
          return true;
        case LIBUSB_ERROR_PIPE:
          return true;
        case LIBUSB_ERROR_INTERRUPTED:
          return true;
        case LIBUSB_ERROR_NO_MEM:
          return true;
        case LIBUSB_ERROR_NOT_SUPPORTED:
          return true;
        case LIBUSB_ERROR_OTHER:
          return true;
      }
      return true;
    }

    /*! \brief Check if ErrorOccuredEvent contains a device disconnected error
     */
    bool isDeviceDisconnectedError(ErrorOccuredEvent const & evt)
    {
      libusb_error err = static_cast<libusb_error>(evt.error.systemNumber());
      switch(err){
        case LIBUSB_ERROR_NO_DEVICE:
          return true;
        default:
          return false;
      }
    }

    /*! \brief Check if transfer handler has pending transfers
     */
    template<typename EventType>
    bool hasNoPendingTransfers(EventType const &)
    {
      Q_ASSERT(transferHandler != 0);
      return !transferHandler->hasPendingTransfers();
    }

    /*! \brief Main state machine entry/ action
     */
    template<typename Event, typename FSM>
    void on_entry(Event const &, FSM &)
    {
      qDebug() << "Entering StateMachine_ ...";
      ///pvTransferHandler.fakeFunction();
      ///transferHandler->someThFakeAction();
    }

    /*! \brief Main state machine exit/ action
     */
    template<typename Event, typename FSM>
    void on_exit(Event const &, FSM &)
    {
      qDebug() << "Leaving StateMachine_ ...";
    }

    /*
     * StateMachine_ states
     */

    /*! \brief Error state
     */
    struct Error : public boost::msm::front::state<>
    {
      typedef boost::mpl::vector1<MustBeStoppedFlag> flag_list;
      /*! \brief Error entry/ action
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM & fsm)
      {
        Q_ASSERT(fsm.transferHandler != 0);
        qDebug() << "Entering Error ...";
      }
      /*! \brief Error exit/ action
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Error ...";
      }
    };

    /*! \brief Disconnected state
     */
    struct Disconnected : public boost::msm::front::state<>
    {
      typedef boost::mpl::vector1<MustBeStoppedFlag> flag_list;
      /*! \brief Disconnected entry/ action
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM & fsm)
      {
        Q_ASSERT(fsm.transferHandler != 0);
        qDebug() << "Entering Disconnected ...";
      }
      /*! \brief Disconnected exit/ action
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Disconnected ...";
      }
    };

    /*! \brief Stopping state
     */
    struct Stopping : public boost::msm::front::state<>
    {
      /*! \brief Stopping entry/ action
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM & fsm)
      {
        qDebug() << "Entering Stopping ...";
        Q_ASSERT(fsm.transferHandler != 0);
        if(fsm.transferHandler->hasPendingTransfers()){
          fsm.transferHandler->submitCancelAllTransfers();
        }else{
          fsm.transferHandler->sendEvent(NoMoreTransferPendingEvent());
        }
        qDebug() << "Entering Stopping done";
        ///fsm.process_event(ErrorOccuredEvent());
        ///fsm.transferHandler->someThFakeAction();
      }
      /*! \brief Stopping exit/ action
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Stopping ...";
      }
    };

    /*! \brief Stopped state
     */
    struct Stopped : public boost::msm::front::state<>
    {
      typedef boost::mpl::vector1<MustBeStoppedFlag> flag_list;
      /*! \brief Stopped entry/
       */
      template<typename Event, typename FSM>
      void on_entry(Event const &, FSM &)
      {
        qDebug() << "Entering Stopped ...";
      }
      /*! \brief Stopped exit/
       */
      template<typename Event, typename FSM>
      void on_exit(Event const &, FSM &)
      {
        qDebug() << "Leaving Stopped ...";
      }
    };

    /*! \brief Initial state of main state machine
     */
    ///typedef boost::mpl::vector<Stopped, AllOk> initial_state;
    typedef Running initial_state;

    // Make transition table a bit more readable
    typedef StateMachine_ sm;

    /*! \brief Main state machine transition table
     */
    struct transition_table : boost::mpl::vector<
      //      Start state     Event                                   Next state     Action                                   Guard
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      row <    Running    , ErrorOccuredEvent                       , Error         , &sm::restoreAllTransfers               , &sm::isUnhandledError          > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      row <    Running    , ErrorOccuredEvent                       , Disconnected  , &sm::restoreAllTransfers               , &sm::isDeviceDisconnectedError > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      _row <   Running    , StopRequestedEvent                      , Stopping                                                                                > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Running    , ControlTransferCompletedEvent                           , &sm::handleControlTransferCompleted                                     > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Running    , BulkOutTransferCompletedEvent                           , &sm::handleBulkOutTransferCompleted                                     > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Running    , BulkInTransferCompletedEvent                            , &sm::handleBulkInTransferCompleted                                      > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Running    , InterruptInTransferCompletedEvent                       , &sm::handleInterruptInTransferCompleted                                 > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      row <    Stopping   , ErrorOccuredEvent                       , Error         , &sm::restoreAllTransfers               , &sm::isUnhandledError          > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      row <    Stopping   , ErrorOccuredEvent                       , Disconnected  , &sm::restoreAllTransfers               , &sm::isDeviceDisconnectedError > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Stopping   , ControlTransferCompletedEvent                           , &sm::restoreControlTransferNNMTP                                        > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Stopping   , BulkOutTransferCompletedEvent                           , &sm::restoreBulkOutTransferNNMTP                                             > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Stopping   , BulkInTransferCompletedEvent                            , &sm::restoreBulkInTransferNNMTP                                              > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      a_irow < Stopping   , InterruptInTransferCompletedEvent                       , &sm::restoreInterruptInTransferNNMTP                                         > ,
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
      _row <   Stopping   , NoMoreTransferPendingEvent              , Stopped                                                                                 >
      // +----------------+-----------------------------------------+---------------+----------------------------------------+-------------------------------+----
    >{};

    /*! \brief Called when no transition exists
     */
    template <class FSM,class Event>
    void no_transition(Event const & e, FSM &, int state)
    {
        qDebug() << "No transition from state " << state << " on event " << typeid(e).name();
    }

    // StateMachine_ members
    mdtUsbtmcTransferHandler *transferHandler;
  };

  /*! \brief State machine class
   */
  struct StateMachine : public boost::msm::back::state_machine<StateMachine_>
  {
    /*! \brief Constructor
     */
    StateMachine(mdtUsbtmcTransferHandler *th)
     : boost::msm::back::state_machine<StateMachine_>(boost::msm::back::states_ << Running(th))
    {
      setTh(th);
      /*
       * We must create a Running submachine object
       *  and pass it to current state machine
       */
      ///set_states(boost::msm::back::states_ << Running(th));
    }
    /*! \brief Process event
     */
    template<class Event>
    void process_event(Event const& evt)
    {
      qDebug() << "* Processing event ...";
      std::lock_guard<std::recursive_mutex> lg(pvMutex);
      boost::msm::back::state_machine<StateMachine_>::process_event(evt);
      qDebug() << "* Processing event DONE";
    }

    /*! \brief Check if a flag is active
     */
    template <typename FlagType>
    bool is_flag_active()
    {
      std::lock_guard<std::recursive_mutex> lg(pvMutex);
      qDebug() << "Checking if flag is active ...";
      return boost::msm::back::state_machine<StateMachine_>::is_flag_active<FlagType>();
    }

    std::recursive_mutex pvMutex;
  };

};


#endif
