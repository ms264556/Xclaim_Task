/*********************************************************************
 **
 * Module: V3USCDRV
 **
 * File Name: v3uscreg.h
 **
 * Authors: Phil Sikora, Jan Skibinski, Vadim Monshinsky, Dan Aizenstros
 **
 * Copyright (c) 1997-1999 V3 Semiconductor. All rights reserved.
 *
 * V3 Semiconductor makes no warranties for the use of its products.	V3 does
 * not assume any liability for errors which may appear in these files or
 * documents, however, we will attempt to notify customers of such errors.
 *
 * V3 Semiconductor retains the right to make changes to components,
 * documentation or specifications without notice.
 *
 * Please verify with V3 Semiconductor to be sure you have the latest
 * specifications before finalizing a design.
 **
 * $Revision: 76 $	$Date: 9/16/99 12:48p $
 * $NoKeywords: $
 **
 * Description:
 **
 * This is the header file for the registers in V3 USC family of devices.
 * There are two basic approaches to accessing registers, one approach
 * has a known base address of the registers to which an offset is added,
 * for each access.
 * The second approach is to map a data structure matching the registers
 * to the physical device.  Writing and reading values from this data
 * structure has the effect of accessing the physical device's registers.
 * For convenience both methods are supported from this include file.
 *
 * Equates in this file are for the most recent stepping of the PCI
 * bridge devices, check the data sheets for older steppings.
 **
 * This include file CAN support three basic modes each of which may
 * have a little and big endian offset and/or data structures.
 * Check the _V3USCREG_H_MODE_XX_ defines determine exactly what is
 * currently supported.
 **
 * Added support for MIPS assembler, which can not handle enum statements
 * or structure definitions. 
 * Support for PMON defines that use pointers, se below table for details
 * of each mode supported by this include file.
 * A separate define enables the little endian data structures when
 * needed.
 **
 ********************************************************************/

#ifndef _V3USCREG_H_
#define _V3USCREG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define V3USCREG_VER 200
#define V3USCREG_VER_STR "V3 Semiconductor (c) 1997-1999 V3USCREG Version 2.00"

/*********************************************************************
 *
 * This section is the base address and offset definition approach
 *
 ********************************************************************/

/*
 * PCI Configuration registers bit definitions
 *
 * For those bit fields larger than one bit a _MASK definition is provide
 * to isolate the value. As well a _SHIFT definition is provide to
 * convert the value to and from its register position and a number
 *
 * The table describes how Register names and their width will be encoded.
 * The DMA_LENGTHX register is a special case since it is 24 bits wide and
 * must be accessed as a 32 bit or 16 and 8 bit quantity or 16 bits with CSR.
 *
 * |--------------------------------------|
 * | Register Name | Suffix       | Width |
 * |--------------------------------------|
 * | LB_IMASK      | LB_IMASK_B   | 8     |
 * |--------------------------------------|
 * | PCI_VENDOR    | PCI_VENDOR_W | 16    |
 * |--------------------------------------|
 * | PCI_CC_REV    | PCI_CC_REV   | 32    |
 * |--------------------------------------|
 * | DMA_LENGHTX   | DMA_LENGHTX  | 24    |
 * |--------------------------------------|
 */

/*
 * Selection of which #defines to select can be determined from
 * the following table:
 **
 * #define _V3USCREG_H_AL_
 * The Windows Visual "C" enviroment
 * - little endian (only) register offsets
 * - USC base address plus register offset mode
 * - USC data structure mapped on to the physical address of the USC
 **
 * #define _V3USCREG_H_AB_
 * - big endian (only) register offsets
 * - USC base address plus register offset mode
 * - USC data structure mapped on to the physical address of the USC
 **
 * #define _V3USCREG_H_BL_
 * The MIPs assembler enviroment
 * - Same as mode A but never to have data structure access
 * - little endian register offsets
 * - USC base address plus register offset mode
 **
 * #define _V3USCREG_H_BB_
 * - Same as mode A but never to have data structure access
 * - big endian register offsets
 * - USC base address plus register offset mode
 ** 
 * #define _V3USCREG_H_CL_
 * The MIPs "C" complier enviroment PMON
 * - little endian register offsets
 * - USC base address plus register offset mode
 *   Note: The register equate is both base and offset, creating a
 *   pointer to the register in memory. (No function call)
 * - USC data structure mapped on to the physical address of the USC
 * Must call the v3uscreg_init_base() function to initialize the base
 * address of USC variable defined in this include file. 
 ** 
 * #define _V3USCREG_H_CB_
 * - big endian register offsets
 * - USC base address plus register offset mode
 *   Note: The register equate is both base and offset, creating a
 *   pointer to the register in memory. (No function call)
 * - USC data structure mapped on to the physical address of the USC
 * Must call the v3uscreg_init_base() function to initialize the base
 * address of USC variable defined in this include file. 
 */  

/* Mode A - Little Endian */
#ifdef _V3USCREG_H_AL_

#define V3REGW(x)		(x)
#define V3REGH(x)		(x)
#define V3REGB(x)		(x)
#define _V3USCREG_H_DATA_STRUCTURE_L_
#endif

/* Mode A - Big Endian */
#ifdef _V3USCREG_H_AB_

#define V3REGW(x)		(x)
#define V3REGH(x)		((x)^2)
#define V3REGB(x)		((x)^3)
#define _V3USCREG_H_DATA_STRUCTURE_B_
#endif

/* Mode B - Little Endian */
#ifdef _V3USCREG_H_BL_

#define V3REGW(x)		(x)
#define V3REGH(x)		(x)
#define V3REGB(x)		(x)
#endif

/* Mode B - Big Endian */
#ifdef _V3USCREG_H_BB_

#define V3REGW(x)		(x)
#define V3REGH(x)		((x)^2)
#define V3REGB(x)		((x)^3)
#endif


/* Mode C - Little Endian */
#ifdef _V3USCREG_H_CL_

/* To use this mode the v3uscreg_init_base() function must be */
/* called with the non-cached address of the V320USC internal registers */

extern char *_v3uscp;

#define V3REGW(x)		*(volatile unsigned long *)(_v3uscp + (x))
#define V3REGH(x)		*(volatile unsigned short *)(_v3uscp + (x))
#define V3REGB(x)		*(volatile unsigned char *)(_v3uscp + (x)) 	
#define _V3USCREG_H_DATA_STRUCTURE_L_
#endif

/* Mode C - Big Endian */
#ifdef _V3USCREG_H_CB_

/* To use this mode the v3uscreg_init_base() function must be */
/* called with the non-cached address of the V320USC internal registers */

extern char *_v3uscp;

#define V3REGW(x)		*(volatile unsigned long *)(_v3uscp + (x))
#define V3REGH(x)		*(volatile unsigned short *)(_v3uscp + ((x)^2))
#define V3REGB(x)		*(volatile unsigned char *)(_v3uscp + ((x)^3))
#define _V3USCREG_H_DATA_STRUCTURE_B_
#endif
 
/*
 * PCI Vendor ID
 * - Offset 00h, Size 16 bits
 */

/*
 * PCI Device ID
 * - Offset 02h, Size 16 bits
 */

/*
 * PCI Command Register
 * - Offset 04h, Size 16 bits
 */
#define PCI_CMD_W_IO_EN				0x0001		/* I/O access */
#define PCI_CMD_W_MEM_EN			0x0002		/* Memory access */
#define PCI_CMD_W_MASTER_EN			0x0004		/* PCI Master */
#define PCI_CMD_W_MWI_EN			0x0010		/* Memory Write and */
									/* Invalidate enable            */
#define PCI_CMD_W_PAR_EN			0x0040		/* Parity error */
#define PCI_CMD_W_SERR_EN			0x0100		/* System error */
									/* If PAR_EN is enabled then SERR is  */
									/* driven in response to parity error */
#define PCI_CMD_W_FBB_EN			0x0200		/* Fast back to back */
									/* transfers when Bus Master     */

/*
 * PCI Status Register
 * - Offset 06h, Size 16 bits
 */
#define PCI_STAT_W_NEW_CAP			0x0010		/* New Capabilites          */
#define PCI_STAT_W_UDF				0x0040		/* User Defined Feature     */
#define PCI_STAT_W_FAST_BACK		0x0008		/* Fast Back to Back Target */
									/* - Used to indicate ability of this   */
									/* device to other Bus Masters          */
#define PCI_STAT_W_PAR_REP			0x0010		/* Data Parity Report when    */
									/* USC is a Bus Master and PERR is driven */
#define PCI_STAT_W_DEVSEL_MASK		0x0600		/* 10-9 Bits Device Select */
									/* Timing                              */
#define PCI_STAT_W_DEVSEL_SHIFT	9

#define PCI_STAT_W_T_ABORT			0x1000		/* Target Abort - set in */
									/* response to a target abort detected */
									/* while USC was a Bus Master          */
#define PCI_STAT_W_M_ABORT			0x2000		/* Master Abort - set in   */
									/* response to a master abort detected */
									/* while USC was a Bus Master          */
#define PCI_STAT_W_SYS_ERR			0x4000		/* System Error - set in */
									/* response to a system error on the */
									/* SERR pin                          */
#define PCI_STAT_W_PAR_ERR			0x8000		/* Parity Error - set in */
									/* response to a parity error on the */
									/* PCI bus                           */

/*
 * PCI Class and Revision Register
 * - Offset 08h, Size 32 bits
 */
#define PCI_CC_REV_VREV_MASK		0x0000000f	/* 3-0 Bits Stepping ID  */
									/* Rev A = 0,Rev B0 = 1, Rev B1 = 2, */
									/* Rev B2 = 3 */
#define PCI_CC_REV_VREV_SHIFT		0

#define PCI_CC_REV_UREV_MASK		0x000000f0	/* 7-4 Bits User Revision ID */
									/* user definable for system revisions   */
#define PCI_CC_REV_UREV_SHIFT		4

#define PCI_CC_REV_PROG_IF_MASK		0x0000ff00	/* 15-8 Bits PCI Programming */
									/* Interface code                        */
#define PCI_CC_REV_PROG_IF_SHIFT	8

#define PCI_CC_REV_SUB_CLASS_MASK	0x00ff0000	/* 23-16 Bits PCI Sub Class */
#define PCI_CC_REV_SUB_CLASS_SHIFT	16

#define PCI_CC_REV_BASE_CLASS_MASK	0xff000000	/* 32-24 Bits PCI Base Class */
#define PCI_CC_REV_BASE_CLASS_SHIFT 24

/*
 * PCI Header and Configuration Register
 * - Offset 0ch, Size 32 bits
 */
#define PCI_HDR_CFG_LINE_SIZE_MASK	0x000000ff	/* Cache Line Size */
#define PCI_HDR_CFG_LINE_SIZE_SHIFT	0

#define PCI_HDR_CFG_LTL_MASK		0x00000700	/* 10-8 Bits Latency Timer */
									/* lower bits unimplemented            */
#define PCI_HDR_CFG_LTL_SHIFT		8

#define PCI_HDR_CFG_LT_MASK			0x0000f800	/* 15-11 Bits Latency Timer */
									/*  multiple 8 clocks                   */
#define PCI_HDR_CFG_LT_SHIFT		11

#define PCI_HDR_CFG_HDR_TYPE_MASK	0x00ff0000	/* 23-16 Bits Header Type */
									/* unimplemented                      */
#define PCI_HDR_CFG_HDR_TYPE_SHIFT	16

#define PCI_HDR_CFG_BIST_MASK		0xff000000	/* 31-24 Bits Built In */
									/* Self-Test                       */
#define PCI_HDR_CFG_BISTH_SHIFT		24

/*
 * PCI Access to local memory map access
 * - Offset 10h, Size 32 bits (I2O mode)
 */
#define PCI_I2O_BASE_IO				0x00000001	/* I/O 1 - I/O space */
									/* 0 - Memory Space              */
#define PCI_I2O_BASE_TYPE_MASK		0x00000006	/* 2-1 Bits Address range */
									/* type                               */
#define PCI_I2O_BASE_TYPE_SHIFT		1			/* 0 - device can be mapped */
									/* any where in a 32 bit address space  */
#define PCI_I2O_BASE_PREFETCH		0x00000008	/* Prefetchable - no effect */
#define PCI_I2O_BASE_ADR_BASE_MASK	0xfff00000	/* 31-20 Bits Base address */
									/* of ATU                              */
#define PCI_I2O_BASE_ADR_BASE_SHIFT	20

