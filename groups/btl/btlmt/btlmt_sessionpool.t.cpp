// btlmt_sessionpool.t.cpp                                            -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#include <btlmt_sessionpool.h>

#include <btlmt_asyncchannel.h>
#include <btlmt_channelpoolchannel.h>
#include <btlmt_channelpool.h>
#include <btlmt_connectoptions.h>
#include <btlmt_listenoptions.h>
#include <btlmt_session.h>
#include <btlmt_sessionfactory.h>

#include <btlb_blob.h>
#include <btlb_blobutil.h>
#include <btlb_pooledblobbufferfactory.h>

#include <btlso_flags.h>
#include <btlso_ipv4address.h>
#include <btlso_inetstreamsocketfactory.h>
#include <btlso_sockethandle.h>
#include <btlso_socketoptions.h>
#include <btlso_streamsocket.h>

#include <bdlf_bind.h>
#include <bdlf_placeholder.h>
#include <bdlf_memfn.h>

#include <bslma_allocator.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_default.h>
#include <bslma_testallocator.h>

#include <bslmt_barrier.h>
#include <bslmt_mutex.h>
#include <bslmt_semaphore.h>
#include <bslmt_threadutil.h>

#include <bsls_atomic.h>

#include <bsl_cstdlib.h>     // atoi()
#include <bsl_iostream.h>
#include <bsl_map.h>
#include <bsl_sstream.h>

using namespace BloombergLP;
using namespace bsl;
using namespace bdlf::PlaceHolders;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
//
// This test driver has grown in an ad-hoc manner and is still incomplete.  A
// thorough review and addition of test cases is still required, but the added
// test cases provide confidence that the core functionality is working.
//
// We begin with a couple of test cases that ensure that a session pool can be
// created, started, and can listen on a specified port, and that data sent to
// that port is received by the session pool.  Next, we test that using blobs
// for data reads works as expected.  After that we test individual
// functionality caused by changes to fix bugs.  The accumulation of all these
// test cases test a significant portion of the component's functionality.
//-----------------------------------------------------------------------------
// CREATORS
// [ 2] btlmt::SessionPool(config, poolStateCallback, *ta = 0);
// [14] btlmt::SessionPool(BlobBufferFactory *, config, poolCb, *ta = 0);
// [ 2] ~btlmt::SessionPool();
//
// MANIPULATORS
// [12] int start();
// [12] int stop();
// [ 8] int stopAndRemoveAllSessions();
// [11] int connect(host, ...., *socketOptions);
// [11] int connect(serverAddr, ...., *socketOptions);
// [ 5] int listen(*h, sscb, port, backlog, *factory, *data, *options);
// [ 4] int listen(*h, sscb, port, backlog, reuse, *factory, *data, *options);
// [  ] int listen(*h, sscb, endpoint, backlog, *factory, *data, *options);
// [  ] int listen(*h, sscb, endpoint, backlog, reuse, *factory, *data, *opts);
// [  ] int closeHandle(int handle);
// [11] int connect(*h, sscb, *name, port, numAtts, time, *s, *f, *data, mode);
// [11] int connect(*h, sscb, endpoint, numAtts, time, *s, *f, *userdata);
// [10] int connect(*h, cb, *name, port, atts, time, *f, *data, mode, *o, *la);
// [10] int connect(*h, sscb, endpoint, numAtts, time, *f, *userdata, *o, *la);
// [  ] int import(*h, sscb, *streamSocket, *factory, *sessionFactory, *data);
// [  ] int import(*h, sscb, *streamSocket, *sessionFactory, *userData);
// [ 9] int setWriteQueueWatermarks(handle, int loWatermark, int hiWatermark);
//
// ACCESSORS
// [  ] const btlmt::ChannelPoolConfiguration& config() const;
// [18] int busyMetrics() const;
// [  ] void getChannelHandleStatistics(*handleInfo) const;
// [18] bool isRunning() const;
// [ 6] int numSessions() const;
// [ 4] int portNumber(int handle) const;
//
// BUG FIXES
// [ 7] Testing removal of intermediate read blob
// [ 6] Testing 'stop' updates numSessions correctly
// [ 5] Testing peer address is set appropriately
// [ 4] Testing removal of inefficiencies in read callback
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [20] USAGE EXAMPLE
//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        bsl::cout << "Error " << __FILE__ << "(" << i << "): " << s
                  << "    (failed)" << bsl::endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\n";aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << "\t"\
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << \
       "\t" << #K << ": " << K << "\t" << #L << ": " << L << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP6_ASSERT(I,J,K,L,M,N,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << "\t"\
       << #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\t" << #N << ": " << N << "\n"; \
       aSsErT(1, #X, __LINE__); } }


//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) bsl::cout << #X " = " << (X) << bsl::endl; // Print identifier
                                                        // and value.
#define Q(X) bsl::cout << "<| " #X " |>" << bsl::endl;  // Quote identifier
                                                        // literally.
#define NL() bsl::cout << bsl::endl;                    // End of line
#define P_(X) bsl::cout << #X " = " << (X) << ", "<< bsl::flush; // P(X)
                                                                 // without
                                                                 // '\n'
#define T_()  bsl::cout << '\t' << bsl::flush;        // Print tab w/o newline
#define L_ __LINE__                           // current Line number

// The following macros facilitate thread-safe streaming to standard output.

#define MTCOUT   coutMutex.lock(); { bsl::cout \
                                         << bslmt::ThreadUtil::selfIdAsInt()  \
                                         << ": "
#define MTENDL   bsl::endl;  } coutMutex.unlock()

#define DUMMYBRACE {  //keep context-aware editors happy about the "}" below

#define MTFLUSH  bsl::flush; } coutMutex.unlock()

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef btlmt::SessionPool                         Obj;
typedef btlso::SocketOptions                       SocketOptions;
typedef btlmt::AsyncChannel::BlobBasedReadCallback BlobReadCallback;
typedef btlso::StreamSocketFactoryDeleter          SocketFactoryDeleter;
typedef btlso::SocketOptions                       SocketOptions;
typedef bsls::TimeInterval                         TimeInterval;
typedef btlso::IPv4Address                         IPAddress;
typedef btlso::IPv4Address                         IPAddress;
typedef btlso::InetStreamSocketFactory<IPAddress>  InetStreamSocketFactory;
typedef btlso::StreamSocket<IPAddress>             StreamSocket;
typedef Obj::SessionStateCallback                  SessionStateCallback;
typedef btlmt::SessionFactory                      SessionFactory;
typedef btlso::StreamSocketFactory<IPAddress>      SocketFactory;

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;

static bslmt::Mutex coutMutex;

//=============================================================================
//                  SUPPORT CLASSES AND FUNCTIONS USED FOR TESTING
//-----------------------------------------------------------------------------
static
btlso::IPv4Address getLocalAddress() {
    // On Cygwin/Windows, binding to btlso::IPv4Address() doesn't seem to work.
    // Wants to bind to localhost/127.0.0.1.

#if defined(BSLS_PLATFORM_OS_CYGWIN) || defined(BSLS_PLATFORM_OS_WINDOWS)
    return btlso::IPv4Address("127.0.0.1", 0);
#else
    return btlso::IPv4Address();
#endif
}

namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE {

void poolStateCallback(int reason, int source, void *)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: (" << reason << ", " << source
               << ") " << MTENDL;
    }
}

void poolStateCallbackWithBarrier(int             state,
                                  int             source,
                                  void           *,
                                  int            *poolState,
                                  bslmt::Barrier *barrier)
{
    if (veryVerbose) {
        MTCOUT << "Pool state callback called with"
               << " State: " << state
               << " Source: "  << source << MTENDL;
    }
    *poolState = state;
    barrier->wait();
}

void poolStateCallbackWithError(int             reason,
                                int             source,
                                int             error,
                                void           *userData,
                                int            *platformError,
                                bslmt::Barrier *barrier)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: " << reason << ", " << " error: "
               << *platformError << MTENDL;
    }
    *platformError = error;
    barrier->wait();
}

void sessionStateCallback(int             state,
                          int             ,
                          btlmt::Session *session,
                          void           *)
{
    switch(state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
      } break;
    }
}

void sessionStateCallbackWithBarrier(int             state,
                                     int             ,
                                     btlmt::Session *session,
                                     void           *,
                                     bslmt::Barrier *barrier)
{
    switch(state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
        barrier->wait();
      } break;
    }
}

void sessionStateCallbackWithCounter(int              state,
                                     int              ,
                                     btlmt::Session  *session,
                                     void            *,
                                     bsls::AtomicInt *numUpConnections)
{
    switch(state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
        ++*numUpConnections;
      } break;
    }
}

void readCb(int         result,
            int        *numNeeded,
            btlb::Blob *data,
            int         channelId)
{
    if (result) {
        // Session is going down.

        return;
    }

    data->removeAll();

    *numNeeded = 1;
}

void readCbWithBlob(int         result,
                    int        *numNeeded,
                    btlb::Blob *data,
                    int         ,
                    btlb::Blob *blob)
{
    if (result) {
        // Session is going down.

        return;                                                       // RETURN
    }

    ASSERT(numNeeded);
    ASSERT(data);
    ASSERT(0 < data->length());

    blob->moveAndAppendDataBuffers(data);

    *numNeeded = 1;
}

void readCbWithReadSize(int            result,
                        int           *numNeeded,
                        btlb::Blob    *data,
                        int            channelId,
                        int            readSize,
                        int            totalDataSize,
                        bslmt::Barrier *barrier)
{
    static int numRead = 0;

    if (result) {
        // Session is going down.

        return;
    }

    ASSERT(numNeeded);
    ASSERT(data);
    ASSERT(0 < data->length());

    const int bytesToRead = bsl::min(data->length(), readSize);

    btlb::BlobUtil::erase(data, 0, bytesToRead);

    numRead += bytesToRead;
    if (numRead >= totalDataSize) {
        barrier->wait();
        numRead = 0;
    }
    *numNeeded = 1;
}

void readCbWithBlobAndBarrier(int             result,
                              int            *numNeeded,
                              btlb::Blob     *data,
                              int             channelId,
                              btlb::Blob     *blob,
                              bslmt::Barrier *barrier)
{
    readCbWithBlob(result, numNeeded, data, channelId, blob);
    barrier->wait();
}

void readCbWithCountAndBarrier(int             result,
                               int            *numNeeded,
                               btlb::Blob     *data,
                               int             ,
                               int            *cbCount,
                               bslmt::Barrier *barrier)
{
    if (result) {
        // Session is going down.

        return;                                                       // RETURN
    }

    *numNeeded = 1;
    data->removeAll();
    ++*cbCount;
    barrier->wait();
}

                            // =================
                            // class TestSession
                            // =================

class TestSession : public btlmt::Session {
    // This class is a concrete implementation of the 'btlmt::Session' protocol
    // to use along with 'Tester' objects.

    // DATA
    btlmt::AsyncChannel *d_channel_p;
    BlobReadCallback    *d_callback_p;

  private:
    // NOT IMPLEMENTED
    TestSession(const TestSession&);
    TestSession& operator=(const TestSession&);

    void blobReadCb(int         result,
                    int        *numNeeded,
                    btlb::Blob *blob,
                    int         channelId);
        // Read callback for session pool.

  public:
    // CREATORS
    TestSession(btlmt::AsyncChannel *channel, BlobReadCallback *callback);
        // Create a new 'TestSession' object for the specified 'channel'.

    ~TestSession();
        // Destroy this object.

    // MANIPULATORS
    virtual int start();
        // Begin the asynchronous operation of this session.

    virtual int stop();
        // Stop the operation of this session.

    // ACCESSORS
    virtual btlmt::AsyncChannel *channel() const;
        // Return the channel associate with this session.
};

                            // -----------------
                            // class TestSession
                            // -----------------

// PRIVATE MANIPULATORS
void TestSession::blobReadCb(int         result,
                             int        *numNeeded,
                             btlb::Blob *blob,
                             int         )
{
    if (result) {
        // Session is going down.

        d_channel_p->close();
        return;                                                       // RETURN
    }

    if (0 != blob->length()) {
        ASSERT(0 == d_channel_p->write(*blob));
    }

    blob->removeAll();

    *numNeeded = 1;
}

// CREATORS
TestSession::TestSession(btlmt::AsyncChannel *channel,
                         BlobReadCallback    *callback)
: d_channel_p(channel)
, d_callback_p(callback)
{
}

TestSession::~TestSession()
{
}

// MANIPULATORS
int TestSession::start()
{
    if (veryVerbose) {
        MTCOUT << "Session started" << MTENDL;
    }

    btlmt::AsyncChannel::BlobBasedReadCallback callback =
                            d_callback_p
                            ? *d_callback_p
                            : bdlf::MemFnUtil::memFn(&TestSession::blobReadCb,
                                                     this);

    d_channel_p->read(1, callback);

    return 0;
}

int TestSession::stop()
{
    if (veryVerbose) {
        MTCOUT << "Session stopped" << MTENDL;
    }

    d_channel_p->close();
    return 0;
}

// ACCESSORS
btlmt::AsyncChannel *TestSession::channel() const
{
    return d_channel_p;
}

                    // =================
                    // class TestFactory
                    // =================

class TestFactory : public btlmt::SessionFactory {
    // This class is a concrete implementation of the 'btlmt::SessionFactory'
    // that simply allocates 'TestSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    BlobReadCallback *d_callback_p;    // read callback (held, not owned)
    bslma::Allocator *d_allocator_p;   // memory allocator (held, not owned)

  public:
    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(TestFactory, bslma::UsesBslmaAllocator);

    // CREATORS
    TestFactory(BlobReadCallback *callback = 0,
                bslma::Allocator *basicAllocator = 0);
        // Create a new 'TestFactory' object.  Optionally specify a
        // 'basicAllocator' used to supply memory.  If 'basicAllocator' is 0,
        // the currently installed default allocator is used.

    virtual ~TestFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(btlmt::AsyncChannel                    *channel,
                          const btlmt::SessionFactory::Callback&  callback);
        // Asynchronously allocate a 'btlmt::Session' object for the
        // specified 'channel', and invoke the specified 'callback' with
        // this session.

