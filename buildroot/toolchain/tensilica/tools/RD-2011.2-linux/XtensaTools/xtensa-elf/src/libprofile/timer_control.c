/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* Turning off and on the timers is abstracted here so that different
   operating systems can easily replace the interrupt control code simply
   by impementing these functions themselves.  We don't use #include here
   because we want this reference to be weak.  */

extern unsigned int _xtos_ints_on(unsigned int interrupt_mask) __attribute__((weak));
extern unsigned int _xtos_ints_off(unsigned int interrupt_mask) __attribute__((weak));

void xt_profile_interrupt_on(unsigned int interrupt_mask)
{
  if (_xtos_ints_on)
    _xtos_ints_on(interrupt_mask);
  else {
    __asm__ volatile ("rsr.intenable a4\n or a2, a4, a2\n wsr.intenable a2\n");
  }
}

void xt_profile_interrupt_off(unsigned int interrupt_mask)
{
  if (_xtos_ints_off)
    _xtos_ints_off(interrupt_mask);
  else 
    __asm__ volatile ("rsr.intenable a4\n xor a2, a4, a2\n wsr.intenable a2\n");
}
