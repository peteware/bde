// btlsos_tcptimedchannel.h                                           -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#ifndef INCLUDED_BTLSOS_TCPTIMEDCHANNEL
#define INCLUDED_BTLSOS_TCPTIMEDCHANNEL

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide synchronous TCP-based communication channel with timeouts.
//
//@SEE_ALSO: btlsos_tcptimedacceptor,  btlsos_tcptimedconnector
//           btlsos_tcpchannel,        btlsos_tcptimedcbchannel
//           btlso_socketoptutil
//
//@DESCRIPTION: This component provides concrete implementation of the blocking
// communication channel with timeouts ('btlsc_timedchannel') over TCP/IPv4
// sockets.  Both timed and non-timed operations are supported (as mandated by
// the protocol).  Additionally, operations to set various socket options and
// to get local and remote addresses are provided.
//
///Thread Safety
///-------------
// The channel is *thread safe*, meaning that any operation can be called on
// *distinct instances* from different threads without any side-effects (which,
// generally speaking, means that there is no 'static'), but not *thread
// enabled* (i.e., two threads cannot safely call methods on the *same
// instance* without external synchronization).  The channel is not
// *async-safe*, meaning that one or more functions cannot be invoked safely
// from a signal handler.
//
///Performance
///-----------
// This channel is optimized for operations with the timeout.  Non-timed
// operations will have worse performance than their respective counterparts in
// the non-timed version of the channel (i.e., 'btlsos_tcpchannel').  If only
// non-timed operations are required, 'btlsos::TcpChannel' should be used
// instead.
//
// The following usage example shows a possible use of this component.  First,
// a pair of sockets connecting each other on the local host are created for
// our example, which could be any connected sockets on different hosts.  The
// channel only needs one of the socket as its I/O request endpoint, while the
// other end of connection will be used to write some data for the channel to
// read:
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Using a Socket Pair
/// - - - - - - - - - - - - - - -
//..
//    btlso::SocketHandle::Handle handles[2];
//    int ret = btlso::SocketImpUtil::socketPair<btlso::IPv4Address>(
//                                  handles,
//                                  btlso::SocketImpUtil::k_SOCKET_STREAM);
//    assert(0 == ret);
//    // The following socket options are set only if necessary.
//
//    ret = btlso::SocketOptUtil::setOption(handles[0],
//                      btlso::SocketOptUtil::k_SOCKETLEVEL,
//                      btlso::SocketOptUtil::k_SENDBUFFER, 8192);
//    assert(0 == ret);
//
//    ret = btlso::SocketOptUtil::setOption(handles[1],
//                      btlso::SocketOptUtil::k_SOCKETLEVEL,
//                      btlso::SocketOptUtil::k_RECEIVEBUFFER, 8192);
//    assert(0 == ret);
//
//    ret = btlso::SocketOptUtil::setOption(handles[1],
//                          btlso::SocketOptUtil::k_TCPLEVEL,
//                          btlso::SocketOptUtil::k_TCPNODELAY, 1);
//    assert(0 == ret);
//..
// Next, create a 'btlso::StreamSocket' object, which is a part of the channel.
// The 'btlso::StreamSocket' object has a field of type
// 'btlso::SocketHandle::Handle', whose value is set to a socket created above.
// Then I/O operations can be invoked on the channel:
//..
//    btlso::InetStreamSocketFactory<btlso::IPv4Address>
//                                                     factory(&testAllocator);
//    btlso::StreamSocket<btlso::IPv4Address> *sSocket =
//                                            factory.allocate(handles[0]);
//    assert(sSocket);
//
//    {
//        // We should guarantee that the 'channel's destructor is
//        // invoked before the corresponding 'streamSocket'
//        // destructor, the behavior is undefined otherwise.
//        // We insure the required order by creating the 'channel'
//        // inside a block while the corresponding 'streamSocket'
//        // object outside the block as above.
//
//        Obj channel(sSocket);
//        assert(0 == channel.isInvalid());
//
//        // Write data at the other side of the channel and so "read"
//        // operations can be done at the channel side.
//        enum { k_LEN = 30 };
//        char writeBuf[k_LEN] = "abcdefghij1234567890",
//             readBuf[k_LEN];
//        int numBytes = 0, augStatus = -1, interruptFlag = 1;
//        int len = btlso::SocketImpUtil::write(handles[1], writeBuf,
//                                                     strlen(writeBuf));
//
//        assert(len == strlen(writeBuf));
//        // Read 5 bytes from the channel.
//        numBytes = 5;
//        augStatus = -1;
//        len = channel.read(&augStatus, readBuf, numBytes, interruptFlag);
//        if (len != numBytes) {
//            assert(0 < augStatus);
//        }
//
//        // We need to set a timeout value which is relative to the
//        // current system time.
//        bsls::TimeInterval timer = bdlt::CurrentTime::now();
//        int milliSec = 100, nanoSec = 400;
//        timer.addMilliseconds(milliSec);
//        timer.addNanoseconds(nanoSec);
//        numBytes = 10;
//        augStatus = -1;
//        len = channel.timedRead(&augStatus, readBuf,
//                                        numBytes, timer, interruptFlag);
//        if (len != numBytes) {
//            assert(0 <= augStatus);
//        }
//
//        timer.addMilliseconds(milliSec);
//        timer.addNanoseconds(nanoSec);
//        numBytes = 20;
//        augStatus = -1;
//        // Try reading 20 bytes from the channel with a timeout value.
//        // The timeout will be reached since no enough data in the
//        // channel.
//        len = channel.timedRead(&augStatus, (char*) readBuf,
//                                        numBytes, timer, interruptFlag);
//        if (len != numBytes) {
//            assert(0 == augStatus);
//        }
//        // Write 1 byte to the channel.
//        numBytes = 1;
//        augStatus = -1;
//        len = channel.write(&augStatus, writeBuf,
//                                    numBytes, interruptFlag);
//        if (len != numBytes) {
//            assert(0 < augStatus);
//        }
//
//        timer.addMilliseconds(milliSec);
//        timer.addNanoseconds(nanoSec);
//        numBytes = 10;
//        augStatus = -1;
//        // Try writing 10 bytes to the channel with a timeout value.
//        len = channel.timedWrite(&augStatus, writeBuf,
//                                         numBytes, timer, interruptFlag);
//        if (len != numBytes) {
//            assert(0 <= augStatus);
//        }
//        assert(0 == channel.isInvalid());
//        channel.invalidate();
//        assert(1 == channel.isInvalid());
//
//        numBytes = 5;
//        enum { e_INVALID = -2 };
//        // Try writing 5 bytes from the channel.
//        len = channel.read(&augStatus, readBuf,
//                                   numBytes, interruptFlag);
//        assert(e_INVALID == len);
//
//        timer.addMilliseconds(milliSec);
//        timer.addNanoseconds(nanoSec);
//        numBytes = 10;
//        augStatus = -1;
//        // Try writing 10 bytes from the channel with a timeout value.
//        len = channel.timedRead(&augStatus, readBuf,
//                                        numBytes, timer, interruptFlag);
//        assert(e_INVALID == len);
//        // Try writing 1 byte to the channel.
//        numBytes = 1;
//        augStatus = -1;
//        len = channel.write(&augStatus, writeBuf,
//                                    numBytes, interruptFlag);
//        assert(e_INVALID == len);
//
//        timer.addMilliseconds(milliSec);
//        timer.addNanoseconds(nanoSec);
//        numBytes = 10;
//        augStatus = -1;
//        // Try writing 10 bytes to the channel with a timeout value.
//        len = channel.timedWrite(&augStatus, writeBuf,
//                                          numBytes, timer, interruptFlag);
//        assert(e_INVALID == len);
//    }
//    factory.deallocate(sSocket);
//..

