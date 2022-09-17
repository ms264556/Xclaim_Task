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

#if !defined (XT_REF_TESTBENCH_H)
#define XT_REF_TESTBENCH_H 1

// Exit routines for HW simulation
//
extern int diag_pass();
extern int diag_fail();

//
// Exit routines with status message
// 
static inline
int pass(const char *msg) 
{
  return diag_pass();
}

static inline
int fail(const char *msg) 
{
  return diag_fail();
}

#endif // XT_REF_TESTBENCH_H

