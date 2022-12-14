// Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2006 Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


#include <stdio.h>
#include <time.h>
#include <xtensa/tie/example.h>

typedef unsigned int            u32;

#define TIMESTAMP fprintf(stdout, "%d: ", clock());

#define LOG(msg)                          { TIMESTAMP; fprintf(stdout, msg);                         fflush(stdout); }
#define LOG1(msg, arg1)                   { TIMESTAMP; fprintf(stdout, msg, arg1);                   fflush(stdout); }
#define LOG2(msg, arg1, arg2)             { TIMESTAMP; fprintf(stdout, msg, arg1, arg2);             fflush(stdout); }
#define LOG3(msg, arg1, arg2, arg3)       { TIMESTAMP; fprintf(stdout, msg, arg1, arg2, arg3);       fflush(stdout); }
#define LOG4(msg, arg1, arg2, arg3, arg4) { TIMESTAMP; fprintf(stdout, msg, arg1, arg2, arg3, arg4); fflush(stdout); }

int main(int argc, char *argv[]) {
  u32 i;

  LOG1("Program Name: %s \n", argv[0]);

  for (i=0; i<3; ++i) {
    if (EMPTY_Q1()) {
      LOG("queue has no data \n");
    }
    else {
      READ_Q1();
      LOG3("consumed 0x%08x%08x%08x \n", RUR_QBUF_2(), RUR_QBUF_1(), RUR_QBUF_0());
    }
  }

  return 0;
}