#ifndef INCLUDED_BTLSCM_VERSION
#include <btlscm_version.h>
#endif

#ifndef INCLUDED_BTLSC_TIMEDCHANNEL
#include <btlsc_timedchannel.h>
#endif

#ifndef INCLUDED_BTLSO_IPV4ADDRESS
#include <btlso_ipv4address.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

namespace BloombergLP {
namespace btlso { template<class ADDRESS> class StreamSocket; }
namespace btlsos {

                          // =====================
                          // class TcpTimedChannel
                          // =====================

class TcpTimedChannel : public btlsc::TimedChannel {
    // This class implements 'btlsc::TimedChannel' protocol over TCP/IP
    // sockets.  It operates on top of the stream-socket interface, which is
    // provided at construction.

    btlso::StreamSocket<btlso::IPv4Address> *d_socket_p;       // not owned
    int                                    d_isInvalidFlag;

    bsl::vector<char>                      d_readBuffer;
    int                                    d_readBufferOffset;
    int                                    d_readBufferedStartPointer;
        // the index of the first unconsumed data in 'd_readBuffer'

    bsl::vector<btls::Iovec>               d_readBuffers;  // working array
    bsl::vector<btls::Ovec>                d_writeBuffers; // working array
    bsl::vector<btls::Ovec>                d_ovecBuffers;  // initial working
                                                           // array for write

  private: // not implemented
    TcpTimedChannel(const TcpTimedChannel&);
    TcpTimedChannel& operator=(const TcpTimedChannel&);

    // PRIVATE MANIPULATORS
    void initializeReadBuffer(int size = -1);
        // Initialize internal read buffer with the optionally specified
        // 'size'.  If 'size' is not specified, the default that is obtained by
        // querying the underlying socket is used.

    template <typename VECTYPE>
    int setupWritev(const VECTYPE *buffers, int numBuffers);
        // Populate 'd_ovecBuffers' with the specified 'buffers', of the
        // specified 'numBuffers' size; return the total length of 'buffers'.

    int writev(int *augStatus, int length, int flags);
        // Write to this channel from 'd_ovecBuffers', collectively of the
        // specified 'length'.  If the specified 'flags' incorporates
        // 'btlsc::Channel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Return 'length' on success, a negative value on error,
        // and the number of bytes newly written from 'buffers' (indicating a
        // partial result) otherwise.  On partial result, load the specified
        // 'augStatus' with a positive value, indicating that an asynchronous
        // event caused the interruption; otherwise, 'augStatus' is unmodified.
        // The behavior is undefined unless 'd_ovecBuffers' is not empty.

    int timedWritev(int                       *augStatus,
                    int                        length,
                    int                        flags,
                    const bsls::TimeInterval&  timeout);
        // Write to this channel from 'd_ovecBuffers', collectively of the
        // specified 'length', or interrupt after the specified absolute
        // 'timeout' is reached.  If the specified 'flags' incorporates
        // 'btlsc::Channel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Return 'length' on success, a negative value on error,
        // and the number of bytes newly written from 'buffers' (indicating a
        // partial result) otherwise.  On partial result, load the specified
        // 'augStatus' with 0 if 'timeout' interrupted this operation or a
        // positive value if the interruption was due to an asynchronous
        // event; otherwise, 'augStatus' is unmodified.  The behavior is
        // undefined unless 'd_ovecBuffers' is not empty.  Note that if the
        // 'timeout' time has already passed, the "write" operation will still
        // be attempted, but the attempt will not block.

