// Module:  LOG4XTENSA
// File:    socketappender.h
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

// 2005.09.01.  Tensilica.  Global replace of log4cplus/LOG4CPLUS with log4xtensa/LOG4XTENSA
//                          to avoid potential conflicts with customer code independently 
//                          using log4cplus.


/** @file */

#ifndef _LOG4XTENSA_SOCKET_APPENDER_HEADER_
#define _LOG4XTENSA_SOCKET_APPENDER_HEADER_

#include <log4xtensa/config.h>
#include <log4xtensa/appender.h>
#include <log4xtensa/helpers/socket.h>

#ifndef UNICODE
#  define LOG4XTENSA_MAX_MESSAGE_SIZE (8*1024)
#else
#  define LOG4XTENSA_MAX_MESSAGE_SIZE (2*8*1024)
#endif


namespace log4xtensa {

    /**
     * Sends {@link LoggingEvent} objects to a remote a log server.
     *
     * <p>The SocketAppender has the following properties:
     *
     * <ul>
     *   <p><li>Remote logging is non-intrusive as far as the log event is
     *   concerned. In other words, the event will be logged with the same
     *   time stamp, {@link org.apache.log4j.NDC}, location info as if it
     *   were logged locally by the client.
     *
     *   <p><li>SocketAppenders do not use a layout.
     *
     *   <p><li>Remote logging uses the TCP protocol. Consequently, if
     *   the server is reachable, then log events will eventually arrive
     *   at the server.
     *
     *   <p><li>If the remote server is down, the logging requests are
     *   simply dropped. However, if and when the server comes back up,
     *   then event transmission is resumed transparently. This
     *   transparent reconneciton is performed by a <em>connector</em>
     *   thread which periodically attempts to connect to the server.
     *
     *   <p><li>Logging events are automatically <em>buffered</em> by the
     *   native TCP implementation. This means that if the link to server
     *   is slow but still faster than the rate of (log) event production
     *   by the client, the client will not be affected by the slow
     *   network connection. However, if the network connection is slower
     *   then the rate of event production, then the client can only
     *   progress at the network rate. In particular, if the network link
     *   to the the server is down, the client will be blocked.
     *
     *   <p>On the other hand, if the network link is up, but the server
     *   is down, the client will not be blocked when making log requests
     *   but the log events will be lost due to server unavailability.
     */
    class LOG4XTENSA_EXPORT SocketAppender : public Appender {
    public:
      // Ctors
        SocketAppender(const log4xtensa::tstring& host, int port, 
                       const log4xtensa::tstring& serverName = tstring());
        SocketAppender(const log4xtensa::helpers::Properties properties);

      // Dtor
        ~SocketAppender();

      // Methods
        virtual void close();

    protected:
        void openSocket();
        virtual void append(const spi::InternalLoggingEvent& event);

      // Data
        log4xtensa::helpers::Socket socket;
        log4xtensa::tstring host;
        int port;
        log4xtensa::tstring serverName;

    private:
      // Disallow copying of instances of this class
        SocketAppender(const SocketAppender&);
        SocketAppender& operator=(const SocketAppender&);
    };

    namespace helpers {
        LOG4XTENSA_EXPORT
        SocketBuffer convertToBuffer(const log4xtensa::spi::InternalLoggingEvent& event,
                                     const log4xtensa::tstring& serverName);

        LOG4XTENSA_EXPORT
        log4xtensa::spi::InternalLoggingEvent readFromBuffer(SocketBuffer& buffer);
    } // end namespace helpers

} // end namespace log4xtensa

#endif // _LOG4XTENSA_SOCKET_APPENDER_HEADER_