    virtual void deallocate(btlmt::Session *session);
        // Deallocate the specified 'session'.
};

                        // -----------------
                        // class TestFactory
                        // -----------------

// CREATORS
TestFactory::TestFactory(BlobReadCallback *callback,
                         bslma::Allocator *basicAllocator)
: d_callback_p(callback)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TestFactory::~TestFactory()
{
}

// MANIPULATORS
void TestFactory::allocate(btlmt::AsyncChannel                    *channel,
                           const btlmt::SessionFactory::Callback&  callback)
{
    if (veryVerbose) {
        MTCOUT << "Allocate factory called: " << MTENDL;
    }

    TestSession *session = new (*d_allocator_p) TestSession(channel,
                                                            d_callback_p);

    callback(0, session);
}

void TestFactory::deallocate(btlmt::Session *session)
{
    if (veryVerbose) {
        MTCOUT << "Deallocate factory called: " << MTENDL;
    }

    d_allocator_p->deleteObjectRaw(session);
}

}  // close namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE

namespace BTLMT_SESSION_POOL_GRACEFUL_SHUTDOWN {

bslmt::Barrier  *s_barrier_p = 0;
int             *s_state_p = 0;
bool             s_enqueueData = true;

const int WRITE_SIZE = 1024 * 1024 * 10;

                            // ===================
                            // class TesterSession
                            // ===================

class TesterSession : public btlmt::Session {
    // This class is a concrete implementation of the 'btlmt::Session' protocol
    // to use along with 'Tester' objects.

    // DATA
    bsl::shared_ptr<btlmt::AsyncChannel>      d_channel_sp;
    bsl::shared_ptr<btlb::BlobBufferFactory>  d_blobFactory_sp;

    // PRIVATE MANIPULATORS
    void readCb(int         result,
                int        *numNeeded,
                btlb::Blob *blob,
                int         channelId);
        // Read callback for session pool.

  private:
    // NOT IMPLEMENTED
    TesterSession(const TesterSession&);
    TesterSession& operator=(const TesterSession&);

  public:
    // CREATORS
    TesterSession(
              const bsl::shared_ptr<btlmt::AsyncChannel>&  channel,
              bslma::Allocator                            *basicAllocator = 0);
        // Create a new 'TesterSession' object for the specified 'channel'.
        // Optionally specify 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.

    ~TesterSession();
        // Destroy this object.

    // MANIPULATORS
    virtual int start();
        // Begin the asynchronous operation of this session.

    virtual int stop();
        // Stop the operation of this session.

    // ACCESSORS
    virtual btlmt::AsyncChannel *channel() const;
        // Return the channel associate with this session.
};

                            // -------------------
                            // class TesterSession
                            // -------------------

// CREATORS
TesterSession::TesterSession(
                   const bsl::shared_ptr<btlmt::AsyncChannel>&  channel,
                   bslma::Allocator                            *basicAllocator)
: d_channel_sp(channel)
{
    bslma::Allocator *allocator = bslma::Default::allocator(basicAllocator);
    d_blobFactory_sp.reset(new (*allocator) btlb::PooledBlobBufferFactory(
                                                                   WRITE_SIZE),
                          allocator);
}

TesterSession::~TesterSession()
{
}

// MANIPULATORS
void TesterSession::readCb(int         result,
                           int        *numNeeded,
                           btlb::Blob *blob,
                           int         )
{
    if (veryVerbose) {
        MTCOUT << "Read callback called with: " << result << MTENDL;
    }

    if (result) {
        d_channel_sp->close();
        return;                                                       // RETURN
    }

    blob->removeAll();

    if (s_enqueueData) {
        btlb::Blob b(d_blobFactory_sp.get());
        b.setLength(WRITE_SIZE);

        ASSERT(0 == d_channel_sp->write(b));

        ASSERT(0 != d_channel_sp->write(b));
    }

    d_channel_sp->close();

    *numNeeded = 0;

    s_barrier_p->wait();
}

int TesterSession::start()
{
    if (veryVerbose) {
        MTCOUT << "Session started" << MTENDL;
    }

    btlmt::AsyncChannel::BlobBasedReadCallback f =
                                 bdlf::MemFnUtil::memFn(&TesterSession::readCb,
                                                        this);

    return d_channel_sp->read(1, f);
}

int TesterSession::stop()
{
    if (veryVerbose) {
        MTCOUT << "Session stopped" << MTENDL;
    }

    return 0;
}

// ACCESSORS
btlmt::AsyncChannel *TesterSession::channel() const
{
    return d_channel_sp.get();
}

                    // ===================
                    // class TesterFactory
                    // ===================

class TesterFactory : public btlmt::SessionFactory {
    // This class is a concrete implementation of the 'btlmt::SessionFactory'
    // that simply allocates 'TesterSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    TesterSession    *d_session_p;    // held not owned

    bslma::Allocator *d_allocator_p;  // memory allocator (held, not owned)

    // NOT IMPLEMENTED
    TesterFactory(const TesterFactory&  original,
                  bslma::Allocator     *basicAllocator);

    public:

    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(TesterFactory,
                                   bslma::UsesBslmaAllocator);

    // CREATORS
    explicit TesterFactory(bslma::Allocator *basicAllocator = 0);
        // Create a new 'TesterFactory' object of the specified 'mode'.
        // Optionally specify 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.

    virtual ~TesterFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(
                         const bsl::shared_ptr<btlmt::AsyncChannel>& channel,
                         const btlmt::SessionFactory::Callback&      callback);
        // Asynchronously allocate a 'btlmt::Session' object for the specified
        // 'channel', and invoke the specified 'callback' with this session.

    virtual void allocate(btlmt::AsyncChannel                    *channel,
                          const btlmt::SessionFactory::Callback&  callback);
        // Asynchronously allocate a 'btlmt::Session' object for the specified
        // 'channel', and invoke the specified 'callback' with this session.

    virtual void deallocate(btlmt::Session *session);
        // Deallocate the specified 'session'.

    TesterSession *session() const;
        // Return the session managed by this factory.
};

                        // -------------------
                        // class TesterFactory
                        // -------------------

// CREATORS
TesterFactory::TesterFactory(bslma::Allocator *basicAllocator)
: d_session_p(0)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TesterFactory::~TesterFactory()
{
}

// MANIPULATORS
void TesterFactory::allocate(
                          const bsl::shared_ptr<btlmt::AsyncChannel>& channel,
                          const btlmt::SessionFactory::Callback&      callback)
{
    if (veryVerbose) {
        MTCOUT << "TesterFactory::allocate called: " << MTENDL;
    }

    d_session_p = new (*d_allocator_p) TesterSession(channel,
                                                     d_allocator_p);

    callback(0, d_session_p);
}

void TesterFactory::allocate(btlmt::AsyncChannel                    *,
                             const btlmt::SessionFactory::Callback&  )
{
    ASSERT(false); // shouldn't get called
}

void TesterFactory::deallocate(btlmt::Session *session)
{
    d_allocator_p->deleteObject(session);
}

TesterSession *TesterFactory::session() const
{
    return d_session_p;
}

void poolStateCallback(int reason, int source, void *)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: (" << reason << ", " << source << ") "
               << MTENDL;
    }
}

void sessionStateCb(int             state,
                    int             ,
                    btlmt::Session *session,
                    void           *)
{
    *s_state_p = state;
    switch (state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
          if (veryVerbose) {
              MTCOUT << "Client from "
                     << session->channel()->peerAddress()
                     << " has disconnected."
                     << MTENDL;
          }
          s_barrier_p->wait();
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
          if (veryVerbose) {
              MTCOUT << "Client connected from "
                     << session->channel()->peerAddress()
                     << MTENDL;
          }
          s_barrier_p->wait();
      } break;
    }
}

}  // close namespace BTLMT_SESSION_POOL_GRACEFUL_SHUTDOWN


namespace BTLMT_SESSION_POOL_SETTING_SOCKETOPTIONS {

using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

int createConnection(
                Obj                                            *sessionPool,
                btlmt::SessionPool::SessionStateCallback       *sessionStateCb,
                btlmt::SessionFactory                          *sessionFactory,
                btlso::StreamSocket<btlso::IPv4Address>        *serverSocket,
                SocketOptions                                  *socketOptions,
                btlso::StreamSocketFactory<btlso::IPv4Address> *socketFactory,
                const btlso::IPv4Address                       *ipAddress)
{
    ASSERT(0 == serverSocket->bind(getLocalAddress()));
    ASSERT(0 == serverSocket->listen(1));

    btlso::IPv4Address serverAddr;
    ASSERT(0 == serverSocket->localAddress(&serverAddr));

    btlmt::ConnectOptions options;
    options.setServerEndpoint(serverAddr);
    options.setTimeout(bsls::TimeInterval(1));
    options.setNumAttempts(1);
    if (socketOptions) {
        options.setSocketOptions(*socketOptions);
    }

    int handleBuffer;
    if (socketOptions) {
        return sessionPool->connect(&handleBuffer,
                                    *sessionStateCb,
                                    sessionFactory,
                                    options);                         // RETURN
    } else {
        BSLS_ASSERT_OPT(socketFactory); // test invariant
        BSLS_ASSERT_OPT(ipAddress);

        typedef btlso::StreamSocketFactoryDeleter Deleter;

        bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                      clientSocket(socketFactory->allocate(),
                                   socketFactory,
                                   &Deleter::deleteObject<btlso::IPv4Address>);
        const int rc = clientSocket->bind(*ipAddress);
        if (rc) {
            return rc;                                                // RETURN
        }

        options.setSocketPtr(&clientSocket);

        return sessionPool->connect(&handleBuffer,
                                    *sessionStateCb,
                                    sessionFactory,
                                    options);                         // RETURN
    }
}

}  // close namespace BTLMT_SESSION_POOL_SETTING_SOCKETOPTIONS


namespace BTLMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS {

using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

bslmt::Mutex                                           mapMutex;
bsl::map<int, btlmt::AsyncChannel *>                   sourceIdToChannelMap;
typedef bsl::map<int, btlmt::AsyncChannel *>::iterator MapIter;

void sessionStateCallbackUsingChannelMapAndCounter(
                                             int              state,
                                             int              handle,
                                             btlmt::Session   *session,
                                             void            *,
                                             bsls::AtomicInt *numUpConnections)
{
    switch(state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
        {
            MapIter iter = sourceIdToChannelMap.find(handle);
            if (iter != sourceIdToChannelMap.end()) {
                sourceIdToChannelMap.erase(iter);
            }
        }
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
        {
            bslmt::LockGuard<bslmt::Mutex> guard(&mapMutex);
            sourceIdToChannelMap[handle] = session->channel();
        }
        ++*numUpConnections;
      } break;
    }
}

const int NUM_BYTES   = 1024 * 1024;
const int NUM_THREADS = 5;

bsl::vector<btlso::StreamSocket<btlso::IPv4Address> *>
                                                    clientSockets(NUM_THREADS);
btlso::InetStreamSocketFactory<btlso::IPv4Address>  socketFactory;

struct ConnectData {
    int                d_index;
    btlso::IPv4Address d_serverAddress;
};

extern "C" void *connectFunction(void *args)
{
    ConnectData              data      = *(const ConnectData *) args;
    const int                INDEX     = data.d_index;
    const btlso::IPv4Address ADDRESS   = data.d_serverAddress;

    btlso::StreamSocket<btlso::IPv4Address> *socket = socketFactory.allocate();
    clientSockets[INDEX] = socket;

    ASSERT(0 == socket->connect(ADDRESS));

    return 0;
}

bsl::vector<btlso::StreamSocket<btlso::IPv4Address> *>
                                                    serverSockets(NUM_THREADS);
bsl::vector<btlso::StreamSocket<btlso::IPv4Address> *>
                                                    acceptSockets(NUM_THREADS);

bsls::AtomicInt numUpConnections(0);

struct ListenData {
    int d_index;
};

extern "C" void *listenFunction(void *args)
{
    ListenData data  = *(const ListenData *) args;
    const int  INDEX = data.d_index;

    btlso::StreamSocket<btlso::IPv4Address> *serverSocket =
                                                      socketFactory.allocate();
    serverSockets[INDEX] = serverSocket;

    ASSERT(0 == serverSocket->bind(getLocalAddress()));
    ASSERT(0 == serverSocket->listen(1));

    serverSocket->setBlockingMode(btlso::Flags::e_BLOCKING_MODE);

    ++numUpConnections;

    btlso::StreamSocket<btlso::IPv4Address> *acceptSocket;
    ASSERT(!serverSocket->accept(&acceptSocket));

    ASSERT(0 == acceptSocket->setBlockingMode(
                                            btlso::Flags::e_NONBLOCKING_MODE));

    acceptSockets[INDEX] = acceptSocket;

    return 0;
}

void runTestFunction(bslmt::ThreadUtil::Handle                *connectThreads,
                     bslmt::ThreadUtil::Handle                *listenThreads,
                     btlmt::SessionPool                       *pool,
                     btlmt::SessionPool::SessionStateCallback *sessionStateCb,
                     TestFactory                              *sessionFactory,
                     const btlb::Blob&                         dataBlob)
{
    btlmt::ListenOptions listenOpts;
    listenOpts.setServerAddress(btlso::IPv4Address());
    listenOpts.setBacklog(5);

    bsl::vector<int> serverHandles(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        ASSERT(0 == pool->listen(&serverHandles[i],
                                 *sessionStateCb,
                                 sessionFactory,
                                 listenOpts));
    }

    ConnectData connectData[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        connectData[i].d_index = i;

        const int PORTNUM = pool->portNumber(serverHandles[i]);
        connectData[i].d_serverAddress = btlso::IPv4Address("127.0.0.1",
                                                            PORTNUM);

        ASSERT(0 == bslmt::ThreadUtil::create(&connectThreads[i],
                                              &connectFunction,
                                              (void *) &connectData[i]));
    }

    while (numUpConnections < NUM_THREADS) {
        bslmt::ThreadUtil::microSleep(50, 0);
    }

    numUpConnections = 0;

    ListenData listenData[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        listenData[i].d_index = i;

        ASSERT(0 == bslmt::ThreadUtil::create(&listenThreads[i],
                                              &listenFunction,
                                              (void *) &listenData[i]));
        bslmt::ThreadUtil::microSleep(100, 0);
    }

    while (numUpConnections < NUM_THREADS) {
        bslmt::ThreadUtil::microSleep(50, 0);
    }

    btlmt::ConnectOptions options;
    options.setTimeout(bsls::TimeInterval(1));
    options.setNumAttempts(10);

    bsl::vector<int> clientHandles(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        btlso::IPv4Address serverAddr;
        ASSERT(0 == serverSockets[i]->localAddress(&serverAddr));

        options.setServerEndpoint(serverAddr);
        ASSERT(0 == pool->connect(&clientHandles[i],
                                  *sessionStateCb,
                                  sessionFactory,
                                  options));
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        ASSERT(0 == bslmt::ThreadUtil::join(listenThreads[i]));
    }

    mapMutex.lock();
    for (int i = 0; i < NUM_THREADS; ++i) {
        MapIter iter = sourceIdToChannelMap.find(serverHandles[i]);
        if (iter != sourceIdToChannelMap.end()) {
            ASSERT(0 == iter->second->write(dataBlob));
        }

        iter = sourceIdToChannelMap.find(clientHandles[i]);
        if (iter != sourceIdToChannelMap.end()) {
            ASSERT(0 == iter->second->write(dataBlob));
        }
    }
    mapMutex.unlock();
}

}  // close namespace BTLMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS

