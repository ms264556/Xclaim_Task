/*
 * drivers/net/gianfar.h
 *
 * Gianfar Ethernet Driver
 * Driver for FEC on MPC8540 and TSEC on MPC8540/MPC8560
 * Based on 8260_io/fcc_enet.c
 *
 * Author: Andy Fleming
 * Maintainer: Kumar Gala
 * Modifier: Sandeep Gopalpet <sandeep.kumar@freescale.com>
 *
 * Copyright 2002-2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 *  Still left to do:
 *      -Add support for module parameters
 *	-Add patch for ethtool phys id
 */
#ifndef __GIANFAR_H
#define __GIANFAR_H

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/mii.h>
#include <linux/phy.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/crc32.h>
#include <linux/workqueue.h>
#include <linux/ethtool.h>

#ifdef CONFIG_GIANFAR_L2SRAM
#include <asm/fsl_85xx_cache_sram.h>
#define ALIGNMENT 0x20
#endif

/* The maximum number of packets to be handled in one call of gfar_poll */
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
#define GFAR_DEV_WEIGHT 16
#else
#define GFAR_DEV_WEIGHT 64
#endif

/* Length for FCB */
#define GMAC_FCB_LEN 8

/* Default padding amount */
#define DEFAULT_PADDING 2

/* Number of bytes to align the rx bufs to */
#define RXBUF_ALIGNMENT 64

/* The number of bytes which composes a unit for the purpose of
 * allocating data buffers.  ie-for any given MTU, the data buffer
 * will be the next highest multiple of 512 bytes. */
#define INCREMENTAL_BUFFER_SIZE 512


#define MAC_ADDR_LEN 6

#define PHY_INIT_TIMEOUT 100000
#define GFAR_PHY_CHANGE_TIME 2

#define DEVICE_NAME "%s: Gianfar Ethernet Controller Version 1.4-skbr1.1.5, "
#define DRV_NAME "gfar-enet"
extern const char gfar_driver_name[];
extern const char gfar_driver_version[];

/* MAXIMUM NUMBER OF QUEUES SUPPORTED */
#define MAX_TX_QS	0x8
#define MAX_RX_QS	0x8

/* MAXIMUM NUMBER OF GROUPS SUPPORTED */
#define MAXGROUPS 0x2

/* These need to be powers of 2 for this driver */
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
#define DEFAULT_TX_RING_SIZE	128
#define DEFAULT_RX_RING_SIZE	128
#else
#define DEFAULT_TX_RING_SIZE	256
#define DEFAULT_RX_RING_SIZE	256
#endif

#define DEFAULT_WK_RING_SIZE	16

#define GFAR_TX_MAX_RING_SIZE   65536
#define GFAR_MIN_RING_SIZE	4

#define GFAR_MAX_FIFO_THRESHOLD 511
#define GFAR_MAX_FIFO_STARVE	511
#define GFAR_MAX_FIFO_STARVE_OFF 511

#define DEFAULT_RX_BUFFER_SIZE  1536
#define DEFAULT_WK_BUFFER_SIZE	2048
#define TX_RING_MOD_MASK(size) (size-1)
#define RX_RING_MOD_MASK(size) (size-1)
#define JUMBO_BUFFER_SIZE 9728
#define JUMBO_FRAME_SIZE 9600

#define DEFAULT_FIFO_TX_THR 0x100
#define DEFAULT_FIFO_TX_STARVE 0x40
#define DEFAULT_FIFO_TX_STARVE_OFF 0x80
#define DEFAULT_BD_STASH 1
#define DEFAULT_STASH_LENGTH	96
#define DEFAULT_STASH_INDEX	0

#define PTP_GET_RX_TIMESTAMP_SYNC	SIOCDEVPRIVATE
#define PTP_GET_RX_TIMESTAMP_DEL_REQ	(SIOCDEVPRIVATE + 1)
#define PTP_GET_RX_TIMESTAMP_FOLLOWUP	(SIOCDEVPRIVATE + 2)
#define PTP_GET_RX_TIMESTAMP_DEL_RESP	(SIOCDEVPRIVATE + 3)
#define PTP_GET_TX_TIMESTAMP		(SIOCDEVPRIVATE + 4)
#define PTP_SET_CNT			(SIOCDEVPRIVATE + 5)
#define PTP_GET_CNT			(SIOCDEVPRIVATE + 6)
#define PTP_SET_FIPER_ALARM		(SIOCDEVPRIVATE + 7)
#define PTP_ADJ_ADDEND			(SIOCDEVPRIVATE + 9)
#define PTP_GET_ADDEND			(SIOCDEVPRIVATE + 10)
#define PTP_GET_RX_TIMESTAMP_PDELAY_REQ	(SIOCDEVPRIVATE + 11)
#define PTP_GET_RX_TIMESTAMP_PDELAY_RESP	(SIOCDEVPRIVATE + 12)
#define PTP_CLEANUP_TIMESTAMP_BUFFERS	(SIOCDEVPRIVATE + 13)
#define DEFAULT_PTP_RX_BUF_SZ		2000
#define GFAR_PTP_CTRL_SYNC		0x0
#define GFAR_PTP_CTRL_DEL_REQ		0x1
#define GFAR_PTP_CTRL_FOLLOWUP		0x2
#define GFAR_PTP_CTRL_DEL_RESP		0x3
#define GFAR_PTP_CTRL_ALL_OTHER		0x5
#define GFAR_PTP_MSG_TYPE_PDREQ		0x02
#define GFAR_PTP_MSG_TYPE_PDRESP	0x03
#define GFAR_PTP_DOMAIN_DLFT		0xe0000181
#define GFAR_PTP_PKT_TYPE_OFFS		0x1f
#define GFAR_PTP_PROTOCOL_OFFS		0x20
#define GFAR_PTP_MULTI_ADDR_OFFS	0x26
#define GFAR_PTP_PORT_OFFS		0x2A
#define GFAR_PTP_MSG_TYPE_OFFS		0x32
#define GFAR_PTP_SEQ_ID_OFFS		0x50
#define GFAR_PTP_CTRL_OFFS		0x52
#define GFAR_PACKET_TYPE_UDP		0x11

/* The number of Exact Match registers */
#define GFAR_EM_NUM	15

/* Latency of interface clock in nanoseconds */
/* Interface clock latency , in this case, means the
 * time described by a value of 1 in the interrupt
 * coalescing registers' time fields.  Since those fields
 * refer to the time it takes for 64 clocks to pass, the
 * latencies are as such:
 * GBIT = 125MHz => 8ns/clock => 8*64 ns / tick
 * 100 = 25 MHz => 40ns/clock => 40*64 ns / tick
 * 10 = 2.5 MHz => 400ns/clock => 400*64 ns / tick
 */
#define GFAR_GBIT_TIME  512
#define GFAR_100_TIME   2560
#define GFAR_10_TIME    25600

#define DEFAULT_TX_COALESCE 1
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
#define DEFAULT_TXCOUNT	22
#define DEFAULT_TXTIME	64
#else
#define DEFAULT_TXCOUNT	16
#define DEFAULT_TXTIME	21
#endif

#define DEFAULT_RXTIME	21

#define DEFAULT_RX_COALESCE 0
#define DEFAULT_RXCOUNT	0

#define GFAR_SUPPORTED (SUPPORTED_10baseT_Half \
		| SUPPORTED_10baseT_Full \
		| SUPPORTED_100baseT_Half \
		| SUPPORTED_100baseT_Full \
		| SUPPORTED_Autoneg \
		| SUPPORTED_MII)

/* TBI register addresses */
#define MII_TBICON		0x11

/* TBICON register bit fields */
#define TBICON_CLK_SELECT	0x0020

/* MAC register bits */
#define MACCFG1_SOFT_RESET	0x80000000
#define MACCFG1_RESET_RX_MC	0x00080000
#define MACCFG1_RESET_TX_MC	0x00040000
#define MACCFG1_RESET_RX_FUN	0x00020000
#define	MACCFG1_RESET_TX_FUN	0x00010000
#define MACCFG1_LOOPBACK	0x00000100
#define MACCFG1_RX_FLOW		0x00000020
#define MACCFG1_TX_FLOW		0x00000010
#define MACCFG1_SYNCD_RX_EN	0x00000008
#define MACCFG1_RX_EN		0x00000004
#define MACCFG1_SYNCD_TX_EN	0x00000002
#define MACCFG1_TX_EN		0x00000001

#define MACCFG2_INIT_SETTINGS	0x00007205
#define MACCFG2_FULL_DUPLEX	0x00000001
#define MACCFG2_IF              0x00000300
#define MACCFG2_MII             0x00000100
#define MACCFG2_GMII            0x00000200
#define MACCFG2_HUGEFRAME	0x00000020
#define MACCFG2_LENGTHCHECK	0x00000010
#define MACCFG2_MPEN		0x00000008

