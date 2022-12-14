/* include/log4xtensa/config.h.  Generated by configure.  */
/* include/log4xtensa/config.h.in.  Generated from configure.in by autoheader.  */
/* Module:  Log4CPLUS
 * File:    config.h.in
 * Created: 6/2001
 * Author:  Tad E. Smith
 *
 *
 * Copyright (C) The Apache Software Foundation. All rights reserved.
 *
 * This software is published under the terms of the Apache Software
 * License version 1.1, a copy of which has been included with this
 * distribution in the LICENSE.APL file.
 */

// 2006.06.29.  Tensilica.  Remove unused macros and replace the PACKAGE macro to reduce namespace pollution.
// 2005.09.15.  Tensilica.  Global replace of log4cplus/LOG4CPLUS with log4xtensa/LOG4XTENSA
//                          to avoid potential conflicts with customer code independently
//                          using log4cplus.


/* Define if your <sys/socket.h> declares type socklen_t.  */
/* #undef socklen_t */

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `lstat' function. */
#define HAVE_LSTAT 1

/* Define to 1 if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define to 1 if you have the <strstream> header file. */
#define HAVE_STRSTREAM 1

/* Define to 1 if you have the <strstream.h> header file. */
/* #undef HAVE_STRSTREAM_H */

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define if this is a single-threaded library. */
#define LOG4XTENSA_SINGLE_THREADED 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

#ifdef _WIN32
#  include <log4xtensa/config-win32.h>

#elif (defined(__APPLE__) || (defined(__MWERKS__) && defined(__MACOS__)))
#  include <log4xtensa/config-macosx.h>

#endif // _WIN32

#if !defined(_WIN32)
#  if !defined(LOG4XTENSA_SINGLE_THREADED)
#    define LOG4XTENSA_USE_PTHREADS
#  endif
#  define LOG4XTENSA_EXPORT

#endif // !_WIN32

#include <log4xtensa/helpers/thread-config.h>