namespace BTLMT_SESSION_POOL_CASE_REMOVE_EXTRA_BLOB {

using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

static int callbackCount = 0;

static int maxLength = 0;
static int maxSize = 0;
static int maxExtra = 0;
static int maxNumBuffers = 0;
static int maxNumDataBuffers = 0;

enum {
    PAYLOAD_SIZE = 320,
    HALF_PAYLOAD_SIZE = 160
};

void readCbWithMetrics(int               result,
                       int              *numNeeded,
                       btlb::Blob       *blob,
                       int               ,
                       int               numBytesToRead,
                       bslmt::Semaphore *semaphore)
{
    static int numBytesRead = 0;

    if (result) {
        // Session is going down.

        return;                                                       // RETURN
    }

    ++callbackCount;

    ASSERT(numNeeded);
    ASSERT(blob);
    ASSERT(0 < blob->length());

    numBytesRead += blob->length();

    int consume = blob->length();
    if (0 == callbackCount % 2) {
        // Every second time leave a bit of data in the input buffer.
        consume -= sizeof(int);
    }

    if (maxLength < blob->length()) {
        maxLength = blob->length();
    }

    if (maxSize < blob->totalSize()) {
        maxSize = blob->totalSize();
    }

    if (maxExtra < (blob->totalSize() - blob->length())) {
        maxExtra = (blob->totalSize() - blob->length());
    }

    if (maxNumDataBuffers < blob->numDataBuffers()) {
        maxNumDataBuffers = blob->numDataBuffers();
    }

    if (maxNumBuffers < blob->numBuffers()) {
        maxNumBuffers = blob->numBuffers();
    }

    btlb::BlobUtil::erase(blob, 0, consume);

    if (numBytesRead < numBytesToRead) {
        *numNeeded = PAYLOAD_SIZE;
    }
    else {
        *numNeeded = 0;
        semaphore->post();
    }
}

}  // close namespace BTLMT_SESSION_POOL_CASE_REMOVE_EXTRA_BLOB

namespace BTLMT_SESSION_POOL_CASE_PEER_ADDRESS {

using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

                    // ========================
                    // class TestSessionFactory
                    // ========================

class TestSessionFactory : public btlmt::SessionFactory {
    // This class is a concrete implementation of the 'btlmt::SessionFactory'
    // that simply allocates 'TestSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    bslmt::Barrier *d_barrier_p;

  public:
    // CREATORS
    TestSessionFactory(bslmt::Barrier *barrier);
        // Create a new 'TestSessionFactory' object using the specified
        // 'barrier'.

    virtual ~TestSessionFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(btlmt::AsyncChannel                   *channel,
                          const btlmt::SessionFactory::Callback& callback);
        // Asynchronously allocate a 'btlmt::Session' object for the specified
        // 'channel', and invoke the specified 'callback' with this session.

    virtual void deallocate(btlmt::Session *session);
        // Deallocate the specified 'session'.

    btlmt::AsyncChannel *channel() const;
        // Return the channel managed by this factory.
};

                        // ------------------------
                        // class TestSessionFactory
                        // ------------------------

// CREATORS
TestSessionFactory::TestSessionFactory(bslmt::Barrier *barrier)
: d_barrier_p(barrier)
{
}

TestSessionFactory::~TestSessionFactory()
{
}

// MANIPULATORS
void TestSessionFactory::allocate(btlmt::AsyncChannel                    *,
                                  const btlmt::SessionFactory::Callback&  )
{
    if (veryVerbose) {
        MTCOUT << "Allocate factory called: " << MTENDL;
    }

    d_barrier_p->wait();
}

void TestSessionFactory::deallocate(btlmt::Session *)
{
    if (veryVerbose) {
        MTCOUT << "Deallocate factory called: " << MTENDL;
    }
}

btlmt::AsyncChannel *TestSessionFactory::channel() const
{
    return 0;
}

}  // close namespace BTLMT_SESSION_POOL_CASE_PEER_ADDRESS

namespace BTLMT_SESSION_POOL_CASE_ALLOCATE_SHARED_CHANNEL {

                            // ===================
                            // class TesterSession
                            // ===================

class TesterSession : public btlmt::Session {
    // This class is a concrete implementation of the 'btlmt::Session' protocol
    // to use along with 'Tester' objects.

    // DATA
    bsl::shared_ptr<btlmt::AsyncChannel>  d_channel_sp;
    bslmt::Barrier                       *d_barrier_p;

    // PRIVATE MANIPULATORS
    void delayedChannelAccessor(
                         bsl::shared_ptr<btlmt::AsyncChannel> spChannel,
                         btlso::IPv4Address                   referenceAddress)
    {
        d_barrier_p->wait();

        ASSERT(spChannel);

        if (veryVerbose) {
            MTCOUT << "Late access of Async Channel, peerAddress = " <<
                    spChannel->peerAddress() << MTENDL;
        }

        ASSERT(spChannel->peerAddress() == referenceAddress);
    }

    void readCb(int         result,
                int        *numNeeded,
                btlb::Blob *blob,
                int         channelId);
        // Read callback for session pool.

  private:
    // NOT IMPLEMENTED
    TesterSession(const TesterSession&);
    TesterSession& operator=(const TesterSession&);

  public:
    // CREATORS
    TesterSession(const bsl::shared_ptr<btlmt::AsyncChannel>&  channel,
                  bslmt::Barrier                              *barrier);
        // Create a new 'TesterSession' object for the specified 'channel'.

    ~TesterSession();
        // Destroy this object.

    // MANIPULATORS
    virtual int start();
        // Begin the asynchronous operation of this session.

    virtual int stop();
        // Stop the operation of this session.

    // ACCESSORS
    virtual btlmt::AsyncChannel *channel() const;
        // Return the channel associate with this session.
};

                    // ===================
                    // class TesterFactory
                    // ===================

class TesterFactory : public btlmt::SessionFactory {
    // This class is a concrete implementation of the 'btlmt::SessionFactory'
    // that simply allocates 'TesterSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    int               d_mode;

    bslmt::Barrier   *d_barrier_p;    // held not owned

    TesterSession    *d_session_p;    // held not owned

    bslma::Allocator *d_allocator_p;  // memory allocator (held, not owned)

  public:

    enum { LISTENER = 0, CONNECTOR };

    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(TesterFactory, bslma::UsesBslmaAllocator);

    // CREATORS
    TesterFactory(int               mode,
                  bslmt::Barrier   *barrier,
                  bslma::Allocator *basicAllocator = 0);
        // Create a new 'TesterFactory' object of the specified 'mode'.
        // Optionally specify 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.

    virtual ~TesterFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(
                         const bsl::shared_ptr<btlmt::AsyncChannel>& channel,
                         const btlmt::SessionFactory::Callback&      callback);
        // Asynchronously allocate a 'btlmt::Session' object for the specified
        // 'channel', and invoke the specified 'callback' with this session.

    virtual void allocate(btlmt::AsyncChannel                    *channel,
                          const btlmt::SessionFactory::Callback&  callback);
        // Asynchronously allocate a 'btlmt::Session' object for the specified
        // 'channel', and invoke the specified 'callback' with this session.

    virtual void deallocate(btlmt::Session *session);
        // Deallocate the specified 'session'.

    TesterSession *session() const;
        // Return the session managed by this factory.
};

                            // -------------------
                            // class TesterSession
                            // -------------------

// CREATORS
TesterSession::TesterSession(
                          const bsl::shared_ptr<btlmt::AsyncChannel>&  channel,
                          bslmt::Barrier                              *barrier)
: d_channel_sp(channel)
, d_barrier_p(barrier)
{
}

TesterSession::~TesterSession()
{
}

// MANIPULATORS
void TesterSession::readCb(int         result,
                           int        *numNeeded,
                           btlb::Blob *blob,
                           int         )
{
    if (veryVerbose) {
        MTCOUT << "Read callback called with: " << result << MTENDL;
    }

    if (result) {
        d_channel_sp->close();
        return;                                                       // RETURN
    }

    ASSERT(0 == d_channel_sp->write(*blob));

    *numNeeded = 1;
    btlb::BlobUtil::erase(blob, 0, blob->length());

    bslmt::ThreadUtil::Handle handle(bslmt::ThreadUtil::invalidHandle());

    ASSERT(0 == bslmt::ThreadUtil::create(&handle,
                                          bdlf::BindUtil::bind(
                                        &TesterSession::delayedChannelAccessor,
                                        this,
                                        d_channel_sp,
                                        d_channel_sp->peerAddress())));

    bslmt::ThreadUtil::detach(handle);
}

int TesterSession::start()
{
    if (veryVerbose) {
        MTCOUT << "Session started" << MTENDL;
    }

    btlmt::AsyncChannel::BlobBasedReadCallback f =
                                 bdlf::MemFnUtil::memFn(&TesterSession::readCb,
                                                        this);

    return d_channel_sp->read(1, f);
}

int TesterSession::stop()
{
    if (veryVerbose) {
        MTCOUT << "Session stopped" << MTENDL;
    }

    d_channel_sp->close();
    d_channel_sp.reset();

    d_barrier_p->wait();

    return 0;
}

// ACCESSORS
btlmt::AsyncChannel *TesterSession::channel() const
{
    return d_channel_sp.get();
}

                        // -------------------
                        // class TesterFactory
                        // -------------------

// CREATORS
TesterFactory::TesterFactory(int               mode,
                             bslmt::Barrier   *barrier,
                             bslma::Allocator *basicAllocator)
: d_mode(mode)
, d_barrier_p(barrier)
, d_session_p(0)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TesterFactory::~TesterFactory()
{
}

// MANIPULATORS
void TesterFactory::allocate(
                          const bsl::shared_ptr<btlmt::AsyncChannel>& channel,
                          const btlmt::SessionFactory::Callback&      callback)
{
    if (veryVerbose) {
        MTCOUT << "TesterFactory::allocate called: " << MTENDL;
        if (LISTENER == d_mode) {
            MTCOUT << "LISTENER" << MTENDL;
        }
        else {
            MTCOUT << "CONNECTOR" << MTENDL;
        }
    }

    d_session_p = new (*d_allocator_p) TesterSession(channel, d_barrier_p);

    callback(0, d_session_p);
}

void TesterFactory::allocate(btlmt::AsyncChannel                    *,
                             const btlmt::SessionFactory::Callback&  )
{
    ASSERT(false); // shouldn't get called
}

void TesterFactory::deallocate(btlmt::Session *session)
{
    d_allocator_p->deleteObject(session);
}

TesterSession *TesterFactory::session() const
{
    return d_session_p;
}

                        // ------------
                        // class Tester
                        // ------------

class Tester {
    btlmt::ChannelPoolConfiguration  d_config;          // pool configuration

    btlmt::SessionPool              *d_sessionPool_p;   // managed pool (owned)

    TesterFactory                    d_sessionFactory;  // factory

    int                              d_portNumber;      // port on which this
                                                        // echo server is
                                                        // listening

    bslma::Allocator                *d_allocator_p;     // memory allocator
                                                        // (held)

  public:

    enum { LISTENER = 0, CONNECTOR };

    // CREATORS
    Tester(int                        mode,
           bslmt::Barrier            *barrier,
           const btlso::IPv4Address&  endPointAddr,
           bslma::Allocator          *basicAllocator = 0);
    ~Tester();

    void poolStateCb(int reason, int source, void *userData);
        // Indicates the status of the whole pool.

    void sessionStateCb(int             state,
                        int             handle,
                        btlmt::Session *session,
                        void           *userData);
        // Per-session state.

    int portNumber() const;

    TesterSession *session() const { return d_sessionFactory.session(); }
};

// CREATORS
Tester::Tester(int                        mode,
               bslmt::Barrier            *barrier,
               const btlso::IPv4Address&  endPointAddr,
               bslma::Allocator          *basicAllocator)
: d_config()
, d_sessionPool_p()
, d_sessionFactory(mode, barrier, basicAllocator)
, d_portNumber(btlso::IPv4Address::k_ANY_PORT)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
    d_config.setMaxThreads(4);                  // 4 I/O threads
    d_config.setMaxConnections(5);
    d_config.setReadTimeout(5.0);               // in seconds
    d_config.setMetricsInterval(10.0);          // seconds
    d_config.setWriteQueueWatermarks(0, 1<<10); // 1Mb
    d_config.setIncomingMessageSizes(1, 100, 1024);

    typedef btlmt::SessionPool::SessionPoolStateCallback SessionPoolStateCb;

    SessionPoolStateCb poolStateCb = bdlf::MemFnUtil::memFn(
                                                          &Tester::poolStateCb,
                                                          this);

    d_sessionPool_p = new (*d_allocator_p) btlmt::SessionPool(d_config,
                                                              poolStateCb,
                                                              basicAllocator);

