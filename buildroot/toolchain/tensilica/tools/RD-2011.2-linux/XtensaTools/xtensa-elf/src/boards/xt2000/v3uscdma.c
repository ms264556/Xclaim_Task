/*********************************************************************
 **
 * Module: V3USC32
 **
 * File Name: v3uscdma.c
 **
 * Authors: Vadim Moshinsky
 **
 * Copyright (c) 1997-1999 V3 Semiconductor. All rights reserved.
 **
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
 * $Revision: 16 $	$Date: 5/20/99 9:37a $
 * $NoKeywords: $
 **
 * Description:
 * This program is meant to demonstrate how to use V3 Semiconductor
 * PCI Bridge Devices. Activating DMA services.
 **
 ********************************************************************/
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <time.h>

#include "v3usc32.h"

#include "v3uscdma.h"


/*!
 *********************************************************************
 **
 * Function: 
 * void V3USC_DmaBdListInit(
 *		V3USCHANDLE hV3, PUSC_DMA_CTX pDmaCtx)
 **
 * Parameters:
 * V3USCHANDLE	hV3			  - Handler of USC card parameters structure
 * PUSC_DMA_CTX	pDmaCtx		  - Pointer to DMA context
  **
 * Return:	
 **
 * Description:
 *	Dma management initialization.Initializes DMA management according 
 *	to input parameters DMA context	and list of descriptors
 **
 **********************************************************************/
DMA_RET_STATUS
V3USC_DmaBdListInit(V3USCHANDLE	hV3, PUSC_DMA_CTX pDmaCtx, DMA_CTLB_ADR	ctlb)
{

	DWORD i;
	DWORD dwAddr;
	PUSC_DMA_DSCR pCurBD;

	/* DMA chaining not allowed */
	if(!pDmaCtx->fChainEnable)
		return FAIL_NOT_SUPRT_OP;

	if(hV3->RevID < V3USC_REV_B0)
		return FAIL_INCORR_VER;

	/* Get actual pointer to memory where placed BD's */
	if(pDmaCtx->BDsBusType == DMA_BD_LOCAL)
	{
		V3USC32_GetPointerToSpace(
			hV3, pDmaCtx->DmaAperture, 
			(DWORD)pDmaCtx->DmaBdFist, (DWORD*)&dwAddr);
		/* Take apparture addr of local BD's */
		pCurBD = (PUSC_DMA_DSCR)dwAddr;
	} else {
		pCurBD = pDmaCtx->pVirtualAddr;
	}

	memset((void*)pCurBD, 0, pDmaCtx->dwBdNum*(sizeof(USC_DMA_DSCR)));

	/* Set CTLB field of the list to link it */
	for(i=0; i<pDmaCtx->dwBdNum; i++) {
		
		((pCurBD)[i]).DmaCtlbAddr.bits.ctlb_bus		= ctlb.bits.ctlb_bus;
		((pCurBD)[i]).DmaCtlbAddr.bits.sa_inc_dis	= ctlb.bits.sa_inc_dis;
		((pCurBD)[i]).DmaCtlbAddr.bits.da_inc_dis	= ctlb.bits.da_inc_dis;

		((pCurBD)[i]).DmaCtlbAddr.bits.ctlb_addr	= 
			(DWORD)&(((pDmaCtx->DmaBdFist)[i+1]).DmaXferCtl.data) >> 4;
	}

	/* Close a ring or set 'end of chain' sign */
	if(pDmaCtx->fRingEnable)
		((pCurBD)[i-1]).DmaCtlbAddr.bits.ctlb_addr = 
			(DWORD)&(((pDmaCtx->DmaBdFist)[0]).DmaXferCtl.data) >> 4;
	else
		((pCurBD)[i-1]).DmaCtlbAddr.data = (DWORD)NULL;

	return SUCCESS_DMA_OP;	
}


/*!
 *********************************************************************
 **
 * Function: 
 * DMA_RET_STATUS V3USC_DmaSetDscr(
 *		V3USCHANDLE hV3, PUSC_DMA_CTX pDmaCtx, DWORD* dwPhisAddr,
 *		DWORD* dwLocalAddr, DWORD dwBufLen, BYTE bControl, BOOL	fLast)
 **
 * Parameters:
 * V3USCHANDLE	hV3			  - Handler of USC card parameters structure
 *	PUSC_DMA_CTX pDmaCtx	 - Pointer to DMA context
 *	DWORD		 dwSrcAddr	 - Source Address
 *	DWORD		 dwDstAddr   - Destination Address
 *	DWORD		 dwBufLen	 - Size in bytes of given buffer
 *	DWORD		 dwParams	 - Contans of DMA_XFER register
 **
 * Return:	DMA_RET_STATUS type
 **
 * Description:
 * Set actual transfer data for current DMA descriptor
 **
 **********************************************************************/