  public:
    // CREATORS
    TcpTimedChannel(
                  btlso::StreamSocket<btlso::IPv4Address> *socket,
                  bslma::Allocator                        *basicAllocator = 0);
        // Create a timed channel attached to the specified stream-oriented
        // 'socket'.  Optionally specify a 'basicAllocator' used to supply
        // memory.  If 'basicAllocator' is 0, the currently installed default
        // allocator is used.  The behavior is undefined unless 'socket' is not
        // 0.

    ~TcpTimedChannel();
        // Destroy this channel and release the underlying socket.

    // MANIPULATORS
    int read(char *buffer, int numBytes, int flags = 0);
    int read(int *augStatus, char *buffer, int numBytes, int flags = 0);
        // Read from this channel into the specified 'buffer' the specified
        // 'numBytes'.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with a positive
        // value, indicating that an asynchronous event caused the
        // interruption; otherwise, 'augStatus' is unmodified.  A partial
        // result typically does not invalidate this channel; hence, this (or
        // another) operation may be retried (with arguments suitably adjusted)
        // with some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error (leaving the contents of 'buffer'
        // undefined); -1 implies that the connection was closed by the peer
        // (but the converse is not guaranteed).  The behavior is undefined
        // unless 'buffer' has sufficient capacity to hold the requested data
        // and '0 < numBytes'.

    int timedRead(char                      *buffer,
                  int                        numBytes,
                  const bsls::TimeInterval&  timeout,
                  int                        flags = 0);
    int timedRead(int                       *augStatus,
                  char                      *buffer,
                  int                        numBytes,
                  const bsls::TimeInterval&  timeout,
                  int                        flags = 0);
        // Read from this channel into the specified 'buffer' the specified
        // 'numBytes' or interrupt after the specified absolute 'timeout' time
        // is reached.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with 0 if 'timeout'
        // interrupted this operation, or a positive value if the interruption
        // was due to an asynchronous event; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted) with some reasonable hope of success.
        // A negative "status", however, indicates a permanent error (leaving
        // the contents of 'buffer' undefined); -1 implies that the connection
        // was closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless 'buffer' has sufficient capacity to
        // hold the requested data and '0 < numBytes'.  Note that if the
        // 'timeout' value has already passed, the "read" operation will still
        // be attempted, but the attempt will not block.

    int readv(const btls::Iovec *buffers, int numBuffers, int flags = 0);
    int readv(int               *augStatus,
              const btls::Iovec *buffers,
              int                numBuffers,
              int                flags = 0);
        // Read from this channel into the specified sequence of 'buffers' of
        // specified sequence length 'numBuffers' the respective numbers of
        // bytes as specified in each corresponding 'btls::Iovec' buffer.  If
        // the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a partial result.  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffers' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with a positive
        // value, indicating that an asynchronous event caused the
        // interruption; otherwise, 'augStatus' is unmodified.  A partial
        // result typically does not invalidate this channel; hence, this (or
        // another) operation may be retried (with arguments suitably adjusted)
        // with some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error (leaving the contents of 'buffers'
        // undefined); -1 implies that the connection was closed by the peer
        // (but the converse is not guaranteed).  The behavior is undefined
        // unless 'buffers' have sufficient capacity to hold the requested data
        // and 0 < numBytes.

    int timedReadv(const btls::Iovec         *buffers,
                   int                        numBuffers,
                   const bsls::TimeInterval&  timeout,
                   int                        flags = 0);
    int timedReadv(int                       *augStatus,
                   const btls::Iovec         *buffers,
                   int                        numBuffers,
                   const bsls::TimeInterval&  timeout,
                   int                        flags = 0);
        // Read from this channel into the specified sequence of 'buffers' of
        // specified sequence length 'numBuffers' the respective numbers of
        // bytes as specified in each corresponding 'btls::Iovec' buffer, or
        // interrupt after the specified absolute 'timeout' time is reached.
        // If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffers' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with 0 if 'timeout'
        // interrupted this operation or a positive value if the interruption
        // was due to an asynchronous event; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted) with some reasonable hope of success.
        // A negative "status", however, indicates a permanent error (leaving
        // the contents of 'buffers' undefined); -1 implies that the connection
        // was closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless 'buffers' have sufficient capacity to
        // hold the requested data and 0 < numBytes.  Note that if the
        // 'timeout' value has already passed, the "read" operation will still
        // be attempted, but the attempt will not block.

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    int readRaw(char *buffer, int numBytes, int flags);
    int readRaw(int *augStatus, char *buffer, int numBytes, int flags = 0);
        // DEPRECATED.  Invoke 'readRaw(char*, int)'.

    int readRaw(char *buffer, int numBytes);
        // *Atomically* read from this channel into the specified 'buffer' *at*
        // *most* the specified 'numBytes'.  Return 'numBytes' on success, a
        // negative value on error, and the number of bytes newly read into
        // 'buffer' (indicating a partial result) otherwise.  A partial result
        // typically does not invalidate this channel; hence, this (or another)
        // operation may be retried (with arguments suitably adjusted) with
        // some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error (leaving the contents of 'buffer'
        // undefined); -1 implies that the connection was closed by the peer
        // (but the converse is not guaranteed).  The behavior is undefined
        // unless 'buffer' has sufficient capacity to hold the requested data
        // and '0 < numBytes'.

