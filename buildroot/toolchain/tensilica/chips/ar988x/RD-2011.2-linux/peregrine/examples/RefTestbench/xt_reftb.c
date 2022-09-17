/* 
 * Customer ID=8327; Build=0x3b95c; Copyright (c) 2009-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Tensilica Inc.  They may be adapted and modified by bona fide
 * purchasers for internal use, but neither the original nor any
 * adapted or modified version may be disclosed or distributed to
 * third parties in any manner, medium, or form, in whole or in part,
 * without the prior written consent of Tensilica Inc.
 *
 * This software and its derivatives are to be executed solely on
 * products incorporating a Tensilica processor.
 */

// Utility routines for returning pass/fail status in HW simulations

#ifdef __XTENSA__

#include <xtensa/hal.h>
#include <xtensa/config/core.h>
#include <xtensa/config/system.h>
#include <xtensa/xt_reftb.h>

#if XCHAL_HAVE_HALT
#include <xtensa/tie/xt_halt.h>
#endif

// Write exit status to this address
// You may change this location by compiling with "-DTB_EXIT_LOCATION=<address>"
// Then you must also change the plusarg "+DVMagicExit" sent to the HW simulator
// or change the argument "--exit_location" sent to the ISS
//
#ifndef TB_EXIT_LOCATION
#define TB_EXIT_LOCATION XSHAL_MAGIC_EXIT
#endif 

// Write diag status result to this address
// This location must be in bypass region
// 
volatile static unsigned int *exit_location = (unsigned int*) TB_EXIT_LOCATION;

// Diags expect status 1 or 2
#define DIAG_PASS_STATUS 1
#define DIAG_FAIL_STATUS 2
#define MAGIC_EXIT_FLAG 13

//
// Return exit status location
//
unsigned int* 
testbench_exit_location()
{
  return (unsigned int*) exit_location;
}

//
// Set exit status for HW simulation
// Monitors.v will detect the halt and exit the simulation. 
//
int set_diag_status(int stat) 
{

#if XCHAL_HAVE_PIF
  xthal_set_region_attribute((void *)exit_location, 4, XCHAL_CA_BYPASS, 0);
#endif

#if XCHAL_HAVE_HALT
// 1) Write status (PASS or FAIL) to magic address      
// 2) Do a memw to make sure write has completed
// 3) Issue halt instruction
//
  *exit_location = stat;  
  XT_HALT();

#else  // XCHAL_HAVE_HALT

// 1) Write MAGIC_EXIT_FLAG to magic address
// 2) Write status (PASS or FAIL) to magic address      
//
  *exit_location = MAGIC_EXIT_FLAG;  
  *exit_location = stat;  

#endif  // XCHAL_HAVE_HALT
  return stat;
}

//
// Exit routines
// Set status then exit
// 

int diag_pass()
{
  return set_diag_status(DIAG_PASS_STATUS);
}

int diag_fail()
{
  return set_diag_status(DIAG_FAIL_STATUS);
}

#else // __XTENSA__

int diag_pass()
{
  return 0;
}

int diag_fail()
{
  return 0;
}


#endif // __XTENSA__