#define ECNTRL_INIT_SETTINGS	0x00001000
#define ECNTRL_TBI_MODE         0x00000020
#define ECNTRL_REDUCED_MODE	0x00000010
#define ECNTRL_R100		0x00000008
#define ECNTRL_REDUCED_MII_MODE	0x00000004
#define ECNTRL_SGMII_MODE	0x00000002

#define MRBLR_INIT_SETTINGS	DEFAULT_RX_BUFFER_SIZE

#define MINFLR_INIT_SETTINGS	0x00000040

/* Tqueue control */
#define TQUEUE_EN0		0x00008000
#define TQUEUE_EN1		0x00004000
#define TQUEUE_EN2		0x00002000
#define TQUEUE_EN3		0x00001000
#define TQUEUE_EN4		0x00000800
#define TQUEUE_EN5		0x00000400
#define TQUEUE_EN6		0x00000200
#define TQUEUE_EN7		0x00000100
#define TQUEUE_EN_ALL		0x0000FF00

#define TR03WT_WT0_MASK		0xFF000000
#define TR03WT_WT1_MASK		0x00FF0000
#define TR03WT_WT2_MASK		0x0000FF00
#define TR03WT_WT3_MASK		0x000000FF

#define TR47WT_WT4_MASK		0xFF000000
#define TR47WT_WT5_MASK		0x00FF0000
#define TR47WT_WT6_MASK		0x0000FF00
#define TR47WT_WT7_MASK		0x000000FF

#define WRRS_TR03WT		0xFFFFFFFF
#define WRRS_TR47WT		0xFFFFFFFF

/* Rqueue control */
#define RQUEUE_EX0		0x00800000
#define RQUEUE_EX1		0x00400000
#define RQUEUE_EX2		0x00200000
#define RQUEUE_EX3		0x00100000
#define RQUEUE_EX4		0x00080000
#define RQUEUE_EX5		0x00040000
#define RQUEUE_EX6		0x00020000
#define RQUEUE_EX7		0x00010000
#define RQUEUE_EX_ALL		0x00FF0000

#define RQUEUE_EN0		0x00000080
#define RQUEUE_EN1		0x00000040
#define RQUEUE_EN2		0x00000020
#define RQUEUE_EN3		0x00000010
#define RQUEUE_EN4		0x00000008
#define RQUEUE_EN5		0x00000004
#define RQUEUE_EN6		0x00000002
#define RQUEUE_EN7		0x00000001
#define RQUEUE_EN_ALL		0x000000FF

/* Wake-On-Lan options */
#define GIANFAR_WOL_PHY		(1 << 0)
#define GIANFAR_WOL_UCAST	(1 << 1)
#define GIANFAR_WOL_MCAST	(1 << 2)
#define GIANFAR_WOL_BCAST	(1 << 3)
#define GIANFAR_WOL_ARP		(1 << 4)
#define GIANFAR_WOL_MAGIC	(1 << 5)

/* Init to do tx snooping for buffers and descriptors */
#define DMACTRL_INIT_SETTINGS   0x000000c3
#define DMACTRL_GRS             0x00000010
#define DMACTRL_GTS             0x00000008

#define TSTAT_TXF_MASK_ALL	0x0000FF00
#define TSTAT_TXF0_MASK		0x00008000

#define TSTAT_CLEAR_THALT_ALL	0xFF000000
#define TSTAT_CLEAR_THALT	0x80000000
#define TSTAT_CLEAR_THALT0	0x80000000
#define TSTAT_CLEAR_THALT1	0x40000000
#define TSTAT_CLEAR_THALT2	0x20000000
#define TSTAT_CLEAR_THALT3	0x10000000
#define TSTAT_CLEAR_THALT4	0x08000000
#define TSTAT_CLEAR_THALT5	0x04000000
#define TSTAT_CLEAR_THALT6	0x02000000
#define TSTAT_CLEAR_THALT7	0x01000000

/* Interrupt coalescing macros */
#define IC_ICEN			0x80000000
#define IC_ICFT_MASK		0x1fe00000
#define IC_ICFT_SHIFT		21
#define mk_ic_icft(x)		\
	(((unsigned int)x << IC_ICFT_SHIFT)&IC_ICFT_MASK)
#define IC_ICTT_MASK		0x0000ffff
#define mk_ic_ictt(x)		(x&IC_ICTT_MASK)

#define mk_ic_value(count, time) (IC_ICEN | \
				mk_ic_icft(count) | \
				mk_ic_ictt(time))
#define get_icft_value(ic)	(((unsigned long)ic & IC_ICFT_MASK) >> \
				 IC_ICFT_SHIFT)
#define get_ictt_value(ic)	((unsigned long)ic & IC_ICTT_MASK)

#define DEFAULT_TXIC mk_ic_value(DEFAULT_TXCOUNT, DEFAULT_TXTIME)
#define DEFAULT_RXIC mk_ic_value(DEFAULT_RXCOUNT, DEFAULT_RXTIME)

#define skip_bd(bdp, stride, base, ring_size) ({ \
	typeof(bdp) new_bd = (bdp) + (stride); \
	(new_bd >= (base) + (ring_size)) ? (new_bd - (ring_size)) : new_bd; })

#define next_bd(bdp, base, ring_size) skip_bd(bdp, 1, base, ring_size)

#define RCTRL_TS_ENABLE		0x01000000
#define RCTRL_PADB_SIZE		(0x8 << 16)
#define RCTRL_PAL_MASK		0x001f0000
#define RCTRL_VLEX		0x00002000
#define RCTRL_FILREN		0x00001000
#define RCTRL_FSQEN		0x00000800
#define RCTRL_GHTX		0x00000400
#define RCTRL_IPCSEN		0x00000200
#define RCTRL_TUCSEN		0x00000100
#define RCTRL_PRSDEP_MASK	0x000000c0
#define RCTRL_PRSDEP_INIT	0x000000c0
#define RCTRL_PRSDEP_L2 	0x00000040
#define RCTRL_PRSDEP_L2L3	0x00000080
#define RCTRL_PRSDEP_L2L3L4	0x000000c0
#define RCTRL_PROM		0x00000008
#define RCTRL_EMEN		0x00000002
#define RCTRL_REQ_PARSER	(RCTRL_VLEX | RCTRL_IPCSEN | \
				 RCTRL_TUCSEN)
#define RCTRL_CHECKSUMMING	(RCTRL_IPCSEN | RCTRL_TUCSEN | \
				RCTRL_PRSDEP_INIT)
#define RCTRL_EXTHASH		(RCTRL_GHTX)
#define RCTRL_VLAN		(RCTRL_PRSDEP_INIT)
#define RCTRL_PADDING(x)	((x << 16) & RCTRL_PAL_MASK)


#define RSTAT_CLEAR_RHALT       0x00800000
#define RSTAT_RXF_ALL_MASK	0x000000FF
#define RSTAT_RXF0_MASK		0x00000080

#define TCTRL_IPCSEN		0x00004000
#define TCTRL_TUCSEN		0x00002000
#define TCTRL_VLINS		0x00001000
#define TCTRL_THDF		0x00000800
#define TCTRL_RFCPAUSE		0x00000010
#define TCTRL_TFCPAUSE		0x00000008
#define TCTRL_TXSCHED_MASK	0x00000006
#define TCTRL_TXSCHED_INIT	0x00000000
#define TCTRL_TXSCHED_PRIO	0x00000002
#define TCTRL_TXSCHED_WRRS	0x00000004
#define TCTRL_INIT_CSUM		(TCTRL_TUCSEN | TCTRL_IPCSEN)

#define IEVENT_INIT_CLEAR	0xffffffff
#define IEVENT_BABR		0x80000000
#define IEVENT_RXC		0x40000000
#define IEVENT_BSY		0x20000000
#define IEVENT_EBERR		0x10000000
#define IEVENT_MSRO		0x04000000
#define IEVENT_GTSC		0x02000000
#define IEVENT_BABT		0x01000000
#define IEVENT_TXC		0x00800000
#define IEVENT_TXE		0x00400000
#define IEVENT_TXB		0x00200000
#define IEVENT_TXF		0x00100000
#define IEVENT_LC		0x00040000
#define IEVENT_CRL		0x00020000
#define IEVENT_XFUN		0x00010000
#define IEVENT_RXB0		0x00008000
#define IEVENT_MAG		0x00000800
#define IEVENT_GRSC		0x00000100
#define IEVENT_RXF0		0x00000080
#define IEVENT_FGPI		0x00000010
#define IEVENT_FIR		0x00000008
#define IEVENT_FIQ		0x00000004
#define IEVENT_DPE		0x00000002
#define IEVENT_PERR		0x00000001
#define IEVENT_RX_MASK          (IEVENT_RXB0 | IEVENT_RXF0 | \
					IEVENT_FGPI | IEVENT_BSY)
