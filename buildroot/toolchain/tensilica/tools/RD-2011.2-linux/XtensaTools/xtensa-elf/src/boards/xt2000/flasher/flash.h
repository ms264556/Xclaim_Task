#ifndef __H_FLASH
#define __H_FLASH


// Flash Programming Interface
//
// $Id$
// Copyright 2001 Tensilica Inc.
// These coded instructions, statements, and computer programs are
// Confidential Proprietary Information of Tensilica Inc. and may not be
// disclosed to third parties or copied in any form, in whole or in part,
// without the prior written consent of Tensilica Inc.


typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned long  BOOL;

#define FALSE 0
#define TRUE  1


typedef struct _FlashInfo
{
  // User needs to fill this in, the rest is 
  // determined at run time.
  DWORD 	flashBaseAddress;

  // --
  DWORD		flashSize;
  DWORD		numBlocks;
  DWORD		blockSize;

} FlashInfo;

typedef FlashInfo *PFlashInfo;



/*+*----------------------------------------------------------------------------
/ Function: FlashInitialize
/
/ Description:  Initializes the Flash driver.
/
/ Parameters: pFlashInfo -- (INPUT/OUTPUT) The field flashBaseAddress must
/			    be set by the caller.
/
/ Returns: TRUE   -- If the flash is found, and working
/          FALSE  -- If the flash is not working, or not at the specified address.
/-**----------------------------------------------------------------------------*/

BOOL FlashInitialize(PFlashInfo pFlashInfo);




/*+*----------------------------------------------------------------------------
/ Function: FlashRead
/
/ Description: Reads a chunk of the flash, and puts it into pBuffer.
/	       This is basically memcpy....
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/             startAddress -- Starting address in the flash (counting from 0)
/
/ Returns: The number of 4-byte WORDS put into pBuffer
/-**----------------------------------------------------------------------------*/

DWORD FlashRead(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer);



/*+*----------------------------------------------------------------------------
/ Function: FlashWrite
/
/ Description: Writes a chunk into the flash, from pBuffer.
/	       It is assumed that the block(s) that are being written
/	       have already been properly erased.
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      startAdrress -- Starting address in the flash (counting from 0)
/
/ Returns: The number of 4-byte WORDS put into pBuffer
/-**----------------------------------------------------------------------------*/

DWORD FlashWrite(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer);



/*+*----------------------------------------------------------------------------
/ Function: FlashBlockWrite
/
/ Description: Writes a chunk into the flash, from pBuffer.
/	       This function takes adavantage of the write buffer on the
/	       strataflash, and is much faster than FlashWrite
/	       It is assumed that the block(s) that are being written
/	       have already been properly erased.
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      startAdrress -- Starting address in the flash (counting from 0)
/
/ Returns: The number of 4-byte WORDS put into pBuffer
/-**----------------------------------------------------------------------------*/

DWORD FlashBlockWrite(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer);



/*+*----------------------------------------------------------------------------
/ Function: FlashErase
/
/ Description: Erases a single block in the flash.
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be erased (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was erased ok
/	   FALSE -- If an error occurred while erasing the block.
/-**----------------------------------------------------------------------------*/

BOOL FlashErase(PFlashInfo pFlashInfo, DWORD blockNumber);



/*+*----------------------------------------------------------------------------
/ Function: FlashProtect
/
/ Description: Turns on the protection bit for a block in the flash
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be protected 
/			      (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was protected ok
/	   FALSE -- If an error occurred while protecting the block.
/-**----------------------------------------------------------------------------*/

DWORD FlashProtect(PFlashInfo pFlashInfo, DWORD blockNumber);


/*+*----------------------------------------------------------------------------
/ Function: FlashUnprotect
/
/ Description: Turns on the protection off bit for a block in the flash
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be unprotected 
/			      (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was unprotected ok
/	   FALSE -- If an error occurred while unprotecting the block.
/-**----------------------------------------------------------------------------*/

DWORD FlashUnprotect(PFlashInfo pFlashInfo, DWORD blockNumber);



/*+*----------------------------------------------------------------------------
/ Function: FlashClearStatusRegister
/
/ Description: Clear the status register
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/
/ Returns: nothing
/-**----------------------------------------------------------------------------*/

void FlashClearStatusRegister(PFlashInfo pFlashInfo);


/*+*----------------------------------------------------------------------------
/ Function: FlashReadMode
/
/ Description: Put the flash back into read mode
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/
/ Returns: nothing
/-**----------------------------------------------------------------------------*/

void FlashReadMode(PFlashInfo pFlashInfo);

#endif
