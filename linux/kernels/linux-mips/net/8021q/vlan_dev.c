/* -*- linux-c -*-
 * INET		802.1Q VLAN
 *		Ethernet-type device handling.
 *
 * Authors:	Ben Greear <greearb@candelatech.com>
 *              Please send support related email to: netdev@vger.kernel.org
 *              VLAN Home Page: http://www.candelatech.com/~greear/vlan.html
 *
 * Fixes:       Mar 22 2001: Martin Bokaemper <mbokaemper@unispherenetworks.com>
 *                - reset skb->pkt_type on incoming packets when MAC was changed
 *                - see that changed MAC is saddr for outgoing packets
 *              Oct 20, 2001:  Ard van Breeman:
 *                - Fix MC-list, finally.
 *                - Flush MC-list on VLAN destroy.
 *
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <net/arp.h>

#include "vlan.h"
#include "vlanproc.h"
#include <linux/if_vlan.h>

#if defined(CONFIG_BRIDGE_VLAN)
#include <linux/if_bridge.h> /* is_bridge_device() */
#endif
/*
 *	Rebuild the Ethernet MAC header. This is called after an ARP
 *	(or in future other address resolution) has completed on this
 *	sk_buff. We now let ARP fill in the other fields.
 *
 *	This routine CANNOT use cached dst->neigh!
 *	Really, it is used only when dst->neigh is wrong.
 *
 * TODO:  This needs a checkup, I'm ignorant here. --BLG
 */
static int vlan_dev_rebuild_header(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);

	switch (veth->h_vlan_encapsulated_proto) {
#ifdef CONFIG_INET
	case htons(ETH_P_IP):

		/* TODO:  Confirm this will work with VLAN headers... */
		return arp_find(veth->h_dest, skb);
#endif
	default:
		pr_debug("%s: unable to resolve type %X addresses.\n",
			 dev->name, ntohs(veth->h_vlan_encapsulated_proto));

		memcpy(veth->h_source, dev->dev_addr, ETH_ALEN);
		break;
	}

	return 0;
}

static inline struct sk_buff *vlan_check_reorder_header(struct sk_buff *skb)
{
	if (vlan_dev_info(skb->dev)->flags & VLAN_FLAG_REORDER_HDR) {
		if (skb_cow(skb, skb_headroom(skb)) < 0)
			skb = NULL;
		if (skb) {
			/* Lifted from Gleb's VLAN code... */
			memmove(skb->data - ETH_HLEN,
				skb->data - VLAN_ETH_HLEN, 12);
			skb->mac_header += VLAN_HLEN;
		}
	}

	return skb;
}

#if 0 && defined(CONFIG_BRIDGE_VLAN)
#define _BR_VLAN_DEBUG
#define dPrintf(args...)    printk(##args)
#else
#undef _BR_VLAN_DEBUG
#define dPrintf(...)
#endif


#ifdef CONFIG_BRIDGE_VLAN
// check to see if there is a vlan device configured
int
has_vlan_device(struct sk_buff *skb, struct net_device *dev)
{
	unsigned short vlan_TCI;
	unsigned short vid;
	struct net_device *vlan_dev;
	struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);

	vlan_TCI = ntohs(vhdr->h_vlan_TCI);

	vid = (vlan_TCI & VLAN_VID_MASK);

	rcu_read_lock();
	vlan_dev = __find_vlan_dev(dev, vid);
	rcu_read_unlock();
	return (vlan_dev? 1 : 0);
}
#endif

static inline void vlan_set_encap_proto(struct sk_buff *skb,
#ifdef CONFIG_BRIDGE_VLAN
        int no_vhdr,
#endif
		struct vlan_hdr *vhdr)
{
	__be16 proto;
	unsigned char *rawp;

	/*
	 * Was a VLAN packet, grab the encapsulated protocol, which the layer
	 * three protocols care about.
	 */
#ifdef CONFIG_BRIDGE_VLAN
    if ( no_vhdr ) {
        proto = eth_hdr(skb)->h_proto;
    } else
#endif
    {
	    proto = vhdr->h_vlan_encapsulated_proto;
    }
	if (ntohs(proto) >= 1536) {
		skb->protocol = proto;
		return;
	}