/*
 * PCI Access to local memory map access
 * - Offset 14h, Size 32 bits
 */
#define PCI_MEM_BASE_IO				0x00000001	/* I/O 1 - I/O space */
									/* 0 - Memory Space              */
#define PCI_MEM_BASE_TYPE_MASK		0x00000006	/* 2-1 Bits Address range */
									/* type                               */
#define PCI_MEM_BASE_TYPE_SHIFT		1			/* 0 - device can be mapped */
									/* any where in a 32 bit address space  */
#define PCI_MEM_BASE_PREFETCH		0x00000008	/* Prefetchable - no effect */
#define PCI_MEM_BASE_ADR_BASE_MASK	0xfff00000	/* 31-20 Bits Base address */
									/* of ATU                              */
#define PCI_MEM_BASE_ADR_BASE_SHIFT	20

/*
 * PCI Access to Internal USC Register
 * - Offset 18h, Size 32 bits
 */
#define PCI_REG_BASE_IO				0x00000001	/* I/O 1 - I/O space */
									/* 0 - Memory Space              */
#define PCI_REG_BASE_TYPE_MASK		0x00000006	/* 2-1 Bits Address range */
									/* type                               */
#define PCI_REG_BASE_TYPE_SHIFT		1			/* 0 - device can be mapped */
									/* any where in a 32 bit address space  */
#define PCI_REG_BASE_PREFETCH		0x00000008	/* Prefetchable - no effect */
#define PCI_REG_BASE_ADR_BASE_MASK	0xfffffe00	/* 31-20 Bits Base address */
									/* of USC registers                    */
#define PCI_REG_BASE_ADR_BASE_SHIFT	9

/*
 * PCI Base Address for Peripheral Access
 * - Offset 1ch, Size 32 bits
 */
#define PCI_PCU_BASE_IO				0x00000001	/* I/O 1 - I/O space */
									/* 0 - Memory Space              */
#define PCI_PCU_BASE_TYPE_MASK		0x00000006	/* 2-1 Bits Address range */
									/* type                               */
#define PCI_PCU_BASE_TYPE_SHIFT		1			/* 0 - device can be mapped */
									/* any where in a 32 bit address space  */
#define PCI_PCU_BASE_PREFETCH		0x00000008	/* Prefetchable - no effect */
#define PCI_PCU_BASE_SIZE_MASK		0x000000f0	/* 7-4 Bits size of aperture */
#define PCI_PCU_BASE_SIZE_SHIFT		4
#define PCI_PCU_BASE_ADR_BASE_MASK	0xffffff00	/* 31-20 Bits Base address */
									/* of peripheral access                */
#define PCI_PCU_BASE_ADR_BASE_SHIFT	8

/*
 * PCI CARDBUS CIS Pointer
 * - Offset 28h, Size 32 bits
 */

/*
 * PCI Sub Vendor ID
 * - Offset 2ch, Size 16 bits
 * - This value must be assigned by PCI SIG
 */

/*
 * PCI Sub System ID
 * - Offset 2eh, Size 16 bits
 * - This value is managed by the vendor
 */

/*
 * PCI Read Only Memory Register
 * - Offset 30h, Size 32 bits
 */
#define PCI_ROM_BASE_ENABLE			0x00000001	/* Expansion ROM enable */
#define PCI_ROM_BASE_PREFETCH		0x00000008	/* Prefetchable enable  */
#define PCI_ROM_BASE_SIZE_MASK		0x00000030	/* 5-4 Bits Aperture size */
#define PCI_ROM_BASE_SIZE_SHIFT		4
#define PCI_ROM_BASE_MAP_MASK		0x0000ff00	/* 15-8 Bits Map address */
#define PCI_ROM_BASE_MAP_SHIFT		8
#define PCI_ROM_BASE_ADR_BASE_MASK	0xffff0000	/* 31-16 Bits Base address */
#define PCI_ROM_BASE_ADR_BASE_SHIFT	16

/*
 * PCI Capablities
 * - Offset 34h, Size 8 bits
 */

/*
 * PCI Bus Parameters Register
 * - Offset 3ch, Size 32 bits
 */
#define PCI_BPARAM_INT_LINE_MASK	0x000000ff	/* 7-0 Bits Interrupt Line */
#define PCI_BPARAM_INT_LINE_SHIFT	0
#define PCI_BPARAM_INT_PIN_MASK		0x00000700	/* 10-8 Bits Interrupt Pin */
									/* 0 - disable, 1 - INTA, 2 - INT B    */
									/* 3 - INT C, 4 - INT C                */
#define PCI_BPARAM_INT_PIN_SHIFT	8
#define PCI_BPARAM_MIN_GRANT_MASK	0x00ff0000	/* 23-16 Bits Minimum Grant */
#define PCI_BPARAM_MIN_GRANT_SHIFT	16
#define PCI_BPARAM_MAX_LAT_MASK		0xff000000	/* 31-24 Bits Maximum Latency */
#define PCI_BPARAM_MAX_LAT_SHIFT	24

/*
 * PCI I2O Map Register
 * - Offset 50h, Size 32 bits
 */
#define PCI_I2O_MAP_ENABLE			0x00000001	/* Enable aperture */
#define PCI_I2O_MAP_REG_EN			0x00000002	/* Register enable */
#define PCI_I2O_MAP_I2O_MODE		0x00000004	/* I2O Mode enable */
#define PCI_I2O_MAP_RD_POST_INH		0x00000008
#define PCI_I2O_MAP_SIZE_1MB		0x00000000
#define PCI_I2O_MAP_SIZE_2MB		0x00000010
#define PCI_I2O_MAP_SIZE_4MB		0x00000020
#define PCI_I2O_MAP_SIZE_8MB		0x00000030
#define PCI_I2O_MAP_SIZE_16MB		0x00000040
#define PCI_I2O_MAP_SIZE_32MB		0x00000050
#define PCI_I2O_MAP_SIZE_64MB		0x00000060
#define PCI_I2O_MAP_SIZE_128MB		0x00000070
#define PCI_I2O_MAP_SIZE_256MB		0x00000080
#define PCI_I2O_MAP_SIZE_512MB		0x00000090
#define PCI_I2O_MAP_SIZE_1GB		0x000000a0
#define PCI_I2O_BYTE_SWAP_NO		0x00000000	/* No swap 32 bits */
#define PCI_I2O_BYTE_SWAP_16		0x00000100	/* 16 bits */
#define PCI_I2O_BYTE_SWAP_8			0x00000200	/* bits */
#define PCI_I2O_BYTE_SWAP_AUTO		0x00000300	/* Auto swap use BE[3:0]   */
#define PCI_I2O_PCI_RD_MB_00		0x00000000
#define PCI_I2O_PCI_RD_MB_01		0x00001000
#define PCI_I2O_PCI_RD_MB_10		0x00002000
#define PCI_I2O_PCI_WR_MB_00		0x00000000
#define PCI_I2O_PCI_WR_MB_01		0x00004000
#define PCI_I2O_PCI_WR_MB_10		0x00008000
#define PCI_I2O_W_FLUSH				0x00010000	/* write prefetch reads */
#define PCI_I2O_MAP_ADR_MASK		0x3ff00000
#define PCI_I2O_MAP_ADR_SHIFT		20

/*
 * PCI MEM Map Register
 * - Offset 54h, Size 32 bits
 */
#define PCI_MEM_MAP_ENABLE			0x00000001	/* Enable aperture */
#define PCI_MEM_MAP_REG_EN			0x00000002	/* Register enable */
#define PCI_MEM_MAP_I2O_MODE		0x00000004	/* I2O Mode enable */
#define PCI_MEM_MAP_RD_POST_INH		0x00000008
#define PCI_MEM_MAP_SIZE_1MB		0x00000000
#define PCI_MEM_MAP_SIZE_2MB		0x00000010
#define PCI_MEM_MAP_SIZE_4MB		0x00000020
#define PCI_MEM_MAP_SIZE_8MB		0x00000030
#define PCI_MEM_MAP_SIZE_16MB		0x00000040
#define PCI_MEM_MAP_SIZE_32MB		0x00000050
#define PCI_MEM_MAP_SIZE_64MB		0x00000060
#define PCI_MEM_MAP_SIZE_128MB		0x00000070
#define PCI_MEM_MAP_SIZE_256MB		0x00000080
#define PCI_MEM_MAP_SIZE_512MB		0x00000090
#define PCI_MEM_MAP_SIZE_1GB		0x000000a0
#define PCI_MEM_BYTE_SWAP_NO		0x00000000	/* No swap 32 bits */
#define PCI_MEM_BYTE_SWAP_16		0x00000100	/* 16 bits */
#define PCI_MEM_BYTE_SWAP_8			0x00000200	/* bits */
#define PCI_MEM_BYTE_SWAP_AUTO		0x00000300	/* Auto swap use BE[3:0]   */
#define PCI_MEM_PCI_RD_MB_00		0x00000000
#define PCI_MEM_PCI_RD_MB_01		0x00001000
#define PCI_MEM_PCI_RD_MB_10		0x00002000
#define PCI_MEM_PCI_WR_MB_00		0x00000000
#define PCI_MEM_PCI_WR_MB_01		0x00004000
#define PCI_MEM_PCI_WR_MB_10		0x00008000
#define PCI_MEM_W_FLUSH				0x00010000	/* write prefetch reads */
#define PCI_MEM_MAP_ADR_MASK		0x3ff00000
#define PCI_MEM_MAP_ADR_SHIFT		20

/*
 * PCI BUS CFG Register
 * -5ch Offset 5ch, Size 32 bits
 */
#define PCI_BUS_CFG_CFG_RETRY		0x00000100
#define PCI_BUS_CFG_PCI_INH			0x00000200
#define PCI_BUS_CFG_I2O_ONLINE		0x00003000
#define PCI_BUS_CFG_I2O_EN			0x00004000
#define PCI_BUS_CFG_I2O_EN_EN		0x00008000
#define PCI_BUS_CFG_PBRST_MAX_4		0x00000000
#define PCI_BUS_CFG_PBRST_MAX_8		0x00010000
#define PCI_BUS_CFG_PBRST_MAX_16	0x00020000
#define PCI_BUS_CFG_PBRST_MAX_256	0x00030000
#define PCI_BUS_CFG_TRDY_STOP		0x00100000
#define PCI_BUS_CFG_PCU_SWAP_00		0x00000000
#define PCI_BUS_CFG_PCU_SWAP_01		0x01000000
#define PCI_BUS_CFG_PCU_SWAP_10		0x02000000
#define PCI_BUS_CFG_PCU_SWAP_11		0x03000000
#define PCI_BUS_CFG_FAST_SCL		0x80000000

/*
 * LB_PCI_BASEx Registers
 * - Offset 60h, Size 32 bits
 * - Offset 64h, Size 32 bits
 */
#define LB_PCI_BASEX_ALOW_MASK		0x00000003	/* select value AD1:0 */
#define LB_PCI_BASEX_ALOW_SHIFT		0x00000000
#define LB_PCI_BASEX_ERR_EN			0x00000004	
#define LB_PCI_BASEX_PREFETCH		0x00000008	/* prefetch */
#define LB_PCI_BASEX_SIZE_DISABLE	0x00000000
#define LB_PCI_BASEX_SIZE_16MB		0x00000010
#define LB_PCI_BASEX_SIZE_32MB		0x00000020
#define LB_PCI_BASEX_SIZE_64MB		0x00000030
#define LB_PCI_BASEX_SIZE_128MB		0x00000040
#define LB_PCI_BASEX_SIZE_256MB		0x00000050
#define LB_PCI_BASEX_SIZE_512MB		0x00000060
#define LB_PCI_BASEX_SIZE_1GB		0x00000070
#define LB_PCI_BASEX_BYTE_SWAP_NO	0x00000000	/* No swap 32 bits */
#define LB_PCI_BASEX_BYTE_SWAP_16	0x00000100	/* 16 bits */
#define LB_PCI_BASEX_BYTE_SWAP_8	0x00000200	/* bits */
#define LB_PCI_BASEX_BYTE_SWAP_AUTO	0x00000300	/* Auto swap use BE[3:0]   */
#define LB_PCI_BASEX_COMBINE		0x00000800	/* Burst Write Combine */

