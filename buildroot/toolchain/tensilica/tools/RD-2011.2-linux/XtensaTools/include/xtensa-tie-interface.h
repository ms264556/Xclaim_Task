/* The Xtensa TIE DLL interface used by TC.
   This is _NOT_ the interface for users -- see xtensa-tie.h.  */

/* Copyright (c) 2004 Tensilica, Inc.  All Rights Reserved.

   This program is the copyrighted work of Tensilica Inc.  You may use
   it under the terms of version 2 of the GNU General Public License as
   published by the Free Software Foundation.  Other use is prohibited
   without the prior written consent of Tensilica Inc.

   This program is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  */

#ifndef XTENSA_TIE_INTERFACE_H
#define XTENSA_TIE_INTERFACE_H


/* Interface version: Used to provide meaningful error messages when
   the DLL interface changes; could potentially be used to implement
   backward compatibility at some point. */
#define TIE_INTERFACE_VERSION 1.0
extern double interface_version (void);


#endif /* XTENSA_TIE_INTERFACE_H */
