// btlsos_tcpcbacceptor.h                                             -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#ifndef INCLUDED_BTLSOS_TCPCBACCEPTOR
#define INCLUDED_BTLSOS_TCPCBACCEPTOR

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a non-blocking acceptor of TCP connections.
//
//@CLASSES:
//  btlsos::TcpCbAcceptor: non-blocking channel allocator
//
//@SEE_ALSO: btlsos_tcptimedcbchannel btlso_tcptimereventmanager
//           btlsos_tcpcbconnector btlsos_tcpcbchannel
//           btlso_socketoptutil btesos_streamsocketfactory
//
//@DESCRIPTION: This component provides a non-blocking single-port acceptor of
// TCP connections, 'btlsos::TcpCbAcceptor', with the capability that adheres
// to the 'btlsc::CbChannelAllocator' protocol.  Both timed and non-timed
// (callback) channels can be allocated in a non-timed fashion as indicated by
// the following table:
//
// Allocations follow the asynchronous (callback-based) nature of the 'btesc'
// protocol; the callbacks are invoked when an allocation succeeds or an error
// occurs.  The acceptor uses the user-installed socket event manager in order
// to monitor (in a non-blocking fashion) the listening socket for incoming
// connection requests.
//
// The acceptor has the flexibility of opening and closing a listening socket
// with no effect on any existing channels managed by this object.  The
// enqueued (i.e., not-yet-completed) allocation requests will, however, fail
// (without invalidating the allocator) and the associated callbacks will be
// invoked when the listening port is closed.
//
///Thread Safety
///-------------
// The acceptor is *thread* *safe*, meaning that any operation can be called on
// *distinct instances* from different threads without any side-effects (which,
// generally speaking, means that there is no 'static' data), but not *thread*
// *enabled* (i.e., two threads cannot safely call methods on the *same*
// *instance* without external synchronization).  This acceptor is not
// *async-safe*, meaning that one or more functions cannot be invoked safely
// from a signal handler.  Note that the thread safety of the acceptor is
// subject to the thread safety of the supplied socket event manager.  If
// distinct instances of acceptor use the *same* instance of a socket event
// manager, both acceptors are thread-safe if and only if that socket event
// manager is *THREAD* *ENABLED*.
//
///Usage
///-----
// The following usage example shows a possible implementation of a multi-user
// echo server.  An echo server accepts connections and, for every connection,
// sends any received data back to the client (until the connection is
// terminated).  This server requires that data is read from an accepted
// connection within a certain time interval or else the connection is dropped
// on timeout.  The echo server is implemented as a separate class
// ('my_EchoServer') that owns the timed callback acceptor.  Various
// configuration parameters such as the timeout values, the queue size and
// input buffer size, and the default port number (as mandated by RFC 862) are
// constants within this class.
//..
//  class my_EchoServer {
//      // This class implements a simple multi-user echo server as specified
//      // by the RFC 862.
//
//      enum {
//          k_READ_SIZE           =  200,  // The number of bytes to be read
//                                         // can be changed, but a larger
//                                         // 'k_READ_SIZE' will require the
//                                         // client to input more data to be
//                                         // echoed.
//          k_DEFAULT_PORT_NUMBER = 9234,  // As specified by the RFC 862
//          k_QUEUE_SIZE          =   16
//      };
//
//      btlsos::TcpCbAcceptor  d_allocator;
//      bsls::TimeInterval     d_acceptTimeout;
//      bsls::TimeInterval     d_readTimeout;
//      bsls::TimeInterval     d_writeTimeout;
//
//      bsl::function<void(btlsc::TimedCbChannel*, int)>
//                             d_allocateFunctor;
//                                         // Cached callback functor.
//
//      bslma::Allocator      *d_allocator_p;
//      char                   d_buffer[k_READ_SIZE];
//
//    private:
//      // Callbacks
//      void allocateCb(btlsc::TimedCbChannel *channel, int status);
//          // Invoked from the socket event manager when a connection is
//          // allocated (i.e., accepted) or an error occurs when allocating.
//          // [...]
//
//      void bufferedReadCb(const char            *buffer,
//                          int                    status,
//                          int                    asyncStatus,
//                          btlsc::TimedCbChannel *channel);
//          // Invoked from the socket event manager when data is read.  [...]
//
//      void readCb(int                    status,
//                  int                    asyncStatus,
//                  btlsc::TimedCbChannel *channel);
//          // Invoked from the socket event manager when data is read.  [...]
//
//      void writeCb(int                    status,
//                   int                    asyncStatus,
//                   btlsc::TimedCbChannel *channel,
//                   int                    numBytes);
//          // Invoked from the socket event manager when data is written.
//          // [...]
//
//    private:
//      // Not implemented:
//      my_EchoServer(const my_EchoServer&);
//      my_EchoServer& operator=(const my_EchoServer&);
//
//    public:
//      // CREATORS
//      my_EchoServer(
//             btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
//             btlso::TimerEventManager                       *manager,
//             bslma::Allocator                               *basicAllocator);
//          // Create an echo server that uses the specified stream socket
//          // 'factory' for the system sockets and the specified socket event
//          // 'manager' to multiplex the events on these sockets.  The
//          // behavior is undefined if either 'factory' or 'manager' is 0.
//
//      ~my_EchoServer();
//          // Destroy this server.  The behavior is undefined unless the
//          // server is shut down properly (i.e., via 'close').
//
//      // MANIPULATORS
//      int open(int portNumber = k_DEFAULT_PORT_NUMBER);
//          // Establish a listening socket on the optionally specified
//          // 'portNumber'; return 0 on success, and a non-zero value
//          // otherwise.  The behavior is undefined unless '0 <= portNumber'
//          // and the listening port is not currently open.
//
//      int close();
//          // Close the listening socket; return 0 on success and a non-zero
//          // value otherwise.  The behavior is undefined unless the listening
//          // socket is currently open.
//  };
//..
// The implementation of the public methods of 'my_EchoServer' is trivial.  For
// the constructor, the socket factory and socket event manager are passed to
// the acceptor, and the allocate callback and (three) event timeouts are
// initialized.  The 'open' and 'close' methods just invoke appropriate methods
// of the acceptor.  The destructor ensures that the state of the acceptor is
// valid:
//..
//  // CREATORS
//  my_EchoServer::my_EchoServer(
//              btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
//              btlso::TimerEventManager                       *manager,
//              bslma::Allocator                               *basicAllocator)
//  : d_allocator(factory, manager, basicAllocator)
//  , d_acceptTimeout(120, 0)
//  , d_readTimeout(5, 0)
//  , d_writeTimeout(5.0)
//  , d_allocator_p(basicAllocator)
//  {
//      assert(factory);
//      assert(manager);
//      d_allocateFunctor = bsl::function<void(btlsc::TimedCbChannel *, int)>(
//          bsl::allocator_arg_t(),
//          bsl::allocator<bsl::function<void(btlsc::TimedCbChannel *, int)> >(
//              basicAllocator),
//          bdlf::MemFnUtil::memFn(&my_EchoServer::allocateCb, this));
//  }
//
//  my_EchoServer::~my_EchoServer() {
//  }
//
//  // MANIPULATORS
//  int my_EchoServer::open(int portNumber) {
//      btlso::IPv4Address serverAddress;
//      serverAddress.setPortNumber(portNumber);
//      int s = d_allocator.open(serverAddress, k_QUEUE_SIZE);
//      if (s) {
//          cout << "Failed to open listening port." << endl;
//          return s;                                                 // RETURN
//      }
//      cout << "server's socket: " << d_allocator.address() << endl;
//      assert(0 == d_allocator.isInvalid());
//      s = d_allocator.allocateTimed(d_allocateFunctor);
//      if (s) {
//          cout << "Can't enqueue an allocation request." << endl;
//          assert(d_allocator.isInvalid());
//          d_allocator.close();
//          return s;                                                 // RETURN
//      }
//      return 0;
//  }
//
//  int my_EchoServer::close() {
//      return d_allocator.close();
//  }
//..
// All the work of accepting connections and reading/writing the data is done
// in the (private) callback methods of 'my_EchoServer'.  When the connection
// is established and the 'allocateCb' method is invoked, the buffered read
// with timeout is initiated on the channel and another allocation request is
// enqueued.  If any error occurs while allocating, the acceptor is shut down.
// If any error, including timeout, occurs while reading or writing data, the
// channel is shut down.  Note that the allocation functor is cached to improve
// performance:
//..
//  void my_EchoServer::allocateCb(btlsc::TimedCbChannel *channel,
//                                 int                    status) {
//      if (channel) {
//          // Accepted a connection.  Issue a read raw request.
//          bsl::function<void(int, int)> callback(bdlf::BindUtil::bindA(
//                        d_allocator_p,
//                        bdlf::MemFnUtil::memFn(&my_EchoServer::readCb, this),
//                        _1,
//                        _2,
//                        channel));
//
//          if (channel->timedReadRaw(d_buffer,
//                                    k_READ_SIZE,
//                                    bdlt::CurrentTime::now()
//                                                + d_readTimeout, callback)) {
//              cout << "Failed to enqueue buffered read request." << endl;
//              d_allocator.deallocate(channel);
//          }
//          if (d_allocator.allocateTimed(d_allocateFunctor)) {
//              d_allocator.close();
//          }
//          return;                                                   // RETURN
//      }
//      assert(0 >= status);    // Interrupts are not enabled.
//      if (0 == status) {
//          // Re-register the functor
//          if (d_allocator.allocateTimed(d_allocateFunctor)) {
//              d_allocator.close();
//          }
//      }
//      else if (-1 == status) {
//          cout << "Allocation request was dequeued." << endl;
//      }
//      else {
//          // Hard-error accepting a connection, invalidate the allocator.
//          cout << "Hard error while accepting a connection: " << status
//               << endl;
//          d_allocator.invalidate();
//      }
//  }
//
//  void my_EchoServer::bufferedReadCb(const char            *buffer,
//                                     int                    status,
//                                     int                    asyncStatus,
//                                     btlsc::TimedCbChannel *channel)
//  {
//      cout << "my_EchoServer::bufferedReadCb: "
//           << " read " << status << " bytes." << endl;
//      assert(channel);
//      if (0 < status) {
//          bsl::function<void(int, int)> callback(bdlf::BindUtil::bindA(
//                       d_allocator_p,
//                       bdlf::MemFnUtil::memFn(&my_EchoServer::writeCb, this),
//                       _1,
//                       _2,
//                       channel,
//                       status));
//
//          if (channel->timedBufferedWrite(buffer,
//                                          status,
//                                          bdlt::CurrentTime::now()
//                                               + d_writeTimeout, callback)) {
//              cout << "Failed to enqueue write request" << endl;
//              d_allocator.deallocate(channel);
//              return;                                               // RETURN
//          }
//          // Re-register read request
//          bsl::function<void(const char *, int, int)> readCallback(
//              bdlf::BindUtil::bindA(
//                       d_allocator_p,
//                       bdlf::MemFnUtil::memFn(&my_EchoServer::bufferedReadCb,
//                                              this),
//                       _1,
//                       _2,
//                       _3,
//                       channel));
//
//          if (channel->timedBufferedRead(k_READ_SIZE,
//                   bdlt::CurrentTime::now() + d_readTimeout, readCallback)) {
//              cout << "Failed to enqueue read request." << endl;
//              d_allocator.deallocate(channel);
//          }
//      }
//      else if (0 == status) {
//          if (0 > asyncStatus) {
//              cout << "Callback dequeued" << endl;
//          }
//          else {
//              cout << "Timed out on read" << endl;
//              d_allocator.deallocate(channel);
//          }
//      }
//      else {
//          // Either time out or an error on the channel
//          cout << "Failed to read data." << endl;
//          d_allocator.deallocate(channel);
//      }
//  }
//
//  void my_EchoServer::readCb(int                    status,
//                             int                    asyncStatus,
//                             btlsc::TimedCbChannel *channel)
//  {
//      cout << "my_EchoServer::readCb: "
//           << " read " << status << " bytes." << endl;
//      assert(channel);
//      if (0 < status) {
//          bsl::function<void(int, int)> callback(
//              bdlf::BindUtil::bindA(
//                       d_allocator_p,
//                       bdlf::MemFnUtil::memFn(&my_EchoServer::writeCb, this),
//                       _1,
//                       _2,
//                       channel,
//                       status));
//
//          if (channel->timedBufferedWrite(d_buffer,
//                                          status,
//                                          bdlt::CurrentTime::now()
//                                                 + d_writeTimeout, callback))
//              {
//                  cout << "Failed to enqueue write request" << endl;
//                  d_allocator.deallocate(channel);
//                  return;                                           // RETURN
//              }
//          // Re-register read request
//          bsl::function<void(int, int)> readCallback(
//                bdlf::BindUtil::bindA(
//                        d_allocator_p,
//                        bdlf::MemFnUtil::memFn(&my_EchoServer::readCb, this),
//                        _1,
//                        _2,
//                        channel));
//
//          if (channel->timedReadRaw(d_buffer, k_READ_SIZE,
//                                    bdlt::CurrentTime::now()
//                                            + d_readTimeout, readCallback)) {
//              cout << "Failed to enqueue read request." << endl;
//              d_allocator.deallocate(channel);
//          }
//      }
//      else if (0 == status) {
//          if (0 > asyncStatus) {
//              cout << "Callback dequeued" << endl;
//          }
//          else {
//              cout << "Timed out on read" << endl;
//              d_allocator.deallocate(channel);
//          }
//      }
//      else {
//          // Either time out or an error on the channel
//          cout << "Failed to read data." << endl;
//          d_allocator.deallocate(channel);
//      }
//  }
//
//  void my_EchoServer::writeCb(int                    status,
//                              int                    asyncStatus,
//                              btlsc::TimedCbChannel *channel,
//                              int                    numBytes)
//  {
//      (void)asyncStatus;
//      if (status != numBytes) {
//          cout << "Failed to send data." << endl;
//          channel->invalidate();
//          d_allocator.deallocate(channel);
//      }
//  }
//..
// Finally, we define the 'main' function, which will create the concrete
// socket factory and concrete socket event manager, create 'my_EchoServer' as
// required, and go into "infinite" loop dispatching registered callbacks:
//..
//  bslma::TestAllocator testAllocator;
//  testAllocator.setNoAbort(1);
//
//  btlso::InetStreamSocketFactory<btlso::IPv4Address> factory(&testAllocator);
//
//  btlso::TcpTimerEventManager::Hint hint
//                                = btlso::TcpTimerEventManager::e_NO_HINT;
//
//  btlso::TcpTimerEventManager manager(hint, &testAllocator);
//  my_EchoServer               echoServer(&factory, &manager, &testAllocator);
//
//  if (echoServer.open()) {
//      cout << "Can't open listening socket." << endl;
//      break; // return -1;
//  }
//
//  for (int i = 0; i < 100; ++i) {
//      // Do it for the finite number of times.
//      int cbs = manager.dispatch(0);
//      assert(0 < cbs);
//  }
//  echoServer.close();
//  // We need another round of dispatch for cleaning things up.
//  int cbs = manager.dispatch(0);
//  assert(0 < cbs);
//  assert(0 == testAllocator.numMismatches());
//..