#define LB_PCI_BASEX_PCI_CMD_MASK	0x0000e000
#define LB_PCI_BASEX_PCI_CMD_SHIFT	13
#define LB_PCI_BASEX_INT_ACK		0x00000000	/* Interrupt Ack */
#define LB_PCI_BASEX_IO				0x00002000	/* I/O Read/Write */
#define LB_PCI_BASEX_MEMORY			0x00006000	/* Memory Read/Write */
#define LB_PCI_BASEX_CONFIG			0x0000a000	/* Configuration Read/Write */
#define LB_PCI_BASEX_MULTI_MEMORY	0x0000c000	/* Multiple Memory Read/Write */
#define LB_PCI_BASEX_MEMORY_INVALIDATE	0x0000e000	/* Multiple Memory Read/e */
												/* Write Invalidate           */
#define LB_PCI_BASEX_MAP_ADR_MASK	0x00ff0000	/* PCI Address map */
#define LB_PCI_BASEX_MAP_ADR_SHIFT	16
#define LB_PCI_BASEX_BASE			0xff000000	/* Local Address base */
#define LB_PCI_BASEX_BASE_ADR_SHIFT	24


/*
 * System Register
 * - Offset 73h, Size 8 bits
 */
#define SYSTEM_B_SPROM_EN			0x01		/* 1 - Software control     */
									/* 0 - Hardware control                 */
#define SYSTEM_B_SDA_IN				0x02		/* Serial EEPROM data input */
#define SYSTEM_B_SDA_IN_SHIFT		1
#define SYSTEM_B_SDA_OUT			0x04		/* Serial EEPROM data output */
									/* SPROM_EN must be enabled              */
#define SYSTEM_B_SCL				0x08		/* Serial EEPROM clock output */
#define SYSTEM_B_LOO_EN				0x10		/* Hot swap LED control       */
#define SYSTEM_B_CFG_LOCK			0x20		/* Configuration Lock         */
#define SYSTEM_B_LOCK				0x40		/* Lock Register Contents set *										/* to 1 the contents become unwritable    */
									/* Clear lock by writing 0xa5 Writing     */									/* 0xa5 will not overwrite the current    */									/* system status values                   */
#define SYSTEM_B_UNLOCK_TOKEN		0xa5
#define SYSTEM_B_RST_OUT			0x80		/* Reset output control */

/*
 * SDRAM Local Base Address Register 
 * - Offset 78h, Size 32 bits
 */
#define LB_SDRAM_BASE_ENABLE		0x1			/* must be enabled to access */
#define LB_SDRAM_BASE_SIZE_64M		0x0
#define LB_SDRAM_BASE_SIZE_128M		0x10
#define LB_SDRAM_BASE_SIZE_256M		0x20
#define LB_SDRAM_BASE_SIZE_512M		0x30
#define LB_SDRAM_BASE_SIZE_1G		0x40

#define LB_SDRAM_BASE_MASK			0xfc000000
#define LB_SDRAM_BASE_SHIFT			26


/*
 * SDRAM Timing Parameters
 * - Offset 8ch, Size 32 bits
 */
#define SDRAM_CFG_TCAS_RD_1			0x0			/* CAS latency */
#define SDRAM_CFG_TCAS_RD_2			0x1			/* comment is cycles */
#define SDRAM_CFG_TCAS_RD_3			0x2

#define SDRAM_CFG_RCD_1				0x0			/* RAS to CAS delay */
#define SDRAM_CFG_RCD_2				0x40		/* comment is cycles */
#define SDRAM_CFG_RCD_3				0x80

#define SDRAM_CFG_RP_1				0x0			/* RAS precharge */
#define SDRAM_CFG_RP_2				0x100		/* comment is cycles */
#define SDRAM_CFG_RP_3				0x200
#define SDRAM_CFG_RP_4				0x300

#define SDRAM_CFG_RAS_2				0x0			/* RAS pulse width */
#define SDRAM_CFG_RAS_3				0x400		/* comment is cycles */
#define SDRAM_CFG_RAS_4				0x800
#define SDRAM_CFG_RAS_5				0xc00

#define SDRAM_CFG_DPL_1				0x0			/* Last data write precharge */
#define SDRAM_CFG_DPL_2				0x1000		/* comment is cycles */

#define SDRAM_CFG_REF_SCALE_MASK	0x00200000
#define SDRAM_CFG_REF_SCALE_SHIFT	21
#define SDRAM_CFG_REF_NDIV_MASK		0x0fc00000
#define SDRAM_CFG_REF_NDIV_SHIFT	22

/*
 * SDRAM Command Register
 * - Offset 8ah, Size 16 bits
 */
#define SDRAM_CMD_W_CMD_IPR			0x1			/* initiate command */

#define SDRAM_CMD_W_CS_SEL_0		0x2			/* Chip select bank 0 */
#define SDRAM_CMD_W_CS_SEL_1		0x4			/* Chip select bank 1 */
#define SDRAM_CMD_W_CS_SEL_2		0x8			/* Chip select bank 2 */
#define SDRAM_CMD_W_CS_SEL_3		0x10		/* Chip select bank 3 */

#define SDRAM_CMD_W_NOOP			0x0			/* SDRAM Commands */
#define SDRAM_CMD_W_BURST_TERM		0x20
#define SDRAM_CMD_W_READ			0x40
#define SDRAM_CMD_W_WRITE			0x60
#define SDRAM_CMD_W_BANK_ACTIVATE	0x80
#define SDRAM_CMD_W_PRECHARGE		0xa0
#define SDRAM_CMD_W_AUTO_REFRESH	0xc0
#define SDRAM_CMD_W_MODE_REGISTER	0xe0

/*
 * SDRAM Block Control Register
 * - Offset 90h, Size 32 bits
 * - Offset 94h, Size 32 bits
 * - Offset 98h, Size 32 bits
 * - Offset 9ch, Size 32 bits
 */
#define SDRAM_BANKX_ENABLE			0x1			/* Enable Bank */
#define SDRAM_BANKX_READ_ONLY		0x2			/* Read only */
#define SDRAM_BANKX_SIZE_512K		0x0
#define SDRAM_BANKX_SIZE_1M			0x10
#define SDRAM_BANKX_SIZE_2M			0x20
#define SDRAM_BANKX_SIZE_4M			0x30
#define SDRAM_BANKX_SIZE_8M			0x40
#define SDRAM_BANKX_SIZE_16M		0x50
#define SDRAM_BANKX_SIZE_32M		0x60
#define SDRAM_BANKX_SIZE_64M		0x70
#define SDRAM_BANKX_SIZE_128M		0x80
#define SDRAM_BANKX_SIZE_256M		0x90

#define SDRAM_BANKX_ROW_MUX_MODE_0	0x0
#define SDRAM_BANKX_ROW_MUX_MODE_1	0x100
#define SDRAM_BANKX_ROW_MUX_MODE_2	0x200
#define SDRAM_BANKX_ROW_MUX_MODE_3	0x300
#define SDRAM_BANKX_ROW_MUX_MODE_4	0x400
#define SDRAM_BANKX_ROW_MUX_MODE_5	0x500
#define SDRAM_BANKX_ROW_MUX_MODE_6	0x600
#define SDRAM_BANKX_ROW_MUX_MODE_7	0x700
#define SDRAM_BANKX_ROW_MUX_MODE_8	0x800
#define SDRAM_BANKX_ROW_MUX_MODE_9	0x900
#define SDRAM_BANKX_ROW_MUX_MODE_A	0xa00
#define SDRAM_BANKX_ROW_MUX_MODE_B	0xb00
#define SDRAM_BANKX_ROW_MUX_MODE_C	0xc00

#define SDRAM_BANKX_COL_MUX_MODE_0	0x0
#define SDRAM_BANKX_COL_MUX_MODE_1	0x1000
#define SDRAM_BANKX_COL_MUX_MODE_2	0x2000
#define SDRAM_BANKX_COL_MUX_MODE_3	0x3000
#define SDRAM_BANKX_COL_MUX_MODE_4	0x4000
#define SDRAM_BANKX_COL_MUX_MODE_5	0x5000
#define SDRAM_BANKX_COL_MUX_MODE_6	0x6000
#define SDRAM_BANKX_COL_MUX_MODE_7	0x7000

#define SDRAM_BANKX_OFFSET_MASK		0x3ff80000
#define SDRAM_BANKX_OFFSET_SHIFT	19


/*
 * Interrupt Configuration Register
 * - Offset e0h, Size 32 bits
 * - Offset e4h, Size 32 bits
 * - Offset e8h, Size 32 bits
 * - Offset 158h, Size 32 bits
 */
#define INT_CFGX_LB_MBI				0x00000001
#define INT_CFGX_PCI_MBI			0x00000002
#define INT_CFGX_I2O_OP_NE			0x00000008
#define INT_CFGX_I2O_IF_NF			0x00000010
#define INT_CFGX_I2O_IP_NE			0x00000020
#define INT_CFGX_I2O_OP_NF			0x00000040
#define INT_CFGX_I2O_OF_NE			0x00000080
#define INT_CFGX_INT0				0x00000100
#define INT_CFGX_INT1				0x00000200
#define INT_CFGX_INT2				0x00000400
#define INT_CFGX_INT3				0x00000800
#define INT_CFGX_TIMER0				0x00001000
#define INT_CFGX_TIMER1				0x00002000
#define INT_CFGX_ENUM				0x00004000
#define INT_CFGX_DMA0				0x00010000
#define INT_CFGX_DMA1				0x00020000
#define INT_CFGX_PWR_STATE			0x00100000
#define INT_CFGX_HBI				0x00200000
#define INT_CFGX_WDI				0x00400000
#define INT_CFGX_BWI				0x00800000
#define INT_CFGX_PSLAVE_PI			0x01000000
#define INT_CFGX_PMASTER_PI			0x02000000
#define INT_CFGX_PCI_T_ABORT		0x04000000
#define INT_CFGX_PCI_M_ABORT		0x08000000
#define INT_CFGX_DRA_PI				0x10000000
#define INT_CFGX_MODE				0x20000000
#define INT_CFGX_DI0				0x40000000
#define INT_CFGX_DI1				0x80000000


/*
 * General Purpose Timer Control Register
 * - Offset 150h, Size 16 bits
 * - Offset 152h, Size 16 bits
 */
#define TIMER_CTLX_W_TI_MODE_0		0x0000		/* Timer input event */
#define TIMER_CTLX_W_TI_MODE_1		0x0001
#define TIMER_CTLX_W_TI_MODE_2		0x0002
#define TIMER_CTLX_W_TI_MODE_3		0x0003

#define TIMER_CTLX_W_CNT_EN_0		0x0000		/* Count enable */
#define TIMER_CTLX_W_CNT_EN_1		0x0004
#define TIMER_CTLX_W_CNT_EN_2		0x0008
#define TIMER_CTLX_W_CNT_EN_3		0x000C

#define TIMER_CTLX_W_TRG_MODE_0		0x0000		/* Trigger mode */
#define TIMER_CTLX_W_TRG_MODE_1		0x0010
#define TIMER_CTLX_W_TRG_MODE_2		0x0020
#define TIMER_CTLX_W_TRG_MODE_3		0x0030

#define TIMER_CTLX_W_TO_MODE_0		0x0000		/* Timer output mode */
#define TIMER_CTLX_W_TO_MODE_1		0x0100
#define TIMER_CTLX_W_TO_MODE_2		0x0200
#define TIMER_CTLX_W_TO_MODE_3		0x0300
#define TIMER_CTLX_W_TO_MODE_4		0x0400
#define TIMER_CTLX_W_TO_MODE_5		0x0500

#define TIMER_CTLX_W_DLTCH_0		0x0000		/* Data latch mode */
#define TIMER_CTLX_W_DLTCH_1		0x0800
#define TIMER_CTLX_W_DLTCH_2		0x1000

#define TIMER_CTLX_W_ENABLE			0x8000		/* Timer enable */


/*
 * DMA Delay Register
 * - Offset 16Ch, Size 8 bits
 */
#define DMA_DELAY_MASK		0x000000ff	
#define DMA_DELAY_SHIFT		0

/*
 * DMA Command / Status Register
 * - Offset 170h, Size 32 bits
 * - Offset 174h, Size 32 bits
 */