    btlmt::SessionPool::SessionStateCallback sessionStateCb =
                                bdlf::MemFnUtil::memFn(&Tester::sessionStateCb,
                                                       this);

    ASSERT(0 == d_sessionPool_p->start());

    int handle;

    btlmt::ConnectOptions connectOptions;
    connectOptions.setServerEndpoint(endPointAddr);
    connectOptions.setTimeout(bsls::TimeInterval(1));
    connectOptions.setNumAttempts(5);

    btlmt::ListenOptions listenOpts;
    btlso::IPv4Address   address;
    address.setPortNumber(d_portNumber);
    listenOpts.setServerAddress(address);
    listenOpts.setBacklog(5);

    if (LISTENER == mode) {
        ASSERT(0 == d_sessionPool_p->listen(&handle,
                                            sessionStateCb,
                                            &d_sessionFactory,
                                            listenOpts));

        d_portNumber = d_sessionPool_p->portNumber(handle);
    }
    else {
        ASSERT(0 == d_sessionPool_p->connect(&handle,
                                             sessionStateCb,
                                             &d_sessionFactory,
                                             connectOptions));
        d_portNumber = endPointAddr.portNumber();
    }
}

Tester::~Tester()
{
    d_sessionPool_p->stop();
    d_allocator_p->deleteObject(d_sessionPool_p);
}

void Tester::poolStateCb(int reason, int source, void *)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: (" << reason << ", " << source << ") "
               << MTENDL;
    }
}

void Tester::sessionStateCb(int             state,
                            int             ,
                            btlmt::Session *session,
                            void           *)
{
    switch (state) {
      case btlmt::SessionPool::e_SESSION_DOWN: {
          if (veryVerbose) {
              MTCOUT << "Client from "
                     << session->channel()->peerAddress()
                     << " has disconnected."
                     << MTENDL;
          }
      } break;
      case btlmt::SessionPool::e_SESSION_UP: {
          if (veryVerbose) {
              MTCOUT << "Client connected from "
                     << session->channel()->peerAddress()
                     << MTENDL;
          }
      } break;
    }
}

int Tester::portNumber() const
{
    return d_portNumber;
}

}  // close namespace BTLMT_SESSION_POOL_CASE_ALLOCATE_SHARED_CHANNEL

namespace BTLMT_SESSION_POOL_BUSY_METRICS {

using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

void runTestFunction(Obj                     *mX,
                     SessionStateCallback    *scb,
                     SessionFactory          *sessionFactory,
                     InetStreamSocketFactory *socketFactory,
                     bslmt::Barrier          *barrier,
                     bool                     expZeroBusyMetrics,
                     int                      numIters)
    // Create a socket pair and import the server socket created using the
    // specified 'socketFactory' into the specified 'mX' session pool using as
    // arguments the specified 'scb' and 'sessionFactory'.  Write data into the
    // client end of the socket pair the specified 'numIters' times and confirm
    // that the value returned by the 'busyMetrics' method matches the
    // specified 'expZeroBusyMetrics'.
{
    btlso::SocketHandle::Handle handles[2];

    int ret = btlso::SocketImpUtil::socketPair<IPAddress>(
                                        handles,
                                        btlso::SocketImpUtil::k_SOCKET_STREAM);

    ASSERT(0 == ret);

    btlso::StreamSocket<IPAddress> *serverSocket =
                                           socketFactory->allocate(handles[0]);
    ASSERT(serverSocket);
    btlso::StreamSocket<IPAddress> *clientSocket =
                                           socketFactory->allocate(handles[1]);
    ASSERT(clientSocket);

    int handle;
    ASSERT(0 == mX->import(&handle,
                           *scb,
                           serverSocket,
                           socketFactory,
                           sessionFactory));

    barrier->wait();

    const int NUM_BYTES = 1024 * 1024;
    bsl::vector<char> buffer(NUM_BYTES);

    for (int i = 0; i < numIters; ++i) {
        int rc = clientSocket->write(buffer.data(), NUM_BYTES);
        const int percent = mX->busyMetrics();
        if (0 != percent) {
            if (expZeroBusyMetrics) {
                ASSERT(false);
            }
            return;                                                   // RETURN
        }
        if (rc <= 0) {
            bslmt::ThreadUtil::microSleep(10, 0);
        }
    }
    ASSERT(expZeroBusyMetrics);
}

}  // close namespace BTLMT_SESSION_POOL_BUSY_METRICS

//=============================================================================
//                                USAGE EXAMPLE
//-----------------------------------------------------------------------------

namespace BTLMT_SESSION_POOL_USAGE_EXAMPLE {

///Usage example
///-------------
// The following example implements a simple echo server.  This server accepts
// connections, reads what it receives right away from the network stream,
// sends it back and closes the connection.
//
// To set up this server, users must create a concrete session class derived
// from 'btlmt::Session' protocol, and a factory for creating instances of this
// concrete session type.  'my_EchoSession' objects are created by a factory
// that must be derived from 'btlmt::SessionFactory'.  A
// 'my_EchoSessionFactory' just allocates and deallocates sessions (with no
// pooling or allocation strategy).  This is the simplest form of factory.
//..
    // my_echoserver.h

                            // ====================
                            // class my_EchoSession
                            // ====================

   class my_EchoSession : public btlmt::Session {
       // This class is a concrete implementation of the 'btlmt::Session'
       // protocol to use along with 'my_EchoServer' objects.

       // DATA
       btlmt::AsyncChannel *d_channel_p;// underlying channel (held, not owned)

       // PRIVATE MANIPULATORS
       void readCb(int         result,
                   int        *numNeeded,
                   btlb::Blob *blob,
                   int         channelId);
           // Read callback for session pool.

     private:
       // NOT IMPLEMENTED
       my_EchoSession(const my_EchoSession&);

       my_EchoSession& operator=(const my_EchoSession&);
     public:
       // CREATORS
       my_EchoSession(btlmt::AsyncChannel *channel);
           // Create a new 'my_EchoSession' object for the specified 'channel'.

       ~my_EchoSession();
           // Destroy this object.

       // MANIPULATORS
       virtual int start();
           // Begin the asynchronous operation of this session.

       virtual int stop();
           // Stop the operation of this session.

       // ACCESSORS
       virtual btlmt::AsyncChannel *channel() const;
           // Return the channel associate with this session.
    };

                        // ===========================
                        // class my_EchoSessionFactory
                        // ===========================

    class my_EchoSessionFactory : public btlmt::SessionFactory {
        // This class is a concrete implementation of the
        // 'btlmt::SessionFactory' that simply allocates 'my_EchoSession'
        // objects.  No specific allocation strategy (such as pooling) is
        // implemented.

        bslma::Allocator *d_allocator_p; // memory allocator (held, not owned)

      public:
        // TRAITS
        BSLMF_NESTED_TRAIT_DECLARATION(my_EchoSessionFactory,
                                       bslma::UsesBslmaAllocator);

        // CREATORS
        my_EchoSessionFactory(bslma::Allocator *basicAllocator = 0);
            // Create a new 'my_EchoSessionFactory' object.  Optionally specify
            // 'basicAllocator' used to supply memory.  If 'basicAllocator' is
            // 0, the currently installed default allocator is used.

        virtual ~my_EchoSessionFactory();
           // Destroy this factory.

        // MANIPULATORS
        virtual void allocate(btlmt::AsyncChannel                   *channel,
                              const btlmt::SessionFactory::Callback& callback);
           // Asynchronously allocate a 'btlmt::Session' object for the
           // specified 'channel', and invoke the specified 'callback' with
           // this session.

        virtual void deallocate(btlmt::Session *session);
           // Deallocate the specified 'session'.
    };
//..
// The implementations of those session and factory types are rather
// straightforward.  'readCb' will be called when the first byte is received.
// It is in this method that the echo logic is implemented.
//..
    // my_echoserver.cpp

                            // --------------------
                            // class my_EchoSession
                            // --------------------

    // PRIVATE MANIPULATORS
    void my_EchoSession::readCb(int           result,
                                int          *numNeeded,
                                btlb::Blob *blob,
                                int           )
    {
        if (result) {
            d_channel_p->close();
            return;                                                   // RETURN
        }

        ASSERT(numNeeded);
        ASSERT(blob);
        ASSERT(0 < blob->length());

        int rc = d_channel_p->write(*blob);
        ASSERT(0 == rc);

        *numNeeded   = 1;
        btlb::BlobUtil::erase(blob, 0, blob->length());

        d_channel_p->close(); // close connection.
    }

    // CREATORS
    my_EchoSession::my_EchoSession(btlmt::AsyncChannel *channel)
    : d_channel_p(channel)
    {
    }

    my_EchoSession::~my_EchoSession()
    {
    }

    // MANIPULATORS
    int my_EchoSession::start()
    {
        btlmt::AsyncChannel::BlobBasedReadCallback
                       callback(bdlf::MemFnUtil::memFn(&my_EchoSession::readCb,
                                                       this));
        return d_channel_p->read(1, callback);
    }

    int my_EchoSession::stop()
    {
        d_channel_p->close();
        return 0;
    }

    // ACCESSORS
    btlmt::AsyncChannel *my_EchoSession::channel() const
    {
        return d_channel_p;
    }

                        // ---------------------------
                        // class my_EchoSessionFactory
                        // ---------------------------

    // CREATORS
    my_EchoSessionFactory::my_EchoSessionFactory(
                                              bslma::Allocator *basicAllocator)
    : d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
    }

    my_EchoSessionFactory::~my_EchoSessionFactory()
    {
    }

    // MANIPULATORS
    void
    my_EchoSessionFactory::allocate(
                              btlmt::AsyncChannel                    *channel,
                              const btlmt::SessionFactory::Callback&  callback)
    {
        my_EchoSession *session = new (*d_allocator_p) my_EchoSession(channel);
        callback(0, session);
    }

    void
    my_EchoSessionFactory::deallocate(btlmt::Session *session)
    {
        d_allocator_p->deleteObjectRaw(session);
    }
//..
//  We now have all the pieces needed to design and implement our echo server.
//  The server itself owns an instance of the above-defined factory.
//..
    // my_echoserver.h (continued)

                        // ===================
                        // class my_EchoServer
                        // ===================

    class my_EchoServer {
        // This class implements a multi-user multi-threaded echo server.

        // DATA
        btlmt::ChannelPoolConfiguration d_config;         // pool
                                                          // configuration

        btlmt::SessionPool             *d_sessionPool_p;  // managed pool
                                                          // (owned)

        my_EchoSessionFactory          d_sessionFactory;  // my_EchoSession
                                                          // factory

        int                            d_portNumber;      // port on which this
                                                          // echo server is
                                                          // listening

        bslmt::Mutex                  *d_coutLock_p;      // mutex protecting
                                                          // bsl::cout

        bslma::Allocator              *d_allocator_p;     // memory allocator
                                                          // (held)

        // PRIVATE MANIPULATORS
        void poolStateCb(int reason, int source, void *userData);
            // Indicates the status of the whole pool.

        void sessionStateCb(int             state,
                            int             handle,
                            btlmt::Session *session,
                            void           *userData);
            // Per-session state.

      private:
        // NOT IMPLEMENTED
        my_EchoServer(const my_EchoServer& original);
        my_EchoServer& operator=(const my_EchoServer& rhs);

      public:
        // TRAITS
        BSLMF_NESTED_TRAIT_DECLARATION(my_EchoServer,
                                       bslma::UsesBslmaAllocator);

        // CREATORS
        my_EchoServer(bslmt::Mutex     *coutLock,
                      int               portNumber,
                      int               numConnections,
                      bool              reuseAddressFlag,
                      bslma::Allocator *basicAllocator = 0);
            // Create an echo server that listens for incoming connections on
            // the specified 'portNumber' managing up to the specified
            // 'numConnections' simultaneous connections.  The echo server will
            // use the specified 'coutLock' to synchronize access to the
            // standard output.  Optionally specify a 'basicAllocator' used to
            // supply memory.  If 'basicAllocator' is 0, the currently
            // installed default allocator is used.  The behavior is undefined
            // unless coutLock is not 0.

        ~my_EchoServer();
            // Destroy this server.

        // MANIPULATORS
        const btlmt::SessionPool& pool() const;
            // Return a non-modifiable reference to the session pool used by
            // this echo server.

        int portNumber() const;
            // Return the actual port number on which this server is listening.
    };
