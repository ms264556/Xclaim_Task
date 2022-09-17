// Flash Programming Interface
//
// $Id$
// Copyright 2002 Tensilica Inc.
// These coded instructions, statements, and computer programs are
// Confidential Proprietary Information of Tensilica Inc. and may not be
// disclosed to third parties or copied in any form, in whole or in part,
// without the prior written consent of Tensilica Inc.


#include <stdio.h>
#include "flash.h"



#define FlashCommandReadID      0x90909090
#define FlashCommandRead        0xFFFFFFFF
#define FlashCommandErase       0x20202020
#define FlashCommandConfirm     0xD0D0D0D0
#define FlashCommandClear       0x50505050
#define FlashCommandWrite       0x40404040
#define FlashCommandLoadPB      0xE0E0E0E0
#define FlashCommandPBWrite     0x0C0C0C0C
#define FlashCommandStatus      0x70707070
#define FlashCommandSuspend     0xB0B0B0B0
#define FlashCommandResume      0xD0D0D0D0
#define FlashCommandReadESR     0x71717171
#define FlashCommandQueryCFI    0x98989898
#define FlashCommandSCSErase    0x28282828
#define FlashCommandSCSWrite    0xE8E8E8E8
#define FlashCommandLockSetup	0x60606060
#define FlashCommandLockConfirm	0x01010101
#define FlashCommandUnlockSetup	0x60606060
#define FlashCommandUnlockConfirm 0xD0D0D0D0
#define FlashStatusReady        0x80808080
#define FlashStatusSuspended    0x40404040
#define FlashStatusError        0x3E3E3E3E
#define FlashStatusBlockError   0x3F3F3F3F


#define LETTERS_QQQQ 0x51515151
#define LETTERS_RRRR 0x52525252
#define LETTERS_YYYY 0x59595959


typedef struct
  {
    DWORD ManufacturerCode[2];			 /* offset  00h */
    DWORD DeviceCode[2];			 /* offset  08h */
    DWORD BlockStatusRegister[2];		 /* offset  10h */
    BYTE  Reserved[0x68];                        /* offset  18h */
    DWORD  SignatureQ[2];                        /* offset  80h */
    DWORD  SignatureR[2];                        /* offset  88h */
    DWORD  SignatureY[2];                        /* offset  90h */
    BYTE  CmdSet[16];                            /* offset  98h */
    BYTE  CmdSetAddr[16];                        /* offset  a8h */
    BYTE  AltCmdSet[16];                         /* offset  b8h */
    BYTE  AltCmdSetAddr[16];                     /* offset  c8h */
    BYTE  MinVcc[8];                             /* offset  d8h */
    BYTE  MaxVcc[8];                             /* offset  e0h */
    BYTE  MinVpp[8];                             /* offset  e8h */
    BYTE  MaxVpp[8];                             /* offset  f0h */
    BYTE  typBytePgmTime[8];                     /* offset  f8h */
    BYTE  typBufferPgmTime[8];                   /* offset 100h */
    BYTE  typBlockEraseTime[8];                  /* offset 108h */
    BYTE  typChipEraseTime[8];                   /* offset 110h */
    BYTE  maxBytePgmTime[8];                     /* offset 118h */
    BYTE  maxBufferPgmTime[8];                   /* offset 120h */
    BYTE  maxBlockEraseTime[8];                  /* offset 128h */
    BYTE  maxChipEraseTime[8];                   /* offset 130h */
    BYTE  DeviceSize[8];                         /* offset 138h */
    BYTE  Interface[16];                         /* offset 140h */
    BYTE  WriteSize[16];                         /* offset 150h */
    BYTE  BlkRegions[8];                         /* offset 160h */
#if !CFI_ERASE_REGIONS
    BYTE  BlksRegion[16];                        /* offset 168h */
#else
    BYTE BlksRegion[MAX_ERASE_REGIONS][32];
#endif
} CFIx8x16QUAD_STRUCT;


/*+*----------------------------------------------------------------------------
/ Function FlashInitialize
/
/ Description:  Initializes the Flash driver.
/
/ Parameters: pFlashInfo -- (INPUT/OUTPUT) The field flashBaseAddress must
/			    be set by the caller.
/
/ Returns: TRUE   -- If the flash is found, and working
/          FALSE  -- If the flash is not working, or not at the specified address.
/-**----------------------------------------------------------------------------*/