#define DMA_CSR_IPR				0x00000001	/* initiate DMA transfer */
#define DMA_CSR_HALT			0x00000002	/* pause DMA transfer */
#define DMA_CSR_DONE			0x00000004	/* DMA transfer complete */
#define DMA_CSR_DCI				0x00000008	/* DMA control interrupt status	*/
#define DMA_CSR_DPE				0x00000010	/* DMA PCI BUS error status */
#define DMA_CSR_DONE_EN			0x00000400	/* DONE interrupt enable */
#define DMA_CSR_DCI_EN			0x00000800	/* DCI interrupt enable */
#define DMA_CSR_DPE_EN			0x00001000	/* DPE interrupt enable */
#define DMA_CSR_PRIORITY		0x00008000	/* DMA channel priority */
#define DMA_CSR_PCI_CMD0_MASK	0x000E0000	/* PCI Command Type 0 */
#define DMA_CSR_PCI_CMD0_SHIFT	17
#define DMA_CSR_PCI_CMD1_MASK	0x00E00000	/* PCI Command Type 1 */
#define DMA_CSR_PCI_CMD1_SHIFT	21
 
/*
 * DMA Transfer Control Register
 * - Offset 180h, Size 32 bits
 * - Offset 190h, Size 32 bits
 */
#define DMA_XFER_DMA_CNT_MASK	0x000FFFFF	/* DMA transfer count */
#define DMA_XFER_DMA_CNT_SHIFT	0
#define DMA_XFER_DTERM_EN		0x00400000	/* External terminate count enable */
#define DMA_XFER_BLOCK_FILL		0x00800000	/* Block fill feature enable */
#define DMA_XFER_DST_BUS		0x01000000	/* DMA destination BUS */
#define DMA_XFER_SRC_BUS		0x02000000	/* DMA source BUS */
#define DMA_XFER_PDST_TYPE		0x04000000	/* PCI destination command type */
#define DMA_XFER_PSRC_TYPE		0x08000000	/* PCI source command type */
#define DMA_XFER_SWAP_MASK		0x30000000	/* Byte swap control */
#define DMA_XFER_SWAP_SHIFT		28
#define DMA_XFER_UPDT_CNT		0x40000000	/* Update count */
#define DMA_XFER_DREQ_EN		0x80000000	/* External DRQ enable */


/*
 * DMA Control Block Register
 * - Offset 180h, Size 32 bits
 * - Offset 190h, Size 32 bits
 */
#define DMA_CTLB_BUS			0x00000001	/* DMA Control block address space */
#define DMA_CTLB_SA_INC_DIS		0x00000004	/* Source BUS address increment disable */
#define DMA_CTLB_DA_INC_DIS		0x00000008	/* Dest BUS address increment disable */
#define DMA_CTLB_ADDR_MASK		0xFFFFFFF0	/* DMA Control block address mask */
#define DMA_CTLB_ADDR_SHIFT		4


/*
 * PCI Configuration registers offsets
 */
#define	V3USC_PCI_VENDOR_W			V3REGH(0x00)
#define	V3USC_PCI_DEVICE_W			V3REGH(0x02)
#define	V3USC_PCI_CMD_W		 		V3REGH(0x04)
#define	V3USC_PCI_STAT_W			V3REGH(0x06)
#define	V3USC_PCI_CC_REV			V3REGW(0x08)
#define	V3USC_PCI_HDR_CFG	 		V3REGW(0x0c)
#define	V3USC_PCI_I2O_BASE	 		V3REGW(0x10)
#define	V3USC_PCI_MEM_BASE	 		V3REGW(0x14)
#define	V3USC_PCI_REG_BASE			V3REGW(0x18)
#define	V3USC_PCI_PCU_BASE			V3REGW(0x1c)
#define	V3USC_PCI_CIS				V3REGW(0x28)
#define	V3USC_PCI_SUB_VENDOR_W		V3REGH(0x2c)
#define	V3USC_PCI_SUB_ID_W			V3REGH(0x2e)
#define	V3USC_PCI_ROM_BASE	 		V3REGW(0x30)
#define	V3USC_PCI_CAPABLITY_B 		V3REGB(0x34)
#define	V3USC_PCI_BPARM				V3REGW(0x3c)
#define	V3USC_PM_CAP_ID_B			V3REGB(0x40)
#define	V3USC_PM_NEXT_ID_B			V3REGB(0x41)
#define	V3USC_PM_CAP_W				V3REGH(0x42)
#define	V3USC_PM_CRS_W				V3REGH(0x44)
#define	V3USC_PM_DATA_B				V3REGB(0x47)
#define	V3USC_PM_PWR_CON			V3REGW(0x48)
#define	V3USC_PM_PWR_DIS			V3REGW(0x4c)
#define	V3USC_PCI_I2O_MAP			V3REGW(0x50)
#define	V3USC_PCI_MEM_MAP			V3REGW(0x54)
#define	V3USC_PCI_BUS_CFG			V3REGW(0x5c)
#define	V3USC_LB_PCI_BASE0			V3REGW(0x60)
#define	V3USC_LB_PCI_BASE1			V3REGW(0x64)
#define	V3USC_LB_PCU_BASE			V3REGW(0x6c)
#define	V3USC_LB_REG_BASE_W			V3REGH(0x70)
#define	V3USC_SYSTEM_B				V3REGB(0x73)
#define	V3USC_LB_ROM_BASE			V3REGW(0x74)
#define	V3USC_LB_SDRAM_BASE			V3REGW(0x78)
#define	V3USC_LB_BUS_CFG			V3REGW(0x7c)
#define	V3USC_HS_CAP_ID_B			V3REGB(0x80)
#define	V3USC_HS_NEXT_ID_B			V3REGB(0x81)
#define	V3USC_HS_CSR_B				V3REGB(0x82)
#define	V3USC_LB_PCI_CTL_W			V3REGH(0x84)
#define	V3USC_T_CY_B				V3REGB(0x87)
#define	V3USC_SDRAM_MA_W			V3REGH(0x88)
#define	V3USC_SDRAM_CMD_W			V3REGH(0x8a)
#define	V3USC_SDRAM_CFG				V3REGW(0x8c)
#define	V3USC_SDRAM_BANK0			V3REGW(0x90)
#define	V3USC_SDRAM_BANK1			V3REGW(0x94)
#define	V3USC_SDRAM_BANK2			V3REGW(0x98)
#define	V3USC_SDRAM_BANK3			V3REGW(0x9c)
#define	V3USC_PCU_SUB0				V3REGW(0xa0)
#define	V3USC_PCU_SUB1				V3REGW(0xa4)
#define	V3USC_PCU_SUB2				V3REGW(0xa8)
#define	V3USC_PCU_TC_WR0			V3REGW(0xb0)
#define	V3USC_PCU_TC_WR1			V3REGW(0xb4)
#define	V3USC_PCU_TC_WR2			V3REGW(0xb8)
#define	V3USC_PCU_TC_WR3			V3REGW(0xbc)
#define	V3USC_PCU_TC_WR4			V3REGW(0xc0)
#define	V3USC_PCU_TC_RD0			V3REGW(0xc8)
#define	V3USC_PCU_TC_RD1			V3REGW(0xcc)
#define	V3USC_PCU_TC_RD2			V3REGW(0xd0)
#define	V3USC_PCU_TC_RD3			V3REGW(0xd4)
#define	V3USC_PCU_TC_RD4			V3REGW(0xd8)
#define	V3USC_INT_CFG0				V3REGW(0xe0)
#define	V3USC_INT_CFG1				V3REGW(0xe4)
#define	V3USC_INT_CFG2				V3REGW(0xe8)
#define	V3USC_INT_STAT				V3REGW(0xec)
#define	V3USC_IOS					V3REGW(0xf0)
#define	V3USC_WD_HBI_W				V3REGH(0xf4)
#define	V3USC_MAIL_WR_STAT_B		V3REGB(0xf8)
#define	V3USC_MAIL_RD_STAT_B		V3REGB(0xf9)
#define	V3USC_PCI_MAIL_IEWR_B		V3REGB(0xfc)
#define	V3USC_PCI_MAIL_IERD_B		V3REGB(0xfd)
#define	V3USC_LB_MAIL_IEWR_B		V3REGB(0xfe)
#define	V3USC_LB_MAIL_IERD_B		V3REGB(0xff)
#define	V3USC_MAIL_DATA0_B			V3REGB(0x100)
#define	V3USC_MAIL_DATA1_B			V3REGB(0x104)
#define	V3USC_MAIL_DATA2_B			V3REGB(0x108)
#define	V3USC_MAIL_DATA3_B			V3REGB(0x10c)
#define	V3USC_MAIL_DATA4_B			V3REGB(0x110)
#define	V3USC_MAIL_DATA5_B			V3REGB(0x114)
#define	V3USC_MAIL_DATA6_B			V3REGB(0x118)
#define	V3USC_MAIL_DATA7_B			V3REGB(0x11c)
#define	V3USC_TIMER_DATA0			V3REGW(0x140)
#define	V3USC_TIMER_DATA1			V3REGW(0x144)
#define	V3USC_TIMER_CTL0_W			V3REGH(0x150)
#define	V3USC_TIMER_CTL1_W			V3REGH(0x152)
#define	V3USC_INT_CFG3				V3REGW(0x158)
#define	V3USC_I2O_ISTAT				V3REGW(0x160)
#define	V3USC_I2O_IMASK				V3REGW(0x164)
#define	V3USC_DMA_DELAY				V3REGW(0x16C)
#define	V3USC_DMA_CSR0				V3REGW(0x170)
#define	V3USC_DMA_CSR1				V3REGW(0x174)
#define	V3USC_DMA_XFER_CTL0			V3REGW(0x180)
#define	V3USC_DMA_SRC_ADR0			V3REGW(0x184)
#define	V3USC_DMA_DST_ADR0			V3REGW(0x188)
#define	V3USC_DMA_CTLB_ADR0			V3REGW(0x18C)
#define	V3USC_DMA_XFER_CTL1			V3REGW(0x190)
#define	V3USC_DMA_SRC_ADR1			V3REGW(0x194)
#define	V3USC_DMA_DST_ADR1			V3REGW(0x198)
#define	V3USC_DMA_CTLB_ADR1			V3REGW(0x19C)

/*
 * I2O Mode offsets 
 */
#define	V3USC_IFL					V3USC_MAIL_DATA0_B
#define	V3USC_IPL					V3USC_MAIL_DATA2_B
#define	V3USC_OPL					V3USC_MAIL_DATA4_B
#define	V3USC_OFL					V3USC_MAIL_DATA6_B

/*********************************************************************
 *
 * This section defines some common tables by the registers
 *
 ********************************************************************/

/*
 * PCI Vendor ID
 * - Offset 00h, Size 16 bits
 */
#define	V3_VENDOR_ID				0x11b0		/* V3 PCI Vendor ID */

/*
 * PCI Device ID
 * - Offset 02h, Size 16 bits
 */
#define V960PBC						0x1			/* V3 Device ID for V960PBC */
#define V961PBC						0x2			/* V3 Device ID for V961PBC */
#define V962PBC						0x4			/* V3 Device ID for V962PBC */
#define V292PBC						0x10		/* V3 Device ID for V292PBC */
												/* MIPS MODE */ 
#define V320USC						0x100		/* V3 Device ID for V320USC */
												/* MIPS mode 9 bit SYSCMD   */ 
#define V320USC_5					0x101		/* V3 Device ID for V320USC */
												/* MIPS mode 5 bit SYSCMD   */ 
#define V320USC_SH3					0x102		/* V3 Device ID for V320USC */
												/* SH3 mode */ 
#define V320USC_SH4					0x103		/* V3 Device ID for V320USC */
												/* SH4 mode */ 
#define V370PDC						0x200		/* V3 Device ID for V370PDC */

/*
 * Stepping of V3USC320 as read back from V3USC_PCI_CC_REV register
 */
#define V3USC_REV_A0 0
#define V3USC_REV_B0 1
#define V3USC_REV_B1 2

/*
 * PCI Bus Parameters Register
 * - Offset 3ch, Size 32 bits
 */
#define INTERRUPT_PIN_DISABLE		0x0			/* Disabled */
#define INTERRUPT_PIN_INTA			0x1			/* Use INTA */
#define INTERRUPT_PIN_INTB			0x2			/* Use INTA */
#define INTERRUPT_PIN_INTC			0x3			/* Use INTA */
#define INTERRUPT_PIN_INTD			0x4			/* Use INTA */

/*
 * PCI Base Address for Peripheral Access 
 * - Offset 1ch, Size 32 bits
 * PCI Intelligent I/O Address Translation Unit Local Bus Address Map Register
 * - Offset ??h, Size 32 bits
 */
