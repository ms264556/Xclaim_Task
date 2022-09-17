#ifndef __SYS_REENT_H__
#define __SYS_REENT_H__

/* this file exists to provide compatibility between newlib 
   style reentrance handling and uClibc reentrance handling.

   Newlib uses an extra parameter to provide a reentrance 
   structure which contains all thread-specific globals. uClibc uses
   function to get the address of errno, other reentrance and global 
   variable issues are handled separately.

   In order to make your libgloss port compatible with both reentrant 
   newlib and uClibc, you need to write functions that:
   * call from the non-reentrant versions to the reentrant ones. ie:
       _write calls _write_r
   * supply a reentrance structure to the reentrant versions
   * copy the values out of the reentrance structure back into uClibc's 
     globals

   See xtensa/syscalls.c for an example.

   The xtensa port wants binary compatibility and xtensa reentrant system 
   calls only refer to errno, so that is the only field in this struct. 
   Fortunately, in newlib if _REENT_SMALL is not defined, then _errno 
   is the first field in the struct, and we can just pass pointers 
   directly to the thread specific errno out of uClibc. That way any write
   to reent->_errno will also write the correct errno and we don't have
   to copy it out by hand. If your port of libgloss refers to more
   than just reent->_errno, you'll have to do it by hand.

   You can extend the _reent structure if you need to. But the layout
   of the fields shared between newlib and uClibc _MUST_ match each other
   if you want binary compatibility. If you just want source level 
   compatibility, then the layout doesn't need to match at all, just the field
   names.
   
   There are probably other ways to do this. In particular, you may want
   to compile libgloss without the reentrant functions defined (it is
   smaller that way, after all) and then you wouldn't need any of this
   hackery. But with this method you will still get reentrance, which may
   be important to you.   

   We intentionally don't publicly prototype the reentrant versions of
   the functions the way newlib does because uClibc's mechanism makes 
   calling the reentrant versions directly unnecessary.
*/

struct _reent
{
  int _errno;
};

#endif