    int timedReadRaw(char                      *buffer,
                     int                        numBytes,
                     const bsls::TimeInterval&  timeout,
                     int                        flags = 0);
    int timedReadRaw(int                       *augStatus,
                     char                      *buffer,
                     int                        numBytes,
                     const bsls::TimeInterval&  timeout,
                     int                        flags = 0);
        // *Atomically* read from this channel into the specified 'buffer' *at*
        // *most* the specified 'numBytes' or interrupt after the specified
        // absolute 'timeout' time is reached.  If the optionally specified
        // 'flags' incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT',
        // "asynchronous events" are permitted to interrupt this operation; by
        // default, such events are ignored.  Optionally specify (as a
        // *leading* argument) 'augStatus' to receive status specific to a
        // "partial result".  Return 'numBytes' on success, a negative value on
        // error, and the number of bytes newly read into 'buffer' (indicating
        // a partial result) otherwise.  On a partial result, load 'augStatus',
        // if supplied, with 0 if 'timeout' interrupted this operation, a
        // positive value if an asynchronous event caused an interruption, or a
        // negative value if the atomic OS-level operation transmitted at least
        // one byte, but less than 'numBytes'; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted) with some reasonable hope of success.
        // A negative "status", however, indicates a permanent error (leaving
        // the contents of 'buffer' undefined); -1 implies that the connection
        // was closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless 'buffer' has sufficient capacity to
        // hold the requested data and '0 < numBytes'.  Note that if the
        // 'timeout' value has already passed, the "read" operation will still
        // be attempted, but the attempt will not block.

    int readvRaw(const btls::Iovec *buffers, int numBuffers, int flags = 0);
    int readvRaw(int               *augStatus,
                 const btls::Iovec *buffers,
                 int                numBuffers,
                 int                flags = 0);
        // *Atomically* read from this channel into the specified sequence of
        // 'buffers' of specified sequence length 'numBuffers' *at* *most* the
        // respective numbers of bytes as specified in each corresponding
        // 'btls::Iovec' buffer.  If the optionally specified 'flags'
        // incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous
        // events" are permitted to interrupt this operation; by default, such
        // events are ignored.  Optionally specify (as a *leading* argument)
        // 'augStatus' to receive status specific to a "partial result".
        // Return 'numBytes' on success, a negative value on error, and the
        // number of bytes newly read into 'buffers' (indicating a partial
        // result) otherwise.  On a partial result, load 'augStatus', if
        // supplied, with a positive value if an asynchronous event interrupted
        // this operation and a negative value if the atomic OS-level operation
        // transmitted at least one byte, but less than 'numBytes'; otherwise,
        // 'augStatus' is unmodified.  A partial result typically does not
        // invalidate this channel; hence, this (or another) operation may be
        // retried (with arguments suitably adjusted) with some reasonable hope
        // of success.  A negative "status", however, indicates a permanent
        // error (leaving the contents of 'buffers' undefined); -1 implies that
        // the connection was closed by the peer (but the converse is not
        // guaranteed).  The behavior is undefined unless 'buffers' have
        // sufficient capacity to hold the requested data and 0 < numBytes.
        // Note that if the specified 'timeout' value has already passed, the
        // "read" operation will still be attempted, but the attempt will not
        // block.

    int timedReadvRaw(int                       *augStatus,
                      const btls::Iovec         *buffers,
                      int                        numBuffers,
                      const bsls::TimeInterval&  timeout,
                      int                        flags = 0);
    int timedReadvRaw(const btls::Iovec         *buffers,
                      int                        numBuffers,
                      const bsls::TimeInterval&  timeout,
                      int                        flags = 0);
        // *Atomically* read from this channel into the specified sequence of
        // 'buffers' of specified sequence length 'numBuffers' *at* *most* the
        // respective numbers of bytes as specified in each corresponding
        // 'btls::Iovec' buffer, or interrupt after the specified absolute
        // 'timeout' time is reached.  If the optionally specified 'flags'
        // incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous
        // events" are permitted to interrupt this operation; by default, such
        // events are ignored.  Optionally specify (as a *leading* argument)
        // 'augStatus' to receive status specific to a "partial result".
        // Return 'numBytes' on success, a negative value on error, and the
        // number of bytes newly read into 'buffers' (indicating a partial
        // result) otherwise.  On a partial result, load 'augStatus', if
        // supplied, with 0 if 'timeout' interrupted this operation, a positive
        // value if an asynchronous event caused an interruption, or a negative
        // value if the atomic OS-level operation transmitted at least one
        // byte, but less than 'numBytes'; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted) with some reasonable hope of success.
        // A negative "status", however, indicates a permanent error (leaving
        // the contents of 'buffers' undefined); -1 implies that the connection
        // was closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless 'buffers' have sufficient capacity to
        // hold the requested data and 0 < numBytes.  Note that if the
        // 'timeout' value has already passed, the "read" operation will still
        // be attempted, but the attempt will not block.

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    int bufferedRead(const char **buffer, int numBytes, int flags = 0);
    int bufferedRead(int         *augStatus,
                     const char **buffer,
                     int          numBytes,
                     int          flags = 0);
        // Read from this channel into a channel-supplied buffer, identified
        // via the specified 'buffer', the specified 'numBytes'.  If the
        // optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  Any
        // positive return value guarantees that 'buffer' will remain valid
        // until this channel is modified.  On a partial result, load
        // 'augStatus', if supplied, with a positive value, indicating that an
        // asynchronous event caused the interruption; otherwise, 'augStatus'
        // is unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried with some
        // reasonable hope of success -- buffered data from a partial result
        // remains available until consumed by subsequent read operations.  A
        // negative "status", however, indicates a permanent error (leaving
        // 'buffer' undefined); -1 implies that the connection was closed by
        // the peer (but the converse is not guaranteed).  The behavior is
        // undefined unless '0 < numBytes'.