	rawp = skb->data;
	if (*(unsigned short *)rawp == 0xFFFF)
		/*
		 * This is a magic hack to spot IPX packets. Older Novell
		 * breaks the protocol design and runs IPX over 802.3 without
		 * an 802.2 LLC layer. We look for FFFF which isn't a used
		 * 802.2 SSAP/DSAP. This won't work for fault tolerant netware
		 * but does for the rest.
		 */
		skb->protocol = htons(ETH_P_802_3);
	else
		/*
		 * Real 802.2 LLC
		 */
		skb->protocol = htons(ETH_P_802_2);
}

/*
 *	Determine the packet's protocol ID. The rule here is that we
 *	assume 802.3 if the type field is short enough to be a length.
 *	This is normal practice and works for any 'now in use' protocol.
 *
 *  Also, at this point we assume that we ARE dealing exclusively with
 *  VLAN packets, or packets that should be made into VLAN packets based
 *  on a default VLAN ID.
 *
 *  NOTE:  Should be similar to ethernet/eth.c.
 *
 *  SANITY NOTE:  This method is called when a packet is moving up the stack
 *                towards userland.  To get here, it would have already passed
 *                through the ethernet/eth.c eth_type_trans() method.
 *  SANITY NOTE 2: We are referencing to the VLAN_HDR frields, which MAY be
 *                 stored UNALIGNED in the memory.  RISC systems don't like
 *                 such cases very much...
 *  SANITY NOTE 2a: According to Dave Miller & Alexey, it will always be
 *  		    aligned, so there doesn't need to be any of the unaligned
 *  		    stuff.  It has been commented out now...  --Ben
 *
 */
int vlan_skb_recv(struct sk_buff *skb, struct net_device *dev,
		  struct packet_type *ptype, struct net_device *orig_dev)
{
	struct vlan_hdr *vhdr;
	struct net_device_stats *stats;
	u16 vlan_id;
	u16 vlan_tci;
#ifdef CONFIG_BRIDGE_VLAN
    int no_vhdr = 0;
#endif

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (skb == NULL)
		goto err_free;

	if (unlikely(!pskb_may_pull(skb, VLAN_HLEN)))
		goto err_free;

	vhdr = (struct vlan_hdr *)skb->data;

#ifdef CONFIG_BRIDGE_VLAN
	if ( eth_hdr(skb)->h_proto != htons(ETH_P_8021Q) ) {
		// untagged frames
		if ( skb->protocol != htons(ETH_P_8021Q) ) {
#ifdef _BR_VLAN_DEBUG
			printk(KERN_ERR "%s: eth_type=%04x vid=%d : no vhdr\n", __FUNCTION__, 
					eth_hdr(skb)->h_proto, skb->tag_vid);
#endif
			kfree_skb(skb);
			return -1;
		}
		// packet for tagged VLAN from an untagged port
		no_vhdr++;
		vlan_id = skb->tag_vid;
		vlan_tci = (skb->tag_priority << 13 | skb->tag_vid);
	} else {
		/* vlan_TCI = ntohs(get_unaligned(&vhdr->h_vlan_TCI)); */
		vlan_tci = ntohs(vhdr->h_vlan_TCI);
		vlan_id = (vlan_tci & VLAN_VID_MASK);
		skb->tag_vid = vlan_id;
	}
#else
	vlan_tci = ntohs(vhdr->h_vlan_TCI);
	vlan_id = vlan_tci & VLAN_VID_MASK;
#endif

	rcu_read_lock();
	skb->dev = __find_vlan_dev(dev, vlan_id);
	if (!skb->dev) {
		pr_debug("%s: ERROR: No net_device for VID: %u on dev: %s\n",
			 __func__, vlan_id, dev->name);
		goto err_unlock;
	}

	stats = &skb->dev->stats;
	stats->rx_packets++;
	stats->rx_bytes += skb->len;
#if 1 /* V54_BSP */
	RX_BROADCAST_STATS(stats, skb);
#endif

#ifdef CONFIG_BRIDGE_VLAN
    if ( ! no_vhdr )
#endif
	{
	    skb_pull_rcsum(skb, VLAN_HLEN);
	}
#if 1 /* V54_BSP */
    {
		unsigned int prio = vlan_get_ingress_priority(skb->dev, vlan_tci);
		if( prio ) {
		    skb->priority &= ~0xf;			/* clear previous priority value */
		    skb->priority |= prio & 0xf;	/* set new priority value		 */
		}
	}
#else
	skb->priority = vlan_get_ingress_priority(skb->dev, vlan_tci);
#endif