BOOL FlashInitialize(PFlashInfo pFlashInfo)
{
  BOOL  	       	        bSuccess = FALSE;
  volatile DWORD 		*pointer = 0;
  volatile CFIx8x16QUAD_STRUCT 	*cfiInfo = 0;


  if (pFlashInfo == NULL)
    goto exit_gracefully;

  pointer = (DWORD *)pFlashInfo->flashBaseAddress;
  *pointer = (DWORD)FlashCommandQueryCFI;

  cfiInfo = (CFIx8x16QUAD_STRUCT *)pointer;


  // Verify that this is really a Flash

  if ( (cfiInfo->SignatureQ[0] == LETTERS_QQQQ) &&
       (cfiInfo->SignatureR[0] == LETTERS_RRRR) &&
       (cfiInfo->SignatureY[0] == LETTERS_YYYY))
    {
      // It is indeed a flash
      // Collect up some information

      // !! for now we'll just fake it

      pFlashInfo->numBlocks = 64;
      pFlashInfo->blockSize = 0x20000 * 4;
      pFlashInfo->flashSize = pFlashInfo->numBlocks*pFlashInfo->blockSize;

      bSuccess = TRUE;

      *pointer = (DWORD)FlashCommandRead;
    }

  
 exit_gracefully:

  return bSuccess;
}
/*-*----------------------------------------------------------------------------*/




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

DWORD FlashRead(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer)
{
  DWORD wordsRead = 0;
  DWORD *pCurrent = (DWORD *)pFlashInfo->flashBaseAddress;

  pCurrent += startAddress / 4;

  while (wordsRead < numWords)
    {
      *pBuffer = *pCurrent;
      ++pBuffer;
      ++pCurrent;
      ++wordsRead;
    }
  
  return wordsRead;
}
/*-*----------------------------------------------------------------------------*/



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

DWORD FlashWrite(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer)
{
  DWORD 	 currentWord  = 0;
  DWORD		 counter      = 0;
  DWORD          maxCycles    = 1000000;
  volatile DWORD *pCurrent    = NULL;

  // For now, just going to write words one at a time, and 
  // ignore write buffers, even though they can greatly 
  // increase performance.

  pCurrent = (DWORD *)pFlashInfo->flashBaseAddress;
  pCurrent = pCurrent + (startAddress / 4);

  while (currentWord < numWords)
    {
      // Put the device into Write mode
      *pCurrent = FlashCommandWrite;
      
      // Write the data
      *pCurrent = pBuffer[currentWord];

      // Now cycle waiting for the status register to
      // say that the write has completed.

      for (counter = 0; counter < maxCycles; ++counter)
	{
	  DWORD status = *pCurrent;

	  if ( (status & 0x80808080) == 0x80808080 )
	    {
	      // Everything completed, check for an error now
	      if ( status & 0x1A1A1A1A )
		{
		  // There was an error writing this word....
		  FlashClearStatusRegister(pFlashInfo);
		  FlashReadMode(pFlashInfo);
		  
		  return currentWord;
		}

	      break;
	    }
	}

      if ( counter == maxCycles )
	{
	  // We timed out.
	  FlashClearStatusRegister(pFlashInfo);
	  FlashReadMode(pFlashInfo);

	  return currentWord;
	}

      ++pCurrent;
      ++currentWord;
    }
  

  FlashClearStatusRegister(pFlashInfo);
  FlashReadMode(pFlashInfo);
		  
  return currentWord;
}
/*-*----------------------------------------------------------------------------*/



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