#define IEVENT_TX_MASK          (IEVENT_TXB | IEVENT_TXF)
#define IEVENT_RTX_MASK         (IEVENT_RX_MASK | IEVENT_TX_MASK)
#define IEVENT_ERR_MASK         \
(IEVENT_RXC | IEVENT_BSY | IEVENT_EBERR | IEVENT_MSRO | \
 IEVENT_BABT | IEVENT_TXC | IEVENT_TXE | IEVENT_LC \
 | IEVENT_CRL | IEVENT_XFUN | IEVENT_FIR | IEVENT_FIQ \
 | IEVENT_DPE | IEVENT_PERR | IEVENT_MAG | IEVENT_BABR)

#define IMASK_INIT_CLEAR	0x00000000
#define IMASK_BABR              0x80000000
#define IMASK_RXC               0x40000000
#define IMASK_BSY               0x20000000
#define IMASK_EBERR             0x10000000
#define IMASK_MSRO		0x04000000
#define IMASK_GRSC              0x02000000
#define IMASK_BABT		0x01000000
#define IMASK_TXC               0x00800000
#define IMASK_TXEEN		0x00400000
#define IMASK_TXBEN		0x00200000
#define IMASK_TXFEN             0x00100000
#define IMASK_LC		0x00040000
#define IMASK_CRL		0x00020000
#define IMASK_XFUN		0x00010000
#define IMASK_RXB0              0x00008000
#define IMASK_MAG		0x00000800
#define IMASK_GTSC              0x00000100
#define IMASK_RXFEN0		0x00000080
#define IMASK_FGPI		0x00000010
#define IMASK_FIR		0x00000008
#define IMASK_FIQ		0x00000004
#define IMASK_DPE		0x00000002
#define IMASK_PERR		0x00000001
#define IMASK_DEFAULT  (IMASK_TXEEN | IMASK_TXFEN | IMASK_TXBEN | \
		IMASK_RXFEN0 | IMASK_BSY | IMASK_EBERR | IMASK_BABR | \
		IMASK_XFUN | IMASK_RXC | IMASK_BABT | IMASK_FGPI | \
		IMASK_FIR | IMASK_FIQ | IMASK_DPE | IMASK_PERR | IMASK_RXB0)
#define IMASK_RTX_DISABLED ((~(IMASK_RXFEN0 | IMASK_RXB0 | IMASK_BSY | \
				IMASK_TXFEN)) & IMASK_DEFAULT)

#ifdef CONFIG_GIANFAR_TXNAPI
#define IMASK_DEFAULT_TX	(IMASK_TXFEN | IMASK_TXBEN)
#define IMASK_DEFAULT_RX	(IMASK_RXFEN0 | IMASK_RXB0 | IMASK_BSY)
#define IMASK_RX_DISABLED	((~IMASK_DEFAULT_RX) \
				& IMASK_DEFAULT)
#define IMASK_TX_DISABLED	((~IMASK_DEFAULT_TX) \
				& IMASK_DEFAULT)
#else
#define IMASK_RX_DISABLED (~(IMASK_RXFEN0 | IMASK_BSY))
#endif
/* Fifo management */
#define FIFO_TX_THR_MASK	0x01ff
#define FIFO_TX_STARVE_MASK	0x01ff
#define FIFO_TX_STARVE_OFF_MASK	0x01ff

/* Attribute fields */

/* This enables rx snooping for buffers and descriptors */
#define ATTR_BDSTASH		0x00000800

#define ATTR_BUFSTASH		0x00004000

#define ATTR_SNOOPING		0x000000c0
#define ATTR_INIT_SETTINGS      ATTR_SNOOPING

#define ATTRELI_INIT_SETTINGS   0x0
#define ATTRELI_EL_MASK		0x3fff0000
#define ATTRELI_EL(x) (x << 16)
#define ATTRELI_EI_MASK		0x00003fff
#define ATTRELI_EI(x) (x)

#define BD_LFLAG(flags) ((flags) << 16)
#define BD_LENGTH_MASK		0x0000ffff

#define CLASS_CODE_UNRECOG		0x00
#define CLASS_CODE_DUMMY1		0x01
#define CLASS_CODE_ETHERTYPE1		0x02
#define CLASS_CODE_ETHERTYPE2		0x03
#define CLASS_CODE_USER_PROG1		0x04
#define CLASS_CODE_USER_PROG2		0x05
#define CLASS_CODE_USER_PROG3		0x06
#define CLASS_CODE_USER_PROG4		0x07
#define CLASS_CODE_TCP_IPV4		0x08
#define CLASS_CODE_UDP_IPV4		0x09
#define CLASS_CODE_AH_ESP_IPV4		0x0a
#define CLASS_CODE_SCTP_IPV4		0x0b
#define CLASS_CODE_TCP_IPV6		0x0c
#define CLASS_CODE_UDP_IPV6		0x0d
#define CLASS_CODE_AH_ESP_IPV6		0x0e
#define CLASS_CODE_SCTP_IPV6		0x0f

#define FPR_FILER_MASK	0xFFFFFFFF
#define MAX_FILER_IDX	0xFF
/* This default RIR value directly corresponds
 * to the 3-bit hash value generated */
#define DEFAULT_RIR0	0x05397700

/* RQFCR register bits */
#define RQFCR_GPI		0x80000000
#define RQFCR_HASHTBL_Q		0x00000000
#define RQFCR_HASHTBL_0		0x00020000
#define RQFCR_HASHTBL_1		0x00040000
#define RQFCR_HASHTBL_2		0x00060000
#define RQFCR_HASHTBL_3		0x00080000
#define RQFCR_HASH		0x00010000
#define RQFCR_CLE		0x00000200
#define RQFCR_RJE		0x00000100
#define RQFCR_AND		0x00000080
#define RQFCR_CMP_EXACT		0x00000000
#define RQFCR_CMP_MATCH		0x00000020
#define RQFCR_CMP_NOEXACT	0x00000040
#define RQFCR_CMP_NOMATCH	0x00000060

/* RQFCR PID values */
#define	RQFCR_PID_MASK		0x00000000
#define	RQFCR_PID_PARSE		0x00000001
#define	RQFCR_PID_ARB		0x00000002
#define	RQFCR_PID_DAH		0x00000003
#define	RQFCR_PID_DAL		0x00000004
#define	RQFCR_PID_SAH		0x00000005
#define	RQFCR_PID_SAL		0x00000006
#define	RQFCR_PID_ETY		0x00000007
#define	RQFCR_PID_VID		0x00000008
#define	RQFCR_PID_PRI		0x00000009
#define	RQFCR_PID_TOS		0x0000000A
#define	RQFCR_PID_L4P		0x0000000B
#define	RQFCR_PID_DIA		0x0000000C
#define	RQFCR_PID_SIA		0x0000000D
#define	RQFCR_PID_DPT		0x0000000E
#define	RQFCR_PID_SPT		0x0000000F

/* RQFPR when PID is 0x0001 */
#define RQFPR_HDR_GE_512	0x00200000
#define RQFPR_LERR		0x00100000
#define RQFPR_RAR		0x00080000
#define RQFPR_RARQ		0x00040000
#define RQFPR_AR		0x00020000
#define RQFPR_ARQ		0x00010000
#define RQFPR_EBC		0x00008000
#define RQFPR_VLN		0x00004000
#define RQFPR_CFI		0x00002000
#define RQFPR_JUM		0x00001000
#define RQFPR_IPF		0x00000800
#define RQFPR_FIF		0x00000400
#define RQFPR_IPV4		0x00000200
#define RQFPR_IPV6		0x00000100
#define RQFPR_ICC		0x00000080
#define RQFPR_ICV		0x00000040
#define RQFPR_TCP		0x00000020
#define RQFPR_UDP		0x00000010
#define RQFPR_TUC		0x00000008
#define RQFPR_TUV		0x00000004
#define RQFPR_PER		0x00000002
#define RQFPR_EER		0x00000001

/* TxBD status field bits */
#define TXBD_READY		0x8000
#define TXBD_PADCRC		0x4000
#define TXBD_WRAP		0x2000
#define TXBD_INTERRUPT		0x1000
#define TXBD_LAST		0x0800
#define TXBD_CRC		0x0400
#define TXBD_DEF		0x0200
#define TXBD_HUGEFRAME		0x0080
#define TXBD_LATECOLLISION	0x0080
#define TXBD_RETRYLIMIT		0x0040
#define	TXBD_RETRYCOUNTMASK	0x003c
#define TXBD_UNDERRUN		0x0002
#define TXBD_TOE		0x0002

/* Tx FCB param bits */
#define TXFCB_VLN		0x80
#define TXFCB_IP		0x40
#define TXFCB_IP6		0x20
#define TXFCB_TUP		0x10
#define TXFCB_UDP		0x08
#define TXFCB_CIP		0x04
#define TXFCB_CTU		0x02
#define TXFCB_NPH		0x01
#define TXFCB_DEFAULT 		(TXFCB_IP|TXFCB_TUP|TXFCB_CTU|TXFCB_NPH)