#ifndef INCLUDED_BTLSCM_VERSION
#include <btlscm_version.h>
#endif

#ifndef INCLUDED_BTLSC_CBCHANNELALLOCATOR
#include <btlsc_cbchannelallocator.h>
#endif

#ifndef INCLUDED_BTLSO_IPV4ADDRESS
#include <btlso_ipv4address.h>
#endif

#ifndef INCLUDED_BDLMA_POOL
#include <bdlma_pool.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSL_DEQUE
#include <bsl_deque.h>
#endif

#ifndef INCLUDED_BSL_FUNCTIONAL
#include <bsl_functional.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

namespace BloombergLP {
namespace btlso { template<class ADDRESS> class StreamSocketFactory; }
namespace btlso { template<class ADDRESS> class StreamSocket; }
namespace btlso { class TimerEventManager; }
namespace btlsos {

class TcpCbAcceptor_Reg; // component-local class declaration

                           // ===================
                           // class TcpCbAcceptor
                           // ===================

class TcpCbAcceptor : public btlsc::CbChannelAllocator {
    // This class implements a 'btesc'-style callback-based channel allocator
    // for a single server-side (i.e., listening) TCP/IPv4 socket.  The
    // allocation requests' callbacks are queued and invoked as appropriate.
    // The status supplied to the callback is negative for failure and positive
    // for a (restartable) interruption on an underlying system call.  A status
    // value of -1 indicates an allocation attempt on an uninitialized acceptor
    // (i.e., no listening socket is established), and a status value of -2
    // indicates failure due to the listening socket being closed.  The
    // acceptor provides a "delayed open" (where a listening socket is
    // established at some point after construction), and allows the listening
    // socket to be closed (and opened again) with no effect on the state of
    // any other channel currently managed by this acceptor.

