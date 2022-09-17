/*  Copyright (c) 2010 Tensilica Inc.  ALL RIGHTS RESERVED.
 *  This program is the copyrighted work of Tensilica Inc.  You may use
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.  Other use is prohibited
 *  without the prior written consent of Tensilica Inc.
 *
 *  This program is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.
 */

#include <xtensa/config/core.h>
#if XCHAL_HAVE_BBE16

#include <string.h>
#include <xtensa/tie/xt_bbe16.h>


void *__vec_memcpy(char *__restrict dest, const char *__restrict src, int n)
{
  valign out_align;
  xb_vec8x16 * __restrict out_align_addr, * __restrict in;
  int i;
  xb_vec8x20 t,t2,tmerge;
  vsel sel;


  if ( !(((int) dest) &1) && !(((int) src) &1)) { // 16-bit aligned
    if (!(((int) src) &15)) { // input is 128-bit aligned
      in = (xb_vec8x16 *) ((int )src);
      out_align_addr = (xb_vec8x16 *)(dest-16);
      out_align = BBE_ZALIGN128();
      i=0;
      if (n>15) {
        for(i = 0; i < n>>4; i++) {
          t = in[i];
          BBE_SA8X16S_IU(t, out_align, out_align_addr, 16);
        }
        BBE_SA8X16S_F(out_align, out_align_addr);
      }
      for (i=16*i; i<n; i++) {
        dest[i] = src[i];
      }
      return dest;
    }
    if (n < 64) {
      return memcpy(dest, src, n);
    }
    sel = BBE_SEL4V8X20(BBE_MOVVI8X20(56) + (xb_vec8x20)(short)(((int)src>>1)&7), 0);
    in = (xb_vec8x16 *) (((int )src) & (~15));
    in--;

    out_align_addr = (xb_vec8x16 *)(dest-16);
    out_align = BBE_ZALIGN128();

    BBE_LV8X16S_XU(t, in, 16);
    for(i = 0; i < n>>4; i++) {
       BBE_LV8X16S_XU(t2, in, 16);
       tmerge = BBE_SEL8X20(t2, t, sel);
       BBE_SA8X16S_IU(tmerge, out_align, out_align_addr, 16);
       t = t2;
    }
    BBE_SA8X16S_F(out_align, out_align_addr);

    for (i=16*i; i<n; i++) {
        dest[i] = src[i];
    }
    return dest;
  } else { // not 16-bit aligned
    return memcpy(dest, src, n);
  }
} /* vec_memcpy */


#endif

#if XCHAL_HAVE_HIFI3

#include <string.h>
#include <xtensa/tie/xt_hifi2.h>

void *__vec_memcpy(char *__restrict dest, const char *__restrict src, int n)
{
  ae_int16x4 * __restrict d_align_addr, * __restrict s_align_addr;
  int i;
  void *orig_dest = dest;


  if (n < 32) {
    return memcpy(dest, src, n);
  }
  if ( (((int) dest) %2) || (((int) src) %2)) { // 16-bit aligned
    if ( (((int) dest) %2) && (((int) src) %2)) { // 16-bit aligned
      *dest++ = *src++;
       n--;
    } else {
      return memcpy(dest, src, n);
    }
  }
  int n2 = n/2;
  valign d_align = AE_ZALIGN64();
  d_align_addr = (ae_int16x4 *) dest;
  s_align_addr = (ae_int16x4 *) src;
  valign s_align = AE_LA64_P(s_align_addr);
  ae_int16x4 t,t2;
  for (i=0; i<n2>>3; i++) {
      AE_LA16X4_IP(t, s_align, s_align_addr);
      AE_LA16X4_IP(t2, s_align, s_align_addr);
      AE_SA16X4_IP(t, d_align, d_align_addr);
      AE_SA16X4_IP(t2, d_align, d_align_addr);
  }
  AE_SA64POS_F(d_align, d_align_addr);
  ae_int16 *s_src = (ae_int16 *) src;
  ae_int16 *s_dest = (ae_int16 *) dest;
  for (i=8*i; i<n2; i++) {
    s_dest[i] = s_src[i];
  }
  if (n % 2) {
    dest[n-1] = src[n-1];
  }
  return orig_dest;

} /* vec_memcpy */


#endif

