/*
 Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2007 Tensilica Inc.  ALL RIGHTS RESERVED.
 These coded instructions, statements, and computer programs are the
 copyrighted works and confidential proprietary information of Tensilica Inc.
 They may not be modified, copied, reproduced, distributed, or disclosed to
 third parties in any manner, medium, or form, in whole or in part, without
 the prior written consent of Tensilica Inc.
*/

#include "xt_library_loader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* This library will be provided by the object created by the 
   package_loadable_lib.pl tool. 

   Observe that the name declared here must match the name used with
   the "-e" option to the tool.
*/
extern xtlib_packaged_library pi_library;

/* Declare a function pointer of the same type as the _start function
   provided in the overlay. This function can have whatever prototype you
   choose, but for convenience, it most often takes and returns function
   pointers for calling into and out of the loadable library.
*/

void * (*pil_start)(void * printf_ptr);

/* Declare a function pointer of same type as the function to call
   in the library. We will call into the library using this function
   pointer.
*/

char * (*foo_ptr) (void);

int main(int argc, char * argv[])
{
  unsigned int bytes = xtlib_pi_library_size(&pi_library);
  void * memory = malloc(bytes);
  xtlib_pil_info lib_info;

  printf("Loading %d-byte position independent library at 0x%x...", 
	 bytes, (unsigned int)memory);
  fflush(NULL);
  pil_start = xtlib_load_pi_library(&pi_library, memory, &lib_info);
  if (pil_start != NULL) {
    printf("succeeded\n");
    /* We successfully loaded the library. Now call it's entry point so it can
       get a pointer to printf, and it will return a pointer to its foo 
       function.
       
       If we wanted to use more functions, we could pass and return arrays of
       pointers.
       
       Also, if the interface to the library is just a single function, then
       the entry point itself could serve as that function.       
    */
    char * string_returned_from_foo;
       
    foo_ptr = pil_start(printf);
    string_returned_from_foo = foo_ptr();
    printf("function returned the string \"%s\"\n", string_returned_from_foo);
  }
  else {
    printf("failed with error code: %d\n", xtlib_error());
  }
  return 0;
}