    bdlma::Pool          d_callbackPool;     // memory pool for registrations
    bdlma::Pool          d_channelPool;      // memory pool for channels

    bsl::deque<TcpCbAcceptor_Reg *>
                         d_callbacks;        // registered callbacks

    bsl::vector<btlsc::CbChannel*>
                         d_channels;         // managed channels

    btlso::TimerEventManager
                        *d_manager_p;

    btlso::StreamSocketFactory<btlso::IPv4Address>
                        *d_factory_p;        // factory used to supply sockets

    btlso::StreamSocket<btlso::IPv4Address>
                        *d_serverSocket_p;   // listening socket

    btlso::IPv4Address   d_serverAddress;    // address of listening socket

    int                  d_isInvalidFlag;    // set if acceptor is invalid

    bsl::function<void()>
                         d_acceptFunctor;    // cached callbacks

    TcpCbAcceptor_Reg   *d_currentRequest_p; // address of the current request

    bslma::Allocator    *d_allocator_p;

  private:
    // Callbacks for socket event manager

    void acceptCb();
        // Invoked from socket event manager when an event occurs on the
        // listening socket.

    void deallocateCb(btlsc::CbChannel *channel);
        // Deallocate resources used by the specified 'channel'.  Invoked from
        // timer event manager.

  private:
    // Not implemented:
    TcpCbAcceptor(const TcpCbAcceptor&);
    TcpCbAcceptor& operator=(const TcpCbAcceptor&);

