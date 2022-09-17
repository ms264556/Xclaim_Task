// Module:  Log4CPLUS
// File:    logger.h
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

// 2005.09.06.  Tensilica.  Use reference return type from getParent().
// 2005.09.06.  Tensilica.  Use reference for TraceLogger::logger.
// 2005.09.03.  Tensilica.  Use reference for getRoot.
// 2005.09.03.  Tensilica.  Use reference return type for Logger::getInstance(name).
// 2005.09.03.  Tensilica.  Use pointers for LoggerList.
// 2005.09.02.  Tensilica.  Use reference return type for Logger::getInstance(name, factory) and
//                          allow the Hierarchy to be specified.
// 2005.09.01.  Tensilica.  Modified to allow sub-classing of Logger by BinaryLogger.
// 2005.09.01.  Tensilica.  Use reference return type for makeNewLoggerInstance.
// 2005.09.01.  Tensilica.  Make Logger::forcedLog() and Logger::log() virtual.
// 2005.09.01.  Tensilica.  Global replace of log4cplus/LOG4CPLUS with log4xtensa/LOG4XTENSA
//                          to avoid potential conflicts with customer code independently 
//                          using log4cplus.


/** @file 
 * This header defines the Logger class and the logging macros. */

#ifndef _LOG4XTENSA_LOGGERHEADER_
#define _LOG4XTENSA_LOGGERHEADER_

#include <log4xtensa/config.h>
#include <log4xtensa/loglevel.h>
#include <log4xtensa/loggingmacros.h>
#include <log4xtensa/tstring.h>
#include <log4xtensa/streams.h>
#include <log4xtensa/helpers/pointer.h>
#include <log4xtensa/spi/appenderattachable.h>
#include <log4xtensa/spi/loggerfactory.h>

#include <memory>
#include <vector>

namespace log4xtensa {
  class BinaryLogger;
}

namespace log4xtensa {
    // Forward declarations
    namespace spi {
        class LoggerImpl;
        typedef helpers::SharedObjectPtr<LoggerImpl> SharedLoggerImplPtr;
    }
    class Appender;
    class Hierarchy;
    class HierarchyLocker;
    class DefaultLoggerFactory;


    /** \typedef std::vector<Logger> LoggerList
     * This is a list of {@link Logger Loggers}. */
    typedef std::vector<Logger*> LoggerList;


    /**
     * This is the central class in the log4xtensa package. One of the
     * distintive features of log4xtensa are hierarchical loggers and their
     * evaluation.
     * <p>
     * See the <a href="../../../../manual.html">user manual</a> for an
     * introduction on this class.
     */
    class LOG4XTENSA_EXPORT Logger : public log4xtensa::spi::AppenderAttachable {
    public:
      // Static Methods
        /**
         * Returns <code>true </code>if the named logger exists 
         * (in the default hierarchy).
         *                
         * @param name The name of the logger to search for.
         */
        static bool exists(const log4xtensa::tstring& name);

        /*
         * Returns all the currently defined loggers in the default
         * hierarchy.
         * <p>
         * The root logger is <em>not</em> included in the returned
         * list.     
         */
        static LoggerList getCurrentLoggers();
     
        /**
         * Return the default Hierarchy instance.
         */
        static Hierarchy& getDefaultHierarchy();

        /**
         * Retrieve a logger with name <code>name</code>.  If the named 
         * logger already exists, then the existing instance will be returned. 
         * Otherwise, a new instance is created. 
         * <p>
         * By default, loggers do not have a set LogLevel but inherit
         * it from the hierarchy. This is one of the central features of
         * log4xtensa.
         * <p>
         * @param name The name of the logger to retrieve.  
         */
        static Logger& getInstance(const log4xtensa::tstring& name);

        /**
         * Like {@link #getInstance(log4xtensa::tstring)} except that the type of logger
         * instantiated depends on the type returned by the {@link
         * spi::LoggerFactory#makeNewLoggerInstance} method of the
         * <code>factory</code> parameter.
         * <p>                         
         * This method is intended to be used by sub-classes.
         * <p>                                  
         * @param name The name of the logger to retrieve.
         * @param factory A {@link spi::LoggerFactory} implementation that will
         * actually create a new Instance.
         */
        static Logger& getInstance(const log4xtensa::tstring& name, spi::LoggerFactory& factory);

        /**
         * Return the root of the default logger hierrachy.
         * <p>
         * The root logger is always instantiated and available. It's
         * name is "root".
         * <p>
         * Nevertheless, calling {@link #getInstance
         * Logger.getInstance("root")} does not retrieve the root logger 
         * but a logger just under root named "root".
         */
        static Logger& getRoot();

        /**
         * Calling this method will <em>safely</em> close and remove all
         * appenders in all the loggers including root contained in the
         * default hierachy.
         * <p>                    
         * Some appenders such as SocketAppender need to be closed before the
         * application exits. Otherwise, pending logging events might be
         * lost.
         * <p>
         * The <code>shutdown</code> method is careful to close nested
         * appenders before closing regular appenders. This is allows
         * configurations where a regular appender is attached to a logger
         * and again to a nested appender.  
         */
        static void shutdown();

      // Non-Static Methods
        /**
         * If <code>assertion</code> parameter is <code>false</code>, then
         * logs <code>msg</code> as an {@link #error(const log4xtensa::tstring&) error} 
         * statement.
         *
         * @param assertion 
         * @param msg The message to print if <code>assertion</code> is
         * false.
         */
        void assertion(bool assertionVal, const log4xtensa::tstring& msg) {
            if(!assertionVal) {
                log(FATAL_LOG_LEVEL, msg);
            }
        }

