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
#include <string.h>

#if XCHAL_HAVE_BBE16

#include <xtensa/tie/xt_bbe16.h>

void * __vec_memset(void *s, int c, size_t n)
{
  int i;
  int addr = ((int) s);
  unsigned char cdata = ((unsigned char) c);
  valign out_align;
  if (n > 0) {
    if (addr & 0x1) {
        *(unsigned char *)addr=cdata;
        n--;
        addr++;
    }
    i = 0;
    if (n > 15) {
      xb_vec8x16 *v_addr = (xb_vec8x16 *) addr;
      v_addr--;
      short sdata= cdata ;
      sdata = sdata | (sdata << 8);
      xb_vec8x20 vdata = sdata;
      out_align = BBE_ZALIGN128();
      for(i = 0; i < n/16; i++) {
        BBE_SA8X16S_IU(vdata, out_align, v_addr, 16);
      }
      BBE_SA8X16S_F(out_align, v_addr);
    }
    for (i=16*i; i<n; i++) {
        ((unsigned char *) addr)[i] = cdata;
    }
  }
  return s;

}

#endif

#if XCHAL_HAVE_HIFI3

#include <xtensa/tie/xt_hifi2.h>

void * __vec_memset(void *s, int c, size_t n)
{
  if (n > 15) {
    int i;
    int addr = ((int) s);
    unsigned char cdata = ((unsigned char) c);
    valign out_align;
    if (addr & 0x1) {
        *(unsigned char *)addr=cdata;
        n--;
        addr++;
    }
    if (n & 0x1) {
        ((unsigned char *)addr)[n-1]=cdata;
        n--;
    }
    // addr is now guaranteed to be 16-bit aligned and n is a multiple of 2
    n = n >> 1;
    ae_int16x4 *v_addr = (ae_int16x4 *) addr;
    short sdata= cdata ;
    sdata = sdata | (sdata << 8);
    ae_int16x4 vdata = sdata;
    out_align = AE_ZALIGN64();
    for(i = 0; i < n>>2; i++) {
        AE_SA16X4_IP(vdata, out_align, v_addr);
    }
    AE_SA64POS_F(out_align, v_addr);
    ae_int16 *saddr = (ae_int16 *)addr;
    for (i=i<<2; i<n; i++) {
        saddr[i] = sdata;
    }
  } else {
    memset(s, c, n);
  }
  return s;

}


#endif

#if XCHAL_HAVE_HIFI2

#include <xtensa/tie/xt_hifi2.h>

void * __vec_memset(void *s, int c, size_t n)
{
  int addr = ((int) s);
  int i;
  if (c==0) { // hifi has 48/56-bit stores but no 64-bit ones
    while (n>0 && addr & 7) {
      *(unsigned char *)addr=0;
      n--;
      addr++;
    }
    ae_p24x2s *simd_addr = (ae_p24x2s *) addr;
    for(i = 0; i < n>>3; i++) {
        simd_addr[i] = AE_ZEROP48();
    }
    for (i=8*i; i<n; i++) {
        ((unsigned char *) addr)[i] = 0;
    }
  } else {
    unsigned char cdata = ((unsigned char) c);
    while (n>0 && addr & 3) {
      *(unsigned char *)addr=cdata;
      n--;
      addr++;
    }
    int data= cdata ;
    data = data | (data << 8) | (data << 16) | (data << 24);
    ae_q32s simd_data = *(ae_q32s *) &data;
    ae_q32s *simd_addr = (ae_q32s *) addr;
    for(i = 0; i < n>>2; i++) {
        simd_addr[i] = simd_data;
    }
    for (i=4*i; i<n; i++) {
        ((unsigned char *) addr)[i] = cdata;
    }
  }
  return s;

}
#endif

