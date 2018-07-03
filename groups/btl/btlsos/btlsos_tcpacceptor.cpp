// btlsos_tcpacceptor.cpp                                             -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#include <btlsos_tcpacceptor.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(btlsos_tcpacceptor_cpp,"$Id$ $CSID$")

#include <btlsos_tcptimedchannel.h>
#include <btlsos_tcpchannel.h>
#include <btlso_streamsocketfactory.h>
#include <btlso_streamsocket.h>
#include <btlsc_flags.h>

#include <bsls_timeinterval.h>
#include <bdlt_currenttime.h>

#include <bsls_assert.h>
#include <bsls_blockgrowth.h>

#include <bsl_algorithm.h>
#include <bsl_cstddef.h>
#include <bsl_vector.h>

namespace BloombergLP {

// ============================================================================
//                             LOCAL DEFINITIONS
// ============================================================================

                     // ===============================
                     // Local typedefs and enumerations
                     // ===============================

enum {
    e_BLOCKING_MODE    = btlso::Flags::e_BLOCKING_MODE,
    e_NONBLOCKING_MODE = btlso::Flags::e_NONBLOCKING_MODE
};

enum {
    k_ARENA_SIZE = sizeof(btlsos::TcpChannel) < sizeof(btlsos::TcpTimedChannel)
                 ? sizeof(btlsos::TcpTimedChannel)
                 : sizeof(btlsos::TcpChannel)
};

enum {
    e_INVALID       = -4,
    e_FAILED        = -3,
    e_UNINITIALIZED = -2,
    e_CANCELLED     = -1,
    e_SUCCESS       =  0
};

                         // =======================
                         // local function allocate
                         // =======================

template <class RESULT>
inline
static RESULT *allocate(int                                     *status,
                        int                                      flags,
                        btlso::StreamSocket<btlso::IPv4Address> *socket,
                        bdlma::Pool                             *pool)
{
    BSLS_ASSERT(socket);
    BSLS_ASSERT(pool);
    BSLS_ASSERT(status);

    // Bring the listening socket into blocking mode.
    int rc = socket->setBlockingMode(btlso::Flags::e_BLOCKING_MODE);
    (void)rc; BSLS_ASSERT(0 == rc);

    btlso::IPv4Address peer;
    btlso::StreamSocket<btlso::IPv4Address> *acceptedConnection = 0;
    while (1) {
        int s = socket->accept(&acceptedConnection, &peer);

        if (acceptedConnection) { break; }

        if (btlso::SocketHandle::e_ERROR_INTERRUPTED != s) {
            *status = e_FAILED;
            socket->setBlockingMode(btlso::Flags::e_NONBLOCKING_MODE);
            return NULL;                                              // RETURN
        }

        if (flags & btlsc::Flags::k_ASYNC_INTERRUPT) {
            *status = 1;  // Any positive number satisfies the contract.
            socket->setBlockingMode(btlso::Flags::e_NONBLOCKING_MODE);

            return NULL;                                              // RETURN
        }
    }

    socket->setBlockingMode(btlso::Flags::e_NONBLOCKING_MODE);

    RESULT *channel = new (*pool) RESULT(acceptedConnection);

    return channel;
}

namespace btlsos {

// ============================================================================
//                           END LOCAL DEFINITIONS
// ============================================================================