/* RxBD status field bits */
#define RXBD_EMPTY		0x8000
#define RXBD_RO1		0x4000
#define RXBD_WRAP		0x2000
#define RXBD_INTERRUPT		0x1000
#define RXBD_LAST		0x0800
#define RXBD_FIRST		0x0400
#define RXBD_MISS		0x0100
#define RXBD_BROADCAST		0x0080
#define RXBD_MULTICAST		0x0040
#define RXBD_LARGE		0x0020
#define RXBD_NONOCTET		0x0010
#define RXBD_SHORT		0x0008
#define RXBD_CRCERR		0x0004
#define RXBD_OVERRUN		0x0002
#define RXBD_TRUNCATED		0x0001
#define RXBD_STATS		0x01ff
#define RXBD_CLEAN              0x3000
#define RXBD_ERR		(RXBD_LARGE | RXBD_SHORT | RXBD_NONOCTET 	\
				| RXBD_CRCERR | RXBD_OVERRUN			\
				| RXBD_TRUNCATED)

/* Rx FCB status field bits */
#define RXFCB_VLN		0x8000
#define RXFCB_IP		0x4000
#define RXFCB_IP6		0x2000
#define RXFCB_TUP		0x1000
#define RXFCB_CIP		0x0800
#define RXFCB_CTU		0x0400
#define RXFCB_EIP		0x0200
#define RXFCB_ETU		0x0100
#define RXFCB_CSUM_MASK		0x0f00
#define RXFCB_PERR_MASK		0x000c
#define RXFCB_PERR_BADL3	0x0008

/* 1588 Module Registers bits */
#define TMR_CTRL_ENABLE		0x00000004
#define TMR_CTRL_RTC_CLK	0x00000003
#define TMR_CTRL_EXT_CLK	0x00000000
#define TMR_CTRL_SYS_CLK	0x00000001
#define TMR_CTRL_TCLK_PRD	0x000A0000
#define TMR_OSC_FREQ		250000000
#define TMR_ADD_VAL		CONFIG_GFAR_PTP_TMR_ADD
#define TMR_CTRL_TCLK_MASK	0x03ff0000
#define TMR_PTPD_MAX_FREQ	0x80000
#define TMR_CTRL_FIPER_START	0x10000000
#define TMR_FIPER1		1000000000
/*Alarm to traigger at 15sec boundary */
#define TMR_ALARM1_L	0xD964B800
#define TMR_ALARM1_H	0x00000045
#define TMR_PRSC	0x2
#define TMR_SEC		1000000000

#define GFAR_INT_NAME_MAX	IFNAMSIZ + 4

#define GFAR_TX_BASE_H	0xf
#define GFAR_RX_BASE_H	0xf

struct txbd8
{
	union {
		struct {
			u16	status;	/* Status Fields */
			u16	length;	/* Buffer length */
		};
		u32 lstatus;
	};
	u32	bufPtr;	/* Buffer Pointer */
};

struct txfcb {
	u8	flags;
	u8	ptp;	/* Least significant bit for enabling Tx Timestamping */
	u8	l4os;	/* Level 4 Header Offset */
	u8	l3os; 	/* Level 3 Header Offset */
	u16	phcs;	/* Pseudo-header Checksum */
	u16	vlctl;	/* VLAN control word */
};

struct rxbd8
{
	union {
		struct {
			u16	status;	/* Status Fields */
			u16	length;	/* Buffer Length */
		};
		u32 lstatus;
	};
	u32	bufPtr;	/* Buffer Pointer */
};

struct rxfcb {
	u16	flags;
	u8	rq;	/* Receive Queue index */
	u8	pro;	/* Layer 4 Protocol */
	u16	reserved;
	u16	vlctl;	/* VLAN control word */
};

struct rmon_mib
{
	u32	tr64;	/* 0x.680 - Transmit and Receive 64-byte Frame Counter */
	u32	tr127;	/* 0x.684 - Transmit and Receive 65-127 byte Frame Counter */
	u32	tr255;	/* 0x.688 - Transmit and Receive 128-255 byte Frame Counter */
	u32	tr511;	/* 0x.68c - Transmit and Receive 256-511 byte Frame Counter */
	u32	tr1k;	/* 0x.690 - Transmit and Receive 512-1023 byte Frame Counter */
	u32	trmax;	/* 0x.694 - Transmit and Receive 1024-1518 byte Frame Counter */
	u32	trmgv;	/* 0x.698 - Transmit and Receive 1519-1522 byte Good VLAN Frame */
	u32	rbyt;	/* 0x.69c - Receive Byte Counter */
	u32	rpkt;	/* 0x.6a0 - Receive Packet Counter */
	u32	rfcs;	/* 0x.6a4 - Receive FCS Error Counter */
	u32	rmca;	/* 0x.6a8 - Receive Multicast Packet Counter */
	u32	rbca;	/* 0x.6ac - Receive Broadcast Packet Counter */
	u32	rxcf;	/* 0x.6b0 - Receive Control Frame Packet Counter */
	u32	rxpf;	/* 0x.6b4 - Receive Pause Frame Packet Counter */
	u32	rxuo;	/* 0x.6b8 - Receive Unknown OP Code Counter */
	u32	raln;	/* 0x.6bc - Receive Alignment Error Counter */
	u32	rflr;	/* 0x.6c0 - Receive Frame Length Error Counter */
	u32	rcde;	/* 0x.6c4 - Receive Code Error Counter */
	u32	rcse;	/* 0x.6c8 - Receive Carrier Sense Error Counter */
	u32	rund;	/* 0x.6cc - Receive Undersize Packet Counter */
	u32	rovr;	/* 0x.6d0 - Receive Oversize Packet Counter */
	u32	rfrg;	/* 0x.6d4 - Receive Fragments Counter */
	u32	rjbr;	/* 0x.6d8 - Receive Jabber Counter */
	u32	rdrp;	/* 0x.6dc - Receive Drop Counter */
	u32	tbyt;	/* 0x.6e0 - Transmit Byte Counter Counter */
	u32	tpkt;	/* 0x.6e4 - Transmit Packet Counter */
	u32	tmca;	/* 0x.6e8 - Transmit Multicast Packet Counter */
	u32	tbca;	/* 0x.6ec - Transmit Broadcast Packet Counter */
	u32	txpf;	/* 0x.6f0 - Transmit Pause Control Frame Counter */
	u32	tdfr;	/* 0x.6f4 - Transmit Deferral Packet Counter */
	u32	tedf;	/* 0x.6f8 - Transmit Excessive Deferral Packet Counter */
	u32	tscl;	/* 0x.6fc - Transmit Single Collision Packet Counter */
	u32	tmcl;	/* 0x.700 - Transmit Multiple Collision Packet Counter */
	u32	tlcl;	/* 0x.704 - Transmit Late Collision Packet Counter */
	u32	txcl;	/* 0x.708 - Transmit Excessive Collision Packet Counter */
	u32	tncl;	/* 0x.70c - Transmit Total Collision Counter */
	u8	res1[4];
	u32	tdrp;	/* 0x.714 - Transmit Drop Frame Counter */
	u32	tjbr;	/* 0x.718 - Transmit Jabber Frame Counter */
	u32	tfcs;	/* 0x.71c - Transmit FCS Error Counter */
	u32	txcf;	/* 0x.720 - Transmit Control Frame Counter */
	u32	tovr;	/* 0x.724 - Transmit Oversize Frame Counter */
	u32	tund;	/* 0x.728 - Transmit Undersize Frame Counter */
	u32	tfrg;	/* 0x.72c - Transmit Fragments Frame Counter */
	u32	car1;	/* 0x.730 - Carry Register One */
	u32	car2;	/* 0x.734 - Carry Register Two */
	u32	cam1;	/* 0x.738 - Carry Mask Register One */
	u32	cam2;	/* 0x.73c - Carry Mask Register Two */
};

struct gfar_extra_stats {
	u64 kernel_dropped;
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
	u64 rx_skbr;
	u64 rx_skbr_free;
#endif
#ifdef CONFIG_NET_GIANFAR_FP
	u64 rx_fast;
#endif
	u64 rx_large;
	u64 rx_short;
	u64 rx_nonoctet;
	u64 rx_crcerr;
	u64 rx_overrun;
	u64 rx_bsy;
	u64 rx_babr;
	u64 rx_trunc;
	u64 eberr;
	u64 tx_babt;
	u64 tx_underrun;
	u64 rx_skbmissing;
	u64 tx_timeout;
	u64 rx_jumbo;
	u64 rx_jumbo_len;
	u64 tx_jumbo;
	u64 tx_jumbo_len;
};

