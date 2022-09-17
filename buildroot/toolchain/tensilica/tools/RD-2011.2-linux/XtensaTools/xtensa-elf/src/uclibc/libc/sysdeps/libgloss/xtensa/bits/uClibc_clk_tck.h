/* Use a default of 100 for CLK_TCK to implement sysconf() and clock().
 * Override this by supplying an arch-specific version of this header file.
 *
 * WARNING: It is assumed that this is a constant integer value usable in
 * preprocessor conditionals!!!
 */

#include <bits/time.h>

/* Since we aren't running on a linux host, the scale factor
   for clock() is actually the cycle count.  */

#define __UCLIBC_CLK_TCK_CONST		CLOCKS_PER_SEC
