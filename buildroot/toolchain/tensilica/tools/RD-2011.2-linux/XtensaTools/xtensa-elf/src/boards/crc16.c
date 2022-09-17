/* crc16.c - 16-bit CRC routine */
/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

static const unsigned short crc16_ltab[16] =
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7
};

static const unsigned short crc16_utab[16] =
{
	0x0000, 0x1081, 0x2102, 0x3183, 0x4204, 0x5285, 0x6306, 0x7387,
	0x8408, 0x9489, 0xa50a, 0xb58b, 0xc60c, 0xd68d, 0xe70e, 0xf78f
};

static unsigned calc_crc16(unsigned short fcs, char* buf, int len)
{
    unsigned char c;
    while (len--) {
        c = (fcs ^ *buf++)&0xff;
        fcs = fcs >> 8 ^ crc16_ltab[c&0x0f] ^ crc16_utab[c>>4];
    }
    return fcs;
}

