// Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2007 Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <xtensa/hal.h>

typedef unsigned int            u32;

u32 quit = 0;

#define TIMESTAMP fprintf(stdout, "%d: ", clock());

#define LOG(msg)                    { TIMESTAMP; fprintf(stdout, msg);                   fflush(stdout); }
#define LOG1(msg, arg1)             { TIMESTAMP; fprintf(stdout, msg, arg1);             fflush(stdout); }
#define LOG2(msg, arg1, arg2)       { TIMESTAMP; fprintf(stdout, msg, arg1, arg2);       fflush(stdout); }



void myhandler(int num) {
  quit = 1;
  LOG1("Got interrupt #%d \n", num);
}


#define INT_NUMBER 0

int main(int argc, char *argv[]) {
  LOG1("Program Name: %s \n", argv[0]);

  _xtos_set_interrupt_handler_arg(INT_NUMBER, myhandler, (void*)INT_NUMBER);
  _xtos_ints_on(1 << INT_NUMBER);

  while (!quit) {
    u32 data = rand();
    WRMSK_EXPSTATE(data, 0xFFFFFFFF);
    LOG2("%s wrote EXPSTATE = 0x%08x \n", argv[0], data);
  }

  WRMSK_EXPSTATE(0xDEADBEEF, 0xFFFFFFFF);

  return 0;
}

