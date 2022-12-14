/*
 *  Universal TUN/TAP device driver.
 *  Copyright (C) 1999-2000 Maxim Krasnyansky <max_mk@yahoo.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */

#ifndef __IF_TUN_H
#define __IF_TUN_H

#include <linux/types.h>
#include <linux/if_ether.h>

#ifdef __KERNEL__

#ifdef TUN_DEBUG

#define DBG  if(tun->debug)printk
#define DBG1 if(debug==2)printk
#else
#define DBG( a... )
#define DBG1( a... )
#endif

#ifdef RKS_TUBE_SUPPORT
struct tube_hook {
	int	(*func)(struct net_device *dev, struct sk_buff *skb, void *opaque);
	void	*opaque;
};
#endif

#define FLT_EXACT_COUNT 8
struct tap_filter {
	unsigned int    count;    /* Number of addrs. Zero means disabled */
	u32             mask[2];  /* Mask of the hashed addrs */
	unsigned char	addr[FLT_EXACT_COUNT][ETH_ALEN];
};

struct tun_file {
	atomic_t count;
	struct tun_struct *tun;
	struct net *net;
};

struct tun_struct {
	struct tun_file		*tfile;
	unsigned int 		flags;
	uid_t			owner;
	gid_t			group;

	struct net_device	*dev;
	struct fasync_struct	*fasync;

	struct tap_filter       txflt;
	struct socket		socket;

#ifdef TUN_DEBUG
	int debug;
#endif
#ifdef RKS_TUBE_SUPPORT
	struct tube_hook	tube;
#endif
};
#endif /* __KERNEL__ */

/* Read queue size */
#define TUN_READQ_SIZE	500

/* TUN device flags */
#define TUN_TUN_DEV 	0x0001	
#define TUN_TAP_DEV	0x0002
#define TUN_TYPE_MASK   0x000f

#define TUN_FASYNC	0x0010
#define TUN_NOCHECKSUM	0x0020
#define TUN_NO_PI	0x0040
#define TUN_ONE_QUEUE	0x0080
#define TUN_PERSIST 	0x0100	
#define TUN_VNET_HDR 	0x0200

/* Ioctl defines */
#define TUNSETNOCSUM  _IOW('T', 200, int) 
#define TUNSETDEBUG   _IOW('T', 201, int) 
#define TUNSETIFF     _IOW('T', 202, int) 
#define TUNSETPERSIST _IOW('T', 203, int) 
#define TUNSETOWNER   _IOW('T', 204, int)
#define TUNSETLINK    _IOW('T', 205, int)
#define TUNSETGROUP   _IOW('T', 206, int)
#define TUNGETFEATURES _IOR('T', 207, unsigned int)
#define TUNSETOFFLOAD  _IOW('T', 208, unsigned int)
#define TUNSETTXFILTER _IOW('T', 209, unsigned int)
#define TUNGETIFF      _IOR('T', 210, unsigned int)
#define TUNGETSNDBUF   _IOR('T', 211, int)
#define TUNSETSNDBUF   _IOW('T', 212, int)

/* TUNSETIFF ifr flags */
#define IFF_TUN		0x0001
#define IFF_TAP		0x0002
#define IFF_NO_PI	0x1000
#define IFF_ONE_QUEUE	0x2000
#define IFF_VNET_HDR	0x4000
#define IFF_TUN_EXCL	0x8000

/* Features for GSO (TUNSETOFFLOAD). */
#define TUN_F_CSUM	0x01	/* You can hand me unchecksummed packets. */
#define TUN_F_TSO4	0x02	/* I can handle TSO for IPv4 packets */
#define TUN_F_TSO6	0x04	/* I can handle TSO for IPv6 packets */
#define TUN_F_TSO_ECN	0x08	/* I can handle TSO with ECN bits. */
#define TUN_F_UFO	0x10	/* I can handle UFO packets */

/* Protocol info prepended to the packets (when IFF_NO_PI is not set) */
#define TUN_PKT_STRIP	0x0001
struct tun_pi {
	__u16  flags;
	__be16 proto;
};

/*
 * Filter spec (used for SETXXFILTER ioctls)
 * This stuff is applicable only to the TAP (Ethernet) devices.
 * If the count is zero the filter is disabled and the driver accepts
 * all packets (promisc mode).
 * If the filter is enabled in order to accept broadcast packets
 * broadcast addr must be explicitly included in the addr list.
 */
#define TUN_FLT_ALLMULTI 0x0001 /* Accept all multicast packets */
struct tun_filter {
	__u16  flags; /* TUN_FLT_ flags see above */
	__u16  count; /* Number of addresses */
	__u8   addr[0][ETH_ALEN];
};

#endif /* __IF_TUN_H */