#define GFAR_RMON_LEN ((sizeof(struct rmon_mib) - 16)/sizeof(u32))
#define GFAR_EXTRA_STATS_LEN (sizeof(struct gfar_extra_stats)/sizeof(u64))

/* Number of stats in the stats structure (ignore car and cam regs)*/
#define GFAR_STATS_LEN (GFAR_RMON_LEN + GFAR_EXTRA_STATS_LEN)

#define GFAR_INFOSTR_LEN 32

struct gfar_stats {
	u64 extra[GFAR_EXTRA_STATS_LEN];
	u64 rmon[GFAR_RMON_LEN];
};

/* IEEE-1588 Timer Controller Registers */
struct gfar_regs_1588 {
	u32	tmr_ctrl;	/* 0x.e00 - Timer Control Register */
	u32	tmr_tevent;	/* 0x.e04 - Timer stamp event register */
	u32	tmr_temask;	/* 0x.e08 - Timer event mask register */
	u32	tmr_pevent;	/* 0x.e0c - Timer stamp event register */
	u32	tmr_pemask;	/* 0x.e10 - Timer event mask register */
	u32	tmr_stat;	/* 0x.e14 - Timer stamp status register */
	u32	tmr_cnt_h;	/* 0x.e18 - Timer counter high register */
	u32	tmr_cnt_l;	/* 0x.e1c - Timer counter low register */
	u32	tmr_add;	/* 0x.e20 - Timer dirft compensation*/
					/*addend register */
	u32	tmr_acc;	/* 0x.e24 - Timer accumulator register */
	u32	tmr_prsc;	/* 0x.e28 - Timer prescale register */
	u8	res24a[4];	/* 0x.e2c - 0x.e2f reserved */
	u32	tmr_off_h;	/* 0x.e30 - Timer offset high register */
	u32	tmr_off_l;	/* 0x.e34 - Timer offset low register */
	u8	res24b[8];	/* 0x.e38 - 0x.e3f reserved */
	u32	tmr_alarm1_h;	/* 0x.e40 - Timer alarm 1 high register */
	u32	tmr_alarm1_l;	/* 0x.e44 - Timer alarm 1 low register */
	u32	tmr_alarm2_h;	/* 0x.e48 - Timer alarm 2 high register */
	u32	tmr_alarm2_l;	/* 0x.e4c - Timer alarm 2 low register */
	u8	res24c[48];	/* 0x.e50 - 0x.e7f reserved */
	u32	tmr_fiper1;	/* 0x.e80 - Timer fixed period register 1 */
	u32	tmr_fiper2;	/* 0x.e84 - Timer fixed period register 2 */
	u32	tmr_fiper3;	/* 0x.e88 - Timer fixed period register 3 */
	u8	res24d[20];	/* 0x.e8c - 0x.ebf reserved */
	u32	tmr_etts1_h;	/* 0x.ea0 - Timer stamp high of*/
					/*general purpose external trigger 1*/
	u32	tmr_etts1_l;	/* 0x.ea4 - Timer stamp low of*/
					/*general purpose external trigger 1*/
	u32	tmr_etts2_h;	/* 0x.ea8 - Timer stamp high of*/
					/*general purpose external trigger 2 */
	u32	tmr_etts2_l;	/* 0x.eac - Timer stamp low of*/
};