DMA_RET_STATUS 
V3USC_DmaSetDscr(
	V3USCHANDLE		hV3,
	PUSC_DMA_CTX	pDmaCtx, 
	DWORD			dwSrcAddr,
	DWORD			dwDstAddr, 
	DWORD			dwBufLen,
	DWORD			dwParams)
{
	PUSC_DMA_DSCR	pCurDescr;
	DWORD			i;
	DWORD			dwAddr;

	/* Get actual pointer to memory where placed BD's */ 
	if(pDmaCtx->BDsBusType == DMA_BD_LOCAL)
	{
		V3USC32_GetPointerToSpace(
			hV3, pDmaCtx->DmaAperture, 
			(DWORD)pDmaCtx->DmaBdFist, (DWORD*)&dwAddr);
		/* Take apparture addr of local BD's */
		pCurDescr = (PUSC_DMA_DSCR)dwAddr;
	} else {
		pCurDescr = pDmaCtx->pVirtualAddr;
	}

	for(i=0; i<pDmaCtx->dwBdNum; i++, pCurDescr++)
		/* if the descriptor is not in use take it */
		if(pCurDescr->DmaXferCtl.bits.cnt == 0) 
				break;

	/* If there is no any vacant descriptors return error */
	if(i == pDmaCtx->dwBdNum)
		return FAIL_FIND_FREE_DSCR;

	/* Set BD parameters */
	pCurDescr->DmaSrcAddr.data		= dwSrcAddr;
	pCurDescr->DmaDstAddr.data		= dwDstAddr;
	pCurDescr->DmaXferCtl.data		= dwParams;
	pCurDescr->DmaXferCtl.bits.cnt  = dwBufLen;

	return SUCCESS_DMA_OP;	
}

/*!
 *********************************************************************
 **
 * Function: 
 * DMA_RET_STATUS V3USC_DmaScanDscr(
 *		V3USCHANDLE hV3, PUSC_DMA_CTX pDmaCtx)
 **
 * Parameters:
 * V3USCHANDLE	hV3			  - Handler of USC card parameters structure
 *	PUSC_DMA_CTX pDmaCtx	 - Pointer to DMA context
 **
 * Return:	DMA_RET_STATUS type
 **
 * Description:
 * Scan descriptor ring and return success if at least one descriptor
 * is vacant for new packet of data.
 **
 **********************************************************************/

DMA_RET_STATUS 
V3USC_DmaScanDscr(V3USCHANDLE hV3, PUSC_DMA_CTX pDmaCtx)
{
	PUSC_DMA_DSCR	pCurDescr;
	DWORD			i;
	DWORD			dwAddr;

	/* Get actual pointer to memory where placed BD's */ 
	if(pDmaCtx->BDsBusType == DMA_BD_LOCAL)
	{
		V3USC32_GetPointerToSpace(
			hV3, pDmaCtx->DmaAperture, 
			(DWORD)pDmaCtx->DmaBdFist, (DWORD*)&dwAddr);
		/* Take apparture addr of local BD's */
		pCurDescr = (PUSC_DMA_DSCR)dwAddr;
	} else {
		pCurDescr = pDmaCtx->pVirtualAddr;
	}

	for(i=0; i<pDmaCtx->dwBdNum; i++, pCurDescr++)
		/* if the descriptor is not in use take it */
		if(pCurDescr->DmaXferCtl.bits.cnt == 0) 
				break;

	/* If there is no any vacant descriptors return error */
	if(i == pDmaCtx->dwBdNum)
		return FAIL_FIND_FREE_DSCR;

	return SUCCESS_DMA_OP;	
}



/*!
 *********************************************************************
 **
 * Function: 
 * DMA_RET_STATUS V3USC_DmaOperate(PUSC_DMA_CTX	pDmaCtx)
 **
 * Parameters:
 *  V3USCHANDLE	hV3		 - Handler of USC card parameters structure
 *	PUSC_DMA_CTX pDmaCtx - Pointer to DMA context
 **
 * Return:	DMA_RET_STATUS
 **
 * Description:
 * Set actual transfer data for current descriptor and start DMA
 **
 **********************************************************************/