//..
// Note that this example server prints information depending on
// implicitly-defined static variables and therefore must use a mutex to
// synchronize access to 'bsl::cout'.  A production application should use a
// proper logging mechanism instead such as the 'bael' logger.
//..
    // my_echoserver.h (continued)

                            // -------------------
                            // class my_EchoServer
                            // -------------------

    // PRIVATE MANIPULATORS
    void my_EchoServer::poolStateCb(int reason, int source, void *)
    {
        if (veryVerbose) {
            d_coutLock_p->lock();
            bsl::cout << "Pool state changed: (" << reason << ", " << source
                      << ") " << bsl::endl;
            d_coutLock_p->unlock();
        }
    }

    void my_EchoServer::sessionStateCb(int             state,
                                       int             ,
                                       btlmt::Session *session,
                                       void           *) {

        switch(state) {
          case btlmt::SessionPool::e_SESSION_DOWN: {
              if (veryVerbose) {
                  d_coutLock_p->lock();
                  bsl::cout << "Client from "
                            << session->channel()->peerAddress()
                            << " has disconnected."
                            << bsl::endl;
                  d_coutLock_p->unlock();
              }
          } break;
          case btlmt::SessionPool::e_SESSION_UP: {
              if (veryVerbose) {
                  d_coutLock_p->lock();
                  bsl::cout << "Client connected from "
                            << session->channel()->peerAddress()
                            << bsl::endl;
                  d_coutLock_p->unlock();
              }
          } break;
        }
    }

    // CREATORS
    my_EchoServer::my_EchoServer(bslmt::Mutex     *lock,
                                 int               portNumber,
                                 int               numConnections,
                                 bool              reuseAddressFlag,
                                 bslma::Allocator *basicAllocator)
    : d_sessionFactory(basicAllocator)
    , d_coutLock_p(lock)
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
        d_config.setMaxThreads(4);                  // 4 I/O threads
        d_config.setMaxConnections(numConnections);
        d_config.setReadTimeout(5.0);               // in seconds
        d_config.setMetricsInterval(10.0);          // seconds
        d_config.setWriteQueueWatermarks(0, 1<<10); // 1Mb
        d_config.setIncomingMessageSizes(1, 100, 1024);

        typedef btlmt::SessionPool::SessionPoolStateCallback
                                                            SessionPoolStateCb;

        SessionPoolStateCb poolStateCb = bdlf::MemFnUtil::memFn(
                                            &my_EchoServer::poolStateCb, this);

        d_sessionPool_p = new (*d_allocator_p) btlmt::SessionPool(
                                                               d_config,
                                                               poolStateCb,
                                                               basicAllocator);

        btlmt::SessionPool::SessionStateCallback sessionStateCb =
                         bdlf::MemFnUtil::memFn(&my_EchoServer::sessionStateCb,
                                                this);

        int rc = d_sessionPool_p->start();
        ASSERT(0 == rc);

        int handle;

        btlmt::ListenOptions listenOpts;
        btlso::IPv4Address   address;
        address.setPortNumber(portNumber);
        listenOpts.setServerAddress(address);
        listenOpts.setBacklog(numConnections);
        btlso::SocketOptions socketOpts;
        socketOpts.setReuseAddress(reuseAddressFlag);
        listenOpts.setSocketOptions(socketOpts);

        ASSERT(0 == d_sessionPool_p->listen(&handle,
                                            sessionStateCb,
                                            &d_sessionFactory,
                                            listenOpts));

        d_portNumber = d_sessionPool_p->portNumber(handle);
    }

    my_EchoServer::~my_EchoServer()
    {
        d_sessionPool_p->stop();
        d_allocator_p->deleteObjectRaw(d_sessionPool_p);
    }

    // ACCESSORS
    const btlmt::SessionPool& my_EchoServer::pool() const
    {
        return *d_sessionPool_p;
    }

    int my_EchoServer::portNumber() const
    {
        return d_portNumber;
    }
//..
// We can implement a simple "Hello World!" example to exercise our echo
// server.
//..
    int usageExample(bslma::Allocator *) {

        enum {
            BACKLOG = 5,
            REUSE   = 1
        };

        my_EchoServer echoServer(&coutMutex, 0, BACKLOG, REUSE);

        btlso::InetStreamSocketFactory<btlso::IPv4Address> factory;
        btlso::StreamSocket<btlso::IPv4Address> *socket = factory.allocate();

        const char STRING[] = "Hello World!";

        const btlso::IPv4Address ADDRESS("127.0.0.1", echoServer.portNumber());

        int rc = socket->connect(ADDRESS);
        ASSERT(0 == rc);

        int numBytes = socket->write(STRING, sizeof(STRING));
        ASSERT(sizeof(STRING) == numBytes);

        char readBuffer[sizeof(STRING)];
        numBytes = socket->read(readBuffer, sizeof(STRING));
        ASSERT(sizeof(STRING) == numBytes);
        ASSERT(0 == bsl::strcmp(readBuffer, STRING));

        factory.deallocate(socket);
        return 0;
    }