struct gfar {
	u32	tsec_id;	/* 0x.000 - Controller ID register */
	u32	tsec_id2;	/* 0x.004 - Controller ID2 register */
	u8	res1[8];
	u32	ievent;		/* 0x.010 - Interrupt Event Register */
	u32	imask;		/* 0x.014 - Interrupt Mask Register */
	u32	edis;		/* 0x.018 - Error Disabled Register */
	u32	emapg;		/* 0x.01c - Group Error mapping register */
	u32	ecntrl;		/* 0x.020 - Ethernet Control Register */
	u32	minflr;		/* 0x.024 - Minimum Frame Length Register */
	u32	ptv;		/* 0x.028 - Pause Time Value Register */
	u32	dmactrl;	/* 0x.02c - DMA Control Register */
	u32	tbipa;		/* 0x.030 - TBI PHY Address Register */
	u8	res2[28];
	u32	fifo_rx_pause;	/* 0x.050 - FIFO receive pause start threshold
					register */
	u32	fifo_rx_pause_shutoff;	/* x.054 - FIFO receive starve shutoff
						register */
	u32	fifo_rx_alarm;	/* 0x.058 - FIFO receive alarm start threshold
						register */
	u32	fifo_rx_alarm_shutoff;	/*0x.05c - FIFO receive alarm  starve
						shutoff register */
	u8	res3[44];
	u32	fifo_tx_thr;	/* 0x.08c - FIFO transmit threshold register */
	u8	res4[8];
	u32	fifo_tx_starve;	/* 0x.098 - FIFO transmit starve register */
	u32	fifo_tx_starve_shutoff;	/* 0x.09c - FIFO transmit starve shutoff register */
	u8	res5[96];
	u32	tctrl;		/* 0x.100 - Transmit Control Register */
	u32	tstat;		/* 0x.104 - Transmit Status Register */
	u32	dfvlan;		/* 0x.108 - Default VLAN Control word */
	u32	tbdlen;		/* 0x.10c - Transmit Buffer Descriptor Data Length Register */
	u32	txic;		/* 0x.110 - Transmit Interrupt Coalescing Configuration Register */
	u32	tqueue;		/* 0x.114 - Transmit queue control register */
	u8	res7[40];
	u32	tr03wt;		/* 0x.140 - TxBD Rings 0-3 round-robin weightings */
	u32	tr47wt;		/* 0x.144 - TxBD Rings 4-7 round-robin weightings */
	u8	res8[52];
	u32	tbdbph;		/* 0x.17c - Tx data buffer pointer high */
	u8	res9a[4];
	u32	tbptr0;		/* 0x.184 - TxBD Pointer for ring 0 */
	u8	res9b[4];
	u32	tbptr1;		/* 0x.18c - TxBD Pointer for ring 1 */
	u8	res9c[4];
	u32	tbptr2;		/* 0x.194 - TxBD Pointer for ring 2 */
	u8	res9d[4];
	u32	tbptr3;		/* 0x.19c - TxBD Pointer for ring 3 */
	u8	res9e[4];
	u32	tbptr4;		/* 0x.1a4 - TxBD Pointer for ring 4 */
	u8	res9f[4];
	u32	tbptr5;		/* 0x.1ac - TxBD Pointer for ring 5 */
	u8	res9g[4];
	u32	tbptr6;		/* 0x.1b4 - TxBD Pointer for ring 6 */
	u8	res9h[4];
	u32	tbptr7;		/* 0x.1bc - TxBD Pointer for ring 7 */
	u8	res9[64];
	u32	tbaseh;		/* 0x.200 - TxBD base address high */
	u32	tbase0;		/* 0x.204 - TxBD Base Address of ring 0 */
	u8	res10a[4];
	u32	tbase1;		/* 0x.20c - TxBD Base Address of ring 1 */
	u8	res10b[4];
	u32	tbase2;		/* 0x.214 - TxBD Base Address of ring 2 */
	u8	res10c[4];
	u32	tbase3;		/* 0x.21c - TxBD Base Address of ring 3 */
	u8	res10d[4];
	u32	tbase4;		/* 0x.224 - TxBD Base Address of ring 4 */
	u8	res10e[4];
	u32	tbase5;		/* 0x.22c - TxBD Base Address of ring 5 */
	u8	res10f[4];
	u32	tbase6;		/* 0x.234 - TxBD Base Address of ring 6 */
	u8	res10g[4];
	u32	tbase7;		/* 0x.23c - TxBD Base Address of ring 7 */
	u8	res10h[64];
	u32	tmr_txts1_id;	/* 0x.280 Tx time stamp identification*/
	u32	tmr_txts2_id;	/* 0x.284 Tx time stamp Identification*/
	u8	res10i[56];
	u32	tmr_txts1_h;	/* 0x.2c0 Tx time stamp high*/
	u32	tmr_txts1_l;	/* 0x.2c4 Tx Time Stamp low*/
	u32	tmr_txts2_h;	/* 0x.2c8 Tx time stamp high*/
	u32	tmr_txts2_l;	/*0x.2cc  Tx Time Stamp low */
	u8	res10j[48];
	u32	rctrl;		/* 0x.300 - Receive Control Register */
	u32	rstat;		/* 0x.304 - Receive Status Register */
	u8	res12[8];
	u32	rxic;		/* 0x.310 - Receive Interrupt Coalescing Configuration Register */
	u32	rqueue;		/* 0x.314 - Receive queue control register */
	u32	rir0;		/* 0x.318 - Ring mapping register 0 */
	u32	rir1;		/* 0x.31c - Ring mapping register 1 */
	u32	rir2;		/* 0x.320 - Ring mapping register 2 */
	u32	rir3;		/* 0x.324 - Ring mapping register 3 */
	u8	res13[8];
	u32	rbifx;		/* 0x.330 - Receive bit field extract control register */
	u32	rqfar;		/* 0x.334 - Receive queue filing table address register */
	u32	rqfcr;		/* 0x.338 - Receive queue filing table control register */
	u32	rqfpr;		/* 0x.33c - Receive queue filing table property register */
	u32	mrblr;		/* 0x.340 - Maximum Receive Buffer Length Register */
	u8	res14[56];
	u32	rbdbph;		/* 0x.37c - Rx data buffer pointer high */
	u8	res15a[4];
	u32	rbptr0;		/* 0x.384 - RxBD pointer for ring 0 */
	u8	res15b[4];
	u32	rbptr1;		/* 0x.38c - RxBD pointer for ring 1 */
	u8	res15c[4];
	u32	rbptr2;		/* 0x.394 - RxBD pointer for ring 2 */
	u8	res15d[4];
	u32	rbptr3;		/* 0x.39c - RxBD pointer for ring 3 */
	u8	res15e[4];
	u32	rbptr4;		/* 0x.3a4 - RxBD pointer for ring 4 */
	u8	res15f[4];
	u32	rbptr5;		/* 0x.3ac - RxBD pointer for ring 5 */
	u8	res15g[4];
	u32	rbptr6;		/* 0x.3b4 - RxBD pointer for ring 6 */
	u8	res15h[4];
	u32	rbptr7;		/* 0x.3bc - RxBD pointer for ring 7 */
	u8	res16[64];
	u32	rbaseh;		/* 0x.400 - RxBD base address high */
	u32	rbase0;		/* 0x.404 - RxBD base address of ring 0 */
	u8	res17a[4];
	u32	rbase1;		/* 0x.40c - RxBD base address of ring 1 */
	u8	res17b[4];
	u32	rbase2;		/* 0x.414 - RxBD base address of ring 2 */
	u8	res17c[4];
	u32	rbase3;		/* 0x.41c - RxBD base address of ring 3 */
	u8	res17d[4];
	u32	rbase4;		/* 0x.424 - RxBD base address of ring 4 */
	u8	res17e[4];
	u32	rbase5;		/* 0x.42c - RxBD base address of ring 5 */
	u8	res17f[4];
	u32	rbase6;		/* 0x.434 - RxBD base address of ring 6 */
	u8	res17g[4];
	u32	rbase7;		/* 0x.43c - RxBD base address of ring 7 */
	u8	res17h[128];
	u32	tmr_rxts_h;	/* 0x.4c0 Rx Time Stamp high*/
	u32	tmr_rxts_l;	/* 0x.4c4 Rx Time Stamp low */
	u8	res17i[56];
	u32	maccfg1;	/* 0x.500 - MAC Configuration 1 Register */
	u32	maccfg2;	/* 0x.504 - MAC Configuration 2 Register */
	u32	ipgifg;		/* 0x.508 - Inter Packet Gap/Inter Frame Gap Register */
	u32	hafdup;		/* 0x.50c - Half Duplex Register */
	u32	maxfrm;		/* 0x.510 - Maximum Frame Length Register */
	u8	res18[12];
	u8	gfar_mii_regs[24];	/* See gianfar_phy.h */
	u32	ifctrl;		/* 0x.538 - Interface control register */
	u32	ifstat;		/* 0x.53c - Interface Status Register */
	u32	macstnaddr1;	/* 0x.540 - Station Address Part 1 Register */
	u32	macstnaddr2;	/* 0x.544 - Station Address Part 2 Register */
	u32	mac01addr1;	/* 0x.548 - MAC exact match address 1, part 1 */
	u32	mac01addr2;	/* 0x.54c - MAC exact match address 1, part 2 */
	u32	mac02addr1;	/* 0x.550 - MAC exact match address 2, part 1 */
	u32	mac02addr2;	/* 0x.554 - MAC exact match address 2, part 2 */
	u32	mac03addr1;	/* 0x.558 - MAC exact match address 3, part 1 */
	u32	mac03addr2;	/* 0x.55c - MAC exact match address 3, part 2 */
	u32	mac04addr1;	/* 0x.560 - MAC exact match address 4, part 1 */
	u32	mac04addr2;	/* 0x.564 - MAC exact match address 4, part 2 */
	u32	mac05addr1;	/* 0x.568 - MAC exact match address 5, part 1 */
	u32	mac05addr2;	/* 0x.56c - MAC exact match address 5, part 2 */
	u32	mac06addr1;	/* 0x.570 - MAC exact match address 6, part 1 */
	u32	mac06addr2;	/* 0x.574 - MAC exact match address 6, part 2 */
	u32	mac07addr1;	/* 0x.578 - MAC exact match address 7, part 1 */
	u32	mac07addr2;	/* 0x.57c - MAC exact match address 7, part 2 */
	u32	mac08addr1;	/* 0x.580 - MAC exact match address 8, part 1 */
	u32	mac08addr2;	/* 0x.584 - MAC exact match address 8, part 2 */
	u32	mac09addr1;	/* 0x.588 - MAC exact match address 9, part 1 */
	u32	mac09addr2;	/* 0x.58c - MAC exact match address 9, part 2 */
	u32	mac10addr1;	/* 0x.590 - MAC exact match address 10, part 1*/
	u32	mac10addr2;	/* 0x.594 - MAC exact match address 10, part 2*/
	u32	mac11addr1;	/* 0x.598 - MAC exact match address 11, part 1*/
	u32	mac11addr2;	/* 0x.59c - MAC exact match address 11, part 2*/
	u32	mac12addr1;	/* 0x.5a0 - MAC exact match address 12, part 1*/
	u32	mac12addr2;	/* 0x.5a4 - MAC exact match address 12, part 2*/
	u32	mac13addr1;	/* 0x.5a8 - MAC exact match address 13, part 1*/
	u32	mac13addr2;	/* 0x.5ac - MAC exact match address 13, part 2*/
	u32	mac14addr1;	/* 0x.5b0 - MAC exact match address 14, part 1*/
	u32	mac14addr2;	/* 0x.5b4 - MAC exact match address 14, part 2*/
	u32	mac15addr1;	/* 0x.5b8 - MAC exact match address 15, part 1*/
	u32	mac15addr2;	/* 0x.5bc - MAC exact match address 15, part 2*/
	u8	res20[192];
	struct rmon_mib	rmon;	/* 0x.680-0x.73c */
	u32	rrej;		/* 0x.740 - Receive filer rejected packet counter */
	u8	res21[188];
	u32	igaddr0;	/* 0x.800 - Indivdual/Group address register 0*/
	u32	igaddr1;	/* 0x.804 - Indivdual/Group address register 1*/
	u32	igaddr2;	/* 0x.808 - Indivdual/Group address register 2*/
	u32	igaddr3;	/* 0x.80c - Indivdual/Group address register 3*/
	u32	igaddr4;	/* 0x.810 - Indivdual/Group address register 4*/
	u32	igaddr5;	/* 0x.814 - Indivdual/Group address register 5*/
	u32	igaddr6;	/* 0x.818 - Indivdual/Group address register 6*/
	u32	igaddr7;	/* 0x.81c - Indivdual/Group address register 7*/
	u8	res22[96];
	u32	gaddr0;		/* 0x.880 - Group address register 0 */
	u32	gaddr1;		/* 0x.884 - Group address register 1 */
	u32	gaddr2;		/* 0x.888 - Group address register 2 */
	u32	gaddr3;		/* 0x.88c - Group address register 3 */
	u32	gaddr4;		/* 0x.890 - Group address register 4 */
	u32	gaddr5;		/* 0x.894 - Group address register 5 */
	u32	gaddr6;		/* 0x.898 - Group address register 6 */
	u32	gaddr7;		/* 0x.89c - Group address register 7 */
	u8	res23a[352];
	u32	fifocfg;	/* 0x.a00 - FIFO interface config register */
	u8	res23b[252];
	u8	res23c[248];
	u32	attr;		/* 0x.bf8 - Attributes Register */
	u32	attreli;	/* 0x.bfc - Attributes Extract Length and Extract Index Register */
	u8	res24[512];
	struct gfar_regs_1588 regs_1588;
	u32	isrg0;		/* 0x.eb0 - Interrupt steering group 0 register */
	u32	isrg1;		/* 0x.eb4 - Interrupt steering group 1 register */
	u32	isrg2;		/* 0x.eb8 - Interrupt steering group 2 register */
	u32	isrg3;		/* 0x.ebc - Interrupt steering group 3 register */
	u8	res25[16];
	u32	rxic0;		/* 0x.ed0 - Ring 0 Rx interrupt coalescing */
	u32	rxic1;		/* 0x.ed4 - Ring 1 Rx interrupt coalescing */
	u32	rxic2;		/* 0x.ed8 - Ring 2 Rx interrupt coalescing */
	u32	rxic3;		/* 0x.edc - Ring 3 Rx interrupt coalescing */
	u32	rxic4;		/* 0x.ee0 - Ring 4 Rx interrupt coalescing */
	u32	rxic5;		/* 0x.ee4 - Ring 5 Rx interrupt coalescing */
	u32	rxic6;		/* 0x.ee8 - Ring 6 Rx interrupt coalescing */
	u32	rxic7;		/* 0x.eec - Ring 7 Rx interrupt coalescing */
	u8	res26[32];
	u32	txic0;		/* 0x.f10 - Ring 0 Tx interrupt coalescing */
	u32	txic1;		/* 0x.f14 - Ring 1 Tx interrupt coalescing */
	u32	txic2;		/* 0x.f18 - Ring 2 Tx interrupt coalescing */
	u32	txic3;		/* 0x.f1c - Ring 3 Tx interrupt coalescing */
	u32	txic4;		/* 0x.f20 - Ring 4 Tx interrupt coalescing */
	u32	txic5;		/* 0x.f24 - Ring 5 Tx interrupt coalescing */
	u32	txic6;		/* 0x.f28 - Ring 6 Tx interrupt coalescing */
	u32	txic7;		/* 0x.f2c - Ring 7 Tx interrupt coalescing */
	u8	res27[208];
};

