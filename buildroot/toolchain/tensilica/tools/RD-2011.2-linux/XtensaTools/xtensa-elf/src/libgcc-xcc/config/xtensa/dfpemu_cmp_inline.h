/* Functions for inlining double precision floating point comparison */

/*  Copyright (c) 2007-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
 *  This program is the copyrighted work of Tensilica Inc.  You may use
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.  Other use is prohibited
 *  without the prior written consent of Tensilica Inc.
 *
 *  This program is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */


#ifndef __DFPEMU_CMP_INLINE_H__
#define __DFPEMU_CMP_INLINE_H__
#include "dfpemu_fp.h"
#include <xtensa/tie/dfpemu_lib.h>

union dfpemu_dblll {
   unsigned long long ll;
   double dbl;
};

inline unsigned long long dfpemu_dbl2ll(double a)
{
  union dfpemu_dblll tmp_a;
  tmp_a.dbl = a;
  return tmp_a.ll;
}
  
inline int dfpemu_eq(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, EQ_OP);
}

inline int dfpemu_ne(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, NE_OP);
}

inline int dfpemu_lt(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, LT_OP);
}

inline int dfpemu_le(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, LE_OP);
}

inline int dfpemu_gt(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, GT_OP);
}

inline int dfpemu_ge(double a, double b)
{
  unsigned long long tmp_a = dfpemu_dbl2ll(a);
  unsigned long long tmp_b = dfpemu_dbl2ll(b);
  F64CMPL((unsigned)tmp_a, (unsigned)tmp_b);
  return F64CMPH(tmp_a >> 32, tmp_b >> 32, GE_OP);
}

#endif /* __DFPEMU_CMP_INLINE_H__ */