#define BYTE_SWAP_NO				0x0			/* No swap 32 bits */
#define BYTE_SWAP_16				0x1			/* 16 bits */
#define BYTE_SWAP_8					0x2			/* 8 bits */
#define BYTE_SWAP_AUTO				0x3			/* Auto swap use BE[3:0]   */
#define APERTURE_SIZE_1M			0x0			/* Aperture size of 1 MB   */
#define APERTURE_SIZE_2M			0x1			/* Aperture size of 2 MB   */
#define APERTURE_SIZE_4M			0x2			/* Aperture size of 4 MB   */
#define APERTURE_SIZE_8M			0x3			/* Aperture size of 8 MB   */
#define APERTURE_SIZE_16M			0x4			/* Aperture size of 16 MB  */
#define APERTURE_SIZE_32M			0x5			/* Aperture size of 32 MB  */
#define APERTURE_SIZE_64M			0x6			/* Aperture size of 64 MB  */
#define APERTURE_SIZE_128M			0x7			/* Aperture size of 128 MB */
#define APERTURE_SIZE_256M			0x8			/* Aperture size of 256 MB */
#define APERTURE_SIZE_512M			0x9			/* Aperture size of 512 MB */
#define APERTURE_SIZE_1G			0xa			/* Aperture size of 1 GB   */

/*
 * Mail box registers short equates
 */
#define EN0							0x0001		/* mailbox bit 0 */
#define EN1							0x0002		/* mailbox bit 1 */
#define EN2							0x0004		/* mailbox bit 2 */
#define EN3							0x0008		/* mailbox bit 3 */
#define EN4							0x0010		/* mailbox bit 4 */
#define EN5							0x0020		/* mailbox bit 5 */
#define EN6							0x0040		/* mailbox bit 6 */
#define EN7							0x0080		/* mailbox bit 7 */

/*
 * Enumerate bits 0-31 for KRF-Tech layer
 */

#define	BIT0 = 0x00000001,
#define	BIT1 = 0x00000002,
#define	BIT2 = 0x00000004,
#define	BIT3 = 0x00000008,
#define	BIT4 = 0x00000010,
#define	BIT5 = 0x00000020,
#define	BIT6 = 0x00000040,
#define	BIT7 = 0x00000080,
#define	BIT8 = 0x00000100,
#define	BIT9 = 0x00000200,
#define	BIT10 = 0x00000400,
#define	BIT11 = 0x00000800,
#define	BIT12 = 0x00001000,
#define	BIT13 = 0x00002000,
#define	BIT14 = 0x00004000,
#define	BIT15 = 0x00008000,
#define	BIT16 = 0x00010000,
#define	BIT17 = 0x00020000,
#define	BIT18 = 0x00040000,
#define	BIT19 = 0x00080000,
#define	BIT20 = 0x00100000,
#define	BIT21 = 0x00200000,
#define	BIT22 = 0x00400000,
#define	BIT23 = 0x00800000,
#define	BIT24 = 0x01000000,
#define	BIT25 = 0x02000000,
#define	BIT26 = 0x04000000,
#define	BIT27 = 0x08000000,
#define	BIT28 = 0x10000000,
#define	BIT29 = 0x20000000,
#define	BIT30 = 0x40000000,
#define	BIT31 = 0x80000000

#ifdef _V3USCREG_H_DATA_STRUCTURE_L_
/*
 * Structure definition for USC registers
 * The registers with multiple words or bytes defined
 * use the register that is at a multiple of four as
 * the name. ie PCI_VENDOR contains the vendor identifier. 
 */
#define _V3USCREG_H_DATA_STRUCTURE_

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pci_vendor :16;		/* vendor id */
  		DWORD pci_device :16;		/* device id */
	} _bit;	
} PCI_VENDOR;

typedef union
{
	DWORD _dword;
	struct
	{                               /* PCI CMD */
		DWORD io_en		:1;			/* I/O access enable */
		DWORD mem_en	:1;			/* memory access enable */
		DWORD master_en	:1;			/* PCI master enable */
		DWORD rfu1		:1;
		DWORD mwi_en	:1;			/* memory write and invalidate */
		DWORD rfu2		:1;
		DWORD par_en	:1;			/* parity enable */
		DWORD rfu3		:1;
		DWORD serr_en	:1;			/* system error */
		DWORD fbb_en	:1;			/* fast back to back enable */
		DWORD rfu4		:6;
		                            /* PCI STAT */
		DWORD rfu5		:4;
		DWORD new_cap	:1;			/* new capabilities */
		DWORD rfu6		:1;
		DWORD udf		:1;			/* user defined features */
		DWORD fast_back	:1;			/* fast back to back target enable */
		DWORD par_rep	:1;			/* data parity error report */
		DWORD devsel	:2;			/* device select timing */
		DWORD rfu7		:1;
		DWORD t_abort	:1;			/* target abort */
		DWORD m_abort	:1;			/* master abort */
		DWORD sys_err	:1;			/* system error */
		DWORD par_err	:1;			/* parity error */
	} _bit;	
} PCI_CMD;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD vrev		:4;			/* V3 revision ID */
		DWORD urev		:4;			/* user revision ID */
		DWORD prog_if	:8;			/* PCI programming interface code */
		DWORD sub_class	:8;			/* PCI subclass code */
		DWORD base_class :8;		/* PCI base class code */
	} _bit;
} PCI_CC_REV;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD line_size	:8;			/* cache line size */
		DWORD ltl		:3;			/* unimplemented latency timer bits */
		DWORD lt		:5;			/* latency timer */
		DWORD hdr_type	:8;			/* unimplemented header type */
		DWORD bist		:8;			/* unimplemented builtin self test */
	} _bit;
} PCI_HDR_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD io		:1;
		DWORD type		:2;
		DWORD prefetch	:1;
		DWORD rfu		:16;
		DWORD base		:12;
	} _bit;
} PCI_I2O_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD io		:1;
		DWORD type		:2;
		DWORD prefetch	:1;
		DWORD rfu		:2;
		DWORD enable	:1;
		DWORD reg_en	:1;
		DWORD rfu1		:1;
		DWORD adr_base	:23;
	} _bit;
} PCI_REG_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD io		:1;
		DWORD type		:2;
		DWORD prefetch	:1;
		DWORD size		:4;
		DWORD base		:24;
	} _bit;
} PCI_PCU_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD cis		:32;
	} _bit;
} PCI_CIS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pci_sub_vendor :16;	/* subsystem vendor id */
  		DWORD pci_sub_device :16;	/* subsystem device id */
	} _bit;	
} PCI_SUB_VENDOR;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD enable 		:1;		/* expansion ROM enable */
  		DWORD rfu1	 		:2;
  		DWORD prefetch 		:1;
		DWORD size			:2;
  		DWORD rfu2	 		:2;
		DWORD map			:8;
 		DWORD base	 		:16;	/* expansion ROM base address */
	} _bit;	
} PCI_ROM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD offset 		:8;
  		DWORD rfu	 		:24;
	} _bit;	
} PCI_CAPABLITY;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD int_line 		:8;		/* interrupt line */
		DWORD int_pin		:3;		/* interrupt pin */
  		DWORD rfu	 		:5;
 		DWORD min_gnt 		:8;		/* minimum grant */
		DWORD max_lat 		:8;		/* maximum latancy */
	} _bit;	
} PCI_BPARAM;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD id	 		:8;
  		DWORD offset		:8;
  		DWORD pm_ver 		:3;
  		DWORD pme_clk 		:1;
  		DWORD aux_pwr 		:1;
  		DWORD dsi	 		:1;
  		DWORD rfu1	 		:3;
  		DWORD d1_supt 		:1;
  		DWORD d2_supt 		:1;
  		DWORD pme_supt 		:4;
  		DWORD pme_cold 		:1;
	} _bit;	
} PM_CAP_ID;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pwr_state		:2;
  		DWORD rfu1			:6;
  		DWORD pme_en		:1;
  		DWORD data_sel		:4;
  		DWORD data_scale	:2;
  		DWORD pme_stat		:1;
  		DWORD rfu	 		:8;
  		DWORD data			:8;
	} _bit;	
} PM_CRS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data_d0		:8;
  		DWORD data_d1		:8;
  		DWORD data_d2		:8;
  		DWORD data_d3		:8;
	} _bit;	
} PM_PWR_CON;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data_d0		:8;
  		DWORD data_d1		:8;
  		DWORD data_d2		:8;
  		DWORD data_d3		:8;
	} _bit;	
} PM_PWR_DIS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD enable		:1;
  		DWORD reg_en		:1;
  		DWORD i2o_mode		:1;
  		DWORD rd_post_inh	:1;
  		DWORD size			:4;
  		DWORD swap			:2;
  		DWORD rfu1	 		:2;
  		DWORD pci_rd_mb		:2;
  		DWORD pci_wr_mb		:2;
  		DWORD w_flush		:1;
  		DWORD rfu2	 		:3;
  		DWORD map_adr 		:10;
		DWORD rfu3			:2;
	} _bit;	
} PCI_I2O_MAP;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu1			:8;
  		DWORD cfg_retry		:1;
  		DWORD pci_inh		:1;
  		DWORD rfu2			:2;
  		DWORD i2o_online	:2;
  		DWORD i2o_en		:1;
  		DWORD i2o_en_en		:1;
  		DWORD pbrst_max		:2;
		DWORD rfu3			:3;
  		DWORD trdy_stop		:1;
  		DWORD rfu4	 		:2;
		DWORD pcu_swap		:2;
		DWORD rfu5	 		:5;
		DWORD fast_scl 		:1;
	} _bit;	
} PCI_BUS_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD alow			:2;
  		DWORD err_en 		:1;
  		DWORD prefetch 		:1;
  		DWORD size	 		:3;
  		DWORD rfu1	 		:1;
  		DWORD swap	 		:2;
  		DWORD rfu2	 		:1;
  		DWORD combine 		:1;
  		DWORD rfu3	 		:1;
  		DWORD pci_cmd 		:3;
  		DWORD map_adr 		:8;
  		DWORD base	 		:8;
	} _bit;	
} LB_PCI_BASEX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD enable		:1;
  		DWORD rfu1			:3;
  		DWORD size	 		:3;
  		DWORD rfu2	 		:17;
  		DWORD base	 		:8;
	} _bit;	
} LB_PCU_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD base			:16;
  		DWORD rfu	 		:8;
		DWORD sprom_en		:1;   /* serial PROM software access enable */
		DWORD sda_in		:1;   /* serial PROM data in */
		DWORD sda_out		:1;   /* serial PROM data out */
		DWORD scl			:1;   /* serial PROM clock output */
		DWORD loo_en		:1;
		DWORD cfg_lock		:1;
		DWORD lock			:1;   /* lock system register contents */
		DWORD rst_out		:1;   /* reset output control */
	} _bit;	
} LB_REG_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu1			:4;
  		DWORD size	 		:3;
  		DWORD rfu2	 		:1;
  		DWORD we	 		:1;
  		DWORD rfu3	 		:15;
  		DWORD base	 		:8;
	} _bit;	
} LB_ROM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD enable		:1;
  		DWORD rfu1	 		:3;
  		DWORD size	 		:4;
  		DWORD rfu2	 		:18;
  		DWORD sdram_base		:6;
	} _bit;	
} LB_SDRAM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD err_en		:1;
  		DWORD merr_en		:1;
  		DWORD poe	 		:1;
  		DWORD clk_dis 		:1;
  		DWORD bw_tc	 		:8;
  		DWORD rfu1	 		:4;
  		DWORD endian 		:1;
  		DWORD rfu2	 		:11;
  		DWORD mb_lt	 		:4;
	} _bit;	
} LB_BUS_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD id			:8;
  		DWORD offset		:8;
  		DWORD rfu1			:1;
  		DWORD eim			:1;
  		DWORD rfu2			:1;
  		DWORD loo			:1;
  		DWORD rfu3			:2;
  		DWORD ext			:1;
  		DWORD ins			:1;
  		DWORD rfu4			:8;
	} _bit;	
} HS_CAP_ID;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD w_flush0		:2;
  		DWORD w_flush1		:2;
  		DWORD rfu1			:3;
  		DWORD one_rd_buf	:1;
  		DWORD lb_rd_pci		:2;
  		DWORD rfu2	 		:4;
  		DWORD lb_sr_pci 	:2;
  		DWORD rfu3	 		:8;
  		DWORD t_cy			:8;
	} _bit;	
} LB_PCI_CTL;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD ma			:15;
  		DWORD rfu1	 		:1;
  		DWORD cmd_ipr 		:1;
  		DWORD cs_sel 		:4;
  		DWORD cmd_type 		:3;
  		DWORD rfu3	 		:8;
	} _bit;	
} SDRAM_MA;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD t_cas_rd		:2;
  		DWORD rfu1	 		:4;
  		DWORD t_rcd	 		:2;
  		DWORD t_rp	 		:2;
  		DWORD t_ras	 		:2;
  		DWORD t_dpl	 		:1;
  		DWORD rfu2	 		:8;
  		DWORD ref_scale		:1;
  		DWORD ref_ndiv		:6;
  		DWORD rfu3			:4;
	} _bit;	
} SDRAM_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD enable		:1;
  		DWORD protect		:1;
  		DWORD rfu1			:2;
  		DWORD size	 		:4;
  		DWORD row_mux		:3;
  		DWORD rfu2	 		:1;
  		DWORD col_mux 		:3;
  		DWORD rfu3	 		:4;
  		DWORD offset 		:11;
  		DWORD rfu4	 		:2;
	} _bit;	
} SDRAM_BANKX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu1			:4;
  		DWORD size	 		:4;
  		DWORD rfu2	 		:12;
  		DWORD sub	 		:8;
  		DWORD rfu3	 		:4;
	} _bit;	
} PCU_SUBX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD t_cy_end		:5;
  		DWORD rfu1	 		:2;
  		DWORD be_mask_en	:1;		/* This bit is reserved in revision A0 */
  		DWORD t_cs_end 		:5;
  		DWORD rfu2	 		:1;
  		DWORD width	 		:2;
  		DWORD t_mrdy 		:5;
  		DWORD rfu3 			:1;
  		DWORD mrdy_mode		:2;
  		DWORD t_start		:3;
  		DWORD rfu4			:1;
  		DWORD t_burst		:2;
  		DWORD rfu5			:1;
  		DWORD wr_en			:1;
	} _bit;	
} PCU_TC_WRX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD t_cy_end		:5;
  		DWORD rfu1	 		:1;
  		DWORD cs_mode 		:1;
  		DWORD byte_pack		:1;		/* This bit is reserved in revision A0 */
  		DWORD t_cs_end 		:5;
  		DWORD rfu2	 		:1;
  		DWORD t_cy_en 		:1;
  		DWORD wr_pol 		:1;
  		DWORD t_mrdy 		:5;
  		DWORD rfu3 			:1;
  		DWORD mrdy_mode		:2;
  		DWORD t_start		:3;
  		DWORD rfu5			:1;
  		DWORD t_burst		:2;
  		DWORD rfu6			:1;
  		DWORD rd_en			:1;
	} _bit;	
} PCU_TC_RDX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD lb_mbi		:1;
  		DWORD pci_mbi		:1;
  		DWORD rfu1			:1;
  		DWORD i2o_op_ne		:1;
  		DWORD i2o_if_nf		:1;
  		DWORD i2o_ip_ne		:1;
  		DWORD i2o_op_nf		:1;
  		DWORD i2o_of_ne		:1;
  		DWORD int0			:1;
  		DWORD int1			:1;
  		DWORD int2			:1;
  		DWORD int3			:1;
  		DWORD timer0		:1;
  		DWORD timer1		:1;
  		DWORD rfu2			:1;
  		DWORD hsenum		:1;
  		DWORD dma0	 		:1;
  		DWORD dma1	 		:1;
  		DWORD rfu3	 		:2;
  		DWORD pwr_state		:1;
  		DWORD hbi			:1;
  		DWORD wdi			:1;
  		DWORD bwi			:1;
  		DWORD pslave_pi		:1;
  		DWORD pmaster_pi	:1;
  		DWORD pci_wr		:1;
  		DWORD pci_rd		:1;
  		DWORD sdram_di		:1;
  		DWORD mode			:1;
  		DWORD di0	 		:1;
  		DWORD di1	 		:1;
	} _bit;	
} INT_CFGX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD lb_mbi		:1;
  		DWORD pci_mbi		:1;
  		DWORD rfu1			:1;
  		DWORD i2o_op_ne		:1;
  		DWORD i2o_if_nf		:1;
  		DWORD i2o_ip_ne		:1;
  		DWORD i2o_op_nf		:1;
  		DWORD i2o_of_ne		:1;
  		DWORD int0			:1;
  		DWORD int1			:1;
  		DWORD int2			:1;
  		DWORD int3			:1;
  		DWORD timer0		:1;
  		DWORD timer1		:1;
  		DWORD rfu2			:1;
  		DWORD hsenum		:1;
  		DWORD dma0	 		:1;
  		DWORD dma1	 		:1;
  		DWORD rfu3	 		:2;
  		DWORD pwr_state		:1;
  		DWORD hbi			:1;
  		DWORD wdi			:1;
  		DWORD bwi			:1;
  		DWORD pslave_pi		:1;
  		DWORD pmaster_pi	:1;
  		DWORD pci_wr		:1;
  		DWORD pci_rd		:1;
  		DWORD sdram_di		:1;
  		DWORD mode			:1;
  		DWORD di0	 		:1;
  		DWORD di1	 		:1;
	} _bit;	
} INT_STAT;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD ic0			:2;
  		DWORD ic1			:2;
  		DWORD ic2			:2;
  		DWORD ic3			:2;
  		DWORD ic4			:2;
  		DWORD ic5			:2;
  		DWORD ic6			:2;
  		DWORD ic7			:2;
  		DWORD ic8			:2;
  		DWORD ic9			:2;
  		DWORD ic10			:2;
  		DWORD ic11			:2;
  		DWORD rfu	 		:8;
	} _bit;	
} IOS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD wd_tc			:8;
  		DWORD hb_tc			:10;
  		DWORD rfu1			:1;
  		DWORD test			:1;
  		DWORD wd_init		:1;
  		DWORD prescale		:1;
  		DWORD rfu2			:1;
  		DWORD wd_en			:1;
  		DWORD rfu3			:8;
	} _bit;	
} WD_HBI;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD wr_stat0		:1;
		DWORD wr_stat1		:1;
		DWORD wr_stat2		:1;
		DWORD wr_stat3		:1;
		DWORD wr_stat4		:1;
		DWORD wr_stat5		:1;
		DWORD wr_stat6		:1;
		DWORD wr_stat7		:1;
		DWORD rd_stat0		:1;
		DWORD rd_stat1		:1; 
		DWORD rd_stat2		:1;
		DWORD rd_stat3		:1;
		DWORD rd_stat4		:1;
		DWORD rd_stat5		:1;
		DWORD rd_stat6		:1;
		DWORD rd_stat7		:1;
		DWORD rfu			:16;
	} _bit;
} MAIL_WR_STAT;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD pci_wr_en0		:1;
		DWORD pci_wr_en1		:1;
		DWORD pci_wr_en2		:1;
		DWORD pci_wr_en3		:1;
		DWORD pci_wr_en4		:1;
		DWORD pci_wr_en5		:1;
		DWORD pci_wr_en6		:1;
		DWORD pci_wr_en7		:1;
		DWORD pci_rd_en0		:1;
		DWORD pci_rd_en1		:1;
		DWORD pci_rd_en2		:1;
		DWORD pci_rd_en3		:1;
		DWORD pci_rd_en4		:1;
		DWORD pci_rd_en5		:1;
		DWORD pci_rd_en6		:1;
		DWORD pci_rd_en7		:1;
		DWORD lb_wr_en0			:1;
		DWORD lb_wr_en1			:1;
		DWORD lb_wr_en2			:1;
		DWORD lb_wr_en3			:1;
		DWORD lb_wr_en4			:1;
		DWORD lb_wr_en5			:1;
		DWORD lb_wr_en6			:1;
		DWORD lb_wr_en7			:1;
		DWORD lb_rd_en0			:1;
		DWORD lb_rd_en1			:1;
		DWORD lb_rd_en2			:1;
		DWORD lb_rd_en3			:1;
		DWORD lb_rd_en4			:1;
		DWORD lb_rd_en5			:1;
		DWORD lb_rd_en6			:1;
		DWORD lb_rd_en7			:1;
	} _bit;
} PCI_MAIL_IEWR;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data0			:8;
		DWORD rfu1			:8;
		DWORD rfu2			:8;
		DWORD rfu3			:8;
	} _bit;	
	DWORD _ifl;
	DWORD _ipl;
} MAIL_DATAX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data			:32;
	} _bit;	
} TIMER_DATAX;