    int timedBufferedRead(const char                **buffer,
                          int                         numBytes,
                          const bsls::TimeInterval&   timeout,
                          int                         flags = 0);
    int timedBufferedRead(int                        *augStatus,
                          const char                **buffer,
                          int                         numBytes,
                          const bsls::TimeInterval&   timeout,
                          int                         flags = 0);
        // Read from this channel into a channel-supplied buffer, identified
        // via the specified 'buffer', the specified 'numBytes', or interrupt
        // after the specified absolute 'timeout' time is reached.  If the
        // optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  Any
        // positive return value guarantees that 'buffer' will remain valid
        // until this channel is modified.  On a partial result, load
        // 'augStatus', if supplied, with 0 if 'timeout' interrupted this
        // operation, or a positive value if the interruption was due to an
        // asynchronous event; otherwise, 'augStatus' is unmodified.  A partial
        // result typically does not invalidate this channel; hence, this (or
        // another) operation may be retried with some reasonable hope of
        // success -- buffered data from a partial result remains available
        // until consumed by subsequent read operations.  A negative "status",
        // however, indicates a permanent error (leaving 'buffer' undefined);
        // -1 implies that the connection was closed by the peer (but the
        // converse is not guaranteed).  The behavior is undefined unless
        // '0 < numBytes'.  Note that if the 'timeout' value has already
        // passed, the "read" operation will still be attempted, but the
        // attempt will not block.

    int bufferedReadRaw(const char **buffer, int numBytes, int flags = 0);
    int bufferedReadRaw(int         *augStatus,
                        const char **buffer,
                        int          numBytes,
                        int          flags = 0);
        // *Atomically* read from this channel into a channel-supplied buffer,
        // identified via the specified 'buffer', *at* *most* the specified
        // 'numBytes'.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  Any
        // positive return value guarantees that 'buffer' will remain valid
        // until this channel is modified.  On a partial result, load
        // 'augStatus', if supplied, with a positive value if an asynchronous
        // event interrupted this operation, or a negative value if the atomic
        // OS-level operation transmitted at least one but less than
        // 'numBytes'; otherwise, 'augStatus' is unmodified.  A partial result
        // typically does not invalidate this channel; hence, this (or another)
        // operation may be retried with some reasonable hope of success --
        // buffered data from a partial result remains available until consumed
        // by subsequent read operations.  A negative "status", however,
        // indicates a permanent error (leaving 'buffer' unset); -1 implies
        // that the connection was closed by the peer (but the converse is not
        // guaranteed).  The behavior is undefined unless '0 < numBytes'.

    int timedBufferedReadRaw(const char                **buffer,
                             int                         numBytes,
                             const bsls::TimeInterval&   timeout,
                             int                         flags = 0);
    int timedBufferedReadRaw(int                        *augStatus,
                             const char                **buffer,
                             int                         numBytes,
                             const bsls::TimeInterval&   timeout,
                             int                         flags = 0);
        // *Atomically* read from this channel into a channel-supplied buffer,
        // identified via the specified 'buffer', *at* *most* the specified
        // 'numBytes' or interrupt after the specified absolute 'timeout' time
        // is reached.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // read into 'buffer' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with 0 if 'timeout'
        // interrupted this operation, a positive value if an asynchronous
        // event caused an interruption, or a negative value if the atomic
        // OS-level operation transmitted at least one byte, but less than
        // 'numBytes'; otherwise, 'augStatus' is unmodified.  A partial result
        // typically does not invalidate this channel; hence, this (or another)
        // operation may be retried with some reasonable hope of success --
        // buffered data from a partial result remains available until consumed
        // by subsequent read operations.  A negative "status", however,
        // indicates a permanent error (leaving 'buffer' unset); -1 implies
        // that the connection was closed by the peer (but the converse is not
        // guaranteed).  The behavior is undefined unless '0 < numBytes'.  Note
        // that if the 'timeout' value has already passed, the "read" operation
        // will still be attempted, but the attempt will not block.

    // = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

    int write(const char *buffer, int numBytes, int flags = 0);
    int write(int *augStatus, const char *buffer, int numBytes, int flags = 0);
        // Write to this channel from the specified 'buffer' the specified
        // 'numBytes'.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Optionally specify (as a *leading* argument) 'augStatus'
        // to receive status specific to a "partial result".  Return 'numBytes'
        // on success, a negative value on error, and the number of bytes newly
        // written from 'buffer' (indicating a partial result) otherwise.  On a
        // partial result, load 'augStatus', if supplied, with a positive
        // value, indicating that an asynchronous event caused the
        // interruption; otherwise, 'augStatus' is unmodified.  A partial
        // result typically does not invalidate this channel; hence, this (or
        // another) operation may be retried (with arguments suitably adjusted)
        // with some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error; -1 implies that the connection was
        // closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless '0 < numBytes'.

    int timedWrite(int                       *augStatus,
                   const char                *buffer,
                   int                        numBytes,
                   const bsls::TimeInterval&  timeout,
                   int                        flags = 0);
    int timedWrite(const char                *buffer,
                   int                        numBytes,
                   const bsls::TimeInterval&  timeout,
                   int                        flags = 0);
        // Write to this channel from the specified 'buffer' the specified
        // 'numBytes' or interrupt after the specified absolute 'timeout' time
        // is reached.  If the optionally specified 'flags' incorporates
        // 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous events" are
        // permitted to interrupt this operation; by default, such events are
        // ignored.  Return 'numBytes' on success, a negative value on error,
        // and the number of bytes newly written from 'buffer' (indicating a
        // partial result) otherwise.  On a partial result, load 'augStatus',
        // if supplied, with 0 if 'timeout' interrupted this operation, or a
        // positive value if the interruption was due to an asynchronous event;
        // otherwise, 'augStatus' is unmodified.  A partial result typically
        // does not invalidate this channel; hence, this (or another) operation
        // may be retried (with arguments suitably adjusted) with some
        // reasonable hope of success.  A negative "status", however, indicates
        // a permanent error; -1 implies that the connection was closed by the
        // peer (but the converse is not guaranteed).  The behavior is
        // undefined unless '0 < numBytes'.  Note that if the 'timeout' value
        // has already passed, the "write" operation will still be attempted,
        // but the attempt will not block.

