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
#ifndef MDT_PORT_MANAGER_H
#define MDT_PORT_MANAGER_H

#include <QString>
#include <QStringList>
#include <QObject>
#include <QThread>
#include "mdtAbstractPort.h"
#include "mdtPortInfo.h"
#include "mdtPortConfig.h"
#include "mdtPortThread.h"
#include "mdtError.h"
#include "mdtFrame.h"
#include "mdtPortTransaction.h"
#include <QByteArray>
#include <QList>
#include <QQueue>
#include <QMap>

class mdtPortThread;

/*! \brief Port manager base class
 *
 * Manages a port based on mdtAbstractPort an several threads based on mdtPortThread.
 *  The goal is to hide the complexity of the port API.
 *
 * Example:
 * \code
 * mdtPortManager m;
 * mdtPort *port;
 * mdtPortConfig *config;
 * QList<QByteArray> responses;
 *
 * // Setup
 * config = new mdtPortConfig;
 * config->setFrameType(mdtFrame::FT_ASCII);
 * config->setEndOfFrameSeq("$");
 *
 * // Init port
 * port = new mdtPort;
 * port->setConfig(config);
 *
 * // Init port manager - We wand blocking mode
 * m.setTransactionsDisabled(true);
 * m.setPort(port);
 * m.addThread(mew mdtPortWriteThread);
 * m.addThread(mew mdtPortReadThread);
 * m.setPortName("/dev/xyz"));
 * if(!m.openPort()){
 *  // Handle error
 * }
 *
 * // Start threads
 * if(!m.start()){
 *  // Handle error
 * }
 *
 * // Send some data
 * if(!m.writeData("Test$")){
 *  // Handle error
 * }
 *
 * // Wait on answer - Timout: 1500 [ms]
 * if(!m.waitReadenFrame(1500)){
 *  // Timout , handle error
 * }
 *
 * // Do something with received data
 * responses = m.readenFrames();
 * for(int i=0; i<responses.size(); i++){
 *  qDebug() << responses.at(i);
 * }
 *
 * // Cleanup - detachPort() will delete port and threads objects
 * m.detachPort(true, true);
 * delete config;
 *
 * \endcode
 *
 * A (recommended) alternative is to disable frames enqueuing and use the newReadenFrame(QByteArray) signal.
 *  - Disable enqueuing with setTransactionsDisabled(false)
 *  - Connect newReadenFrame(QByteArray) to a slot
 */
class mdtPortManager : public QThread
{
 Q_OBJECT

 public:

  mdtPortManager(QObject *parent = 0);

  /*! \brief Destructor
   *
   * If a port was set, the manager will stop (if running), and port will be closed (if open).
   *
   * Note that port set by setPort() and threads are not deleted.
   */
  virtual ~mdtPortManager();

  /*! \brief Scan for available ports
   *
   * This method is implemented is port's specific subclass.
   *  Default implementation returns a empty list.
   *
   * Note that returned list must be freed by user
   *  after usage. (for.ex. with qDeletAll() and QList::clear() ).
   *
   * \pre Manager must no running
   */
  virtual QList<mdtPortInfo*> scan();

  /*! \brief Set port object
   *
   * \pre port must be a valid pointer to the expected class instance (for ex: mdtSerialPort).
   * \pre Manager must no running
   */
  virtual void setPort(mdtAbstractPort *port);

  /*! \brief Detach port
   *
   * Will detach port from each thread and from port manager.
   *  If port was not set, this method does nothing.
   *  If manager is running, it will be stopped.
   *  If port is open, it will be closed.
   *
   * \param deletePort If true, the port object that was set with setPort() will be deleted.
   * \param deleteThreads If true, each thread added with addThread() will be deleted.
   */
  void detachPort(bool deletePort, bool deleteThreads);

  /*! \brief Add a thread and assign it to port
   *
   * \pre Port must be set with setPort before using this method
   * \pre Manager must no running
   * \pre thread must be a valid pointer
   */
  void addThread(mdtPortThread *thread);

