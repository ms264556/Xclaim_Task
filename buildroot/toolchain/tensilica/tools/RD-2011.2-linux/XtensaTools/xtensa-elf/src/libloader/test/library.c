/* Copyright (c) 1999-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.
*/

#include <stdio.h>
	
/* declare a printf function pointer */
int (*printf_ptr)(const char *format, ...);
/* replace all calls to printf with calls through the pointer. */
#define printf printf_ptr
	
/* This is the function provided by the library. */
char * foo(void)
{
  printf("executing function foo\n");
  return "this string returned from foo";
}
	
/* This function is called from the main application. */
void * _start(int (*printf_func)(const char * format, ...))
{
  printf_ptr = printf_func;

  return foo;
}