	pr_debug("%s: priority: %u for TCI: %hu\n",
		 __func__, skb->priority, vlan_tci);

	switch (skb->pkt_type) {
	case PACKET_BROADCAST: /* Yeah, stats collect these together.. */
		/* stats->broadcast ++; // no such counter :-( */
		break;

	case PACKET_MULTICAST:
		stats->multicast++;
		break;

	case PACKET_OTHERHOST:
		/* Our lower layer thinks this is not local, let's make sure.
		 * This allows the VLAN to have a different MAC than the
		 * underlying device, and still route correctly.
		 */
		if (!compare_ether_addr(eth_hdr(skb)->h_dest,
					skb->dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
		break;
	default:
		break;
	}

#ifdef CONFIG_BRIDGE_VLAN
    vlan_set_encap_proto(skb, no_vhdr, vhdr);
#else
    vlan_set_encap_proto(skb, vhdr);
#endif

#ifdef CONFIG_BRIDGE_VLAN
        /* for the case: bridge port's untagged vlan id == mgmt vlan id */
    if (!no_vhdr)
#endif
	    skb = vlan_check_reorder_header(skb);
	if (!skb) {
		stats->rx_errors++;
		goto err_unlock;
	}

	netif_rx(skb);
	rcu_read_unlock();
	return NET_RX_SUCCESS;

err_unlock:
	rcu_read_unlock();
err_free:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static inline u16
vlan_dev_get_egress_qos_mask(struct net_device *dev, struct sk_buff *skb)
{
	struct vlan_priority_tci_mapping *mp;

	mp = vlan_dev_info(dev)->egress_priority_map[(skb->priority & 0xF)];
	while (mp) {
		if (mp->priority == skb->priority) {
			return mp->vlan_qos; /* This should already be shifted
					      * to mask correctly with the
					      * VLAN's TCI */
		}
		mp = mp->next;
	}
	return 0;
}

/*
 *	Create the VLAN header for an arbitrary protocol layer
 *
 *	saddr=NULL	means use device source address
 *	daddr=NULL	means leave destination address (eg unresolved arp)
 *
 *  This is called when the SKB is moving down the stack towards the
 *  physical devices.
 */
static int vlan_dev_hard_header(struct sk_buff *skb, struct net_device *dev,
				unsigned short type,
				const void *daddr, const void *saddr,
				unsigned int len)
{
	struct vlan_hdr *vhdr;
	unsigned int vhdrlen = 0;
	u16 vlan_tci = 0;
	int rc;

	if (WARN_ON(skb_headroom(skb) < dev->hard_header_len))
		return -ENOSPC;

	if (!(vlan_dev_info(dev)->flags & VLAN_FLAG_REORDER_HDR)) {
		vhdr = (struct vlan_hdr *) skb_push(skb, VLAN_HLEN);

		vlan_tci = vlan_dev_info(dev)->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(dev, skb);
		vhdr->h_vlan_TCI = htons(vlan_tci);

		/*
		 *  Set the protocol type. For a packet of type ETH_P_802_3 we
		 *  put the length in here instead. It is up to the 802.2
		 *  layer to carry protocol information.
		 */
		if (type != ETH_P_802_3)
			vhdr->h_vlan_encapsulated_proto = htons(type);
		else
			vhdr->h_vlan_encapsulated_proto = htons(len);

		skb->protocol = htons(ETH_P_8021Q);
		type = ETH_P_8021Q;
		vhdrlen = VLAN_HLEN;
	}

	/* Before delegating work to the lower layer, enter our MAC-address */
	if (saddr == NULL)
		saddr = dev->dev_addr;

	/* Now make the underlying real hard header */
	dev = vlan_dev_info(dev)->real_dev;
	rc = dev_hard_header(skb, dev, type, daddr, saddr, len + vhdrlen);
	if (rc > 0)
		rc += vhdrlen;
	return rc;
}

static netdev_tx_t vlan_dev_hard_start_xmit(struct sk_buff *skb,
					    struct net_device *dev)
{
	int i = skb_get_queue_mapping(skb);
	struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);
	unsigned int len;
	int ret;

#ifdef CONFIG_BRIDGE_VLAN
	/*
	 * If the virtual device (e.g. br0.10) is set as management interface, 
	 * then prioritize its outbound traffic as indicated in member mgmt_priority.
	 */
	if( vlan_dev_info(dev)->mgmt_priority != MGMT_NULL_PRI ) {
		skb->priority &= ~0xf;  /* lower 4 bits reserved for traffic class */
		skb->priority |= vlan_dev_info(dev)->mgmt_priority & 0xf;
	}

	/* Ruckus: Make skb agree with VLAN header. */
	skb->tag_priority = 0;
	skb->tag_vid = vlan_dev_info(dev)->vlan_id;
#endif /* CONFIG_BRIDGE_VLAN */

#if 1 /* CONFIG_BRIDGE_SET_MAC */
	// this is needed to support using eth0.100 as a separte interface
	// and may be attaching that to a different bridge.
	// only set VLAN header if real_dev is not a bridge
	// If real_dev is a bridge, then packet is going up the stack
	if (!is_bridge_device(vlan_dev_info(dev)->real_dev)) {
#endif
    /* Handle non-VLAN frames if they are sent to us, for example by DHCP.
    	 *
    	 * NOTE: THIS ASSUMES DIX ETHERNET, SPECIFICALLY NOT SUPPORTING
    	 * OTHER THINGS LIKE FDDI/TokenRing/802.3 SNAPs...
    	 */
    	if (veth->h_vlan_proto != htons(ETH_P_8021Q) ||
    	    vlan_dev_info(dev)->flags & VLAN_FLAG_REORDER_HDR) {
    		unsigned int orig_headroom = skb_headroom(skb);
    		u16 vlan_tci;

    		vlan_dev_info(dev)->cnt_encap_on_xmit++;

    		vlan_tci = vlan_dev_info(dev)->vlan_id;
    		vlan_tci |= vlan_dev_get_egress_qos_mask(dev, skb);
    		skb = __vlan_put_tag(skb, vlan_tci);
    		if (!skb) {
    			txq->tx_dropped++;
    			return NETDEV_TX_OK;
    		}

    		if (orig_headroom < VLAN_HLEN)
    			vlan_dev_info(dev)->cnt_inc_headroom_on_tx++;
    	}
#if 1 /* CONFIG_BRIDGE_SET_MAC */
	}
#endif


	skb->dev = vlan_dev_info(dev)->real_dev;
	len = skb->len;
	ret = dev_queue_xmit(skb);

	if (likely(ret == NET_XMIT_SUCCESS)) {
		txq->tx_packets++;
		txq->tx_bytes += len;
#if 1 /* V54_BSP */
		TX_BROADCAST_STATS(&dev->stats, skb, 1);
#endif
	} else
		txq->tx_dropped++;

	return NETDEV_TX_OK;
}

static netdev_tx_t vlan_dev_hwaccel_hard_start_xmit(struct sk_buff *skb,
						    struct net_device *dev)
{
	int i = skb_get_queue_mapping(skb);
	struct netdev_queue *txq = netdev_get_tx_queue(dev, i);
	u16 vlan_tci;
	unsigned int len;
	int ret;

	vlan_tci = vlan_dev_info(dev)->vlan_id;
	vlan_tci |= vlan_dev_get_egress_qos_mask(dev, skb);
	skb = __vlan_hwaccel_put_tag(skb, vlan_tci);

	skb->dev = vlan_dev_info(dev)->real_dev;
	len = skb->len;
	ret = dev_queue_xmit(skb);

	if (likely(ret == NET_XMIT_SUCCESS)) {
		txq->tx_packets++;
		txq->tx_bytes += len;
#if 1 /* V54_BSP */
		TX_BROADCAST_STATS(&dev->stats, skb, 1);
#endif
	} else
		txq->tx_dropped++;

	return NETDEV_TX_OK;
}

static int vlan_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	/* TODO: gotta make sure the underlying layer can handle it,
	 * maybe an IFF_VLAN_CAPABLE flag for devices?
	 */
	if (vlan_dev_info(dev)->real_dev->mtu < new_mtu)
		return -ERANGE;

	dev->mtu = new_mtu;

	return 0;
}

void vlan_dev_set_ingress_priority(const struct net_device *dev,
				   u32 skb_prio, u16 vlan_prio)
{
	struct vlan_dev_info *vlan = vlan_dev_info(dev);

	if (vlan->ingress_priority_map[vlan_prio & 0x7] && !skb_prio)
		vlan->nr_ingress_mappings--;
	else if (!vlan->ingress_priority_map[vlan_prio & 0x7] && skb_prio)
		vlan->nr_ingress_mappings++;

	vlan->ingress_priority_map[vlan_prio & 0x7] = skb_prio;
}

int vlan_dev_set_egress_priority(const struct net_device *dev,
				 u32 skb_prio, u16 vlan_prio)
{
	struct vlan_dev_info *vlan = vlan_dev_info(dev);
	struct vlan_priority_tci_mapping *mp = NULL;
	struct vlan_priority_tci_mapping *np;
	u32 vlan_qos = (vlan_prio << 13) & 0xE000;

	/* See if a priority mapping exists.. */
	mp = vlan->egress_priority_map[skb_prio & 0xF];
	while (mp) {
		if (mp->priority == skb_prio) {
			if (mp->vlan_qos && !vlan_qos)
				vlan->nr_egress_mappings--;
			else if (!mp->vlan_qos && vlan_qos)
				vlan->nr_egress_mappings++;
			mp->vlan_qos = vlan_qos;
			return 0;
		}
		mp = mp->next;
	}

	/* Create a new mapping then. */
	mp = vlan->egress_priority_map[skb_prio & 0xF];
	np = kmalloc(sizeof(struct vlan_priority_tci_mapping), GFP_KERNEL);
	if (!np)
		return -ENOBUFS;

	np->next = mp;
	np->priority = skb_prio;
	np->vlan_qos = vlan_qos;
	vlan->egress_priority_map[skb_prio & 0xF] = np;
	if (vlan_qos)
		vlan->nr_egress_mappings++;
	return 0;
}

/* Flags are defined in the vlan_flags enum in include/linux/if_vlan.h file. */
int vlan_dev_change_flags(const struct net_device *dev, u32 flags, u32 mask)
{
	struct vlan_dev_info *vlan = vlan_dev_info(dev);
	u32 old_flags = vlan->flags;

	if (mask & ~(VLAN_FLAG_REORDER_HDR | VLAN_FLAG_GVRP))
		return -EINVAL;

	vlan->flags = (old_flags & ~mask) | (flags & mask);

	if (netif_running(dev) && (vlan->flags ^ old_flags) & VLAN_FLAG_GVRP) {
		if (vlan->flags & VLAN_FLAG_GVRP)
			vlan_gvrp_request_join(dev);
		else
			vlan_gvrp_request_leave(dev);
	}
	return 0;
}

void vlan_dev_get_realdev_name(const struct net_device *dev, char *result)
{
	strncpy(result, vlan_dev_info(dev)->real_dev->name, 23);
}

static int vlan_dev_open(struct net_device *dev)
{
	struct vlan_dev_info *vlan = vlan_dev_info(dev);
	struct net_device *real_dev = vlan->real_dev;
	int err;

	if (!(real_dev->flags & IFF_UP))
		return -ENETDOWN;

	if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr)) {
		err = dev_unicast_add(real_dev, dev->dev_addr);
		if (err < 0)
			goto out;
	}

	if (dev->flags & IFF_ALLMULTI) {
		err = dev_set_allmulti(real_dev, 1);
		if (err < 0)
			goto del_unicast;
	}
	if (dev->flags & IFF_PROMISC) {
		err = dev_set_promiscuity(real_dev, 1);
		if (err < 0)
			goto clear_allmulti;
	}

	memcpy(vlan->real_dev_addr, real_dev->dev_addr, ETH_ALEN);

	if (vlan->flags & VLAN_FLAG_GVRP)
		vlan_gvrp_request_join(dev);

	netif_carrier_on(dev);
	return 0;