DWORD FlashBlockWrite(PFlashInfo pFlashInfo, DWORD startAddress, DWORD numWords, DWORD *pBuffer)
{
  DWORD 	 currentWord  = 0;
  DWORD		 counter      = 0;
  DWORD          maxCycles    = 1000000;
  DWORD		 entries      = 32;
  DWORD		 mask	      = 31;   // entries - 1
  DWORD		 padWords     = 0;
  BOOL           bPad         = FALSE;
  volatile DWORD *pCurrent    = NULL;
  volatile DWORD *pBlock      = NULL;

  // For now, just going to write words one at a time, and 
  // ignore write buffers, even though they can greatly 
  // increase performance.

  pCurrent = (DWORD *)pFlashInfo->flashBaseAddress;
  pCurrent = pCurrent + (startAddress / 4);
  pBlock   = pCurrent;

  // Check if the starting address is aligned with the 
  // write buffer for maximal performance.

  if ( ((DWORD)(pCurrent) >> 2) & mask )
    {
      // It is not aligned, so we'll fill the first few places with 0xFF and then
      // move on from there.
      DWORD dummy = (DWORD)pCurrent;

      bPad = TRUE;

      padWords = (dummy >> 2) & mask;
      pCurrent = pCurrent - padWords;
    }


  while (currentWord < numWords)
    {
      DWORD bufWordsAvail = 0x20;

      pBlock   = pCurrent;
      counter = 0;

      // Put the device into Write mode
      // And wait for it to respond that the buffer is ready
      do
	{
	  *pBlock = FlashCommandSCSWrite;
	  
	  if ( counter == maxCycles )
	    {
	      *pCurrent = FlashCommandClear;
	      return -1;
	    }
	  ++counter;

	} while ( (*pBlock & 0x80808080) != 0x80808080 );
	  
      counter = 0;

      // Now write out the WriteQueueCount
      *pBlock = 0x1f1f1f1f;

      if ( bPad == TRUE )
	{
	  DWORD currentPad = 0;
	  
	  for (currentPad = 0; currentPad < padWords; --currentPad)
	    {
	      *pCurrent = 0xFFFFFFFF;
	      ++pCurrent;
	      --bufWordsAvail;
	    }
	  bPad = FALSE;
	}


      while ( currentWord < numWords &&
	      bufWordsAvail > 0 )
	{
	  // Write the data
	  *pCurrent = pBuffer[currentWord];
	  ++currentWord;
	  ++pCurrent;
	  --bufWordsAvail;
	}

      // Now send the write confirm message
      *pBlock = FlashCommandConfirm;

      // Now cycle waiting for the status register to
      // say that the write has completed.

      for (counter = 0; counter < maxCycles; ++counter)
	{
	  DWORD status = *pBlock;

	  if ( (status & 0x80808080) == 0x80808080 )
	    {
	      // Everything completed, check for an error now
	      if ( status & 0x1A1A1A1A )
		{
		  // There was an error writing this word....
		  FlashClearStatusRegister(pFlashInfo);
		  FlashReadMode(pFlashInfo);
		  
		  return currentWord;
		}

	      break;
	    }
	}

      if ( counter == maxCycles )
	{
	  // We timed out.
	  FlashClearStatusRegister(pFlashInfo);
	  FlashReadMode(pFlashInfo);

	  return currentWord;
	}
    }
  

  FlashClearStatusRegister(pFlashInfo);
  FlashReadMode(pFlashInfo);
		  
  return currentWord;
}
/*-*----------------------------------------------------------------------------*/




/*+*----------------------------------------------------------------------------
/ Function: FlashEraseBlock
/
/ Description: Erases a single block in the flash.
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be erased (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was erased ok
/	   FALSE -- If an error occurred while erasing the block.
/-**----------------------------------------------------------------------------*/