        /**
         * Close all attached appenders implementing the AppenderAttachable
         * interface.  
         */
        void closeNestedAppenders();

        /**
         * Check whether this logger is enabled for a given {@link
         * LogLevel} passed as parameter.
         *
         * @return boolean True if this logger is enabled for <code>ll</code>.
         */
        bool isEnabledFor(LogLevel ll) const;

        /**
         * This generic form is intended to be used by wrappers. 
         */
        virtual void log(LogLevel ll, const log4xtensa::tstring& message,
                 const char* file=NULL, int line=-1);

        /**
         * This method creates a new logging event and logs the event
         * without further checks.  
         */
        virtual void forcedLog(LogLevel ll, const log4xtensa::tstring& message,
                       const char* file=NULL, int line=-1);

        /**
         * Call the appenders in the hierrachy starting at
         * <code>this</code>.  If no appenders could be found, emit a
         * warning.
         * <p>
         * This method calls all the appenders inherited from the
         * hierarchy circumventing any evaluation of whether to log or not
         * to log the particular log request.
         *
         * @param spi::InternalLoggingEvent the event to log.
         */
        void callAppenders(const spi::InternalLoggingEvent& event);

        /**
         * Starting from this logger, search the logger hierarchy for a
         * "set" LogLevel and return it. Otherwise, return the LogLevel of the
         * root logger.
         * <p>
         * The Logger class is designed so that this method executes as
         * quickly as possible.
         */
        LogLevel getChainedLogLevel() const;

        /**
         * Returns the assigned {@link LogLevel}, if any, for this Logger.  
         *           
         * @return LogLevel - the assigned LogLevel, can be <code>NOT_SET_LOG_LEVEL</code>.
         */
        LogLevel getLogLevel() const;

        /**
         * Set the LogLevel of this Logger.
         */
        void setLogLevel(LogLevel);

        /**
         * Return the the {@link Hierarchy} where this <code>Logger</code> instance is
         * attached.
         */
        Hierarchy& getHierarchy() const;

        /**
         * Return the logger name.  
         */
        log4xtensa::tstring getName() const;

        /**
         * Get the additivity flag for this Logger instance.  
         */
        bool getAdditivity() const;

        /**
         * Set the additivity flag for this Logger instance.
         */
        void setAdditivity(bool additive);


      // AppenderAttachable Methods
        virtual void addAppender(SharedAppenderPtr newAppender);

        virtual SharedAppenderPtrList getAllAppenders();

        virtual SharedAppenderPtr getAppender(const log4xtensa::tstring& name);

        virtual void removeAllAppenders();

        virtual void removeAppender(SharedAppenderPtr appender);

        virtual void removeAppender(const log4xtensa::tstring& name);

      // Dtor
        ~Logger();

        /**
         * Used to retrieve the parent of this Logger in the
         * Logger tree.
         */
        Logger& getParent();

    protected:
      // Data
        /** This is a pointer to the implementation class. */
        spi::LoggerImpl *value;

    private:
      // Copy Ctor
        Logger(const Logger& rhs);
        Logger& operator=(const Logger& rhs);

      // Ctors
        /**
         * This constructor created a new <code>Logger</code> instance 
         * with a pointer to a Logger implementation.
         * <p>
         * You should not create loggers directly.
         *
         * @param ptr A pointer to the Logger implementation.  This value
         *            cannot be NULL.  
         */
        Logger(spi::LoggerImpl *ptr);
        Logger(const spi::SharedLoggerImplPtr& val);

      // Methods
        void init();
        void validate(const char *file, int line) const;

      // Friends
        friend class log4xtensa::spi::LoggerImpl;
        friend class log4xtensa::Hierarchy;
        friend class log4xtensa::HierarchyLocker;
        friend class log4xtensa::DefaultLoggerFactory;
        friend class log4xtensa::BinaryLogger;
    };


    /**
     * This class is used to create the default implementation of
     * the Logger class
     */
    class LOG4XTENSA_EXPORT DefaultLoggerFactory : public spi::LoggerFactory {
    public:
        Logger& makeNewLoggerInstance(const log4xtensa::tstring& name, Hierarchy& h);
    };



    /**
     * This class is used to produce "Trace" logging.  When an instance of
     * this class is created, it will log a <code>"ENTER: " + msg</code>
     * log message if TRACE_LOG_LEVEL is enabled for <code>logger</code>.
     * When an instance of this class is destroyed, it will log a
     * <code>"ENTER: " + msg</code> log message if TRACE_LOG_LEVEL is enabled
     * for <code>logger</code>.
     * <p>
     * @see LOG4XTENSA_TRACE
     */
    class LOG4XTENSA_EXPORT TraceLogger {
    public:
        TraceLogger(Logger& l, const log4xtensa::tstring& _msg,
                    const char* _file=NULL, int _line=-1) 
          : logger(l), msg(_msg), file(_file), line(_line)
        { if(logger.isEnabledFor(TRACE_LOG_LEVEL))
              logger.forcedLog(TRACE_LOG_LEVEL, LOG4XTENSA_TEXT("ENTER: ") + msg, file, line); 
        }

        ~TraceLogger()
        { if(logger.isEnabledFor(TRACE_LOG_LEVEL))
              logger.forcedLog(TRACE_LOG_LEVEL, LOG4XTENSA_TEXT("EXIT:  ") + msg, file, line); 
        }

    private:
        Logger& logger;
        log4xtensa::tstring msg;
        const char* file;
        int line;
    };

} // end namespace log4xtensa


#endif // _LOG4XTENSA_LOGGERHEADER_