clear_allmulti:
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(real_dev, -1);
del_unicast:
	if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
		dev_unicast_delete(real_dev, dev->dev_addr);
out:
	netif_carrier_off(dev);
	return err;
}

#if 1 /* V54_BSP */
int vlan_dev_set_mgmt_priority(char* dev_name, int prio)
{
	struct net_device *dev = dev_get_by_name(&init_net, dev_name);
	int rv = 0;
	if (dev) {
		if (dev->priv_flags & IFF_802_1Q_VLAN) {
			vlan_dev_info(dev)->mgmt_priority = prio;
			rv = 0;
		} else {
			rv = -EINVAL;
		}
		dev_put(dev);
	} else {
		rv = -ENODEV;
	}
	return rv;
}
#endif

static int vlan_dev_stop(struct net_device *dev)
{
	struct vlan_dev_info *vlan = vlan_dev_info(dev);
	struct net_device *real_dev = vlan->real_dev;

	if (vlan->flags & VLAN_FLAG_GVRP)
		vlan_gvrp_request_leave(dev);

	dev_mc_unsync(real_dev, dev);
	dev_unicast_unsync(real_dev, dev);
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(real_dev, -1);
	if (dev->flags & IFF_PROMISC)
		dev_set_promiscuity(real_dev, -1);

	if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
		dev_unicast_delete(real_dev, dev->dev_addr);

	netif_carrier_off(dev);
	return 0;
}