#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
#define GFAR_DEFAULT_RECYCLE_MAX 64
#define GFAR_DEFAULT_RECYCLE_TRUESIZE (SKB_DATA_ALIGN(DEFAULT_RX_BUFFER_SIZE \
		+ RXBUF_ALIGNMENT + NET_SKB_PAD) + sizeof(struct sk_buff))

/* Socket buffer recycling handler for Gianfar driver. This structure has own
 * spinlock to prevent simultaneous access. The member recycle_queue holds
 * top of recyclable socket buffer which are owned by this interface.
 * Maximu size of recyclable buffers are defined by recycle_max, and
 * current size of list is recycle_count.
 */
struct gfar_skb_handler {
	/* Lock for buffer recycling queue */
	spinlock_t	lock;
	short int	recycle_max;
	short int	recycle_count;
	short int	recycle_enable;
	struct sk_buff *recycle_queue;
};

extern void gfar_free_recycle_queue(struct gfar_skb_handler *sh,
		int lock_flag);
#endif

/* Structure for PTP Time Stamp */
struct gfar_ptp_time {
	u32	high;
	u32	low;
};

struct gfar_ptp_data_t {
	int	key;
	struct	gfar_ptp_time	item;
};

struct gfar_ptp_circular_t {
	int	front;
	int	end;
	int	size;
	struct	gfar_ptp_data_t *data_buf;
};

/* Flags related to gianfar device features */
#define FSL_GIANFAR_DEV_HAS_GIGABIT		0x00000001
#define FSL_GIANFAR_DEV_HAS_COALESCE		0x00000002
#define FSL_GIANFAR_DEV_HAS_RMON		0x00000004
#define FSL_GIANFAR_DEV_HAS_MULTI_INTR		0x00000008
#define FSL_GIANFAR_DEV_HAS_CSUM		0x00000010
#define FSL_GIANFAR_DEV_HAS_VLAN		0x00000020
#define FSL_GIANFAR_DEV_HAS_EXTENDED_HASH	0x00000040
#define FSL_GIANFAR_DEV_HAS_PADDING		0x00000080
#define FSL_GIANFAR_DEV_HAS_MAGIC_PACKET	0x00000100
#define FSL_GIANFAR_DEV_HAS_BD_STASHING		0x00000200
#define FSL_GIANFAR_DEV_HAS_BUF_STASHING	0x00000400
#define FSL_GIANFAR_DEV_HAS_ARP_PACKET		0x00000800

#if (MAXGROUPS == 2)
#define DEFAULT_MAPPING 	0xAA
#else
#define DEFAULT_MAPPING 	0xFF
#endif

#define ISRG_SHIFT_TX	0x10
#define ISRG_SHIFT_RX	0x18

/* The same driver can operate in two modes */
/* SQ_SG_MODE: Single Queue Single Group Mode
 * 		(Backward compatible mode)
 * MQ_MG_MODE: Multi Queue Multi Group mode
 */
enum {
	SQ_SG_MODE = 0,
	MQ_MG_MODE
};

/**
 *	struct gfar_priv_tx_q - per tx queue structure
 *	@txlock: per queue tx spin lock
 *	@tx_skbuff:skb pointers
 *	@skb_curtx: to be used skb pointer
 *	@skb_dirtytx:the last used skb pointer
 *	@qindex: index of this queue
 *	@dev: back pointer to the dev structure
 *	@grp: back pointer to the group to which this queue belongs
 *	@tx_bd_base: First tx buffer descriptor
 *	@cur_tx: Next free ring entry
 *	@dirty_tx: First buffer in line to be transmitted
 *	@tx_ring_size: Tx ring size
 *	@num_txbdfree: number of free TxBds
 *	@txcoalescing: enable/disable tx coalescing
 *	@txic: transmit interrupt coalescing value
 *	@txcount: coalescing value if based on tx frame count
 *	@txtime: coalescing value if based on time
 */
struct gfar_priv_tx_q {
	spinlock_t txlock __attribute__ ((aligned (SMP_CACHE_BYTES)));
	struct sk_buff ** tx_skbuff;
	/* Buffer descriptor pointers */
	struct	txbd8 *tx_bd_base;
	struct	txbd8 *cur_tx;
	struct	txbd8 *dirty_tx;
	struct	net_device *dev;
	struct gfar_priv_grp *grp;
	u16	skb_curtx;
	u16	skb_dirtytx;
	u16	qindex;
	unsigned int tx_ring_size;
	unsigned int num_txbdfree;
	/* Configuration info for the coalescing features */
	unsigned char txcoalescing;
	unsigned long txic;
	unsigned short txcount;
	unsigned short txtime;
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
	struct gfar_skb_handler *local_sh; /*per_cpu*/
#endif
};

/*
 * Per Rx queue stats
 */
struct rx_q_stats {
	unsigned long rx_packets;
	unsigned long rx_bytes;
	unsigned long rx_dropped;
};

/**
 *	struct gfar_priv_rx_q - per rx queue structure
 *	@rxlock: per queue rx spin lock
 *	@rx_skbuff: skb pointers
 *	@skb_currx: currently use skb pointer
 *	@rx_bd_base: First rx buffer descriptor
 *	@cur_rx: Next free rx ring entry
 *	@qindex: index of this queue
 *	@dev: back pointer to the dev structure
 *	@rx_ring_size: Rx ring size
 *	@rxcoalescing: enable/disable rx-coalescing
 *	@rxic: receive interrupt coalescing vlaue
 */

struct gfar_priv_rx_q {
	spinlock_t rxlock __attribute__ ((aligned (SMP_CACHE_BYTES)));
	struct	sk_buff ** rx_skbuff;
	struct	rxbd8 *rx_bd_base;
	struct	rxbd8 *cur_rx;
	struct	net_device *dev;
	struct gfar_priv_grp *grp;
	struct	rx_q_stats stats;
	u16	skb_currx;
	u16	qindex;
	unsigned int	rx_ring_size;
	/* RX Coalescing values */
	unsigned char rxcoalescing;
	unsigned long rxic;
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
	unsigned int rx_skbuff_truesize;
	struct gfar_skb_handler skb_handler;
	struct gfar_skb_handler *local_sh; /*per_cpu*/
#endif
};

/**
 *	struct gfar_priv_grp - per group structure
 *	@napi: the napi poll function
 *	@priv: back pointer to the priv structure
 *	@regs: the ioremapped register space for this group
 *	@grp_id: group id for this group
 *	@interruptTransmit: The TX interrupt number for this group
 *	@interruptReceive: The RX interrupt number for this group
 *	@interruptError: The ERROR interrupt number for this group
 *	@int_name_tx: tx interrupt name for this group
 *	@int_name_rx: rx interrupt name for this group
 *	@int_name_er: er interrupt name for this group
 */

struct gfar_priv_grp {
	spinlock_t grplock __attribute__ ((aligned (SMP_CACHE_BYTES)));
#ifdef CONFIG_GIANFAR_TXNAPI
#ifdef CONFIG_GFAR_SW_PKT_STEERING
	struct napi_struct napi_tx[NR_CPUS];
#else
	struct napi_struct napi_tx;
#endif
	struct napi_struct napi_rx;
#else
	struct	napi_struct napi;
#endif
	struct gfar_private *priv;
	struct gfar __iomem *regs;
	unsigned int grp_id;
	unsigned long rx_bit_map;
	unsigned long tx_bit_map;
	unsigned long num_tx_queues;
	unsigned long num_rx_queues;
	unsigned int rstat;
	unsigned int tstat;
	unsigned int imask;
	unsigned int ievent;
	unsigned int interruptTransmit;
	unsigned int interruptReceive;
	unsigned int interruptError;