  public:
    // CREATORS
    TcpCbAcceptor(
           btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
           btlso::TimerEventManager                       *manager,
           bslma::Allocator                               *basicAllocator = 0);
        // Create a callback acceptor that uses the specified 'factory' (to
        // create stream sockets) and the specified 'manager' (to monitor for
        // incoming connections).  Optionally specify a 'basicAllocator' used
        // to supply memory.  If 'basicAllocator' is 0, the currently installed
        // default allocator is used.  The behavior is undefined if either
        // 'factory' or 'manager' is 0.  Note that the acceptor is created in a
        // valid state (as reported by the 'isInvalid' method) and subsequent
        // allocation requests will be enqueued successfully, but that such
        // allocations will fail with a status of -1 until the listening socket
        // is created (using 'open').

    TcpCbAcceptor(
           btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
           btlso::TimerEventManager                       *manager,
           int                                             numElements,
           bslma::Allocator                               *basicAllocator = 0);
        // Create a callback acceptor, with enough internal capacity to
        // accommodate up to the specified 'numElements' channels without
        // reallocation, that uses the specified 'factory' to create stream
        // sockets and the specified 'manager' to monitor for incoming
        // connections.  Optionally specify a 'basicAllocator' used to supply
        // memory.  If 'basicAllocator' is 0, the currently installed default
        // allocator is used.  'delete' are used.  The behavior is undefined if
        // either 'factory' or 'manager' is 0 and unless '0 < numElements'.
        // Note that the acceptor is created in a valid state (as reported by
        // the 'isInvalid' method) and that subsequent allocation requests will
        // be enqueued successfully, but the allocations will fail with a
        // status of -1 until the listening socket is created (using 'open').

