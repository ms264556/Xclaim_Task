/*********************************************************************
 **
 * Module: USC_DMA
 **
 * File Name: v3uscdma.h
 **
 * Authors: Vadim Moshinsky
 **
 * Copyright (c) 1997-1999 V3 Semiconductor. All rights reserved.
 *
 * V3 Semiconductor makes no warranties for the use of its products.  V3 does
 * not assume any liability for errors which may appear in these files or
 * documents, however, we will attempt to notify customers of such errors.
 *
 * V3 Semiconductor retains the right to make changes to components,
 * documentation or specifications without notice.
 *
 * Please verify with V3 Semiconductor to be sure you have the latest
 * specifications before finalizing a design.
 **
 * $Revision: 12 $	$Date: 5/10/99 12:17p $
 * $NoKeywords: $
 **
 * Description:
 * This file consisting in prototypes and definitions for
 * DMA services using USC based board
 **
 ********************************************************************/

#ifndef _V3USCDMA_H_
#define _V3USCDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * If the global variables are to be instantiated in a specific module
 * then GLOBAL_VARIABLES is define just prior to including this file.
 * The GLOBAL_VARIABLES variable is promptly undefined to prevent other
 * modules from instantiating their global variables.
 * The _GLOBAL_VARIABLES_USC_DIAG_H_ define can then be used to istantiate
 * and global variables.
 * LOCAL_EXTERN is a macro for extern or nothing depending on how this include
 * file is called the variable will be accessible.
 */

#undef LOCAL_EXTERN
#ifdef GLOBAL_VARIABLES
	#undef GLOBAL_VARIABLES
	#define _GLOBAL_VARIABLES_USC_DMA_H_
	#define LOCAL_EXTERN
#else /* GLOBAL VARIABLES */
	#define LOCAL_EXTERN extern
#endif /* GLOBAL_VARIABLES */

/*********************************************************************
 **
 * Include files that this include file depends on.
 **
 ********************************************************************/


/*********************************************************************
 **
 * Equates
 **
 ********************************************************************/

typedef enum DmaStatus_tag {
	SUCCESS_DMA_OP			= 0,
	FAIL_ALLOC_DSCR			= 1,
	FAIL_FIND_FREE_DSCR		= 2,
	FAIL_INCORR_VER			= 3,
	DMA_STATUS_TIMEOUT		= 4,
	FAIL_NOT_SUPRT_OP		= 5

} DMA_RET_STATUS;


typedef enum DmaBdBusType_tag {
	DMA_BD_LOCAL		= 0,
	DMA_BD_PCI			= 1

} BD_BUS_TYPE;


typedef enum
{
    V3_DMA_0 = 0,
    V3_DMA_1 = 1
} V3_DMA_CHANNEL;


/*********************************************************************
 **
 * Data Structures
 **
 ********************************************************************/
typedef struct _dma_dscr			/* DMA descriptor structure				*/
{
	DMA_XFER_CTL DmaXferCtl;		/* Next DMA_XFER_CTLx value				*/
	DMA_SRC_ADR  DmaSrcAddr;		/* Next DMA_SRC_ADRx value				*/
	DMA_DST_ADR	 DmaDstAddr;		/* Next DMA_DSTRx value					*/
	DMA_CTLB_ADR DmaCtlbAddr;		/* Next DMA_CTLBx value					*/

} USC_DMA_DSCR, *PUSC_DMA_DSCR;


typedef struct _dma_ctx				/* DMA context structure				*/
{
	BYTE			bChannelId;		/* DMA channel in USC chip				*/
	BOOL			fChainEnable;	/* Descriptor chain is busy				*/
	BOOL			fRingEnable;	/* Descriptor chain is busy				*/
	BOOL			fBlocking;		/* Poll DMA transfer complete			*/
	DWORD			dwBdNum;		/* Number of BD's in the list			*/
	V3USC_BADDR		DmaAperture;	/* DMA aperture using with this context	*/
	PVOID			pVirtualAddr;	/* Virtual address of allocated buffer  */
	BD_BUS_TYPE		BDsBusType;		/* 1 - PCI to PCI, 0 - LOCAL			*/
	PUSC_DMA_DSCR	DmaBdFist;		/* First BD in list related to this DMA */

} USC_DMA_CTX, *PUSC_DMA_CTX;


/************************************************************************************
 **
 * Global Variables with scope of only the module that defined GLOBAL_VARIABLES
 **
 ************************************************************************************/

#ifdef _GLOBAL_VARIABLES_V3USCDMA_H_

#endif /* _GLOBAL_VARIABLES_V3USCDMA_H_ */

/************************************************************************************
 **
 * Global Variables to be exported from this module 
 **
 ************************************************************************************/

/************************************************************************************
 **
 * Function Prototypes
 **
 ************************************************************************************/
DMA_RET_STATUS
V3USC_DmaBdListInit(			
	V3USCHANDLE		hV3,			/* Handler of USC card parameters structure		*/
	PUSC_DMA_CTX	pDmaCtx,		/* Pointer to DMA context						*/
	DMA_CTLB_ADR	ctlb			/* descriptors control block					*/
);

DMA_RET_STATUS 
V3USC_DmaSetDscr(
	V3USCHANDLE		hV3,			/* Handler of USC card parameters structure		*/
	PUSC_DMA_CTX	pDmaCtx,		/* Pointer to DMA context						*/
	DWORD			dwSrcAddr,		/* Source address								*/
	DWORD			dwDstAddr,		/* Destination address							*/
	DWORD			dwBufLen,		/* number of bytes to transfer					*/
	DWORD			dwParams		/* XFER parameters								*/
);

DMA_RET_STATUS
V3USC_DmaOperate(			
	V3USCHANDLE		hV3,			/* Handler of USC card parameters				*/
	PUSC_DMA_CTX	pDmaCtx,		/* Pointer to DMA context						*/
	DWORD			dwParams		/* CSR parameters								*/
);

DMA_RET_STATUS
V3USC_DmaScanDscr(
	V3USCHANDLE hV3,				/* Handler of USC card parameters				*/
	PUSC_DMA_CTX pDmaCtx			/* Pointer to DMA context						*/
);

void 
V3USC_DmaHalt(
	V3USCHANDLE		hV3,			/* Handler of USC card parameters				*/
	V3_DMA_CHANNEL	dmaChannel		/* DMA channel to apply							*/
);

BOOL 
V3USC_DmaIsDone(
	V3USCHANDLE		hV3,			/* Handler of USC card parameters				*/
	V3_DMA_CHANNEL	dmaChannel		/* DMA channel to apply							*/
);

DMA_RET_STATUS
V3USC_DmaReset(			
	V3USCHANDLE		hV3				/* Handler of USC card parameters				*/
);


#ifdef __cplusplus
}
#endif

#endif /* _V3USCDMA_H_ */

