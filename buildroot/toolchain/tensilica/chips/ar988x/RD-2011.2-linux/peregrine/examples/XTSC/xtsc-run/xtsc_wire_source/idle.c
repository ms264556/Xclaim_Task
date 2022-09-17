// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <stdio.h>
#include <time.h>

typedef unsigned int u32;

#define TIMESTAMP fprintf(stdout, "%d: ", clock());
#define LOG(msg) { TIMESTAMP; fprintf(stdout, msg); fflush(stdout); }
#define LOG1(msg, arg1) { TIMESTAMP; fprintf(stdout, msg, arg1); fflush(stdout); }


int main(int argc, char *argv[]) {
  u32 start = clock();

  LOG1("Program '%s' idling for 400000 clock cycles: \n", argv[0]);

  while (clock() < start + 400000) ;

  LOG("Done idling.\n");

  return 0;
}