    ~TcpCbAcceptor();
        // Invalidate this allocator, close the listening socket, invoke all
        // pending callbacks, destroy all managed channels, and destroy this
        // allocator.

    // MANIPULATORS
    virtual int allocate(const Callback& callback, int flags = 0);
        // Initiate a non-blocking operation to allocate a callback channel;
        // execute the specified 'callback' functor after the allocation
        // operation terminates.  If the optionally specified 'flags'
        // incorporates 'btlsc::Flag::k_ASYNC_INTERRUPT', "asynchronous events"
        // are permitted to interrupt the allocation; by default, such events
        // are ignored.  Return 0 on successful initiation, and a non-zero
        // value otherwise (in which case 'callback' will not be invoked).
        //
        // When invoked, 'callback' is passed the (possibly null) address of a
        // callback channel and an integer "status".  If that address is not 0,
        // the allocation succeeded and status has no meaning; a non-null
        // channel address will remain valid until deallocated explicitly (see
        // 'deallocate').  If the address is 0, a positive status indicates an
        // interruption due to an asynchronous event; subsequent allocation
        // attempts may succeed.  A status of -1 implies that the allocation
        // operation was "canceled" (synchronously) by the caller (see
        // 'cancelAll') and, often, may be retried successfully.  A status less
        // than -1 indicates a more persistent error, but not necessarily a
        // permanent one; the allocator itself may still be valid (see
        // 'isInvalid').  The behavior is undefined unless 'callback' is valid.