  /*! \brief Detach threads from port and remove threads
   *
   * \param releaseMemory If true, all threads are deleted
   * \pre Port manager must not running.
   */
  void removeThreads(bool releaseMemory);

  /*! \brief Start threads
   *
   * \pre Port must be set with setPort() before use of this method.
   */
  bool start();

  /*! \brief Get the running state
   *
   * If one of the threads is running, true is returned.
   *
   * If port was not set, it returns false.
   */
  bool isRunning();

  /*! \brief Stop threads
   * 
   * \pre Port must be set with setPort() before use of this method.
   */
  void stop();

  /*! \brief Set port name
   *
   * Set the port name to internally port object.
   * Does nothing else. To open the port, use openPort().
   *
   * \pre Port must be set before with setPort()
   * \sa mdtAbstractPort
   */
  void setPortName(const QString &portName);

  /*! \brief Set port info
   *
   * Store given port info, and call setPortName() with
   *  port info's stored port name (see mdtPortInfo::portName() ).
   *
   * Setting a port info can be usefull if other informations are
   *  needed later in application (f.ex. mdtPortInfo::displayText() ).
   * You can get port informations later with portInfo().
   */
  void setPortInfo(mdtPortInfo info);

  /*! \brief Get port info
   */
  mdtPortInfo portInfo();

  /*! \brief Get the port's config object
   * 
   * Usefull to alter internal port configuration
   *
   * \pre Port must be set with setPort() before use of this method.
   */
  virtual mdtPortConfig &config();

  /*! \brief Open the port
   *
   * Will try to open port defined with setPortName().
   *
   * \return True on success, false else.
   * \pre Port must be set with setPort() before use of this method.
   * \todo Should return mdtAbstractPort::error_t
   */
  bool openPort();

  /*! \brief Close the port
   * 
   * This stops the threads (if exists) and close the port.
   *
   * If port was never set (with setPort() ), this method does nothing.
   */
  void closePort();

  /*! \brief Enable transactions support
   *
   * Transaction is usefull for protocols wich supports
   *  frame identification (like transaction ID in MODBUS/TCP
   *  or bTag in USBTMC).
   *
   * If transaction support is enabled, port manager can
   *  work on both event (signal) and blocking mode "on the fly".
   *  Each time a frame comes in, the newReadenFrame(mdtPortTransaction) signal is emited.
   *  Optionnaly, it's possible to request that a frame
   *  must be enqueued for a given transaction (regarding a specific ID),
   *  witch is requierd for blocking mode (see waitOnFrame() ).
   *
   * Note that not all port manager support transactions.
   *  For example, this default implementation cannot work with transactions.
   *  By default, they are disabled, and no frame is enqueued (see setTransactionsDisabled()).
   *  See specific subclass for details.
   *
   * \pre Port manager must not running when this method is called.
   */
  void setTransactionsEnabled();

  /*! \brief Diseable transactions support
   *
   * See setTransactionsEnabled() for details about transactions.
   *
   * If transactions are disabled, it's only possible to choose
   *  once if blocking mode is supported.
   *  Each time a frame comes in, the newReadenFrame(int, QByteArray) signal is emited.
   *
   * In this mode, waitOnFrame() will allways timeout.
   *
   * \param enqueueIncommingFrames If true, each incomming frame is enqueued (usefull for blocking mode).
   *                                Note: if true, don't forget to take each frame with readenFrame() or readenFrames().
   *
   * \pre Port manager must not running when this method is called.
   */
  void setTransactionsDisabled(bool enqueueIncommingFrames);

  /*! \brief Get a new transaction
   *
   * If transactions pool is empty, a new transaction is created.
   *  Note that each transaction should be put back in
   *  pool with restoreTransaction().
   *
   * \return A empty transaction (mdtPortTransaction::clear() is called internally).
   *
   * \post Returned transaction is valid (never Null).
   */
  mdtPortTransaction *getNewTransaction();

