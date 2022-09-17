/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

FILE *fdopen(int filedes, const char *mode)
{
	intptr_t cur_mode;

#ifdef __TARGET_xtensa__
	/* fcntl() not supported */
	if ( *mode == 'r' )
		cur_mode = S_IREAD;
	else
		cur_mode = S_IWRITE;
	return _stdio_fopen(cur_mode, mode, NULL, filedes);
#else
	return (((cur_mode = fcntl(filedes, F_GETFL))) != -1)
		? _stdio_fopen(cur_mode, mode, NULL, filedes) 
		: NULL;
#endif
}
