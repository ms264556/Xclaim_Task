// Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
  printf("%d: Hello, core0 World!\n", clock());
  return 0;
}

