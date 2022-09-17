/* Define the machine-dependent type `jmp_buf'.  Xtensa version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#if __XTENSA_WINDOWED_ABI__

/* The jmp_buf structure for Xtensa windowed ABI holds the following
   (where "proc" is the procedure that calls setjmp): 4-12 registers
   from the window of proc, the 4 words from the save area at proc's $sp
   (in case a subsequent alloca in proc moves $sp), and the return
   address within proc.  Everything else is saved on the stack in the
   normal save areas.  The jmp_buf structure is:
  
  	struct jmp_buf {
  	    int regs[12];
  	    int save[4];
  	    void *return_address;
  	}
  
   See the setjmp code for details.  */

#define _JBLEN		17	/* 12 + 4 + 1 */

#else /* __XTENSA_CALL0_ABI__ */

#define _JBLEN		6	/* a0, a1, a12, a13, a14, a15 */

#endif /* __XTENSA_CALL0_ABI__ */

typedef int __jmp_buf[_JBLEN];
