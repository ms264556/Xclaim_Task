/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#ifndef LS_HEADER
#define LS_HEADER

#ifndef __XTENSA__

unsigned char
XT_L8UI(unsigned char *addr, int offset)
{
    unsigned char *mem = addr;
    return mem[offset];
}

unsigned short
XT_L16UI(unsigned short *addr, int offset)
{
    unsigned char *mem = (unsigned char *) (((unsigned) addr) & ~0x1);
    unsigned short b0, b1;
    if (IsaMemoryOrder == LittleEndian) {
	b0 = mem[offset+0];
	b1 = mem[offset+1];
    } else {
	b1 = mem[offset+0];
	b0 = mem[offset+1];
    }
    return (b1 << 8) | b0;
}

short
XT_L16SI(short *addr, int offset)
{
    unsigned char *mem = (unsigned char *) (((unsigned) addr) & ~0x1);
    unsigned short b0;
    short b1;
    if (IsaMemoryOrder == LittleEndian) {
	b0 = mem[offset+0];
	b1 = mem[offset+1];
    } else {
	b1 = mem[offset+0];
	b0 = mem[offset+1];
    }
    return (b1 << 8) | b0;
}

unsigned
XT_L32I(unsigned *addr, int offset)
{
    unsigned char *mem = (unsigned char *) (((unsigned) addr) & ~0x3);
    unsigned int b0, b1, b2, b3;
    if (IsaMemoryOrder == LittleEndian) {
	b0 = mem[offset+0];
	b1 = mem[offset+1];
	b2 = mem[offset+2];
	b3 = mem[offset+3];
    } else {
	b3 = mem[offset+0];
	b2 = mem[offset+1];
	b1 = mem[offset+2];
	b0 = mem[offset+3];
    }
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

void
XT_S8I(unsigned char data, unsigned char *addr, int offset)
{
    unsigned char *mem = addr;
    mem[offset] = data;
}

void
XT_S16I(unsigned short data, unsigned short *addr, int offset)
{
    unsigned char *mem = (unsigned char *) (((unsigned) addr) & ~0x1);
    if (IsaMemoryOrder == LittleEndian) {
	mem[offset+0] = data & 0xff;
	mem[offset+1] = (data >> 8) & 0xff;
    } else {
	mem[offset+1] = data & 0xff;
	mem[offset+0] = (data >> 8) & 0xff;
    }
}

void
XT_S32I(unsigned data, unsigned *addr, int offset)
{
    unsigned char *mem = (unsigned char *) (((unsigned) addr) & ~0x3);
    if (IsaMemoryOrder == LittleEndian) {
	mem[offset+0] = data & 0xff;
	mem[offset+1] = (data >> 8) & 0xff;
	mem[offset+2] = (data >> 16) & 0xff;
	mem[offset+3] = (data >> 24) & 0xff;
    } else {
	mem[offset+3] = data & 0xff;
	mem[offset+2] = (data >> 8) & 0xff;
	mem[offset+1] = (data >> 16) & 0xff;
	mem[offset+0] = (data >> 24) & 0xff;
    }
}

#endif /* __XTENSA__ */

#endif /* LS_HEADER */