    int allocateTimed(const TimedCallback& timedCallback, int flags = 0);
        // Initiate a non-blocking operation to allocate a timed callback
        // channel; execute the specified 'timedCallback' functor after the
        // allocation operation terminates.  If the optionally specified
        // 'flags' incorporates 'btlsc::Flag::k_ASYNC_INTERRUPT', "asynchronous
        // events" are permitted to interrupt the allocation; by default, such
        // events are ignored.  Return 0 on successful initiation, and a
        // non-zero value otherwise (in which case 'timedCallback' will not be
        // invoked).
        //
        // When invoked, 'timedCallback' is passed the (possibly null) address
        // of a timed callback channel and an integer "status".  If that
        // address is not 0, the allocation succeeded and status has no
        // meaning; a non-null channel address will remain valid until
        // deallocated explicitly (see 'deallocate').  If the address is 0, a
        // positive status indicates an interruption due to an asynchronous
        // event; subsequent allocation attempts may succeed.  A status of -1
        // implies that the allocation operation was "canceled" (synchronously)
        // by the caller (see 'cancelAll') and, often, may be retried
        // successfully.  A status less than -1 indicates a more persistent
        // error, but not necessarily a permanent one; the allocator itself may
        // still be valid (see 'isInvalid').  The behavior is undefined unless
        // 'callback' is valid.