typedef	union
{
	USHORT _word;
	struct
	{
  		DWORD ti_mode		:2;
  		DWORD cnt_en		:2;
  		DWORD trg			:2;
  		DWORD rfu1	 		:2;
  		DWORD to_mode 		:3;
  		DWORD dltch	 		:2;
  		DWORD rfu2	 		:2;
  		DWORD enable 		:1;
	} _bit;
} TIMER_CTLX;

typedef union
{
	DWORD _dword;
	struct {
		TIMER_CTLX	timer0;
		TIMER_CTLX	timer1;
	} _struct;
} TIMER_CTL;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu1			:3;
  		DWORD out_post		:1;
  		DWORD rfu2	 		:28;
	} _bit;	
} PCI_I2O_ISTAT;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu1			:3;
  		DWORD out_post		:1;
  		DWORD rfu2	 		:28;
	} _bit;	
} PCI_I2O_IMASK;


typedef union
{
	DWORD data;
	struct
	{
  		DWORD delay			:8;
		DWORD rfu1			:24;
	} bits;	
} DMA_DELAY;

typedef union
{
	DWORD data;
	struct
	{
  		DWORD ipr			:1;
  		DWORD halt			:1;
  		DWORD done	 		:1;
  		DWORD dci	 		:1;
		DWORD dpe	 		:1;
		DWORD rfu1	 		:5;
		DWORD done_en 		:1;
		DWORD dci_en 		:1;
		DWORD dpe_en 		:1;
		DWORD rfu2	 		:2;
		DWORD priority 		:1;
		DWORD rfu3	 		:1;
		DWORD cmd0	 		:3;
		DWORD rfu4	 		:1;
		DWORD cmd1	 		:3;
		DWORD rfu5	 		:8;
	} bits;	
} DMA_CSR;

typedef union
{
	DWORD data;
	struct
	{
  		DWORD cnt			:21;
		DWORD rfu			:1;
  		DWORD dterm_en		:1;
  		DWORD block_fill	:1;
  		DWORD dst_bus 		:1;
		DWORD src_bus 		:1;
		DWORD pdst_type		:1;
		DWORD psrc_type 	:1;
		DWORD swap	 		:2;
		DWORD updt_cnt		:1;
		DWORD dreq_en 		:1;
	} bits;	
} DMA_XFER_CTL;

typedef union
{
	DWORD data;
	struct
	{
  		DWORD ctlb_bus		:1;
  		DWORD rfu1			:1;
  		DWORD sa_inc_dis	:1;
  		DWORD da_inc_dis	:1;
		DWORD ctlb_addr		:28;
	} bits;	
} DMA_CTLB_ADR;


typedef union
{
	DWORD data;
	struct
	{
  		DWORD data			:32;
	} bits;	
} DMA_SRC_ADR;


typedef union
{
	DWORD data;
	struct
	{
  		DWORD data			:32;
	} bits;	
} DMA_DST_ADR;

#endif /* _V3USCREG_H_DATA_STRUCTURE_L_ */


#ifdef _V3USCREG_H_DATA_STRUCTURE_B_
/*
 * Structure definition for USC registers
 * The registers with multiple words or bytes defined
 * use the register that is at a multiple of four as
 * the name. ie PCI_VENDOR contains the vendor identifier. 
 */
#define _V3USCREG_H_DATA_STRUCTURE_

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pci_device :16;		/* device id */
  		DWORD pci_vendor :16;		/* vendor id */
	} _bit;	
} PCI_VENDOR;