//..

}  // close namespace BTLMT_SESSION_POOL_USAGE_EXAMPLE

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? bsl::atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;
    veryVeryVerbose = argc > 4;

    bsl::cout << "TEST " << __FILE__ << " CASE " << test << bsl::endl;

    bslma::TestAllocator ta("ta", veryVeryVerbose);

    switch (test) { case 0:  // Zero is always the leading case.
      case 20: {
        // --------------------------------------------------------------------
        // TEST USAGE EXAMPLE
        //   The usage example from the header has been incorporated into this
        //   test driver.  All references to 'assert' have been replaced with
        //   'ASSERT'.  Call the test example function and assert that it works
        //   as expected.
        //
        // Plan:
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "USAGE EXAMPLE" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_USAGE_EXAMPLE;

        usageExample(&ta);
        ASSERT(0 == ta.numBytesInUse());
        ASSERT(0 == ta.numMismatches());

      } break;
      case 19: {
        // --------------------------------------------------------------------
        // TESTING 'isRunning'
        //
        // Concerns:
        //: 1 'isRunning' returns 'false' for a session pool object on which
        //:   'start' has either never been called or has not been called
        //:   following a 'stop' call.
        //:
        //: 2 'isRunning' returns 'true' for a session pool object on which
        //:   'start' has been called without a subsequent 'stop' call.
        //
        // Plan:
        //: 1 Create a session pool object. Confirm that 'isRunning' returns
        //:   'false'.
        //:
        //: 2 Call 'start' on that object and confirm that 'isRunning' returns
        //:   'true'.
        //:
        //: 3 Call 'start' and 'stop' alternately and confirm that 'isRunning'
        //:   returns the correct value.
        //
        // Testing:
        //   bool isRunning() const;
        // --------------------------------------------------------------------

        if (verbose) cout << "TESTING 'isRunning'" << endl
                          << "===================" << endl;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(3);

        typedef btlmt::SessionPool::SessionPoolStateCallback
                                                            SessionPoolStateCb;

        SessionPoolStateCb poolCb;

        Obj mX(config, poolCb);  const btlmt::SessionPool& X = mX;

        ASSERT(false == X.isRunning());

        mX.start();
        ASSERT(true == X.isRunning());

        mX.stop();
        ASSERT(false == X.isRunning());

        mX.start();
        ASSERT(true == X.isRunning());

        mX.stopAndRemoveAllSessions();
        ASSERT(false == X.isRunning());
      } break;
      case 18: {
        // --------------------------------------------------------------------
        // Testing 'busyMetrics'
        //
        // Concerns:
        //: 1 The 'busyMetrics' method returns 0 if the session pool object has
        //:   not been started.
        //:
        //: 2 The 'busyMetrics' method returns 0 if the 'collectTimeMetrics' is
        //:   'false' (QoI).
        //:
        //: 3 If session pool was started then 'busyMetrics' returns an
        //:   accurate reflection of the percent of time spent processing data.
        //
        // Plan:
        //: This function simply forwards to the underlying channel pool for
        //: the value returned.  We will follow these steps:
        //:
        //: 1 Create a channel pool configuration object.  Set the
        //:   'metricsInterval' to a small value.
        //:
        //: 2 Set the 'collectTimeMetrics' attribute to 'false'.
        //:
        //: 3 Create a session pool object, mX.  Confirm that 'busyMetrics'
        //:   returns 0.
        //:
        //: 4 Start mX, establish a connection, and write some data to the
        //:   connection.  Confirm that after each step 'busyMetrics' returns
        //:   0.
        //:
        //: 5 Set the 'collectTimeMetrics' attribute to 'true'.
        //:
        //: 6 Follow steps 3-5 on a new session pool object and confirm that
        //:   'busyMetrics' returns a non-zero value.
        //
        // Testing:
        //    int busyMetrics() const;
        // --------------------------------------------------------------------

        if (verbose) cout << "Testing 'busyMetrics'" << endl
                          << "=====================" << endl;

        using namespace BTLMT_SESSION_POOL_BUSY_METRICS;

        typedef btlmt::SessionPool::SessionStateCallback     SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(3);
        config.setWriteQueueWatermarks(0, 1024 * 1024 * 100);  // 100 Mb
        config.setMetricsInterval(.1);

        btlso::InetStreamSocketFactory<IPAddress> socketFactory;
        BlobReadCallback rcb(&readCb);
        TestFactory sessionFactory(&rcb);

        // Set collectMetrics 'false'
        config.setCollectTimeMetrics(false);

        {
            Obj mX(config, &poolStateCallback);  const Obj& X = mX;
            ASSERT(0 == X.busyMetrics());

            ASSERT(0 == mX.start());
            ASSERT(0 == mX.stop());
            ASSERT(0 == X.busyMetrics());
        }

        {
            Obj mX(config, &poolStateCallback);  const Obj& X = mX;

            ASSERT(0 == mX.start());

            bslmt::Barrier barrier(2);

            SessionCb sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);

            runTestFunction(&mX, &sessionStateCb, &sessionFactory,
                            &socketFactory, &barrier, true, 100);

            bslmt::ThreadUtil::microSleep(100, 0);

            ASSERT(0 == X.busyMetrics());
        }

        // Set collectMetrics 'true'
        config.setCollectTimeMetrics(true);

        {
            Obj mX(config, &poolStateCallback);  const Obj& X = mX;
            ASSERT(0 == X.busyMetrics());

            ASSERT(0 == mX.start());
            ASSERT(0 == mX.stop());
            ASSERT(0 == X.busyMetrics());
        }

        {
            Obj mX(config, &poolStateCallback);  const Obj& X = mX;

            ASSERT(0 == mX.start());

            bslmt::Barrier barrier(2);

            SessionCb sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);

            runTestFunction(&mX, &sessionStateCb, &sessionFactory,
                            &socketFactory, &barrier, false, 1000);

            bslmt::ThreadUtil::microSleep(0, 1);

            ASSERT(0 == X.busyMetrics());
        }
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // TESTING close with and without enqueued data
        //
        // Concerns:
        //: 1 Calling 'close' on an AsyncChannel ensures that the channel is
        //:   not closed till there is any write data enqueued.
        //:
        //: 2 Calling 'close' on an AsyncChannel that does not have any write
        //:   data enqueued results in an immediate channel shutdown.
        //:
        //: 3 Calling 'close' on an AsyncChannel works irrespective of if the
        //:   channel is an accepting or connecting channel.
        //
        // Plan:
        //: 1 Create a listening socket on a SessionPool object and connect to
        //:   it from another socket.  Write a large amount of data on the
        //:   AsyncChannel using it's 'write' method such that the call fails
        //:   and enqueues data to be subsequently written.  Call 'close'
        //:   immediately after the 'write' call.  Confirm that the channel is
        //:   closed (via a SESSION_DOWN callback) only after all the data has
        //:   been successfully written to the connection.
        //:
        //: 2 Create a listening socket on a SessionPool object and connect to
        //:   it from another socket.  Call 'close' immediately after that
        //:   channel receives some data.  Confirm that the channel is closed
        //:   (via a SESSION_DOWN callback) immediately.
        //:
        //: 3 Perform steps 1 and 2 for a connecting channel and verify that
        //:   the channel is closed as expected.
        //
        // Testing:
        //-------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING CLOSE WITH AND WITHOUT "
                               << "ENQUEUED DATA" << bsl::endl
                               << "=============================="
                               << "============="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GRACEFUL_SHUTDOWN;

        if (verbose) bsl::cout << "Testing on accepting socket "
                               << "with data enqueued." << bsl::endl;
        {
            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);                  // 4 I/O threads
            config.setWriteQueueWatermarks(0, 1<<10); // 1KB

            Obj pool(config, poolStateCallback);

            bslmt::Barrier barrier(2);
            int            state;

            s_barrier_p   = &barrier;
            s_state_p     = &state;
            s_enqueueData = true;

            ASSERT(0 == pool.start());

            int handle;
            TesterFactory sessionFactory;

            btlso::IPv4Address serverAddress;

            btlmt::ListenOptions listenOpts;
            listenOpts.setServerAddress(serverAddress);
            listenOpts.setBacklog(5);
            listenOpts.setAllowHalfOpenConnections(true);
            btlso::SocketOptions socketOpts;
            socketOpts.setReuseAddress(true);
            listenOpts.setSocketOptions(socketOpts);

            ASSERT(0 == pool.listen(&handle,
                                    sessionStateCb,
                                    &sessionFactory,
                                    listenOpts));

            const int PORTNUM = pool.portNumber(handle);

            btlso::IPv4Address address("127.0.0.1",
                                       btlso::IPv4Address::k_ANY_PORT);
            address.setPortNumber(PORTNUM);

            InetStreamSocketFactory  factory;
            StreamSocket            *socket = factory.allocate();

            ASSERT(0 == socket->connect(address));

            barrier.wait();

            ASSERT(Obj::e_SESSION_UP == state);

            const char STRING[] = "Hello World!";

            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            // Wait for read callback to be called.

            barrier.wait();

            const int BUFSIZE = 1024 * 1024;  // 1 MB
            char buffer[BUFSIZE];

            int totalDataRead = 0;
            while (totalDataRead < WRITE_SIZE) {
                const int rc = socket->read(buffer, BUFSIZE);
                if (rc > 0) {
                    totalDataRead += rc;
                }
            }

            // Wait for SESSION_DOWN to be called.

            barrier.wait();

            if (veryVerbose) {
                MTCOUT << "Bringing down the channel" << MTENDL;
            }

            socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

            factory.deallocate(socket);
        }

        if (verbose) bsl::cout << "Testing on accepting socket "
                               << "with NO data enqueued." << bsl::endl;
        {
            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);                  // 4 I/O threads
            config.setWriteQueueWatermarks(0, 1<<10); // 1KB

            Obj pool(config, poolStateCallback);

            bslmt::Barrier barrier(2);
            int            state;

            s_barrier_p   = &barrier;
            s_state_p     = &state;
            s_enqueueData = false;

            ASSERT(0 == pool.start());

            int handle;
            TesterFactory sessionFactory;

            btlso::IPv4Address serverAddress;
            btlmt::ListenOptions listenOpts;
            listenOpts.setServerAddress(serverAddress);
            listenOpts.setBacklog(5);
            listenOpts.setAllowHalfOpenConnections(true);
            btlso::SocketOptions socketOpts;
            socketOpts.setReuseAddress(true);
            listenOpts.setSocketOptions(socketOpts);

            ASSERT(0 == pool.listen(&handle,
                                    sessionStateCb,
                                    &sessionFactory,
                                    listenOpts));

            const int PORTNUM = pool.portNumber(handle);

            btlso::IPv4Address address("127.0.0.1",
                                       btlso::IPv4Address::k_ANY_PORT);
            address.setPortNumber(PORTNUM);

            InetStreamSocketFactory  factory;
            StreamSocket            *socket = factory.allocate();

            ASSERT(0 == socket->connect(address));

            barrier.wait();

            ASSERT(Obj::e_SESSION_UP == state);

            const char STRING[] = "Hello World!";

            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            // Wait for read callback to be called.

            barrier.wait();

            // Wait for SESSION_DOWN to be called.

            barrier.wait();

            if (veryVerbose) {
                MTCOUT << "Bringing down the channel" << MTENDL;
            }

            socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

            factory.deallocate(socket);
        }

        if (verbose) bsl::cout << "Testing on connecting socket "
                               << "with data enqueued." << bsl::endl;
        {
            InetStreamSocketFactory  factory;
            StreamSocket            *serverSocket = factory.allocate();

            ASSERT(0 == serverSocket->bind(getLocalAddress()));
            ASSERT(0 == serverSocket->listen(1));

            IPAddress serverAddr;
            ASSERT(0 == serverSocket->localAddress(&serverAddr));

            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);                  // 4 I/O threads
            config.setWriteQueueWatermarks(0, 1<<10); // 1KB

            Obj pool(config, poolStateCallback);

            ASSERT(0 == pool.start());

            bslmt::Barrier barrier(2);
            int            state;

            s_barrier_p   = &barrier;
            s_state_p     = &state;
            s_enqueueData = true;

            TesterFactory sessionFactory;

            btlmt::ConnectOptions connectOptions;
            connectOptions.setServerEndpoint(serverAddr);
            connectOptions.setTimeout(bsls::TimeInterval(1));
            connectOptions.setNumAttempts(1);
            connectOptions.setAllowHalfOpenConnections(true);

            int handleBuffer;
            int rc = pool.connect(&handleBuffer,
                                  sessionStateCb,
                                  &sessionFactory,
                                  connectOptions);
            ASSERT(0 == rc);

            StreamSocket *socket;
            do {
                rc = serverSocket->accept(&socket);
            } while (rc);

            barrier.wait();

            ASSERT(Obj::e_SESSION_UP == state);

            const char STRING[] = "Hello World!";

            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            // Wait for read callback to be called.

            barrier.wait();

            const int BUFSIZE = 1024 * 1024;  // 1 MB
            char buffer[BUFSIZE];

            int totalDataRead = 0;
            while (totalDataRead < WRITE_SIZE) {
                const int rc = socket->read(buffer, BUFSIZE);
                if (rc > 0) {
                    totalDataRead += rc;
                }
            }

            // Wait for SESSION_DOWN to be called.

            barrier.wait();

            if (veryVerbose) {
                MTCOUT << "Bringing down the channel" << MTENDL;
            }

            socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

            factory.deallocate(socket);
        }

        if (verbose) bsl::cout << "Testing on connecting socket "
                               << "with NO data enqueued." << bsl::endl;
        {
            InetStreamSocketFactory  factory;
            StreamSocket            *serverSocket = factory.allocate();

            ASSERT(0 == serverSocket->bind(getLocalAddress()));
            ASSERT(0 == serverSocket->listen(1));

            IPAddress serverAddr;
            ASSERT(0 == serverSocket->localAddress(&serverAddr));

            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);                  // 4 I/O threads
            config.setWriteQueueWatermarks(0, 1<<10); // 1KB

            Obj pool(config, poolStateCallback);

            ASSERT(0 == pool.start());

            bslmt::Barrier barrier(2);
            int            state;

            s_barrier_p   = &barrier;
            s_state_p     = &state;
            s_enqueueData = false;

            TesterFactory sessionFactory;

            btlmt::ConnectOptions connectOptions;
            connectOptions.setServerEndpoint(serverAddr);
            connectOptions.setTimeout(bsls::TimeInterval(1));
            connectOptions.setNumAttempts(1);
            connectOptions.setAllowHalfOpenConnections(true);

            int handleBuffer;
            int rc = pool.connect(&handleBuffer,
                                  sessionStateCb,
                                  &sessionFactory,
                                  connectOptions);
            ASSERT(0 == rc);

            StreamSocket *socket;
            do {
                rc = serverSocket->accept(&socket);
            } while (rc);

            barrier.wait();

            ASSERT(Obj::e_SESSION_UP == state);

            const char STRING[] = "Hello World!";

            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            // Wait for read callback to be called.

            barrier.wait();

            // Wait for SESSION_DOWN to be called.

            barrier.wait();

            if (veryVerbose) {
                MTCOUT << "Bringing down the channel" << MTENDL;
            }

            socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

            factory.deallocate(socket);
        }
      } break;
      case 16: {
        // --------------------------------------------------------------------
        // CHECKING MEMORY USAGE
        //
        // Concerns:
        //: 1 Memory allocated and used by SessionPool does not grow beyond
        //:   bound.
        //
        // Plan:
        //: 1 Create a SessionPool object providing it a 'btlb::Blob'-based
        //:   read callback that reads only a specified amount of bytes during
        //:   each invocation.  Also provide a test allocator to measure the
        //:   memory used by the object.
        //:
        //: 2 Establish a connection and write a large amount of data through
        //:   it.
        //:
        //: 3 After all the data is read check that the amount of memory used
        //:   is below the expected threshold.
        //:
        //: 4 Repeat steps 1 - 3 for different values of bytes read in the read
        //:   callback.
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "CHECKING MEMORY USAGE" << bsl::endl
                               << "=====================" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        typedef btlmt::SessionPool::SessionPoolStateCallback SessionPoolStateCb;
        typedef btlmt::SessionPool::SessionStateCallback     SessionStateCb;

        const int MAX_BUF_SIZE = 1024;  // 1K buffers
        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(1);
        config.setIncomingMessageSizes(1, 1, MAX_BUF_SIZE);

        // ChannelPool allocates a maximum of 32 buffers each of the max
        // incoming message size (MAX_BUF_SIZE in this test case).  But
        // because Blob uses a pool that does an exponential increase the
        // memory used may be double the maximum memory. We set the
        // threshold to double of that amount just to be safe.

        const int MAX_NUM_BYTES = 4 * 32 * MAX_BUF_SIZE;
        const int DATA_READ_SIZES[] = { 8, 16, 32, 128, 512,
                                        1024, 4096, 8192, 16396 };
        const size_t NUM_DATA_READ_SIZES = sizeof DATA_READ_SIZES /
                                                       sizeof *DATA_READ_SIZES;

        const int TOTAL_DATA_SIZE = 1024 * 1024;

        for (size_t j = 0; j < NUM_DATA_READ_SIZES; ++j) {
            const int DATA_READ_SIZE = DATA_READ_SIZES[j];

            bslmt::Barrier barrier(2);
            BlobReadCallback callback;
            BlobReadCallback cb(bdlf::BindUtil::bind(&readCbWithReadSize,
                                                     _1,
                                                     _2,
                                                     _3,
                                                     _4,
                                                     DATA_READ_SIZE,
                                                     TOTAL_DATA_SIZE,
                                                     &barrier));
            callback = cb;

            TestFactory factory(&callback);

            SessionPoolStateCb poolCb         = &poolStateCallback;
            SessionStateCb     sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);

            bslma::TestAllocator testAllocator;
            Obj mX(config, poolCb, &testAllocator);

            int rc = mX.start();
            ASSERT(0 == rc);

            int handle;
            btlso::SocketOptions socketOptions;
            socketOptions.setReuseAddress(true);

            btlmt::ListenOptions listenOptions;
            listenOptions.setBacklog(5);
            listenOptions.setSocketOptions(socketOptions);

            rc = mX.listen(&handle,
                           sessionStateCb,
                           &factory,
                           listenOptions);
            ASSERT(0 == rc);

            const int PORTNUM = mX.portNumber(handle);

            btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

            btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;
            btlso::StreamSocket<btlso::IPv4Address> *socket =
            socketFactory.allocate();

            rc = socket->connect(ADDRESS);
            ASSERT(0 == rc);

            barrier.wait();

            int numWritten = 0;

            // Write 1 MB
            const int BUF_SIZE = 32768;
            const char BUFFER[BUF_SIZE] = { 'Z' };

            while (numWritten < TOTAL_DATA_SIZE) {
                rc = socket->write(BUFFER, BUF_SIZE);
                if (rc > 0) {
                    numWritten += rc;
                }
            }

            barrier.wait();
            socketFactory.deallocate(socket);
            const int TA_MAX_BYTES = testAllocator.numBytesMax();
            if (veryVerbose) {
                P_(DATA_READ_SIZE) P(TA_MAX_BYTES)
            }
            LOOP2_ASSERT(TA_MAX_BYTES, MAX_NUM_BYTES,
                         TA_MAX_BYTES < MAX_NUM_BYTES);
        }
      } break;
      case 15: {
        // --------------------------------------------------------------------
        // TESTING: platform errors are returned correctly
        //
        // Concerns:
        //: 1 Synchronous errors in channel pool methods are returned correctly
        //:   through an optionally-specified argument.
        //:
        //: 2 Asynchronous errors in channel pool methods are returned
        //:   correctly through the pool state callback's third argument.
        //
        // Plan:
        //: 1 Invoke 'connect' and 'listen' methods passing arguments that
        //:   cause the operation to fail.
        //:
        //: 2 Verify that for synchronous failures the optionally-specified
        //:   error argument has the platform-specific error in it.  For
        //:   asynchrounous errors confirm that the pool state callback's
        //:   third arugment contains the platform error.
        //
        // Testing:
        //-------------------------------------------------------------------

      } break;
      case 14: {
        // --------------------------------------------------------------------
        // Test Constructor taking BlobBufferFactory
        //
        // Concerns:
        //: 1 Blob buffers are dispensed from the provided factory and outlive
        //:   the session pool.
        //:
        //: 2 No memory leaks in this usage mode
        //
        // Plan:
        //: 1 Create a BlobBufferFactory.
        //:
        //: 2 Create a TestAllocator and an enclosing scope for the
        //:   session pool.
        //:
        //: 3 Using the scope and allocator from step 2, and the blob buffer
        //:   factory from step 1, create a session pool object.
        //:
        //: 4 Connecting to a listening socket configured in step 3.
        //:
        //: 5 Send a message through the socket to a callback that saves the
        //:   resulting blob outside the scope created in step 2.
        //:
        //: 6 Shut down the pool and close the scope from step 2.
        //:
        //: 7 Verify the contents of the blob are still valid, and that no
        //:   memory has been leaked.
        //:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "EXTERNAL BLOB BUFFER FACTORY" << bsl::endl
                               << "============================" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        bslma::DefaultAllocatorGuard defaultAllocGuard(&ta);
        bslma::TestAllocator pa("poolAllocator", veryVeryVerbose);

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(4);

        const int      SIZE = 88;

        btlb::PooledBlobBufferFactory blobFactory(SIZE/2);
        btlb::Blob                    blob;
        bslmt::Barrier                barrier(2);

        BlobReadCallback callback = bdlf::BindUtil::bind(
                                                    &readCbWithBlobAndBarrier,
                                                    _1,
                                                    _2,
                                                    _3,
                                                    _4,
                                                    &blob,
                                                    &barrier);
        TestFactory factory(&callback);

        typedef btlmt::SessionPool::SessionPoolStateCallback PoolStateCb;
        typedef btlmt::SessionPool::SessionStateCallback     SessionStateCb;

        SessionStateCb sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);
        const char FILL = 'X';
        BSLS_ASSERT(0 == SIZE % 2); // test invariant
        {
            Obj mX(&blobFactory, config, &poolStateCallback, &pa);

            int rc = mX.start();
            ASSERT(0 == rc);

            btlmt::ListenOptions listenOpts;
            listenOpts.setServerAddress(btlso::IPv4Address());
            listenOpts.setBacklog(5);
            listenOpts.setAllowHalfOpenConnections(false);
            btlso::SocketOptions socketOpts;
            socketOpts.setReuseAddress(true);
            listenOpts.setSocketOptions(socketOpts);

            int handle;
            rc = mX.listen(&handle,
                           sessionStateCb,
                           &factory,
                           listenOpts);
            ASSERT(0 == rc);

            const int PORTNUM = mX.portNumber(handle);

            btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

            btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;
            btlso::StreamSocket<btlso::IPv4Address> *socket =
                socketFactory.allocate();

            rc = socket->connect(ADDRESS);
            ASSERT(0 == rc);

            barrier.wait();

            char data[SIZE];
            bsl::memset(data, FILL, SIZE);
            ASSERT(SIZE == socket->write(data, SIZE));

            barrier.wait();
            barrier.wait();

            socketFactory.deallocate(socket);
        }

        LOOP_ASSERT(blob.length(), SIZE == blob.length());
        for (int i = 0; i < blob.numBuffers(); ++i) {
            ASSERT(SIZE/2 == blob.buffer(i).size());
            for (int j = 0; j < SIZE/2; ++j) {
                LOOP3_ASSERT(i, j, blob.buffer(i).data()[j],
                             FILL == blob.buffer(i).data()[j]);
            }
        }

        ASSERT(0 != pa.numAllocations());
        ASSERT(0 == pa.numBytesInUse());
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // TESTING 'allocate' with shared async channel
        //   Ensure that the 'channel' passed to the session objects remains
        //   valid following the session pool's deallocation of it.
        //
        // Concerns:
        //   1. The async channel remains valid for access after the session
        //      pool has deallocated it.
        //
        // Plan:
        //   1. For concern 1, invoke 'allocate' with a shared pointer to the
        //      channel. Coordinate access to the underlying
        //      btlmt::AsyncChannel at a later point in time than would have
        //      been possible prior to the addition of the introduction of
        //      channel sharing.
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'allocate' with shared channel"
                               << bsl::endl
                               << "======================================"
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_CASE_ALLOCATE_SHARED_CHANNEL;

        btlso::InetStreamSocketFactory<btlso::IPv4Address> factory;
        btlso::StreamSocket<btlso::IPv4Address> *socket = factory.allocate();

        btlso::IPv4Address address("127.0.0.1",
                                   btlso::IPv4Address::k_ANY_PORT);

        bslmt::Barrier barrier(3);

        Tester tester(Tester::LISTENER, &barrier, address, &ta);

        int portNumber = tester.portNumber();

        address.setPortNumber(portNumber);

        ASSERT(0 == socket->connect(address));

        const char STRING[] = "Hello World!";

        ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

        char readBuffer[sizeof(STRING)];
        ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));

        ASSERT(0 == bsl::strcmp(readBuffer, STRING));

        if (veryVerbose) {
            MTCOUT << "Bringing down the channel" << MTENDL;
        }

        socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

        factory.deallocate(socket);

        barrier.wait();

      } break;
      case 12: {
        // --------------------------------------------------------------------
        // TESTING CALLING 'start' AFTER 'stop'
        //   Ensure that 'start' after a 'stop' works as expected.
        //
        // Concerns:
        //: 1. 'start' after a 'stop' restarts the session pool.
        //
        // Plan:
        //: 1 Create a session pool object, mX.
        //:
        //: 2 Open a pre-defined number of listening sockets in mX.
        //:
        //: 3 Create a number of threads that each connect to one of the
        //:   listening sockets in mX.
        //:
        //: 4 Create a number of threads that each listen on a socket.
        //:
        //: 5 Open a session in mX by connecting a listening sockets created
        //:   in step 4.
        //:
        //: 6 Write large amount of data through mX across all the open
        //:   channels.
        //:
        //: 7 Invoke 'stopAndRemoveAllSessions' and confirm that no sessions
        //:   are outstanding.
        //:
        //: 8 Call 'start' and check the return value.
        //:
        //: 9 Repeat steps 2 - 7 to confirm that opening connections and
        //:   transferring data works.
        //:
        //
        // Testing:
        //   int start();
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'start' after 'stop'" << bsl::endl
                               << "============================" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS;

        typedef btlmt::SessionPool::SessionStateCallback     SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(2 * NUM_THREADS);
        config.setWriteQueueWatermarks(0, NUM_BYTES * 10);  // 1Mb

        SessionCb sessionStateCb = bdlf::BindUtil::bind(
                                &sessionStateCallbackUsingChannelMapAndCounter,
                                _1,
                                _2,
                                _3,
                                _4,
                                &numUpConnections);

        const int                     SIZE = 1024 * 1024; // 1 MB
        btlb::PooledBlobBufferFactory factory(SIZE);
        btlb::Blob                    dataBlob(&factory);
        dataBlob.setLength(NUM_BYTES);

        TestFactory sessionFactory;
        Obj         mX(config, &poolStateCallback);

        ASSERT(0 == mX.start());

        bslmt::ThreadUtil::Handle connectThreads[NUM_THREADS];
        bslmt::ThreadUtil::Handle listenThreads[NUM_THREADS];

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 != mX.numSessions());

        ASSERT(0 == mX.stopAndRemoveAllSessions());

        ASSERT(0 == mX.numSessions());

        ASSERT(0 == mX.start());
        mapMutex.lock();
        sourceIdToChannelMap.clear();
        mapMutex.unlock();

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 == mX.stopAndRemoveAllSessions());

        ASSERT(0 == mX.numSessions());
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING 'connect' with socket options
        //   Ensure that the 'connect' that takes a socket option object
        //   returns a session up callback on success and user callback on
        //   error.
        //
        // Concerns:
        //   1. The session state callback is called with success if setting of
        //      the socket options succeeds.
        //
        //   2. A user callback is invoked on encountering an asynchronous
        //      failure while setting the socket options.
        //
        // Plan:
        //   1. For concern 1, invoke 'connect' with a socket option expected
        //      to succeed.  Verify that a session state callback is invoked
        //      informing the user of connect success.
        //
        //   1. For concern 2, invoke 'connect' with a socket option expected
        //      to fail.  Verify that a pool state callback with is invoked
        //      informing the user of the connect error.
        //
        // Testing:
        //   int connect(host, ...., *socketOptions);
        //   int connect(serverAddr, ...., *socketOptions);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'connect' with socket options"
                               << bsl::endl
                               << "====================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_SETTING_SOCKETOPTIONS;

        if (verbose) cout << "connect(IPv4Address...) with socket options"
                          << endl;
        {
            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);

            bslmt::Barrier channelCbBarrier(2);
            btlmt::SessionPool::SessionStateCallback sessionStateCb(
                         bdlf::BindUtil::bind(&sessionStateCallbackWithBarrier,
                                              _1, _2, _3, _4,
                                              &channelCbBarrier));

            int              poolState;
            bslmt::Barrier   poolCbBarrier(2);
            btlmt::SessionPool::SessionPoolStateCallback
                poolStateCb(bdlf::BindUtil::bind(&poolStateCallbackWithBarrier,
                                                 _1, _2, _3,
                                                 &poolState,
                                                 &poolCbBarrier));

            TestFactory sessionFactory;

            Obj pool(config, poolStateCb);

            ASSERT(0 == pool.start());

            btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;

            {

                typedef btlso::StreamSocketFactoryDeleter Deleter;

                bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                            socket(socketFactory.allocate(),
                                   &socketFactory,
                                   &Deleter::deleteObject<btlso::IPv4Address>);

                SocketOptions opt;  opt.setKeepAlive(true); // always succeeds
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                &opt, 0, 0);
                ASSERT(!rc);

                channelCbBarrier.wait();
            }