    int writeRaw(const char *buffer, int numBytes, int flags = 0);
    int writeRaw(int        *augStatus,
                 const char *buffer,
                 int         numBytes,
                 int         flags = 0);
        // *Atomically* write to this channel from the specified 'buffer' *at*
        // *most* the specified 'numBytes'.  If the optionally specified
        // 'flags' incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT',
        // "asynchronous events" are permitted to interrupt this operation; by
        // default, such events are ignored.  Optionally specify (as a
        // *leading* argument) 'augStatus' to receive status specific to a
        // "partial result".  Return 'numBytes' on success, a negative value on
        // error, and the number of bytes newly written from 'buffer'
        // (indicating a partial result) otherwise.  On a partial result, load
        // 'augStatus', if supplied, with a positive value if an asynchronous
        // event interrupted this operation and a negative value if the atomic
        // OS-level operation transmitted at least one byte, but less than
        // 'numBytes'; otherwise, 'augStatus' is unmodified.  A partial result
        // typically does not invalidate this channel; hence, this (or another)
        // operation may be retried (with arguments suitably adjusted) with
        // some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error; -1 implies that the connection was
        // closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless '0 < numBytes'.  Note that if the
        // specified 'timeout' value has already passed, the "write" operation
        // will still be attempted, but the attempt will not block.

    int timedWriteRaw(int                       *augStatus,
                      const char                *buffer,
                      int                        numBytes,
                      const bsls::TimeInterval&  timeout,
                      int                        flags = 0);
    int timedWriteRaw(const char                *buffer,
                      int                        numBytes,
                      const bsls::TimeInterval&  timeout,
                      int                        flags = 0);
        // *Atomically* write to this channel from the specified 'buffer' *at*
        // *most* the specified 'numBytes', or interrupt after the specified
        // absolute 'timeout' time is reached.  If the optionally specified
        // 'flags' incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT',
        // "asynchronous events" are permitted to interrupt this operation; by
        // default, such events are ignored.  Optionally specify (as a
        // *leading* argument) 'augStatus' to receive status specific to a
        // "partial result".  Return 'numBytes' on success, a negative value on
        // error, and the number of bytes newly written from 'buffer'
        // (indicating a partial result) otherwise.  On a partial result, load
        // 'augStatus', if supplied, with 0 if 'timeout' interrupted this
        // operation, a positive value if an asynchronous event caused an
        // interruption, or a negative value if the atomic OS-level operation
        // transmitted at least one byte, but less than 'numBytes'; otherwise,
        // 'augStatus' is unmodified.  A partial result typically does not
        // invalidate this channel; hence, this (or another) operation may be
        // retried (with arguments suitably adjusted) with some reasonable hope
        // of success.  A negative "status", however, indicates a permanent
        // error; -1 implies that the connection was closed by the peer (but
        // the converse is not guaranteed).  The behavior is undefined unless
        // '0 < numBytes'.  Note that if the 'timeout' value has already
        // passed, the "write" operation will still be attempted, but the
        // attempt will not block.

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    int writev(const btls::Ovec *buffers, int numBuffers, int flags = 0);
    int writev(const btls::Iovec *buffers, int numBuffers, int flags = 0);
    int writev(int              *augStatus,
               const btls::Ovec *buffers,
               int               numBuffers,
               int               flags = 0);
    int writev(int               *augStatus,
               const btls::Iovec *buffers,
               int                numBuffers,
               int                flags = 0);
        // Write to this channel from the specified sequence of 'buffers' of
        // specified sequence length 'numBuffers' the respective numbers of
        // bytes as specified in each corresponding 'btls::Ovec' (or
        // 'btls::Iovec') buffer.  If the optionally specified 'flags'
        // incorporates 'btlsc::Channel::ASYNC_INTERRUPT', "asynchronous
        // events" are permitted to interrupt this operation; by default, such
        // events are ignored.  Optionally specify (as a *leading* argument)
        // 'augStatus' to receive status specific to a partial result.  Return
        // the total number of bytes in 'buffers' on success, a negative value
        // on error, and the number of bytes newly written from 'buffers'
        // (indicating a partial result) otherwise.  On a partial result, load
        // 'augStatus', if supplied, with a positive value, indicating that an
        // asynchronous event caused the interruption; otherwise, 'augStatus'
        // is unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted, see 'btls::IovecUtil::pivot') with some
        // reasonable hope of success.  A negative "status", however, indicates
        // a permanent error; -1 implies that the connection was closed by the
        // peer (but the converse is not guaranteed).  The behavior is
        // undefined unless '0 < numBuffers'.