static int vlan_dev_set_mac_address(struct net_device *dev, void *p)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	struct sockaddr *addr = p;
	int err;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	if (!(dev->flags & IFF_UP))
		goto out;

	if (compare_ether_addr(addr->sa_data, real_dev->dev_addr)) {
		err = dev_unicast_add(real_dev, addr->sa_data);
		if (err < 0)
			return err;
	}

	if (compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
		dev_unicast_delete(real_dev, dev->dev_addr);

out:
	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	return 0;
}

static int vlan_dev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	struct ifreq ifrr;
	int err = -EOPNOTSUPP;

	strncpy(ifrr.ifr_name, real_dev->name, IFNAMSIZ);
	ifrr.ifr_ifru = ifr->ifr_ifru;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		if (netif_device_present(real_dev) && ops->ndo_do_ioctl)
			err = ops->ndo_do_ioctl(real_dev, &ifrr, cmd);
		break;
	}

	if (!err)
		ifr->ifr_ifru = ifrr.ifr_ifru;

	return err;
}

static int vlan_dev_neigh_setup(struct net_device *dev, struct neigh_parms *pa)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int err = 0;

	if (netif_device_present(real_dev) && ops->ndo_neigh_setup)
		err = ops->ndo_neigh_setup(real_dev, pa);

	return err;
}