                            // -----------------
                            // class TcpAcceptor
                            // -----------------

// CREATORS
TcpAcceptor::TcpAcceptor(
                btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
                bslma::Allocator                               *basicAllocator)
: d_pool(k_ARENA_SIZE, basicAllocator)
, d_channels(basicAllocator)
, d_factory_p(factory)
, d_serverSocket_p(0)
, d_isInvalidFlag(0)
{

    BSLS_ASSERT(d_factory_p);
}

TcpAcceptor::TcpAcceptor(
               btlso::StreamSocketFactory<btlso::IPv4Address> *factory,
               int                                             initialCapacity,
               bslma::Allocator                               *basicAllocator)
: d_pool(k_ARENA_SIZE,
         bsls::BlockGrowth::BSLS_CONSTANT,
         initialCapacity,
         basicAllocator)
, d_channels(basicAllocator)
, d_factory_p(factory)
, d_serverSocket_p(0)
, d_isInvalidFlag(0)
{
    BSLS_ASSERT(d_factory_p);
    BSLS_ASSERT(0 < initialCapacity);
}

TcpAcceptor::~TcpAcceptor() {
    // INVARIANTS: If listening socket is open, it is in a non-blocking mode.

    invalidate();
    BSLS_ASSERT(d_factory_p);

    if (d_serverSocket_p) {
        close();
    }
    // Deallocate channels
    while (d_channels.size()) {
        btlsc::Channel *ch = d_channels[d_channels.size()-1];
        BSLS_ASSERT(ch);
        ch->invalidate();
        deallocate(ch);
    }
    BSLS_ASSERT(0 == d_channels.size());
}

// MANIPULATORS
void TcpAcceptor::deallocate(btlsc::Channel *channel) {
    BSLS_ASSERT(channel);
    char *arena = (char *) channel;
    TcpTimedChannel *c =
        dynamic_cast<TcpTimedChannel*>(channel);
    btlso::StreamSocket<btlso::IPv4Address> *s = NULL;

    if (c) {
        s = c->socket();
    }
    else {
        TcpChannel *c =
            dynamic_cast<TcpChannel*>(channel);
        BSLS_ASSERT(c);
        s = c->socket();
    }
    BSLS_ASSERT(s);

    d_factory_p->deallocate(s);
    channel->invalidate();

    bsl::vector<btlsc::Channel*>::iterator idx =
               bsl::lower_bound(d_channels.begin(), d_channels.end(), channel);
    BSLS_ASSERT(idx != d_channels.end() && *idx == channel);
    d_channels.erase(idx);

    d_pool.deallocate(arena);
    return ;
}

btlsc::Channel *TcpAcceptor::allocate(int *status, int flags)
{
    BSLS_ASSERT(status);

    if (d_isInvalidFlag || NULL == d_serverSocket_p) {
        if (NULL == d_serverSocket_p) {
            *status = e_UNINITIALIZED;
        }
        else {
            *status = e_INVALID;
        }
        return NULL;                                                  // RETURN
    }

    TcpChannel *channel = BloombergLP::allocate<TcpChannel> (
                                                         status,
                                                         flags,
                                                         d_serverSocket_p,
                                                         &d_pool);

    if (channel) {
        bsl::vector<btlsc::Channel*>::iterator idx =
               bsl::lower_bound(d_channels.begin(), d_channels.end(),
                                static_cast<btlsc::Channel*>(channel));
        d_channels.insert(idx, channel);
    }
    return channel;
}

btlsc::TimedChannel *TcpAcceptor::allocateTimed(int *status, int flags)
{
    BSLS_ASSERT(status);
    if (d_isInvalidFlag || NULL == d_serverSocket_p) {
        if (NULL == d_serverSocket_p) {
            *status = e_UNINITIALIZED;
        }
        else {
            *status = e_INVALID;
        }
        return NULL;                                                  // RETURN
    }

    btlsc::TimedChannel *channel =
        BloombergLP::allocate<TcpTimedChannel>(status, flags,
                                                      d_serverSocket_p,
                                                      &d_pool);
    if (channel) {
        bsl::vector<btlsc::Channel*>::iterator idx =
               bsl::lower_bound(d_channels.begin(), d_channels.end(),
                                static_cast<btlsc::Channel*>(channel));
        d_channels.insert(idx, channel);
    }
    return channel;
}

int TcpAcceptor::close() {
    BSLS_ASSERT(NULL != d_serverSocket_p);
    BSLS_ASSERT(d_serverAddress.portNumber());  // Address is valid.

    d_factory_p->deallocate(d_serverSocket_p);
    d_serverSocket_p = NULL;
    d_serverAddress = btlso::IPv4Address();
    return e_SUCCESS;
}

int TcpAcceptor::open(const btlso::IPv4Address& endpoint,
                      int                       queueSize,
                      int                       reuseAddressFlag) {
    BSLS_ASSERT(0 < queueSize);
    BSLS_ASSERT(NULL == d_serverSocket_p);

    enum {
        e_CANT_SET_OPTIONS     = -7,
        e_INVALID_ACCEPTOR     = -6,
        e_ALLOCATION_FAILED    = -5,
        e_BIND_FAILED          = -4,
        e_LISTEN_FAILED        = -3,
        e_LOCALINFO_FAILED     = -2,
        e_BLOCKMODE_FAILED     = -1
    };

    if (d_isInvalidFlag) {
        return e_INVALID_ACCEPTOR;                                    // RETURN
    }
    d_serverSocket_p = d_factory_p->allocate();
    if (!d_serverSocket_p) {
        return e_ALLOCATION_FAILED;                                   // RETURN
    }

    if (reuseAddressFlag) {
        if (0 != d_serverSocket_p->setOption(
                                      btlso::SocketOptUtil::k_SOCKETLEVEL,
                                      btlso::SocketOptUtil::k_REUSEADDRESS,
                                      reuseAddressFlag)) {
            d_factory_p->deallocate(d_serverSocket_p);
            d_serverSocket_p = NULL;
            return e_CANT_SET_OPTIONS;                                // RETURN
        }
    }

    if (0 != d_serverSocket_p->bind(endpoint)) {
        d_factory_p->deallocate(d_serverSocket_p);
        d_serverSocket_p = NULL;
        return e_BIND_FAILED;                                         // RETURN
    }

    if (0 != d_serverSocket_p->localAddress(&d_serverAddress)) {
        d_factory_p->deallocate(d_serverSocket_p);
        d_serverSocket_p = NULL;
        return e_BIND_FAILED;                                         // RETURN
    }
    BSLS_ASSERT(d_serverAddress.portNumber());

    if (0 != d_serverSocket_p->listen(queueSize)) {
        d_factory_p->deallocate(d_serverSocket_p);
        d_serverSocket_p = NULL;
        return e_LISTEN_FAILED;                                       // RETURN
    }

    if (0 != d_serverSocket_p->setBlockingMode(
                                           btlso::Flags::e_NONBLOCKING_MODE)) {
        d_factory_p->deallocate(d_serverSocket_p);
        d_serverSocket_p = NULL;
        return e_BLOCKMODE_FAILED;                                    // RETURN
    }
    return e_SUCCESS;
}

int TcpAcceptor::setOption(int level, int option, int value)
{
    BSLS_ASSERT(!d_isInvalidFlag);
    BSLS_ASSERT(d_serverSocket_p);

    return d_serverSocket_p->setOption(level, option, value);
}

// ACCESSORS
int TcpAcceptor::getOption(int *result, int level, int option) const
{
    BSLS_ASSERT(!d_isInvalidFlag);
    BSLS_ASSERT(d_serverSocket_p);

    return d_serverSocket_p->socketOption(result, level, option);
}

}  // close package namespace
}  // close enterprise namespace

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
