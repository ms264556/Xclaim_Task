/*******************************************************************************

Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xt2000-display.c   Part of board-support-package for XT2000 board.

Implementation of BSP display functions on the 8-char LED display.

NOTE:
This implementation "fits in around" the pre-existing XT2000 support in 
libxt2000.a. It merely adds the xtbsp-*() functions to libxt2000.a, leaving
pre-existing functions alone. No attempt has been made to produce a clean 
XT2000 support package like the XT-AV60 package, because XT2000 is deprecated
and only used internally.

*******************************************************************************/

#include <xtensa/xtbsp.h>               /* BSP API */
#include <xtensa/xt2000.h>              /* XT2000 board information */

#define DISPLAY_VISIBLE_LEN 8           /* length (chars) of visible window */


#ifdef XTBOARD_LED_VADDR

static unsigned (*ledp)[DISPLAY_VISIBLE_LEN] = 
                        (unsigned(*)[])(XTBOARD_LED_VADDR + 0xe0);


int xtbsp_display_exists(void)
{
    return 1;
}

void xtbsp_display_init(void)
{
    /* nothing to do! */
}

void xtbsp_display_char(unsigned pos, const char c)
{
    if (pos >= DISPLAY_VISIBLE_LEN)
        return;

    *(char*) &(*ledp)[pos] = c;
}

void xtbsp_display_string(const char *s)
{
    volatile unsigned *p = &(*ledp)[0];
    unsigned *endp = &(*ledp)[DISPLAY_VISIBLE_LEN];

    /* display chars up to end of string or visible window */
    while (*s != '\0' && p < endp) {
        *(char*)p = *s++;
        ++p;
    }

    /* pad with spaces to end of visible window */
    while (p < endp) {
        *(char*)p = ' ';
        ++p;
    }
}

void xtbsp_display_blank(void)
{
    xtbsp_display_string("");
}

#else

int xtbsp_display_exists(void)
{
    return 0;
}

void xtbsp_display_init(void)
{
}

void xtbsp_display_char(unsigned pos, const char c)
{
}

void xtbsp_display_string(const char *s)
{
}

void xtbsp_display_blank(void)
{
}

#endif