  /*! \brief Wait until data can be written
   *
   * Wait until a frame is available in write frames pool.
   *  The wait will not breack Qt's event loop, so this method
   *  can be called from GUI Thread. (See wait() for details).
   *
   * \param timeout Timeout [ms]
   * \param granularity Sleep time between each call of event processing [ms]<br>
   *                     A little value needs more CPU and big value can freese the GUI.
   *                     Should be between 50 and 100, and must be > 0.
   *                     Note that timeout must be a multiple of granularity.
   * \return True if a frame is available before timeout, false else.
   * \pre Granularity must be > 0.
   * \pre Port must be set with setPort() before calling this method.
   */
  bool waitOnWriteReady(int timeout, int granularity = 50);

  /*! \brief Write data by copy
   *
   * Data will be passed to the mdtPort's write queue by copy.
   *  This method returns immediatly after enqueue,
   *  and don't wait until data was written.
   *
   * \param data Data to write
   * \return 0 on success or value < 0 on error. In this implementation,
   *          the only possible error is mdtAbstractPort::WriteQueueEmpty .
   *          Some subclass can return a frame ID on success,
   *          or a other error. See subclass documentation for details.
   * \pre Port must be set with setPort() before use of this method.
   *
   * Subclass notes:<br>
   *  This method can be reimplemented in subclass if needed.
   *  Typically usefull if some encoding is needed before the
   *  frame is submitted to port.
   *  A frame must be taken from port's write frames pool with mdtAbstractPort::writeFramesPool()
   *  dequeue() method (see Qt's QQueue documentation for more details on dequeue() ),
   *  then added to port's write queue with mdtAbstractPort::addFrameToWrite() .
   *  If protocol supports frame identification (like MODBUS's transaction ID or USBTMC's bTag),
   *   it should be returned here and incremented.
   */
  virtual int writeData(QByteArray data);

  /*! \brief Wait until a complete frame is available
   *
   * This method will return when a complete frame was readen.
   *  This is usefull for query/answer protocols.
   *
   * Internally, a couple of sleep and process event are called, so 
   * Qt's event loop will not be broken.
   *
   * \param timeout Maximum wait time [ms]. Must be a multiple of 50 [ms]
   * \return True if Ok, false on timeout
   * \sa newReadenFrame()
   */
  bool waitReadenFrame(int timeout = 500);

  /*! \brief Wait on readen frame with defined ID
   *
   * Will return when frame with given ID was read or after timeout.
   *
   * Internally, a couple of sleep and process event are called, so 
   * Qt's event loop will not be broken.
   *
   * \param id Frame ID. Depending on protocol, this can be a transaction ID or what else.
   * \param timeout Maximum wait time [ms]. Must be a multiple of granularity [ms]
   * \param granularity Sleep time between each call of event processing [ms]<br>
   *                     A little value needs more CPU and big value can freese the GUI.
   *                     Should be between 50 and 100, and must be > 0.
   *                     Note that timeout must be a multiple of granularity.
   * \return True if Ok, false on timeout. If id was not found in transactions lists,
   *           a warning will be generated in mdtError system, and false will be returned.
   */
  bool waitOnFrame(int id, int timeout = 500, int granularity = 50);

  /*! \brief Get data by frame ID
   *
   * The frame ID is a protocol specific identification.
   *  F.ex. in MODBUS/TCP, the transaction ID is used,
   *  or bTag for USBTMC.
   *
   * If found, the frame is removed from received queue.
   *  (A second call with same ID will return a empty QByteArray).
   *
   * If ID was not found, a empty QByteArray is returned.
   *
   * Note: if transactions support is diseabled, a empty QByteArray is allways returned.
   */
  QByteArray readenFrame(int id);

  /*! \brief Get all readen data
   *
   * Note that calling this methode will clear the internal queue.
   *  (A second call will return a empty list).
   */
  QList<QByteArray> readenFrames();

  /*! \brief Wait some time without break the GUI's event loop
   *
   * This is a helper method that provide a blocking wait.
   *  Internally, a couple of sleep and event processing
   *  is done, avoiding freesing the GUI.
   *
   * This wait method is not precise.
   *
   * \param msecs Time to wait [ms]
   * \param granularity Sleep time between each call of event processing [ms]<br>
   *                     A little value needs more CPU and big value can freese the GUI.
   *                     Should be between 50 and 100, and must be > 0.
   *                     Note that msecs must be a multiple of granularity.
   * \pre granularity must be > 0.
   */
  static void wait(int msecs, int granularity = 50);