	char int_name_tx[GFAR_INT_NAME_MAX];
	char int_name_rx[GFAR_INT_NAME_MAX];
	char int_name_er[GFAR_INT_NAME_MAX];
#ifdef CONFIG_GFAR_SW_PKT_STEERING
	struct fsl_msg_unit *msg_virtual_tx[NR_CPUS];
	char int_name_vtx[NR_CPUS][GFAR_INT_NAME_MAX];
#endif
};

/* Struct stolen almost completely (and shamelessly) from the FCC enet source
 * (Ok, that's not so true anymore, but there is a family resemblence)
 * The GFAR buffer descriptors track the ring buffers.  The rx_bd_base
 * and tx_bd_base always point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct gfar_private {

	/* Indicates how many tx, rx queues are enabled */
	unsigned int num_tx_queues;
	unsigned int num_rx_queues;
	unsigned int num_grps;
	unsigned int mode;

	/* The total tx and rx ring size for the enabled queues */
	unsigned int total_tx_ring_size;
	unsigned int total_rx_ring_size;

	struct device_node *node;
	struct net_device *ndev;
	struct of_device *ofdev;

	struct gfar_priv_grp gfargrp[MAXGROUPS];
	struct gfar_priv_tx_q *tx_queue[MAX_TX_QS];
	struct gfar_priv_rx_q *rx_queue[MAX_RX_QS];

	/* RX per device parameters */
	unsigned int rx_buffer_size;
	unsigned int rx_stash_size;
	unsigned int rx_stash_index;

	u32 cur_filer_idx;

	/* wake up ring */
	struct rxbd8 *wk_bd_base;
	struct rxbd8 *cur_wk;

	/* wake up ring parameters */
	unsigned int wk_ring_size;
	unsigned int wk_buffer_size;

	/* wake up buffer */
	unsigned long wk_buf_vaddr;
	unsigned long wk_buf_paddr;
	unsigned long wk_buf_align_vaddr;
	unsigned long wk_buf_align_paddr;

	struct vlan_group *vlgrp;

	/* Hash registers and their width */
	u32 __iomem *hash_regs[16];
	int hash_width;

	/* global parameters */
	unsigned int fifo_threshold;
	unsigned int fifo_starve;
	unsigned int fifo_starve_off;

	/* Bitfield update lock */
	spinlock_t bflock;

	phy_interface_t interface;
	struct device_node *phy_node;
	struct device_node *tbi_node;
	u32 device_flags;
	unsigned char rx_csum_enable:1,
		extended_hash:1,
		bd_stash_en:1,
		filer_en:1,
		wol_en:1; /* Wake-on-LAN enabled */
	unsigned short padding;

	/* PHY stuff */
	struct phy_device *phydev;
	struct mii_bus *mii_bus;
	int oldspeed;
	int oldduplex;
	int oldlink;

	uint32_t msg_enable;

	struct work_struct reset_task;

	u8 ip_addr[4];
	int wol_opts;

	/* Network Statistics */
	struct gfar_extra_stats extra_stats;
	struct gfar_ptp_circular_t rx_time_sync;
	struct gfar_ptp_circular_t rx_time_del_req;

	/* 1588 stuff */
	struct gfar_ptp_circular_t rx_time_pdel_req;
	struct gfar_ptp_circular_t rx_time_pdel_resp;
	struct gfar_regs_1588 __iomem *ptimer;
	struct resource timer_resource;
	uint32_t ptimer_present;
#ifdef CONFIG_GIANFAR_L2SRAM
	int bd_in_ram;
#endif
#ifdef CONFIG_GFAR_SW_PKT_STEERING
	int sps; /*flag for s/w packet steering */
#endif
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
	unsigned int skbuff_truesize;
	struct gfar_skb_handler skb_handler;
#endif
};

#ifdef CONFIG_GFAR_SW_PKT_STEERING
#define INTR_COALESCE_CNT 22
#define INTR_COALESCE_TIMEOUT 32000 /* in nSecs */
#define GFAR_CPU_BUFF_SIZE 64

/* producer-consumer buffer for inter cpu packet transfer */
struct shared_buffer {
	struct sk_buff *buffer[GFAR_CPU_BUFF_SIZE];
	int in;
	int out;
	atomic_t buff_cnt;
};

struct gfar_cpu_dev {
	struct net_device dev;
	struct napi_struct napi;
	struct shared_buffer tx_queue;
	int intr_coalesce_cnt;
	struct hrtimer intr_coalesce_timer;
	struct fsl_msg_unit *msg_virtual_rx;
	char int_name[GFAR_INT_NAME_MAX];
	int enabled;
#ifdef CONFIG_GFAR_SKBUFF_RECYCLING
	struct gfar_skb_handler sh;
#endif
};

extern int rcv_pkt_steering;
#endif

extern unsigned int ftp_rqfpr[MAX_FILER_IDX + 1];
extern unsigned int ftp_rqfcr[MAX_FILER_IDX + 1];

static inline u32 gfar_read(volatile unsigned __iomem *addr)
{
	u32 val;
	val = in_be32(addr);
	return val;
}

static inline void gfar_write(volatile unsigned __iomem *addr, u32 val)
{
	out_be32(addr, val);
}

static inline void gfar_read_filer(struct gfar_private *priv,
		unsigned int far, u32 *fcr, u32 *fpr)
{
	struct gfar __iomem *regs = priv->gfargrp[0].regs;

	gfar_write(&regs->rqfar, far);
	*fcr = gfar_read(&regs->rqfcr);
	*fpr = gfar_read(&regs->rqfpr);
}

 static inline void gfar_write_filer(struct gfar_private *priv,
		unsigned int far, unsigned int fcr, unsigned int fpr)
{
	struct gfar __iomem *regs = priv->gfargrp[0].regs;

	gfar_write(&regs->rqfar, far);
	gfar_write(&regs->rqfcr, fcr);
	gfar_write(&regs->rqfpr, fpr);
}

static inline void lock_rx_qs(struct gfar_private *priv)
{
	int i = 0x0;

	for (i = 0; i < priv->num_rx_queues; i++)
		spin_lock(&priv->rx_queue[i]->rxlock);
}

static inline void lock_tx_qs(struct gfar_private *priv)
{
	int i = 0x0;

	for (i = 0; i < priv->num_tx_queues; i++)
		spin_lock(&priv->tx_queue[i]->txlock);
}

static inline void unlock_rx_qs(struct gfar_private *priv)
{
	int i = 0x0;

	for (i = 0; i < priv->num_rx_queues; i++)
		spin_unlock(&priv->rx_queue[i]->rxlock);
}

static inline void unlock_tx_qs(struct gfar_private *priv)
{
	int i = 0x0;

	for (i = 0; i < priv->num_tx_queues; i++)
		spin_unlock(&priv->tx_queue[i]->txlock);
}

#define GFAR_JUMBO_TX   0
#define GFAR_JUMBO_RX   1

static inline void gfar_jumbo_inc(struct gfar_private *priv, unsigned int len, int rx)
{
	if (len > ETH_FRAME_LEN) {
		if (rx) {
            priv->extra_stats.rx_jumbo++;
            priv->extra_stats.rx_jumbo_len += len;
        }
        else {
            priv->extra_stats.tx_jumbo++;
            priv->extra_stats.tx_jumbo_len += len;
        }
	}
}

extern irqreturn_t gfar_receive(int irq, void *dev_id);
extern int startup_gfar(struct net_device *dev);
extern void stop_gfar(struct net_device *dev);
extern void gfar_halt(struct net_device *dev);
extern void gfar_1588_start(struct net_device *dev);
extern void gfar_1588_stop(struct net_device *dev);
extern int gfar_ptp_init(struct gfar_private *priv);
extern void gfar_ptp_cleanup(struct gfar_private *priv);
extern int gfar_ptp_do_txstamp(struct sk_buff *skb);
extern void pmuxcr_guts_write(void);
extern void gfar_ptp_store_rxstamp(struct net_device *dev, struct sk_buff *skb);
extern int gfar_ioctl_1588(struct net_device *dev, struct ifreq *ifr, int cmd);
extern void gfar_phy_test(struct mii_bus *bus, struct phy_device *phydev,
		int enable, u32 regnum, u32 read);
extern void gfar_configure_tx_coalescing(struct gfar_private *priv,
					unsigned long tx_mask);
extern void gfar_configure_rx_coalescing(struct gfar_private *priv,
					unsigned long rx_mask);
void gfar_init_sysfs(struct net_device *dev);

extern const struct ethtool_ops gfar_ethtool_ops;

#ifdef CONFIG_NET_GIANFAR_FP
extern int netdev_fastroute;
#endif
#endif /* __GIANFAR_H */
