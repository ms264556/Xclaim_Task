/*******************************************************************************

Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xthifi2-bus.c       Bus Address Translation for XT-HiFi2 board.

Support for virtual to S6000 bus address mapping in XT-HiFi2,
for use in setting up DMA transfers. Note the bus address is 
not the Xtensa physical addresses, but is outside the processor.

This is a Tensilica reimplementation of corresponding SBIOS functions
required by DMA driven drivers ported from SBIOS. It assumes a static
memory map as previously set up by the miniboot loader for XT-HiFi2.

*******************************************************************************/


#include <xtensa/xthifi2.h>
#include <xtensa/xthifi2-sx-types.h>
#include <xtensa/xthifi2-bus.h>


/*
Converts a virtual address to the address that would be generated on
the processor bus, given the fixed memory mapping of XT-HiFi2.  
Note: In case one of the ranges we translate ends at the top (0xffffffff),
      we have to use (addr <= base + size - 1), not (addr < base + size).
*/

sx_uint32 sx_virt_to_bus(void *vaddr)
{
    unsigned addr = (unsigned)vaddr;

    /* Translate local memory addresses */
    #ifdef XCHAL_DATARAM0_VADDR
    if ((addr >= XCHAL_DATARAM0_VADDR) && 
            (addr <= XCHAL_DATARAM0_VADDR + XCHAL_DATARAM0_SIZE - 1))
        return S6_PIF_DATARAM1_BASE | 
                (addr & (S6_PIF_DATARAM1_END - S6_PIF_DATARAM1_BASE));
    #endif

    /* Translate system RAM virtual addreses */
    #ifdef XTBOARD_RAM_VADDR
    if (addr >= XTBOARD_RAM_VADDR && 
            addr <= XTBOARD_RAM_VADDR + XTBOARD_RAM_SIZE - 1)
        return XTHIFI2_DDR_BASE + (addr - XTBOARD_RAM_VADDR);
    #endif

    /* Translate system ROM virtual addreses */
    #ifdef XTBOARD_ROM_VADDR
    if (addr >= XTBOARD_ROM_VADDR && 
            addr <= XTBOARD_ROM_VADDR + XTBOARD_ROM_SIZE - 1)
        return XTHIFI2_DDR_TOP - XTBOARD_ROM_SIZE + (addr - XTBOARD_ROM_VADDR);
    #endif

    /* I/O and other addresses don't need translation */
    return addr;
}