typedef union
{
	DWORD _dword;
	struct
	{                               /* PCI STAT */
		DWORD par_err	:1;			/* parity error */
		DWORD sys_err	:1;			/* system error */
		DWORD m_abort	:1;			/* master abort */
		DWORD t_abort	:1;			/* target abort */
		DWORD rfu7		:1;
		DWORD devsel	:2;			/* device select timing */
		DWORD par_rep	:1;			/* data parity error report */
		DWORD fast_back	:1;			/* fast back to back target enable */
		DWORD udf		:1;			/* user defined features */
		DWORD rfu6		:1;
		DWORD new_cap	:1;			/* new capabilities */
		DWORD rfu5		:4;
		                            /* PCI CMD */
		DWORD rfu4		:6;
		DWORD fbb_en	:1;			/* fast back to back enable */
		DWORD serr_en	:1;			/* system error */
		DWORD rfu3		:1;
		DWORD par_en	:1;			/* parity enable */
		DWORD rfu2		:1;
		DWORD mwi_en	:1;			/* memory write and invalidate */
		DWORD rfu1		:1;
		DWORD master_en	:1;			/* PCI master enable */
		DWORD mem_en	:1;			/* memory access enable */
		DWORD io_en		:1;			/* I/O access enable */
	} _bit;	
} PCI_CMD;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD base_class :8;		/* PCI base class code */
		DWORD sub_class	:8;			/* PCI subclass code */
		DWORD prog_if	:8;			/* PCI programming interface code */
		DWORD urev		:4;			/* user revision ID */
		DWORD vrev		:4;			/* V3 revision ID */
	} _bit;
} PCI_CC_REV;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD bist		:8;			/* unimplemented builtin self test */
		DWORD hdr_type	:8;			/* unimplemented header type */
		DWORD lt		:5;			/* latency timer */
		DWORD ltl		:3;			/* unimplemented latency timer bits */
		DWORD line_size	:8;			/* cache line size */
	} _bit;
} PCI_HDR_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD base		:12;
		DWORD rfu		:16;
		DWORD prefetch	:1;
		DWORD type		:2;
		DWORD io		:1;
	} _bit;
} PCI_I2O_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD adr_base	:23;
		DWORD rfu1		:1;
		DWORD reg_en	:1;
		DWORD enable	:1;
		DWORD rfu		:2;
		DWORD prefetch	:1;
		DWORD type		:2;
		DWORD io		:1;
	} _bit;
} PCI_REG_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD base		:24;
		DWORD size		:4;
		DWORD prefetch	:1;
		DWORD type		:2;
		DWORD io		:1;
	} _bit;
} PCI_PCU_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD cis		:32;
	} _bit;
} PCI_CIS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pci_sub_device :16;	/* subsystem device id */
  		DWORD pci_sub_vendor :16;	/* subsystem vendor id */
	} _bit;	
} PCI_SUB_VENDOR;

typedef union
{
	DWORD _dword;
	struct
	{
 		DWORD base	 		:16;	/* expansion ROM base address */
		DWORD map			:8;
  		DWORD rfu2	 		:2;
		DWORD size			:2;
  		DWORD prefetch 		:1;
  		DWORD rfu1	 		:2;
  		DWORD enable 		:1;		/* expansion ROM enable */
	} _bit;	
} PCI_ROM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu	 		:24;
  		DWORD offset 		:8;
	} _bit;	
} PCI_CAPABLITY;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD max_lat 		:8;		/* maximum latancy */
 		DWORD min_gnt 		:8;		/* minimum grant */
  		DWORD rfu	 		:5;
		DWORD int_pin		:3;		/* interrupt pin */
  		DWORD int_line 		:8;		/* interrupt line */
	} _bit;	
} PCI_BPARAM;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD pme_cold 		:1;
  		DWORD pme_supt 		:4;
  		DWORD d2_supt 		:1;
  		DWORD d1_supt 		:1;
  		DWORD rfu1	 		:3;
  		DWORD dsi	 		:1;
  		DWORD aux_pwr 		:1;
  		DWORD pme_clk 		:1;
  		DWORD pm_ver 		:3;
  		DWORD offset		:8;
  		DWORD id	 		:8;
	} _bit;	
} PM_CAP_ID;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data			:8;
  		DWORD rfu	 		:8;
  		DWORD pme_stat		:1;
  		DWORD data_scale	:2;
  		DWORD data_sel		:4;
  		DWORD pme_en		:1;
  		DWORD rfu1			:6;
  		DWORD pwr_state		:2;
	} _bit;	
} PM_CRS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data_d3		:8;
  		DWORD data_d2		:8;
  		DWORD data_d1		:8;
  		DWORD data_d0		:8;
	} _bit;	
} PM_PWR_CON;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data_d3		:8;
  		DWORD data_d2		:8;
  		DWORD data_d1		:8;
  		DWORD data_d0		:8;
	} _bit;	
} PM_PWR_DIS;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD rfu3			:2;
  		DWORD map_adr 		:10;
  		DWORD rfu2	 		:3;
  		DWORD w_flush		:1;
  		DWORD pci_wr_mb		:2;
  		DWORD pci_rd_mb		:2;
  		DWORD rfu1	 		:2;
  		DWORD swap			:2;
  		DWORD size			:4;
  		DWORD rd_post_inh	:1;
  		DWORD i2o_mode		:1;
  		DWORD reg_en		:1;
  		DWORD enable		:1;
	} _bit;	
} PCI_I2O_MAP;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD fast_scl 		:1;
		DWORD rfu5	 		:5;
		DWORD pcu_swap		:2;
  		DWORD rfu4	 		:2;
  		DWORD trdy_stop		:1;
		DWORD rfu3			:3;
  		DWORD pbrst_max		:2;
  		DWORD i2o_en_en		:1;
  		DWORD i2o_en		:1;
  		DWORD i2o_online	:2;
  		DWORD rfu2			:2;
  		DWORD pci_inh		:1;
  		DWORD cfg_retry		:1;
  		DWORD rfu1			:8;
	} _bit;	
} PCI_BUS_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD base	 		:8;
  		DWORD map_adr 		:8;
  		DWORD pci_cmd 		:3;
  		DWORD rfu3	 		:1;
  		DWORD combine 		:1;
  		DWORD rfu2	 		:1;
  		DWORD swap	 		:2;
  		DWORD rfu1	 		:1;
  		DWORD size	 		:3;
  		DWORD prefetch 		:1;
  		DWORD err_en 		:1;
  		DWORD alow			:2;
	} _bit;	
} LB_PCI_BASEX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD base	 		:8;
  		DWORD rfu2	 		:17;
  		DWORD size	 		:3;
  		DWORD rfu1			:3;
  		DWORD enable		:1;
	} _bit;	
} LB_PCU_BASE;

typedef union
{
	DWORD _dword;
	struct
	{

		DWORD rst_out		:1;   /* reset output control */
		DWORD lock			:1;   /* lock system register contents */
		DWORD cfg_lock		:1;
		DWORD loo_en		:1;
		DWORD scl			:1;   /* serial PROM clock output */
		DWORD sda_out		:1;   /* serial PROM data out */
		DWORD sda_in		:1;   /* serial PROM data in */
		DWORD sprom_en		:1;   /* serial PROM software access enable */
  		DWORD rfu	 		:8;
  		DWORD base			:16;
	} _bit;	
} LB_REG_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD base	 		:8;
  		DWORD rfu3	 		:15;
  		DWORD we	 		:1;
  		DWORD rfu2	 		:1;
  		DWORD size	 		:3;
  		DWORD rfu1			:4;
	} _bit;	
} LB_ROM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD sdram_base	:6;
  		DWORD rfu2	 		:18;
  		DWORD size	 		:4;
  		DWORD rfu1	 		:3;
  		DWORD enable		:1;
	} _bit;	
} LB_SDRAM_BASE;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD mb_lt	 		:4;
  		DWORD rfu2	 		:11;
  		DWORD endian 		:1;
  		DWORD rfu1	 		:4;
  		DWORD bw_tc	 		:8;
  		DWORD clk_dis 		:1;
  		DWORD poe	 		:1;
  		DWORD merr_en		:1;
  		DWORD err_en		:1;
	} _bit;	
} LB_BUS_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu4			:8;
  		DWORD ins			:1;
  		DWORD ext			:1;
  		DWORD rfu3			:2;
  		DWORD loo			:1;
  		DWORD rfu2			:1;
  		DWORD eim			:1;
  		DWORD rfu1			:1;
  		DWORD offset		:8;
  		DWORD id			:8;
	} _bit;	
} HS_CAP_ID;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD t_cy			:8;
  		DWORD rfu3	 		:8;
  		DWORD lb_sr_pci 	:2;
  		DWORD rfu2	 		:4;
  		DWORD lb_rd_pci		:2;
  		DWORD one_rd_buf	:1;
  		DWORD rfu1			:3;
  		DWORD w_flush1		:2;
  		DWORD w_flush0		:2;
	} _bit;	
} LB_PCI_CTL;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu3	 		:8;
  		DWORD cmd_type 		:3;
  		DWORD cs_sel 		:4;
  		DWORD cmd_ipr 		:1;
  		DWORD rfu1	 		:1;
  		DWORD ma			:15;
	} _bit;	
} SDRAM_MA;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu3			:4;
  		DWORD ref_ndiv		:6;
  		DWORD ref_scale		:1;
  		DWORD rfu2	 		:8;
  		DWORD t_dpl	 		:1;
  		DWORD t_ras	 		:2;
  		DWORD t_rp	 		:2;
  		DWORD t_rcd	 		:2;
  		DWORD rfu1	 		:4;
  		DWORD t_cas_rd		:2;
	} _bit;	
} SDRAM_CFG;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu4	 		:2;
  		DWORD offset 		:11;
  		DWORD rfu3	 		:4;
  		DWORD col_mux 		:3;
  		DWORD rfu2	 		:1;
  		DWORD row_mux		:3;
  		DWORD size	 		:4;
  		DWORD rfu1			:2;
  		DWORD protect		:1;
  		DWORD enable		:1;
	} _bit;	
} SDRAM_BANKX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu3	 		:4;
  		DWORD sub	 		:8;
  		DWORD rfu2	 		:12;
  		DWORD size	 		:4;
  		DWORD rfu1			:4;
	} _bit;	
} PCU_SUBX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD wr_en			:1;
  		DWORD rfu5			:1;
  		DWORD t_burst		:2;
  		DWORD rfu4			:1;
  		DWORD t_start		:3;
  		DWORD mrdy_mode		:2;
  		DWORD rfu3 			:1;
  		DWORD t_mrdy 		:5;
  		DWORD width	 		:2;
  		DWORD rfu2	 		:1;
  		DWORD t_cs_end 		:5;
  		DWORD be_mask_en	:1;		/* This bit is reserved in revision A0 */
  		DWORD rfu1	 		:2;
  		DWORD t_cy_end		:5;
	} _bit;	
} PCU_TC_WRX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rd_en			:1;
  		DWORD rfu6			:1;
  		DWORD t_burst		:2;
  		DWORD rfu5			:1;
  		DWORD t_start		:3;
  		DWORD mrdy_mode		:2;
  		DWORD rfu3 			:1;
  		DWORD t_mrdy 		:5;
  		DWORD wr_pol 		:1;
  		DWORD t_cy_en 		:1;
  		DWORD rfu2	 		:1;
  		DWORD t_cs_end 		:5;
  		DWORD byte_pack		:1;		/* This bit is reserved in revision A0 */
  		DWORD cs_mode		:1;
  		DWORD rfu1	 		:1;
  		DWORD t_cy_end		:5;
	} _bit;	
} PCU_TC_RDX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD di1	 		:1;
  		DWORD di0	 		:1;
  		DWORD mode			:1;
  		DWORD sdram_di		:1;
  		DWORD pci_rd		:1;
  		DWORD pci_wr		:1;
  		DWORD pmaster_pi	:1;
  		DWORD pslave_pi		:1;
  		DWORD bwi			:1;
  		DWORD wdi			:1;
  		DWORD hbi			:1;
  		DWORD pwr_state		:1;
  		DWORD rfu3	 		:2;
  		DWORD dma1	 		:1;
  		DWORD dma0	 		:1;
  		DWORD hsenum		:1;
  		DWORD rfu2			:1;
  		DWORD timer1		:1;
  		DWORD timer0		:1;
  		DWORD int3			:1;
  		DWORD int2			:1;
  		DWORD int1			:1;
  		DWORD int0			:1;
  		DWORD i2o_of_ne		:1;
  		DWORD i2o_op_nf		:1;
  		DWORD i2o_ip_ne		:1;
  		DWORD i2o_if_nf		:1;
  		DWORD i2o_op_ne		:1;
  		DWORD rfu1			:1;
  		DWORD pci_mbi		:1;
  		DWORD lb_mbi		:1;
	} _bit;	
} INT_CFGX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD di1	 		:1;
  		DWORD di0	 		:1;
  		DWORD mode			:1;
  		DWORD sdram_di		:1;
  		DWORD pci_rd		:1;
  		DWORD pci_wr		:1;
  		DWORD pmaster_pi	:1;
  		DWORD pslave_pi		:1;
  		DWORD bwi			:1;
  		DWORD wdi			:1;
  		DWORD hbi			:1;
  		DWORD pwr_state		:1;
  		DWORD rfu3	 		:2;
  		DWORD dma1	 		:1;
  		DWORD dma0	 		:1;
  		DWORD hsenum		:1;
  		DWORD rfu2			:1;
  		DWORD timer1		:1;
  		DWORD timer0		:1;
  		DWORD int3			:1;
  		DWORD int2			:1;
  		DWORD int1			:1;
  		DWORD int0			:1;
  		DWORD i2o_of_ne		:1;
  		DWORD i2o_op_nf		:1;
  		DWORD i2o_ip_ne		:1;
  		DWORD i2o_if_nf		:1;
  		DWORD i2o_op_ne		:1;
  		DWORD rfu1			:1;
  		DWORD pci_mbi		:1;
  		DWORD lb_mbi		:1;
	} _bit;	
} INT_STAT;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu	 		:8;
  		DWORD ic11			:2;
  		DWORD ic10			:2;
  		DWORD ic9			:2;
  		DWORD ic8			:2;
  		DWORD ic7			:2;
  		DWORD ic6			:2;
  		DWORD ic5			:2;
  		DWORD ic4			:2;
  		DWORD ic3			:2;
  		DWORD ic2			:2;
  		DWORD ic1			:2;
  		DWORD ic0			:2;
	} _bit;	
} IOS;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu3			:8;
  		DWORD wd_en			:1;
  		DWORD rfu2			:1;
  		DWORD prescale		:1;
  		DWORD wd_init		:1;
  		DWORD test			:1;
  		DWORD rfu1			:1;
  		DWORD hb_tc			:10;
  		DWORD wd_tc			:8;
	} _bit;	
} WD_HBI;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD rfu			:16;
		DWORD rd_stat7		:1;
		DWORD rd_stat6		:1;
		DWORD rd_stat5		:1;
		DWORD rd_stat4		:1;
		DWORD rd_stat3		:1;
		DWORD rd_stat2		:1;
		DWORD rd_stat1		:1; 
		DWORD rd_stat0		:1;
		DWORD wr_stat7		:1;
		DWORD wr_stat6		:1;
		DWORD wr_stat5		:1;
		DWORD wr_stat4		:1;
		DWORD wr_stat3		:1;
		DWORD wr_stat2		:1;
		DWORD wr_stat1		:1;
		DWORD wr_stat0		:1;
	} _bit;
} MAIL_WR_STAT;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD lb_rd_en7			:1;
		DWORD lb_rd_en6			:1;
		DWORD lb_rd_en5			:1;
		DWORD lb_rd_en4			:1;
		DWORD lb_rd_en3			:1;
		DWORD lb_rd_en2			:1;
		DWORD lb_rd_en1			:1;
		DWORD lb_rd_en0			:1;
		DWORD lb_wr_en7			:1;
		DWORD lb_wr_en6			:1;
		DWORD lb_wr_en5			:1;
		DWORD lb_wr_en4			:1;
		DWORD lb_wr_en3			:1;
		DWORD lb_wr_en2			:1;
		DWORD lb_wr_en1			:1;
		DWORD lb_wr_en0			:1;
		DWORD pci_rd_en7		:1;
		DWORD pci_rd_en6		:1;
		DWORD pci_rd_en5		:1;
		DWORD pci_rd_en4		:1;
		DWORD pci_rd_en3		:1;
		DWORD pci_rd_en2		:1;
		DWORD pci_rd_en1		:1;
		DWORD pci_rd_en0		:1;
		DWORD pci_wr_en7		:1;
		DWORD pci_wr_en6		:1;
		DWORD pci_wr_en5		:1;
		DWORD pci_wr_en4		:1;
		DWORD pci_wr_en3		:1;
		DWORD pci_wr_en2		:1;
		DWORD pci_wr_en1		:1;
		DWORD pci_wr_en0		:1;
	} _bit;
} PCI_MAIL_IEWR;

