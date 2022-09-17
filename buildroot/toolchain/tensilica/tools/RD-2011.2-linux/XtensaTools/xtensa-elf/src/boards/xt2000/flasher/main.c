// Copyright 2005 Tensilica Inc.
// These coded instructions, statements, and computer programs are
// Confidential Proprietary Information of Tensilica Inc. and may not be
// disclosed to third parties or copied in any form, in whole or in part,
// without the prior written consent of Tensilica Inc.

//  modified flash routine for a 32/64MB flash module
// This program does a 32/64MB erase
// and a 32/64MB program
// and gives direction via printf's


#include "flash.h"
#include <stdio.h>
#include <xtensa/xt2000.h>
#define FLASH_BASE_ADDRESS XTBOARD_FLASH_VADDR
// 0x18000000
#define FPGA_REGISTERS_ADDRESS XT2000_FPGAREGS_VADDR
// 0x1d020000

// Defaults to 32MB, change to 64 for 64MB module
#define FLASH_SIZE 32					 
// Defaults to 64 sectors (64 * 512kbytes) = 32Mbytes
#define PROGRAM_SIZE ((FLASH_SIZE *2*512*1024)/4)


int main()
{
  int       i           		= 0;
  FlashInfo flashInfo   		= {0};
  BOOL      success     		= FALSE;
  DWORD     *program_buffer             = 0;
  DWORD     program_size_in_words       = PROGRAM_SIZE;
  DWORD     wordsWritten		= 0;
  int       *pFpgaRegs                  = (int*)FPGA_REGISTERS_ADDRESS;
  volatile int program_flash = 0;
  int       kbd_input;

  // Clear the protection bit on the flash in the fpga registers
  pFpgaRegs[3] = 0;

  // If you want a bigger/smaller buffer, just break before this line
  // and change the variable program_size_in_words to a the proper value

  program_buffer = (DWORD *)malloc( program_size_in_words * 4);


  flashInfo.flashBaseAddress = FLASH_BASE_ADDRESS;

  success = FlashInitialize(&flashInfo);
  
  if (success)
  {
    printf("\n\n********%dMB Flash ERASE and PROGRAM********",FLASH_SIZE);
    printf("\n Press 'e'(cr) to erase");
    while (getchar() != 'e')
	;
    printf("\n Flash erase");
    // This will erase all 64 sectors...
    for (i = 0; i < (FLASH_SIZE*2); ++i)
    {
	 printf(".");
	 fflush(stdout);
       success = FlashEraseBlock(&flashInfo, i);
       if ( !success )
         break;
    }
  }
  else
	printf("\n Flash Initialilize not successful");

  if (success)
  {
    printf("\nFlash successfully erased");
    printf("\n To program the flash follow these 4 steps, at the Xtensa GDB (xt-gdb) prompt:");	
    printf("\n1. (xt-gdb) CTRL-C");
    printf("\n2. (xt-gdb) restore <filename> binary &program_buffer[<offset>/4]");
    printf("\n3. (xt-gdb) set program_flash=1");
    printf("\n4. (xt-gdb) c");
    printf("\n where <filename> is the name of a binary file containing the image to write,");
    printf("\n and <offset> is the offset in bytes from start of Flash to your reset vector.");
  }
  else
   printf("\nError, Flash not erased");
  fflush(stdout);

  while (program_flash == 0)
	;

  printf("\n Flash programming... takes 60 seconds");
  fflush(stdout);

  if (program_buffer && success)
  {
    //
    // And viola, your program will be written to the flash
    //
    // If you don't want to write at the beginning of the flash, just
    // change that ZERO 

    wordsWritten = FlashBlockWrite(&flashInfo, 0, program_size_in_words, program_buffer);
  }

 if (success)
  {
    printf("\n Flash programmed successfully     Dwords written=%08x", wordsWritten);
  }
  else
   printf("\nError, Flash not programmed");
  program_flash = 0;

  return 0;
}
  