#if defined(CONFIG_FCOE) || defined(CONFIG_FCOE_MODULE)
static int vlan_dev_fcoe_ddp_setup(struct net_device *dev, u16 xid,
				   struct scatterlist *sgl, unsigned int sgc)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int rc = 0;

	if (ops->ndo_fcoe_ddp_setup)
		rc = ops->ndo_fcoe_ddp_setup(real_dev, xid, sgl, sgc);

	return rc;
}

static int vlan_dev_fcoe_ddp_done(struct net_device *dev, u16 xid)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int len = 0;

	if (ops->ndo_fcoe_ddp_done)
		len = ops->ndo_fcoe_ddp_done(real_dev, xid);

	return len;
}

static int vlan_dev_fcoe_enable(struct net_device *dev)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int rc = -EINVAL;

	if (ops->ndo_fcoe_enable)
		rc = ops->ndo_fcoe_enable(real_dev);
	return rc;
}

static int vlan_dev_fcoe_disable(struct net_device *dev)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int rc = -EINVAL;

	if (ops->ndo_fcoe_disable)
		rc = ops->ndo_fcoe_disable(real_dev);
	return rc;
}
#endif

static void vlan_dev_change_rx_flags(struct net_device *dev, int change)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;

	if (change & IFF_ALLMULTI)
		dev_set_allmulti(real_dev, dev->flags & IFF_ALLMULTI ? 1 : -1);
	if (change & IFF_PROMISC)
		dev_set_promiscuity(real_dev, dev->flags & IFF_PROMISC ? 1 : -1);
}