BOOL FlashEraseBlock(PFlashInfo pFlashInfo, DWORD blockNumber)
{
  BOOL  bSuccess   = FALSE;
  volatile DWORD *pTmp      = (DWORD*)pFlashInfo->flashBaseAddress;
  DWORD status     = 0;
  DWORD iterations = 0;

 
  pTmp = pTmp + ((blockNumber*pFlashInfo->blockSize)/4);

  *pTmp = FlashCommandErase;
  *pTmp = FlashCommandConfirm;

  // Now have to wait for success or failure
  // For the strata flash, this has a maximum worst case time
  // of 16 seconds!

  for(;;)
    {
      if ( iterations > 10000000 )
	break;

      status = *pTmp;
      
      // Check status for the finish bits
      // Bit 7 checks for completion, 
      if ( (status & 0x80808080) == 0x80808080 )
	{
	  // Check if any of the error bits are set
	  if (status & 0x20202020)
	      FlashClearStatusRegister(pFlashInfo);
	  else	
	      bSuccess = TRUE;

	  FlashReadMode(pFlashInfo);
	  break;
	}

      ++iterations;
    }

  return bSuccess;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: FlashLockBlock
/
/ Description: Turns on the lock bit for a block in the flash
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be locked 
/			      (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was locked ok
/	   FALSE -- If an error occurred while locking the block.
/-**----------------------------------------------------------------------------*/

DWORD FlashLockBlock(PFlashInfo pFlashInfo, DWORD blockNumber)
{
  BOOL  bSuccess   = FALSE;
  volatile DWORD *pTmp      = (DWORD*)pFlashInfo->flashBaseAddress;
  DWORD status     = 0;
  DWORD iterations = 0;
 
  pTmp = pTmp + ((blockNumber*pFlashInfo->blockSize)/4);

  *pTmp = FlashCommandLockSetup;
  *pTmp = FlashCommandLockConfirm;

  // Now have to wait for success or failure
  // For the strata flash, this has a maximum worst case time
  // of 16 seconds!

  for(;;)
    {
      if ( iterations > 10000000 )
	break;

      status = *pTmp;
      
      // Check status for the finish bits
      // Bit 7 checks for completion, 
      if ( (status & 0x80808080) == 0x80808080 )
	{
	  // Check if any of the error bits are set
	  if (status & 0x20202020)
	      FlashClearStatusRegister(pFlashInfo);
	  else	
	      bSuccess = TRUE;

	  FlashReadMode(pFlashInfo);
	  break;
	}

      ++iterations;
    }

  return bSuccess;
}
/*-*----------------------------------------------------------------------------*/

 

/*+*----------------------------------------------------------------------------
/ Function: FlashLockStatusBlock
/
/ Description: Returns the lock status of a block.
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/	      blockNumber  -- The block to be locked 
/			      (from 0 to pFlashInfo->numBlocks-1)
/
/ Returns: TRUE  -- If the block was locked
/	   FALSE -- If the block was not locked
/-**----------------------------------------------------------------------------*/

DWORD FlashLockStatusBlock(PFlashInfo pFlashInfo, DWORD blockNumber)
{
  BOOL  	       	        bLock = FALSE;
  volatile DWORD 		*pointer = (DWORD*)pFlashInfo->flashBaseAddress;
  volatile CFIx8x16QUAD_STRUCT 	*cfiInfo = 0;
  int				status;

  pointer = pointer + ((blockNumber*pFlashInfo->blockSize)/4);
  *pointer = (DWORD)FlashCommandQueryCFI;

  cfiInfo = (CFIx8x16QUAD_STRUCT *)pointer;

  status = cfiInfo->BlockStatusRegister[0];
  FlashReadMode(pFlashInfo);

  return status;

}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: FlashUnlock
/
/ Description: Turns off ALL of the protection bits
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/
/ Returns: TRUE  -- If the block was unprotected ok
/	   FALSE -- If an error occurred while unprotecting the block.
/-**----------------------------------------------------------------------------*/

DWORD FlashUnlock(PFlashInfo pFlashInfo)
{
  BOOL bSuccess = FALSE;
  volatile DWORD *pTmp      = (DWORD*)pFlashInfo->flashBaseAddress;
  DWORD iterations = 0;
  DWORD status     = 0;

  *pTmp = FlashCommandUnlockSetup;
  *pTmp = FlashCommandUnlockConfirm;

  // Now have to wait for success or failure
  // For the strata flash, this has a maximum worst case time
  // of 16 seconds!

  for(;;)
    {
      if ( iterations > 10000000 )
	break;

      status = *pTmp;
      
      // Check status for the finish bits
      // Bit 7 checks for completion, 
      if ( (status & 0x80808080) == 0x80808080 )
	{
	  // Check if any of the error bits are set
	  if (status & 0x20202020)
	      FlashClearStatusRegister(pFlashInfo);
	  else	
	      bSuccess = TRUE;

	  FlashReadMode(pFlashInfo);
	  break;
	}

      ++iterations;
    }

  return bSuccess;
}

/*-*----------------------------------------------------------------------------*/


/*+*----------------------------------------------------------------------------
/ Function: FlashClearStatusRegister
/
/ Description: Clear the status register
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/
/ Returns: nothing
/-**----------------------------------------------------------------------------*/

void FlashClearStatusRegister(PFlashInfo pFlashInfo)
{
  volatile DWORD *pCurrent = (DWORD *)pFlashInfo->flashBaseAddress;


  *pCurrent = FlashCommandClear;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: FlashReadMode
/
/ Description: Put the flash back into read mode
/
/ Parameters: pFlashInfo   -- From FlashInitialize
/
/ Returns: nothing
/-**----------------------------------------------------------------------------*/

void FlashReadMode(PFlashInfo pFlashInfo)
{
  volatile DWORD *pCurrent = (DWORD *)pFlashInfo->flashBaseAddress;

  *pCurrent = FlashCommandRead;
}
/*-*----------------------------------------------------------------------------*/



