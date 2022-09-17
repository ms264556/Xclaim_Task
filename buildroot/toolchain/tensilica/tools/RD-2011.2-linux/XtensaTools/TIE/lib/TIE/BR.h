/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#ifndef BR_HEADER
#define BR_HEADER

#ifndef __XTENSA__

typedef unsigned char xtbool;
typedef unsigned char xtbool2;
typedef unsigned char xtbool4;
typedef unsigned char xtbool8;
typedef unsigned short xtbool16;

xtbool
XT_ANDB(xtbool bs, xtbool bt)
{
    return 0x1 & (bs & bt);
}

xtbool
XT_ANDBC(xtbool bs, xtbool bt)
{
    return 0x1 & (bs & !bt);
}

xtbool
XT_ORB(xtbool bs, xtbool bt)
{
    return 0x1 & (bs | bt);
}

xtbool
XT_ORBC(xtbool bs, xtbool bt)
{
    return 0x1 & (bs | !bt);
}

xtbool
XT_XORB(xtbool bs, xtbool bt)
{
    return 0x1 & (bs ^ bt);
}

xtbool
XT_ANY4(xtbool4 bs4)
{
    return (bs4 & 0xf) != 0;
}

xtbool
XT_ALL4(xtbool4 bs4)
{
    return (bs4 & 0xf) == 0xf;
}

xtbool
XT_ANY8(xtbool8 bs8)
{
    return (bs8 & 0xff) != 0;
}

xtbool
XT_ALL8(xtbool8 bs8)
{
    return (bs8 & 0xff) == 0xf;
}

#endif /* __XTENSA__ */

#endif /* BR_HEADER */