 public slots:

  /*! \brief Cancel read and write operations
   *
   * Default implementation calls mdtAbstractPort::flush().
   */
  virtual void abort();

  /*! \brief Try to 
   * 
   */
  

  /*! \brief Called by the read thread whenn a complete frame was readen
   *
   * \sa mdtPortThread
   */
  virtual void fromThreadNewFrameReaden();

  /*! \brief Manage errors comming from port threads
   */
  void onThreadsErrorOccured(int error);

 signals:

  /*! \brief Emitted when new frame was readen
   *
   * This variant is sent when transactions support is OFF.
   *  (See setTransactionsDisabled() for details).
   */
  void newReadenFrame(QByteArray data);

  /*! \brief Emitted when new frame was readen
   *
   * This variant is sent when transactions support is ON.
   *  (See setTransactionsDisabled() for details).
   */
  void newReadenFrame(mdtPortTransaction transaction);

  /*! \brief Emitted when error number has changed
   */
  void errorStateChanged(int error);

 protected:

  /*! \brief Restore a transaction into pool
   *
   * \pre transaction must be a valid pointer (not Null)
   */
  void restoreTransaction(mdtPortTransaction *transaction);

  /*! \brief Add a transaction to pending queue
   *
   * Add a existing transaction to the pending queue.
   *  A transaction can be requested with getNewTransaction().
   *
   * If id and queryReplyMode are sufficient, see addTransaction(id, bool).
   *
   * \pre transaction must be a valid pointer.
   */
  void addTransaction(mdtPortTransaction *transaction);

  /*! \brief Add a transaction to pending queue
   *
   * Will get a new transaction and add it to pending transactions queue.
   *
   * \param id Frame ID (f.ex. transaction ID in MODBUS/TCP, bTag in USBTMC)
   * \param queryReplyMode If true, transaction will be keeped in transactions done
   *                        queue until readenFrame() or readenFrames() is called.
   *                        If false, transaction will be removed from transactions done
   *                        queue just after \todo sigName is emited.
   */
  void addTransaction(int id, bool queryReplyMode);

  /*! \brief Get pending transaction matching id
   *
   * \return A valid transaction if exists, else a Null pointer.
   */
  mdtPortTransaction *pendingTransaction(int id);

  /*! \brief Remove a transaction from DONE queue and restore it to pool
   *
   * If transaction not exists in DONE queue, this method simply makes nothing.
   */
  void removeTransactionDone(int id);

  /*! \brief Add a transaction to the DONE queue
   *
   * \pre transaction must be a valid pointer.
   */
  void enqueueTransactionDone(mdtPortTransaction *transaction);

  /*! \brief Add a transaction to the RX queue
   *
   * \pre transaction must be a valid pointer.
   */
  void enqueueTransactionRx(mdtPortTransaction *transaction);

  /*! \brief Emit signals for each done transactions
   *
   * The emited signals depend on setup ....
   *
   * Internally, transactions are restored to pool.
   *
   * \note Clarify + implement
   */
  void commitFrames();

  mdtAbstractPort *pvPort;
  QList<mdtPortThread*> pvThreads;

 private:

  mdtPortInfo pvPortInfo;
  QQueue<mdtPortTransaction*> pvTransactionsPool;
  QMap<int, mdtPortTransaction*> pvTransactionsPending; // Used for query that are sent to device
  QMap<int, mdtPortTransaction*> pvTransactionsRx;      // Used when transaction's response was received
  QMap<int, mdtPortTransaction*> pvTransactionsDone;    // Used for query/reply mode transactions
  QQueue<QByteArray> pvReadenFrames;                    // Used if transactions are OFF

  bool pvEnqueueAllReadenFrames;  // See setTransactionsDisabled()
  bool pvTransactionsEnabled;

  // Diseable copy
  Q_DISABLE_COPY(mdtPortManager);
};
#endif  // #ifndef MDT_PORT_MANAGER_H