    int timedWritev(const btls::Ovec          *buffers,
                    int                        numBuffers,
                    const bsls::TimeInterval&  timeout,
                    int                        flags = 0);
    int timedWritev(const btls::Iovec         *buffers,
                    int                        numBuffers,
                    const bsls::TimeInterval&  timeout,
                    int                        flags = 0);
    int timedWritev(int                       *augStatus,
                    const btls::Ovec          *buffers,
                    int                        numBuffers,
                    const bsls::TimeInterval&  timeout,
                    int                        flags = 0);
    int timedWritev(int                       *augStatus,
                    const btls::Iovec         *buffers,
                    int                        numBuffers,
                    const bsls::TimeInterval&  timeout,
                    int                        flags = 0);
        // Write to this channel from the specified sequence of 'buffers' of
        // specified sequence length 'numBuffers' the respective numbers of
        // bytes as specified in each corresponding 'btls::Ovec' (or
        // 'btls::Iovec') buffer, or interrupt after the specified
        // absolute 'timeout' time is reached.  If the optionally specified
        // 'flags' incorporates 'btlsc::Channel::ASYNC_INTERRUPT',
        // "asynchronous events" are permitted to interrupt this operation; by
        // default, such events are ignored.  Optionally specify (as a
        // *leading* argument) 'augStatus' to receive status specific to a
        // partial result.  Return the total number of bytes in 'buffers' on
        // success, a negative value on error, and the number of bytes newly
        // written from 'buffers' (indicating a partial result) otherwise.  On
        // a partial result, load 'augStatus', if supplied, with 0 if 'timeout'
        // interrupted this operation or a positive value if the interruption
        // was due to an asynchronous event; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted, see 'btls::IovecUtil::pivot') with some
        // reasonable hope of success.  A negative "status", however, indicates
        // a permanent error; -1 implies that the connection was closed by the
        // peer (but the converse is not guaranteed).  The behavior is
        // undefined unless '0 < numBuffers'.  Note that if the 'timeout' time
        // has already passed, the "write" operation will still be attempted,
        // but the attempt will not block.

    int writevRaw(const btls::Ovec *buffers, int numBuffers, int flags = 0);
    int writevRaw(const btls::Iovec *buffers, int numBuffers, int flags = 0);
    int writevRaw(int              *augStatus,
                  const btls::Ovec *buffers,
                  int               numBuffers,
                  int               flags = 0);
    int writevRaw(int               *augStatus,
                  const btls::Iovec *buffers,
                  int                numBuffers,
                  int                flags = 0);
        // *Atomically* write to this channel, from the specified sequence of
        // 'buffers' of specified sequence length 'numBuffers', *at* *most* the
        // respective numbers of bytes as specified in each corresponding
        // 'btls::Ovec' (or 'btls::Iovec') buffer.  If the optionally specified
        // 'flags' incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT',
        // "asynchronous events" are permitted to interrupt this operation; by
        // default, such events are ignored.  Optionally specify (as a
        // *leading* argument) 'augStatus' to receive status specific to a
        // "partial result".  Return 'numBytes' on success, a negative value on
        // error, and the number of bytes newly written from ' buffer'
        // (indicating a partial result) otherwise.  On a partial result, load
        // 'augStatus', if supplied, with a positive value if an asynchronous
        // event interrupted this operation and a negative value if the atomic
        // OS-level operation transmitted at least one byte, but less than
        // 'numBytes'; otherwise, 'augStatus' is unmodified.  A partial result
        // typically does not invalidate this channel; hence, this (or another)
        // operation may be retried (with arguments suitably adjusted) with
        // some reasonable hope of success.  A negative "status", however,
        // indicates a permanent error; -1 implies that the connection was
        // closed by the peer (but the converse is not guaranteed).  The
        // behavior is undefined unless 0 < numBytes.

    int timedWritevRaw(const btls::Ovec          *buffers,
                       int                        numBuffers,
                       const bsls::TimeInterval&  timeout,
                       int                        flags = 0);
    int timedWritevRaw(const btls::Iovec         *buffers,
                       int                        numBuffers,
                       const bsls::TimeInterval&  timeout,
                       int                        flags = 0);
    int timedWritevRaw(int                       *augStatus,
                       const btls::Ovec          *buffers,
                       int                        numBuffers,
                       const bsls::TimeInterval&  timeout,
                       int                        flags = 0);
    int timedWritevRaw(int                       *augStatus,
                       const btls::Iovec         *buffers,
                       int                        numBuffers,
                       const bsls::TimeInterval&  timeout,
                       int                        flags = 0);
        // *Atomically* write to this channel, from the specified sequence of
        // 'buffers' of specified sequence length 'numBuffers', *at* *most* the
        // respective numbers of bytes as specified in each corresponding
        // 'btls::Ovec' buffer, or interrupt after the specified absolute
        // 'timeout' time is reached.  If the optionally specified 'flags'
        // incorporates 'btlsc::TimedChannel::ASYNC_INTERRUPT', "asynchronous
        // events" are permitted to interrupt this operation; by default, such
        // events are ignored.  Optionally specify (as a *leading* argument)
        // 'augStatus' to receive status specific to a "partial result".
        // Return 'numBytes' on success, a negative value on error, and the
        // number of bytes newly written from 'buffer' (indicating a partial
        // result) otherwise.  On a partial result, load 'augStatus', if
        // supplied, with 0 if 'timeout' interrupted this operation, a positive
        // value if an asynchronous event caused an interruption, or a negative
        // value if the atomic OS-level operation transmitted at least one
        // byte, but less than 'numBytes'; otherwise, 'augStatus' is
        // unmodified.  A partial result typically does not invalidate this
        // channel; hence, this (or another) operation may be retried (with
        // arguments suitably adjusted) with some reasonable hope of success.
        // A negative "status", however, indicates a permanent error; -1
        // implies that the connection was closed by the peer (but the converse
        // is not guaranteed).  The behavior is undefined unless 0 < numBytes.
        // Note that if the 'timeout' value has already passed, the "write"
        // operation will still be attempted, but the attempt will not block.