#ifndef BSLS_PLATFORM_OS_WINDOWS
            {

                typedef btlso::StreamSocketFactoryDeleter Deleter;

                bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                    socket(socketFactory.allocate(),
                           &socketFactory,
                           &Deleter::deleteObject<btlso::IPv4Address>);
                ASSERT(socket);

                SocketOptions opt;  opt.setSendTimeout(1); // fails on all
                                                           // platforms except
                                                           // windows
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                &opt, 0, 0);
                ASSERT(!rc);

                poolCbBarrier.wait();
                ASSERT(btlmt::SessionPool::e_CONNECT_FAILED == poolState);
            }
#endif
            ASSERT(0 == pool.stopAndRemoveAllSessions());
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING 'connect' with a user-specified local address
        //   Ensure that the 'connect' that takes a client address returns a
        //   session up callback on success and user callback on error.
        //
        // Concerns:
        //   1. The session state callback is called with success if binding
        //      to the local address succeeds.
        //
        //   2. A user callback is invoked on encountering an asynchronous
        //      failure while binding to a provided local address.
        //
        // Plan:
        //   1. For concern 1, invoke 'connect' with a good IP address.  Verify
        //      that a session state callback is invoked informing the user of
        //      connect success.
        //
        //   2. For concern 2, invoke 'connect' with a bad IP address.  Verify
        //      that a pool state callback is invoked informing the user of
        //      the connect error.
        //
        // Testing:
        //   int connect(host, ...., *socketOptions, *clientAddr);
        //   int connect(serverAddr, ...., *socketOptions, *clientAddr);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'connect' with a local address"
                               << bsl::endl
                               << "======================================"
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_SETTING_SOCKETOPTIONS;

        if (verbose) cout << "connect(IPv4Address...) with client address"
                          << endl;
        {
            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(1);

            bslmt::Barrier channelCbBarrier(2);
            btlmt::SessionPool::SessionStateCallback sessionStateCb(
                         bdlf::BindUtil::bind(&sessionStateCallbackWithBarrier,
                                              _1, _2, _3, _4,
                                              &channelCbBarrier));

            int             poolState;
            bslmt::Barrier  poolCbBarrier(2);
            btlmt::SessionPool::SessionPoolStateCallback
                poolStateCb(bdlf::BindUtil::bind(&poolStateCallbackWithBarrier,
                                                 _1, _2, _3,
                                                 &poolState,
                                                 &poolCbBarrier));

            TestFactory sessionFactory;

            Obj pool(config, poolStateCb);

            ASSERT(0 == pool.start());

            btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;

            {
                typedef btlso::StreamSocketFactoryDeleter Deleter;

                bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                            socket(socketFactory.allocate(),
                                   &socketFactory,
                                   &Deleter::deleteObject<btlso::IPv4Address>);

                btlso::IPv4Address address("127.0.0.1", 45000); // good address
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                0, &socketFactory,
                                                &address);
                ASSERT(!rc);

                if (veryVerbose) {
                    MTCOUT << "Waiting on channel barrier..." << MTENDL;
                }
                channelCbBarrier.wait();
            }

            {

                typedef btlso::StreamSocketFactoryDeleter Deleter;

                bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                            socket(socketFactory.allocate(),
                                   &socketFactory,
                                   &Deleter::deleteObject<btlso::IPv4Address>);

                btlso::IPv4Address address("1.1.1.1", 45248);  // bad address
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                0, &socketFactory,
                                                &address);
                if (!rc) {
                    if (veryVerbose) {
                        MTCOUT << "Waiting on pool barrier..." << MTENDL;
                    }
                    poolCbBarrier.wait();
                    ASSERT(btlmt::SessionPool::e_CONNECT_FAILED == poolState);
                }
            }

            ASSERT(0 == pool.stopAndRemoveAllSessions());
        }
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING 'setWriteQueueWatermarks'
        //   The 'setWriteQueueWatermarks' method has the expected effect.
        //
        // Concerns:
        //   1. That 'setWriteQueueWatermarks' fails when passed an invalid
        //      session id.
        //
        //   2. That 'setWriteQueueWatermarks' correctly passes its arguments
        //      to btlmt::ChannelPool::setWriteQueueWatermarks'.
        //
        // Plan:
        //   1. For concern 1, invoke 'setWriteQueueWatermarks' with an invalid
        //      session id and verify that the method fails.
        //
        //   2. For concern 2, create a session, capturing the session id of a
        //      client connection.  Invoke the method using that session id and
        //      representative (valid) values for the low- and high-water
        //      marks.  Assert that the method succeeds in each case.
        //
        // Testing:
        //   int setWriteQueueWatermarks(int, int, int);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'setWriteQueueWatermarks'"
                               << bsl::endl
                               << "================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        enum {
            LOW_WATERMARK =  512,
            HIGH_WATERMARK  = 4096
        };

        typedef btlmt::SessionPool::SessionStateCallback     SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(4);
        config.setWriteQueueWatermarks(LOW_WATERMARK, HIGH_WATERMARK);

        bsls::AtomicInt numUpConnections(0);

        PoolCb poolStateCb(&poolStateCallback);
        SessionCb sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithCounter,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &numUpConnections);

        Obj sessionPool(config, poolStateCb);

        ASSERT(0 == sessionPool.start());

        TestFactory factory;

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(5);
        btlso::SocketOptions socketOpts;
        socketOpts.setReuseAddress(true);
        listenOpts.setSocketOptions(socketOpts);

        int handle = 0;
        ASSERT(0 == sessionPool.listen(&handle,
                                       sessionStateCb,
                                       &factory,
                                       listenOpts));
        const int PORTNUM = sessionPool.portNumber(handle);

        btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        btlmt::ConnectOptions connectOptions;
        connectOptions.setServerEndpoint(ADDRESS);
        connectOptions.setTimeout(bsls::TimeInterval(1));
        connectOptions.setNumAttempts(5);

        ASSERT(0 == sessionPool.connect(&handle,
                                        sessionStateCb,
                                        &factory,
                                        connectOptions));

        while (numUpConnections < 2) {
            bslmt::ThreadUtil::microSleep(50, 0);
        }

        ASSERT(0 != sessionPool.setWriteQueueWatermarks(handle + 666,
                                                        LOW_WATERMARK + 1,
                                                        HIGH_WATERMARK - 1));

        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        LOW_WATERMARK,
                                                        HIGH_WATERMARK));
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        LOW_WATERMARK,
                                                        LOW_WATERMARK));

        sessionPool.setWriteQueueWatermarks(handle,
                                            LOW_WATERMARK,
                                            HIGH_WATERMARK);
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        HIGH_WATERMARK,
                                                        HIGH_WATERMARK));

        sessionPool.setWriteQueueWatermarks(handle,
                                            LOW_WATERMARK,
                                            HIGH_WATERMARK);
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        LOW_WATERMARK - 2,
                                                        LOW_WATERMARK));

        sessionPool.setWriteQueueWatermarks(handle,
                                            LOW_WATERMARK,
                                            HIGH_WATERMARK);
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        LOW_WATERMARK - 2,
                                                        LOW_WATERMARK - 1));

        sessionPool.setWriteQueueWatermarks(handle,
                                            LOW_WATERMARK,
                                            HIGH_WATERMARK);
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        HIGH_WATERMARK,
                                                        HIGH_WATERMARK + 2));

        sessionPool.setWriteQueueWatermarks(handle,
                                            LOW_WATERMARK,
                                            HIGH_WATERMARK);
        ASSERT(0 == sessionPool.setWriteQueueWatermarks(handle,
                                                        HIGH_WATERMARK + 1,
                                                        HIGH_WATERMARK + 2));

        ASSERT(0 == sessionPool.stop());
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // TESTING 'stopAndRemoveAllSessions'
        //
        // Concerns
        //: 1 Session pool stops all threads and removes all sessions when
        //:   this function is called.
        //:
        //: 2 Any resources associated with any session is released.
        //
        // Plan:
        //: 1 Create a session pool object, mX.
        //:
        //: 2 Open a pre-defined number of listening sockets in mX.
        //:
        //: 3 Create a number of threads that each connect to one of the
        //:   listening sockets in mX.
        //:
        //: 4 Create a number of threads that each listen on a socket.
        //:
        //: 5 Open a session in mX by connecting a listening sockets created
        //:   in step 4.
        //:
        //: 6 Write large amount of data through mX across all the open
        //:   channels.
        //:
        //: 7 Invoke 'stopAndRemoveAllSessions' and confirm that no sessions
        //:   are outstanding.
        //
        // Testing:
        //   int stopAndRemoveAllSessions();
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'stopAndRemoveAllChannels'"
                               << bsl::endl
                               << "=================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS;

        typedef btlmt::SessionPool::SessionStateCallback SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(2 * NUM_THREADS);
        config.setWriteQueueWatermarks(0, NUM_BYTES * 10);  // 1Mb

        PoolCb poolStateCb(&poolStateCallback);
        SessionCb sessionStateCb = bdlf::BindUtil::bind(
                                &sessionStateCallbackUsingChannelMapAndCounter,
                                _1,
                                _2,
                                _3,
                                _4,
                                &numUpConnections);

        TestFactory sessionFactory;
        Obj         mX(config, poolStateCb);

        ASSERT(0 == mX.start());

        const int                     SIZE = 1024 * 1024; // 1 MB
        btlb::PooledBlobBufferFactory factory(SIZE);
        btlb::Blob                    dataBlob(&factory);
        dataBlob.setLength(NUM_BYTES);

        bslmt::ThreadUtil::Handle connectThreads[NUM_THREADS];
        bslmt::ThreadUtil::Handle listenThreads[NUM_THREADS];

        memset(connectThreads, 0, sizeof(connectThreads));
        memset(listenThreads, 0, sizeof(listenThreads));

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 != mX.numSessions());

        const int rc = mX.stopAndRemoveAllSessions();
        ASSERT(0 == rc);

        ASSERT(0 == mX.numSessions());

        for (int i = 0; i < NUM_THREADS; ++i) {
            bslmt::ThreadUtil::join(connectThreads[i]);
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING removal of intermediate read blob
        //  Ensure that session pool does not allocate and hold on to
        //  additional memory buffers when clients read all the data.
        //
        // Concerns
        //: 1 Session pool does not allocate and hold on to memory buffers
        //:   indefinitely even if the client is reading all the data.
        //
        // Plan:
        //: 1 Create a session pool object, 'mX', and listen on a port on the
        //:   local machine.
        //:
        //: 2 Create a socket and 'connect' to the port number on which the
        //:   session pool is listening.
        //:
        //: 3 Write data on the socket in multiple attempts and monitor the
        //:   size and length of the blob returned by session pool in the data
        //:   callback.
        //:
        //: 4 Verify that the blob provided in the data callback does not
        //:   unnecessarily hoard memory.
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING removal of intermediate read blob"
                               << bsl::endl
                               << "========================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_CASE_REMOVE_EXTRA_BLOB;

        {
            btlmt::ChannelPoolConfiguration config;
            config.setMaxThreads(4);

            char payload[PAYLOAD_SIZE];
            bsl::memset(payload, 'X', PAYLOAD_SIZE);
            const int NUM_WRITES = 10000;

            bslma::TestAllocator ta;
            bslmt::Semaphore     semaphore;
            BlobReadCallback     callback =
                                        bdlf::BindUtil::bind(
                                                     &readCbWithMetrics,
                                                     _1,
                                                     _2,
                                                     _3,
                                                     _4,
                                                     NUM_WRITES * PAYLOAD_SIZE,
                                                     &semaphore);

            TestFactory sessionFactory(&callback, &ta);
            Obj         mX(config, &poolStateCallback, &ta);

            ASSERT(0 == mX.start());

            btlmt::ListenOptions listenOpts;
            listenOpts.setServerAddress(btlso::IPv4Address());
            listenOpts.setBacklog(5);

            int handle = 0;
            ASSERT(0 == mX.listen(&handle,
                                  &sessionStateCallback,
                                  &sessionFactory,
                                  listenOpts));

            const int PORTNUM = mX.portNumber(handle);

            btlso::InetStreamSocketFactory<btlso::IPv4Address> factory;
            btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                            factory.allocate();
            bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                  smp(socket,
                      &factory,
                      &SocketFactoryDeleter::deleteObject<btlso::IPv4Address>);

            const btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);
            int rc = socket->connect(ADDRESS);
            ASSERT(!rc);

            for (int i = 0; i < NUM_WRITES; ++i) {
                socket->write(payload, PAYLOAD_SIZE);
            }

            semaphore.wait();

            if (veryVerbose) {
                MTCOUT << "TA In Use: " << ta.numBytesInUse() << MTENDL;
                MTCOUT << "TA In Use Blocks: "
                       << ta.numBlocksInUse()
                       << MTENDL;
                MTCOUT << "TA In Max: " << ta.numBytesMax() << MTENDL;
                MTCOUT << "maxLength: " << maxLength << MTENDL;
                MTCOUT << "maxSize: " << maxSize << MTENDL;
                MTCOUT << "maxExtra: " << maxExtra << MTENDL;
                MTCOUT << "maxNumBuffers: " << maxNumBuffers << MTENDL;
                MTCOUT << "maxNumDataBuffers: " << maxNumDataBuffers << MTENDL;
            }
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING 'stop' updates numSessions correctly
        //  Ensure that the bug where d_numSessions is decremented in stop and
        //  then again when the session handle is destroyed has been fixed.
        //
        // Concerns:
        //: 1 d_numSessions is not decremented twice after a call to 'stop'.
        //
        // Plan:
        //: 1 Create a session pool object, 'mX', and listen on a port on the
        //:   local machine.
        //:
        //: 2 Define NUM_SESSIONS with a value of 2 as the number of sessions
        //:   to be created.
        //:
        //: 3 Open NUM_SESSIONS sockets and 'connect' to the port number on
        //:   which the session pool is listening.
        //:
        //: 4 Confirm that numSessions on mX returns NUM_SESSIONS.
        //:
        //: 5 Call 'stop' on the session pool and confirm that numSessions
        //:   returns 0.
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TEST 'stop' updates numSessions correctly"
                               << bsl::endl
                               << "========================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        typedef btlmt::SessionPool::SessionStateCallback     SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        const int NUM_SESSIONS = 5;
        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(NUM_SESSIONS);
        config.setMaxConnections(NUM_SESSIONS);

        PoolCb          poolStateCb(&poolStateCallback);
        bslmt::Barrier  barrier(NUM_SESSIONS + 1);
        bsls::AtomicInt numUpConnections(0);

        SessionCb sessionStateCb(bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithCounter,
                                              _1, _2, _3, _4,
                                              &numUpConnections));

        Obj mX(config, poolStateCb);  const Obj& X = mX;

        ASSERT(0 == mX.start());

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(5);

        int         handle;
        TestFactory sessionFactory;
        int rc = mX.listen(&handle,
                           sessionStateCb,
                           &sessionFactory,
                           listenOpts);
        ASSERT(!rc);

        const int PORTNUM = X.portNumber(handle);

        btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;
        bsl::vector<bslma::ManagedPtr<
                                    btlso::StreamSocket<btlso::IPv4Address> > >
                                                         sockets(NUM_SESSIONS);
        for (int i = 0; i < NUM_SESSIONS; ++i) {
            btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                      socketFactory.allocate();

            ASSERT(0 == socket->connect(ADDRESS));

            bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                  smp(socket,
                      &socketFactory,
                      &SocketFactoryDeleter::deleteObject<btlso::IPv4Address>);
            sockets[i] = smp;
        }

        while (numUpConnections < NUM_SESSIONS) {
            bslmt::ThreadUtil::microSleep(50, 0);
        }

        int ns = X.numSessions();
        LOOP_ASSERT(ns, NUM_SESSIONS == ns);

        ASSERT(0 == mX.stop());

        ns = X.numSessions();
        LOOP_ASSERT(ns, 0 == ns);
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING peer address is set appropriately
        //
        // Plan:
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING peer address is set appropriately"
                               << bsl::endl
                               << "========================================="
                               << bsl::endl;

        using namespace BTLMT_SESSION_POOL_CASE_PEER_ADDRESS;

        typedef btlmt::SessionPool::SessionStateCallback SessionCb;
        typedef btlmt::SessionPool::SessionPoolStateCallback PoolCb;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(1);

        PoolCb    poolStateCb    = &poolStateCallback;
        SessionCb sessionStateCb = &sessionStateCallback;

        Obj mX(config, poolStateCb);
        ASSERT(0 == mX.start());

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(1);

        bslmt::Barrier barrier(2);
        int            handle;
        TestSessionFactory sessionFactory(&barrier);
        mX.listen(&handle, sessionStateCb, &sessionFactory, listenOpts);

        btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;
        btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                      socketFactory.allocate();
        bslma::ManagedPtr<btlso::StreamSocket<btlso::IPv4Address> >
                  smp(socket,
                      &socketFactory,
                      &SocketFactoryDeleter::deleteObject<btlso::IPv4Address>);

        const int PORTNUM = mX.portNumber(handle);

        btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        ASSERT(0 == socket->connect(ADDRESS));

        socket->shutdown(btlso::Flags::e_SHUTDOWN_BOTH);

        barrier.wait();
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING removal of inefficiencies in read callback
        //
        // Plan:
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose)
              bsl::cout << "TESTING removal of inefficiencies in read callback"
                        << bsl::endl
                        << "=================================================="
                        << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        btlmt::ChannelPoolConfiguration config;
        config.setMaxThreads(4);

        int            cbCount = 0;
        bslmt::Barrier barrier(2);

        BlobReadCallback callback = bdlf::BindUtil::bind(
                                                    &readCbWithCountAndBarrier,
                                                    _1,
                                                    _2,
                                                    _3,
                                                    _4,
                                                    &cbCount,
                                                    &barrier);
        TestFactory factory(&callback);

        typedef btlmt::SessionPool::SessionPoolStateCallback
                                                            SessionPoolStateCb;
        typedef btlmt::SessionPool::SessionStateCallback    SessionStateCb;

        SessionPoolStateCb poolCb         = &poolStateCallback;
        SessionStateCb     sessionStateCb = bdlf::BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);

        Obj mX(config, poolCb);

        int rc = mX.start();
        ASSERT(0 == rc);

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(5);
        btlso::SocketOptions socketOpts;
        socketOpts.setReuseAddress(true);
        listenOpts.setSocketOptions(socketOpts);

        int handle;
        rc = mX.listen(&handle,
                       sessionStateCb,
                       &factory,
                       listenOpts);
        ASSERT(0 == rc);

        const int PORTNUM = mX.portNumber(handle);

        const char STRING[] = "Hello World!";

        btlso::IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        btlso::InetStreamSocketFactory<btlso::IPv4Address> socketFactory;
        btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                      socketFactory.allocate();

        rc = socket->connect(ADDRESS);
        ASSERT(0 == rc);

        barrier.wait();

        rc = socket->write(STRING, sizeof(STRING));
        ASSERT(sizeof(STRING) == rc);

        barrier.wait();

        socketFactory.deallocate(socket);

        LOOP_ASSERT(cbCount, 1 == cbCount);
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // BLOB BASED EXAMPLE
        //
        // Plan:
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "BLOB BASED EXAMPLE" << bsl::endl
                               << "==================" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        bslma::TestAllocator da("default", veryVeryVerbose);
        bslma::DefaultAllocatorGuard defaultAllocGuard(&da);

        TestFactory factory(0, &ta);

        btlmt::ChannelPoolConfiguration config;

        typedef btlmt::SessionPool::SessionPoolStateCallback
                                                            SessionPoolStateCb;
        typedef btlmt::SessionPool::SessionStateCallback    SessionStateCb;

        SessionPoolStateCb poolStateCb    = &poolStateCallback;
        SessionStateCb     sessionStateCb = &sessionStateCallback;

        Obj sessionPool(config, poolStateCb, &ta);
        int rc = sessionPool.start();
        ASSERT(0 == rc);

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(5);
        btlso::SocketOptions socketOpts;
        socketOpts.setReuseAddress(false);
        listenOpts.setSocketOptions(socketOpts);

        int handle;
        rc = sessionPool.listen(&handle,
                                sessionStateCb,
                                &factory,
                                listenOpts);
        ASSERT(0 == rc);

        {
            btlso::InetStreamSocketFactory<btlso::IPv4Address> factory(&ta);
            btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                            factory.allocate();

            const char STRING[] = "Hello World!";

            const btlso::IPv4Address ADDRESS("127.0.0.1",
                                            sessionPool.portNumber(handle));
            ASSERT(0 == socket->connect(ADDRESS));
            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            char readBuffer[sizeof(STRING)];
            ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));
            ASSERT(0 == bsl::strcmp(readBuffer, STRING));

            factory.deallocate(socket);
        }

        ASSERT(0 != ta.numBytesInUse());
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TEST ALLOCATOR PROPAGATION
        //
        // Plan: Install a default allocator, then execute the substance of
        // the usage example, keeping the SessionPool object in-scope after
        // the test is complete.  Then verify that no memory is outstanding
        // through the default allocator.
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "ALLOCATOR PROPAGATION" << bsl::endl
                               << "=====================" << bsl::endl;

        using namespace BTLMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        bslma::TestAllocator da("default", veryVeryVerbose);
        bslma::DefaultAllocatorGuard defaultAllocGuard(&da);

        TestFactory factory(0, &ta);

        btlmt::ChannelPoolConfiguration config;

        typedef btlmt::SessionPool::SessionPoolStateCallback
                                                            SessionPoolStateCb;
        typedef btlmt::SessionPool::SessionStateCallback    SessionStateCb;

        SessionPoolStateCb poolStateCb    = &poolStateCallback;
        SessionStateCb     sessionStateCb = &sessionStateCallback;

        Obj mX(config, poolStateCb, &ta);
        int rc = mX.start();
        ASSERT(0 == rc);

        btlmt::ListenOptions listenOpts;
        listenOpts.setServerAddress(btlso::IPv4Address());
        listenOpts.setBacklog(5);
        btlso::SocketOptions socketOpts;
        socketOpts.setReuseAddress(false);
        listenOpts.setSocketOptions(socketOpts);

        int handle;
        rc = mX.listen(&handle,
                       sessionStateCb,
                       &factory,
                       listenOpts);
        ASSERT(0 == rc);

        {
            btlso::InetStreamSocketFactory<btlso::IPv4Address> factory(&ta);
            btlso::StreamSocket<btlso::IPv4Address> *socket =
                                                            factory.allocate();

            const char STRING[] = "Hello World!";

            const btlso::IPv4Address ADDRESS("127.0.0.1",
                                             mX.portNumber(handle));
            ASSERT(0 == socket->connect(ADDRESS));
            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            char readBuffer[sizeof(STRING)];
            ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));
            ASSERT(0 == bsl::strcmp(readBuffer, STRING));

            factory.deallocate(socket);
        }
        ASSERT(0 != ta.numBytesInUse());
      } break;

      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //
        // Plan:
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        btlmt::ChannelPoolConfiguration config;

        Obj X(config, btlmt::SessionPool::SessionPoolStateCallback());
      } break;
      default: {
        bsl::cerr << "WARNING: CASE `" << test << "' NOT FOUND." << bsl::endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        bsl::cerr << "Error, non-zero test status = " << testStatus << "."
                  << bsl::endl;
    }
    return testStatus;
}

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