static void vlan_dev_set_rx_mode(struct net_device *vlan_dev)
{
	dev_mc_sync(vlan_dev_info(vlan_dev)->real_dev, vlan_dev);
	dev_unicast_sync(vlan_dev_info(vlan_dev)->real_dev, vlan_dev);
}

/*
 * vlan network devices have devices nesting below it, and are a special
 * "super class" of normal network devices; split their locks off into a
 * separate class since they always nest.
 */
static struct lock_class_key vlan_netdev_xmit_lock_key;
static struct lock_class_key vlan_netdev_addr_lock_key;

static void vlan_dev_set_lockdep_one(struct net_device *dev,
				     struct netdev_queue *txq,
				     void *_subclass)
{
	lockdep_set_class_and_subclass(&txq->_xmit_lock,
				       &vlan_netdev_xmit_lock_key,
				       *(int *)_subclass);
}

static void vlan_dev_set_lockdep_class(struct net_device *dev, int subclass)
{
	lockdep_set_class_and_subclass(&dev->addr_list_lock,
				       &vlan_netdev_addr_lock_key,
				       subclass);
	netdev_for_each_tx_queue(dev, vlan_dev_set_lockdep_one, &subclass);
}

static const struct header_ops vlan_header_ops = {
	.create	 = vlan_dev_hard_header,
	.rebuild = vlan_dev_rebuild_header,
	.parse	 = eth_header_parse,
};

static const struct net_device_ops vlan_netdev_ops, vlan_netdev_accel_ops;

static int vlan_dev_init(struct net_device *dev)
{
	struct net_device *real_dev = vlan_dev_info(dev)->real_dev;
	int subclass = 0;

	netif_carrier_off(dev);

	/* IFF_BROADCAST|IFF_MULTICAST; ??? */
	dev->flags  = real_dev->flags & ~(IFF_UP | IFF_PROMISC | IFF_ALLMULTI);
	dev->iflink = real_dev->ifindex;
	dev->state  = (real_dev->state & ((1<<__LINK_STATE_NOCARRIER) |
					  (1<<__LINK_STATE_DORMANT))) |
		      (1<<__LINK_STATE_PRESENT);

	dev->features |= real_dev->features & real_dev->vlan_features;
	dev->features |= NETIF_F_LLTX;
	dev->gso_max_size = real_dev->gso_max_size;

	/* ipv6 shared card related stuff */
	dev->dev_id = real_dev->dev_id;

	if (is_zero_ether_addr(dev->dev_addr))
		memcpy(dev->dev_addr, real_dev->dev_addr, dev->addr_len);
	if (is_zero_ether_addr(dev->broadcast))
		memcpy(dev->broadcast, real_dev->broadcast, dev->addr_len);

#if defined(CONFIG_FCOE) || defined(CONFIG_FCOE_MODULE)
	dev->fcoe_ddp_xid = real_dev->fcoe_ddp_xid;
#endif

	if (real_dev->features & NETIF_F_HW_VLAN_TX) {
		dev->header_ops      = real_dev->header_ops;
		dev->hard_header_len = real_dev->hard_header_len;
		dev->netdev_ops         = &vlan_netdev_accel_ops;
	} else {
		dev->header_ops      = &vlan_header_ops;
		dev->hard_header_len = real_dev->hard_header_len + VLAN_HLEN;
		dev->netdev_ops         = &vlan_netdev_ops;
	}

	if (is_vlan_dev(real_dev))
		subclass = 1;

	vlan_dev_set_lockdep_class(dev, subclass);
	return 0;
}

static void vlan_dev_uninit(struct net_device *dev)
{
	struct vlan_priority_tci_mapping *pm;
	struct vlan_dev_info *vlan = vlan_dev_info(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(vlan->egress_priority_map); i++) {
		while ((pm = vlan->egress_priority_map[i]) != NULL) {
			vlan->egress_priority_map[i] = pm->next;
			kfree(pm);
		}
	}
}

static int vlan_ethtool_get_settings(struct net_device *dev,
				     struct ethtool_cmd *cmd)
{
	const struct vlan_dev_info *vlan = vlan_dev_info(dev);
	return dev_ethtool_get_settings(vlan->real_dev, cmd);
}

