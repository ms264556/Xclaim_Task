/*****************************************************************
*
*	Flash for Xtensa XT2000 board
*    
*	Module: flash_assert.h
*
* Copyright 2002 Tensilica Inc.
* These coded instructions, statements, and computer programs are
* Confidential Proprietary Information of Tensilica Inc. and may not be
* disclosed to third parties or copied in any form, in whole or in part,
* without the prior written consent of Tensilica Inc.
*
******************************************************************/
 
#define FLASHWARN 0
#define FLASHERROR 1
#define FLASHFATAL 2

#define flash_assert(cond, message, level, num) \
			{\
				if(!(cond))\
				{\
				  if((level) == FLASHWARN)\
				  {\
				    fprintf(stderr, "\nWarning %s %d <%s> : %s %d\n", __FILE__, __LINE__, #cond, (message), num);\
				  }\
				  else if((level) == FLASHERROR)\
				  {\
				    fprintf(stderr, "\nError %s %d <%s> : %s %d\n", __FILE__, __LINE__, #cond, (message), num);\
				  }\
				  else\
				  {\
				    fprintf(stderr, "\nFatal %s %d <%s> : %s %d\n", __FILE__, __LINE__, #cond, (message), num);\
				    exit(1);\
				  }\
				}\
			}\