DMA_RET_STATUS 
V3USC_DmaOperate(V3USCHANDLE hV3, PUSC_DMA_CTX pDmaCtx, DWORD dwParams)
{

	DWORD			dwUscReg;
	PUSC_DMA_DSCR	pCurDescr;
	DWORD			dwAddr;

	/* Stop DMA if working */
	if(V3USC_DmaIsDone(hV3, (V3_DMA_CHANNEL) pDmaCtx->bChannelId))
		V3USC_DmaHalt(hV3, (V3_DMA_CHANNEL) pDmaCtx->bChannelId);

	/* Get actual pointer to memory where placed BD's */ 
	if(pDmaCtx->BDsBusType == DMA_BD_LOCAL)
	{
		V3USC32_GetPointerToSpace(
			hV3, pDmaCtx->DmaAperture, 
			(DWORD)pDmaCtx->DmaBdFist, (DWORD*)&dwAddr);
		/* Take apparture addr of local BD's */
		pCurDescr = (PUSC_DMA_DSCR)dwAddr;
	} else {
		pCurDescr = pDmaCtx->pVirtualAddr;
	}


	/* write next BD address */
	dwUscReg = V3USC_DMA_CTLB_ADR0 + (pDmaCtx->bChannelId << 4);
	V3USC_WriteRegDWord(hV3, dwUscReg, pCurDescr->DmaCtlbAddr.data);

	/* write source address pointer */
	dwUscReg = V3USC_DMA_SRC_ADR0 + (pDmaCtx->bChannelId << 4);
	V3USC_WriteRegDWord(hV3, dwUscReg, pCurDescr->DmaSrcAddr.data);

	/* write destination address pointer */
	dwUscReg = V3USC_DMA_DST_ADR0 + (pDmaCtx->bChannelId << 4);
	V3USC_WriteRegDWord(hV3, dwUscReg, pCurDescr->DmaDstAddr.data);

	/* write control of current BD */
	dwUscReg = V3USC_DMA_XFER_CTL0 + (pDmaCtx->bChannelId << 4);
	V3USC_WriteRegDWord(hV3, dwUscReg, pCurDescr->DmaXferCtl.data);

	/* write DMA parameters register */
	dwUscReg = V3USC_DMA_CSR0 + (pDmaCtx->bChannelId << 2);
	V3USC_WriteRegDWord(hV3, dwUscReg, dwParams);

    /* if blocking then check BIT24 to wait for transfer to complete */
	if (pDmaCtx->fBlocking) 
	    while (!V3USC_DmaIsDone(hV3, pDmaCtx->bChannelId));

	return SUCCESS_DMA_OP;
}



/*!
 *********************************************************************
 **
 * Function: 
 * void V3USC_DmaHalt(V3USCHANDLE hV3, V3_DMA_CHANNEL dmaChannel)
 **
 * Parameters:
 *  V3USCHANDLE	hV3				- Handler of USC card parameters structure
 *	V3_DMA_CHANNEL dmaChannel	- DMA channel to apply
 **
 * Return:	none
 **
 * Description:
 * Pauses DMA transactions untill them started again. See user manual
 **
 **********************************************************************/
void 
V3USC_DmaHalt(V3USCHANDLE hV3, V3_DMA_CHANNEL dmaChannel)
{
	DWORD dwUscRegVal;

	/* Get contains of Status / Control register */
	dwUscRegVal = V3USC_ReadRegDWord(hV3, V3USC_DMA_CSR0 + (dmaChannel << 2));
	/* Set the HALT bit */
	dwUscRegVal |= DMA_CSR_HALT;
	/* Write the register back */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_CSR0 + (dmaChannel << 2), dwUscRegVal);
}



/*!
 *********************************************************************
 **
 * Function: 
 * BOOL V3USC_DmaIsDone(V3PBCHANDLE hV3, V3_DMA_CHANNEL dmaChannel)
 **
 * Parameters:
 *  V3USCHANDLE	hV3				- Handler of USC card parameters structure
 *	V3_DMA_CHANNEL dmaChannel	- DMA channel to apply
 **
 * Return:	TRUE if DONE bit is set
 **
 * Description:
 * Checks whether DMA transfers active or not.See user manual
 **
 **********************************************************************/
BOOL 
V3USC_DmaIsDone(V3USCHANDLE hV3, V3_DMA_CHANNEL dmaChannel)
{
	if (V3USC_ReadRegDWord(hV3, 
		V3USC_DMA_CSR0 + (dmaChannel << 2)) & DMA_CSR_DONE)
			return TRUE;

   return FALSE;
}


/*!
 *********************************************************************
 **
 * Function: 
 * void V3USC_DmaReset(V3USCHANDLE hV3)
 **
 * Parameters:
 *  V3USCHANDLE	hV3		 - Handler of USC card parameters structure
 **
 * Return:	DMA_RET_STATUS
 **
 * Description:
 * Reset DMA engines by means of running it with zero parameters
 * ERRATA for first revission of B0
 **
 **********************************************************************/
DMA_RET_STATUS 
V3USC_DmaReset(V3USCHANDLE hV3)
{
	/* init next BD address */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_CTLB_ADR0, 0);
	V3USC_WriteRegDWord(hV3, V3USC_DMA_CTLB_ADR1, 0);

	/* init source address pointer */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_SRC_ADR0, 0);
	V3USC_WriteRegDWord(hV3, V3USC_DMA_SRC_ADR1, 0);

	/* init destination address pointer */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_DST_ADR0, 0);
	V3USC_WriteRegDWord(hV3, V3USC_DMA_DST_ADR1, 0);

	/* init control of current BD */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_XFER_CTL0, 0);
	V3USC_WriteRegDWord(hV3, V3USC_DMA_XFER_CTL0, 0);

	/* write DMA parameters register */
	V3USC_WriteRegDWord(hV3, V3USC_DMA_CSR0, DMA_CSR_IPR);
    while (!V3USC_DmaIsDone(hV3, V3_DMA_0))
		/* ANALYZE TIMEOUT */
		;

	V3USC_WriteRegDWord(hV3, V3USC_DMA_CSR1, DMA_CSR_IPR);
    while (!V3USC_DmaIsDone(hV3, V3_DMA_1))
		/* ANALYZE TIMEOUT */
		;

	return SUCCESS_DMA_OP;
}