static void vlan_ethtool_get_drvinfo(struct net_device *dev,
				     struct ethtool_drvinfo *info)
{
	strcpy(info->driver, vlan_fullname);
	strcpy(info->version, vlan_version);
	strcpy(info->fw_version, "N/A");
}

static u32 vlan_ethtool_get_rx_csum(struct net_device *dev)
{
	const struct vlan_dev_info *vlan = vlan_dev_info(dev);
	return dev_ethtool_get_rx_csum(vlan->real_dev);
}

static u32 vlan_ethtool_get_flags(struct net_device *dev)
{
	const struct vlan_dev_info *vlan = vlan_dev_info(dev);
	return dev_ethtool_get_flags(vlan->real_dev);
}

static const struct ethtool_ops vlan_ethtool_ops = {
	.get_settings	        = vlan_ethtool_get_settings,
	.get_drvinfo	        = vlan_ethtool_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_rx_csum		= vlan_ethtool_get_rx_csum,
	.get_flags		= vlan_ethtool_get_flags,
};

static const struct net_device_ops vlan_netdev_ops = {
	.ndo_change_mtu		= vlan_dev_change_mtu,
	.ndo_init		= vlan_dev_init,
	.ndo_uninit		= vlan_dev_uninit,
	.ndo_open		= vlan_dev_open,
	.ndo_stop		= vlan_dev_stop,
	.ndo_start_xmit =  vlan_dev_hard_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= vlan_dev_set_mac_address,
	.ndo_set_rx_mode	= vlan_dev_set_rx_mode,
	.ndo_set_multicast_list	= vlan_dev_set_rx_mode,
	.ndo_change_rx_flags	= vlan_dev_change_rx_flags,
	.ndo_do_ioctl		= vlan_dev_ioctl,
	.ndo_neigh_setup	= vlan_dev_neigh_setup,
#if defined(CONFIG_FCOE) || defined(CONFIG_FCOE_MODULE)
	.ndo_fcoe_ddp_setup	= vlan_dev_fcoe_ddp_setup,
	.ndo_fcoe_ddp_done	= vlan_dev_fcoe_ddp_done,
	.ndo_fcoe_enable	= vlan_dev_fcoe_enable,
	.ndo_fcoe_disable	= vlan_dev_fcoe_disable,
#endif
};

static const struct net_device_ops vlan_netdev_accel_ops = {
	.ndo_change_mtu		= vlan_dev_change_mtu,
	.ndo_init		= vlan_dev_init,
	.ndo_uninit		= vlan_dev_uninit,
	.ndo_open		= vlan_dev_open,
	.ndo_stop		= vlan_dev_stop,
	.ndo_start_xmit =  vlan_dev_hwaccel_hard_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= vlan_dev_set_mac_address,
	.ndo_set_rx_mode	= vlan_dev_set_rx_mode,
	.ndo_set_multicast_list	= vlan_dev_set_rx_mode,
	.ndo_change_rx_flags	= vlan_dev_change_rx_flags,
	.ndo_do_ioctl		= vlan_dev_ioctl,
	.ndo_neigh_setup	= vlan_dev_neigh_setup,
#if defined(CONFIG_FCOE) || defined(CONFIG_FCOE_MODULE)
	.ndo_fcoe_ddp_setup	= vlan_dev_fcoe_ddp_setup,
	.ndo_fcoe_ddp_done	= vlan_dev_fcoe_ddp_done,
	.ndo_fcoe_enable	= vlan_dev_fcoe_enable,
	.ndo_fcoe_disable	= vlan_dev_fcoe_disable,
#endif
};

void vlan_setup(struct net_device *dev)
{
	ether_setup(dev);

	dev->priv_flags		|= IFF_802_1Q_VLAN;
	dev->priv_flags		&= ~IFF_XMIT_DST_RELEASE;
	dev->tx_queue_len	= 0;

	dev->netdev_ops		= &vlan_netdev_ops;
	dev->destructor		= free_netdev;
	dev->ethtool_ops	= &vlan_ethtool_ops;

	memset(dev->broadcast, 0, ETH_ALEN);
}