    virtual void cancelAll();
        // Immediately cancel all pending operations on this allocator,
        // invoking each registered allocation callback in turn.  Each callback
        // will be invoked with a null channel and a status of -1.  This method
        // may be invoked successfully on an invalid allocator; however,
        // calling the method does not invalidate the allocator.  Note that
        // calling 'cancelAll' from a callback that has itself been canceled
        // simply extends the set of canceled operations to include any new
        // ones initiated since the previous 'cancelAll' was invoked.

    int close();
        // Close the listening socket and invoke all enqueued callbacks with a
        // status of -2.  Return 0 on success, and a non-zero value otherwise.
        // A listening socket yields a non-zero server address (see 'address').
        // The behavior is undefined unless the listening socket is
        // successfully established.  Note that closing a listening port has no
        // effect on any other channel managed by this allocator.

    virtual void deallocate(btlsc::CbChannel *channel);
        // Terminate all operations on the specified 'channel', invoke each
        // pending callback with the appropriate status, and reclaim all
        // afforded channel services.  The behavior is undefined unless
        // 'channel' is currently allocated from this allocator, (i.e., was
        // previously obtained from this instance and has not subsequently been
        // deallocated).  Note that this method can never block.

    virtual void invalidate();
        // Place this allocator in a permanently invalid state.  No subsequent
        // allocations will succeed.  Note that invalidating this allocator has
        // no effect on the state of any channel managed by it nor on the
        // listening socket, which, if established, must be closed explicitly
        // (using the 'close' method).

    int open(const btlso::IPv4Address& endpoint,
             int                       queueSize,
             int                       reuseAddress = 1);
        // Establish a listening socket having the specified 'queueSize'
        // maximum number of pending connections on the specified 'endpoint'.
        // Optionally specify a 'reuseAddress' value to be used for setting
        // 'REUSEADDRESS' option.  If 'reuseAddress' is not specified, 1 (i.e.,
        // if the REUSEADDRESS option is enabled) is used.  Return 0 on
        // success, and a non-zero value otherwise.  The behavior is undefined
        // unless '0 < queueSize' and the listening socket is closed.

    int setOption(int level, int option, int value);
        // Set the specified socket 'option' having the specified 'level' on
        // the listening socket to the specified 'value'.  Return 0 on success
        // and a non-zero value otherwise.  (The list of commonly supported
        // options is available in 'btlso_socketoptutil'.)  The behavior is
        // undefined unless the listening socket is established.  Note that all
        // sockets allocated from this acceptor will inherit the options'
        // values set on the listening socket.

    // ACCESSORS
    const btlso::IPv4Address& address() const;
        // Return the (fully qualified) address of the listening socket, or an
        // invalid address '(ANY_ADDRESS, ANY_PORT)' if the server is not
        // established (see 'btlso_ipv4address').

    int getOption(int *result, int level, int option) const;
        // Load into the specified 'result' the current value of the specified
        // 'option' of the specified 'level' set on the listening socket.
        // Return 0 on success and a non-zero value otherwise.  The list of
        // commonly supported options (and levels) is enumerated in
        // 'btlso_socketoptutil'.  The behavior is undefined if 'result' is 0
        // and unless the listening socket has been established.

    virtual int isInvalid() const;
        // Return 1 if this channel allocator is *permanently* invalid (i.e.,
        // no subsequent allocation requests will succeed).

    int numChannels() const;
        // Return the number of channels currently allocated from this
        // acceptor.
};

// ----------------------------------------------------------------------------
//                             INLINE DEFINITIONS
// ----------------------------------------------------------------------------

inline
const btlso::IPv4Address& TcpCbAcceptor::address() const
{
    return d_serverAddress;
}

inline
int TcpCbAcceptor::numChannels() const
{
    return static_cast<int>(d_channels.size());
}
}  // close package namespace

}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