typedef union
{
	DWORD _dword;
	struct
	{
		DWORD rfu3			:8;
		DWORD rfu2			:8;
		DWORD rfu1			:8;
  		DWORD data0			:8;
	} _bit;	
	DWORD _ifl;
	DWORD _ipl;
} MAIL_DATAX;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD data			:32;
	} _bit;	
} TIMER_DATAX;

typedef union
{
	USHORT _word;
	struct
	{
  		DWORD enable 		:1;
  		DWORD rfu2	 		:2;
  		DWORD dltch	 		:2;
  		DWORD to_mode 		:3;
  		DWORD rfu1	 		:2;
  		DWORD trg			:2;
  		DWORD cnt_en		:2;
  		DWORD ti_mode		:2;
	} _bit;
} TIMER_CTLX;

typedef union
{
	DWORD _dword;
	struct {
		TIMER_CTLX	timer1;
		TIMER_CTLX	timer0;
	} _struct;
} TIMER_CTL;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu2	 		:28;
  		DWORD out_post		:1;
  		DWORD rfu1			:3;
	} _bit;	
} PCI_I2O_ISTAT;

typedef union
{
	DWORD _dword;
	struct
	{
  		DWORD rfu2	 		:28;
  		DWORD out_post		:1;
  		DWORD rfu1			:3;
	} _bit;	
} PCI_I2O_IMASK;

typedef union
{
	DWORD data;
	struct
	{
		DWORD rfu1			:24;
  		DWORD delay			:8;
	} bits;	
} DMA_DELAY;

typedef union
{
	DWORD data;
	struct
	{
		DWORD rfu5	 		:8;
		DWORD cmd1	 		:3;
		DWORD rfu4	 		:1;
		DWORD cmd0	 		:3;
		DWORD rfu3	 		:1;
		DWORD priority 		:1;
		DWORD rfu2	 		:2;
		DWORD dpe_en 		:1;
		DWORD dci_en 		:1;
		DWORD done_en 		:1;
		DWORD rfu1	 		:5;
		DWORD dpe	 		:1;
  		DWORD dci	 		:1;
  		DWORD done	 		:1;
  		DWORD halt			:1;
  		DWORD ipr			:1;
	} bits;	
} DMA_CSR;

typedef union
{
	DWORD data;
	struct
	{
		DWORD dreq_en 		:1;
		DWORD updt_cnt		:1;
		DWORD swap	 		:2;
		DWORD psrc_type 	:1;
		DWORD pdst_type		:1;
		DWORD src_bus 		:1;
  		DWORD dst_bus 		:1;
  		DWORD block_fill	:1;
  		DWORD dterm_en		:1;
		DWORD rfu			:1;
  		DWORD cnt			:21;
	} bits;	
} DMA_XFER_CTL;

typedef union
{
	DWORD data;
	struct
	{
		DWORD ctlb_addr		:28;
  		DWORD da_inc_dis	:1;
  		DWORD sa_inc_dis	:1;
  		DWORD rfu1			:1;
  		DWORD ctlb_bus		:1;
	} bits;	
} DMA_CTLB_ADR;

typedef union
{
	DWORD data;
	struct
	{
  		DWORD data			:32;
	} bits;	
} DMA_SRC_ADR;

typedef union
{
	DWORD data;
	struct
	{
  		DWORD data			:32;
	} bits;	
} DMA_DST_ADR;

#endif /* _V3USCREG_H_DATA_STRUCTURE_B_ */


#ifdef _V3USCREG_H_DATA_STRUCTURE_
/*
 * PCI Configuration registers I2O structures
 */
typedef struct 
{
	PCI_VENDOR		_pci_vendor;
	PCI_CMD			_pci_cmd;
	PCI_CC_REV		_pci_cc_rev;
	PCI_HDR_CFG		_pci_io_cfg;
	PCI_I2O_BASE	_pci_i2o_base;
	DWORD			_rfu0[1];
	PCI_REG_BASE	_pci_reg_base;
	PCI_PCU_BASE	_pci_pcu_base;
	DWORD 			_rfu1[2];	
	PCI_CIS			_pci_cis;
	PCI_SUB_VENDOR	_pci_sub_vendor;	
	PCI_ROM_BASE	_pci_rom_base;
	PCI_CAPABLITY	_pci_capablity;
	DWORD 			_rfu2[1];	
	PCI_BPARAM		_pci_bparam;
	PM_CAP_ID		_pm_cap_id;
	PM_CRS			_pm_crs;
	PM_PWR_CON		_pm_pwr_con;
	PM_PWR_DIS		_pm_pwr_dis;
	PCI_I2O_MAP		_pci_i2o_map;
	DWORD			_rfu3[2];	
	PCI_BUS_CFG		_pci_bus_cfg;		
	LB_PCI_BASEX	_lb_pci_base0;
	LB_PCI_BASEX	_lb_pci_base1;
	DWORD 			_rfu4[1];	
	LB_PCU_BASE		_lb_pcu_base;
	LB_REG_BASE		_lb_reg_base;  /* contains SYSTEM register */
	LB_ROM_BASE		_lb_rom_base;
	LB_SDRAM_BASE	_lb_sdram_base;
	LB_BUS_CFG		_lb_bus_cfg;
	HS_CAP_ID		_hs_cap_id;
	LB_PCI_CTL		_lb_pci_ctl;
	SDRAM_MA		_sdram_ma;
	SDRAM_CFG		_sdram_cfg;
	SDRAM_BANKX		_sdram_bank0;
	SDRAM_BANKX		_sdram_bank1;
	SDRAM_BANKX		_sdram_bank2;
	SDRAM_BANKX		_sdram_bank3;
	PCU_SUBX		_pcu_sub0;
	PCU_SUBX		_pcu_sub1;
	PCU_SUBX		_pcu_sub2;
	DWORD			_rfu5[1];
	PCU_TC_WRX		_pcu_tc_wr0;
	PCU_TC_WRX		_pcu_tc_wr1;
	PCU_TC_WRX		_pcu_tc_wr2;
	PCU_TC_WRX		_pcu_tc_wr3;
	PCU_TC_WRX		_pcu_tc_wr4;
	DWORD 			_rfu6[1];	
	PCU_TC_RDX		_pcu_tc_rd0;
	PCU_TC_RDX		_pcu_tc_rd1;
	PCU_TC_RDX		_pcu_tc_rd2;
	PCU_TC_RDX		_pcu_tc_rd3;
	PCU_TC_RDX		_pcu_tc_rd4;
	DWORD			_rfu7[1];
	INT_CFGX		_int_cfg0;
	INT_CFGX		_int_cfg1;
	INT_CFGX		_int_cfg2;
	INT_STAT		_int_stat;
	IOS				_ios;
	WD_HBI			_wd_hbi;
	MAIL_WR_STAT	_mail_wr_stat;
	PCI_MAIL_IEWR	_pci_mail_iewr;
	MAIL_DATAX		_mail_data0;
	MAIL_DATAX		_mail_data1;
	MAIL_DATAX		_mail_data2;
	MAIL_DATAX		_mail_data3;
	MAIL_DATAX		_mail_data4;
	MAIL_DATAX		_mail_data5;
	MAIL_DATAX		_mail_data6;
	MAIL_DATAX		_mail_data7;
	DWORD 			_rfu8[8];	
	TIMER_DATAX		_timer_data0;
	TIMER_DATAX		_timer_data1;
	DWORD 			_rfu9[2];	
	TIMER_CTL		_timer_ctl;
	DWORD 			_rfu10[1];	
	INT_CFGX		_int_cfg3;
	DWORD 			_rfu11[1];	
	PCI_I2O_ISTAT	_pci_i2o_istat;
	PCI_I2O_IMASK	_pci_i2o_imask;
	DWORD 			_rfu12[1];	
	DMA_DELAY		_dma_delay;
	DMA_CSR			_dma_csr0;
	DMA_CSR			_dma_csr1;
	DWORD 			_rfu13[2];
	DMA_XFER_CTL	_dma_xfer_ctl0;
	DMA_SRC_ADR		_dma_src_adr0;
	DMA_DST_ADR		_dma_dst_adr0;
	DMA_CTLB_ADR	_dma_ctlb_adr0;
	DMA_XFER_CTL	_dma_xfer_ctl1;
	DMA_SRC_ADR		_dma_src_adr1;
	DMA_DST_ADR		_dma_dst_adr1;
	DMA_CTLB_ADR	_dma_ctlb_adr1;

} USC_REG;

#endif /* _V3USCREG_H_DATA_STRUCTURE_ */

#ifdef __cplusplus
}
#endif

#endif /* _V3USCREG_H_ */