    void invalidate();
        // Make this channel invalid; no subsequent operations can be completed
        // successfully.

    int getLocalAddress(btlso::IPv4Address *result);
        // Load into the specified 'result' the complete IP address associated
        // with the local (i.e., this process) end-point of this channel.
        // Return 0 on success and a non-zero value otherwise.

    int getOption(int *result, int level, int option);
        // Load into the specified 'result' the current value of the specified
        // 'option' of the specified 'level' set on the underlying socket.
        // Return 0 on success and a non-zero value otherwise.  The list of
        // commonly-supported options (and levels) is enumerated in
        // 'btlso_socketoptutil'.

    int getPeerAddress(btlso::IPv4Address *result);
        // Load into the specified 'result' the complete IP address associated
        // with the remote (i.e., peer process) end-point of this channel.
        // Return 0 on success and a non-zero value otherwise.

    int setOption(int level, int option, int value);
        // Set the specified socket 'option' of the specified 'level' on the
        // underlying socket to the specified 'value'.  Return 0 on success and
        // a non-zero value otherwise.  (The list of commonly-supported options
        // is available in 'btlso_socketoptutil'.)

    // ACCESSORS
    int isInvalid() const;
        // Return 1 if *any* transmission error has occurred or if the channel
        // has been explicitly invalidated (via 'invalidate') and 0 otherwise.
        // Once a channel is invalid, no operations can be completed
        // successfully.  Note also that 0 return value does NOT guarantee that
        // a subsequent I/O operation would not fail.

    btlso::StreamSocket<btlso::IPv4Address> *socket() const;
        // Return the address of the stream-socket used by this channel.

};

// ----------------------------------------------------------------------------
//                             INLINE DEFINITIONS
// ----------------------------------------------------------------------------

inline
int TcpTimedChannel::readv(const btls::Iovec *buffers,
                           int                numBuffers,
                           int                flags)
{
    int unused;
    return readv(&unused, buffers, numBuffers, flags);
}

inline
int TcpTimedChannel::readRaw(int *, char* buffer, int numBytes, int)
{
    return readRaw(buffer, numBytes);
}

inline
int TcpTimedChannel::readRaw(char* buffer, int numBytes, int)
{
    return readRaw(buffer, numBytes);
}

inline
int TcpTimedChannel::timedReadv(const btls::Iovec         *buffers,
                                int                        numBuffers,
                                const bsls::TimeInterval&  timeout,
                                int                        flags)
{
    int unused;
    return timedReadv(&unused, buffers, numBuffers, timeout, flags);
}

inline
int TcpTimedChannel::writev(const btls::Ovec *buffers,
                            int numBuffers, int flags)
{
    int unused;
    return writev(&unused, buffers, numBuffers, flags);
}

inline
int TcpTimedChannel::writev(const btls::Iovec *buffers,
                            int numBuffers, int flags)
{
    int unused;
    return writev(&unused, buffers, numBuffers, flags);
}

inline
int TcpTimedChannel::writev(int *augStatus, const btls::Ovec *buffers,
                            int numBuffers, int flags)
{
    int length = setupWritev(buffers, numBuffers);
    return writev(augStatus, length, flags);
}

inline
int TcpTimedChannel::writev(int *augStatus, const btls::Iovec *buffers,
                       int numBuffers, int flags)
{
    int length = setupWritev(buffers, numBuffers);
    return writev(augStatus, length, flags);
}

inline
int TcpTimedChannel::timedWritev(const btls::Ovec          *buffers,
                                 int                        numBuffers,
                                 const bsls::TimeInterval&  timeout,
                                 int                        flags)
{
    int unused;
    return timedWritev(&unused, buffers, numBuffers, timeout, flags);
}

inline
int TcpTimedChannel::timedWritev(const btls::Iovec         *buffers,
                                 int                        numBuffers,
                                 const bsls::TimeInterval&  timeout,
                                 int                        flags)
{
    int unused;
    return timedWritev(&unused, buffers, numBuffers, timeout, flags);
}

inline
int TcpTimedChannel::timedWritev(int                       *augStatus,
                                 const btls::Ovec          *buffers,
                                 int                        numBuffers,
                                 const bsls::TimeInterval&  timeout,
                                 int                        flags)
{
    int length = setupWritev(buffers, numBuffers);
    return timedWritev(augStatus, length, flags, timeout);
}

inline
int TcpTimedChannel::timedWritev(int                       *augStatus,
                                 const btls::Iovec         *buffers,
                                 int                        numBuffers,
                                 const bsls::TimeInterval&  timeout,
                                 int                        flags)
{
    int length = setupWritev(buffers, numBuffers);
    return timedWritev(augStatus, length, flags, timeout);
}

template <typename VECTYPE>
inline
int TcpTimedChannel::setupWritev(const VECTYPE *buffers, int numBuffers)
{
    BSLS_ASSERT(buffers);
    BSLS_ASSERT(0 < numBuffers);

    int length = 0;
    d_ovecBuffers.resize(numBuffers);
    for (int i = 0; i < numBuffers; ++i) {
        int thisBufferLength = buffers[i].length();
        length += thisBufferLength;
        d_ovecBuffers[i].setBuffer(buffers[i].buffer(), thisBufferLength);
    }
    return length;
}

inline
void TcpTimedChannel::invalidate()
{
    d_isInvalidFlag = 1;
}

inline
btlso::StreamSocket<btlso::IPv4Address> *TcpTimedChannel::socket() const
{
    return d_socket_p;
}

inline
int TcpTimedChannel::isInvalid() const
{
    return d_isInvalidFlag;
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
