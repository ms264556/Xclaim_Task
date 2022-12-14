/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2007 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

//#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#if !defined(V54_BSP)
#include "ag7240_phy.h"
#include "ag7240.h"
#include "eth_diag.h"
#else
#include "ag7100_phy.h"
#include "ag7100.h"
#include "athrs16_phy.h"
#include "athr_phy.h"
#endif

#if defined(CONFIG_VENETDEV)
#define HEADER_EN   1
#endif

/* PHY selections and access functions */
typedef enum {
    PHY_SRCPORT_INFO, 
    PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum {
    PHY_SRCPORT_NONE,
    PHY_SRCPORT_VLANTAG, 
    PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#define DRV_DEBUG 0
#if DRV_DEBUG
#define DRV_DEBUG_PHYSETUP  0x00000001
#define DRV_DEBUG_PHYCHANGE 0x00000002
#define DRV_DEBUG_PHYSTATUS 0x00000004

int PhyDebug = DRV_DEBUG_PHYSETUP;

#define DRV_PRINT(FLG, X)                           \
{                                                   \
    if (PhyDebug & (FLG)) {                      \
        printk X;                                   \
    }                                               \
}
#else
#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)
#endif

#define ATHR_LAN_PORT_VLAN          1
#define ATHR_WAN_PORT_VLAN          2


/*depend on connection between cpu mac and s16 mac*/
#if !defined(V54_BSP)
#if defined (CONFIG_PORT0_AS_SWITCH)
#define ENET_UNIT_LAN 0  
#define ENET_UNIT_WAN 1
#define CFG_BOARD_AP96 1

#elif defined (CONFIG_AG7240_GE0_IS_CONNECTED)
#define ENET_UNIT_LAN 0  
#define CFG_BOARD_PB45 0
#define CFG_BOARD_AP96 1

#else
#define ENET_UNIT_LAN 1  
#define ENET_UNIT_WAN 0
#define CFG_BOARD_PB45 1
#endif
#else
#define ENET_UNIT_LAN 0  
#define CFG_BOARD_PB45 0
#define CFG_BOARD_AP96 0
#endif


#define TRUE    1
#define FALSE   0

#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4
#define ATHR_IND_PHY 4

#define MODULE_NAME "ATHRS16"
#if !defined(V54_BSP)
extern int xmii_val;
extern int ag7240_open(struct net_device *dev);
extern int ag7240_stop(struct net_device *dev);
extern ag7240_mac_t *ag7240_macs[2];
extern char *dup_str[];
extern int  ag7242_check_link(ag7240_mac_t *mac);
#endif

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    uint32_t phyBase;
    uint32_t phyAddr;          /* PHY registers associated with this phy port */
    uint32_t VLANTableSetting; /* Value to be written to VLAN table */
} athrPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number
 * MAC port 0 - CPU port 0x100
 */
static athrPhyInfo_t athrPhyInfo[] = {
    {TRUE,   /* phy port 0 -- MAC port 1 0x200 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY0_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- MAC port 2 0x300 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY1_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- MAC port 3 0x400 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY2_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- MAC port 4 0x500 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY3_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or MAC port 5 0x600 */
     FALSE,
     ENET_UNIT_LAN,//ENET_UNIT_WAN,
     0,
     ATHR_PHY4_ADDR, 
     ATHR_LAN_PORT_VLAN   /* Send to all ports */
    },
    
    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_LAN,
     0,
     0x00, 
     ATHR_LAN_PORT_VLAN    /* Send to all ports */
    },
};

static uint8_t athr16_init_flag = 0;

//#define ATHR_PHY_MAX (sizeof(ipPhyInfo) / sizeof(ipPhyInfo[0]))
#if 1 // pb92 #if !defined(V54_BSP)
#define ATHR_PHY_MAX 5
#else
#define ATHR_PHY_MAX 4  /* 4 lan ports */
#endif

/* Range of valid PHY IDs is [MIN..MAX] */
#define ATHR_ID_MIN 0
#define ATHR_ID_MAX (ATHR_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define ATHR_IS_ENET_PORT(phyUnit) (athrPhyInfo[phyUnit].isEnetPort)
#define ATHR_IS_PHY_ALIVE(phyUnit) (athrPhyInfo[phyUnit].isPhyAlive)
#define ATHR_ETHUNIT(phyUnit) (athrPhyInfo[phyUnit].ethUnit)
#define ATHR_PHYBASE(phyUnit) (athrPhyInfo[phyUnit].phyBase)
#define ATHR_PHYADDR(phyUnit) (athrPhyInfo[phyUnit].phyAddr)
#define ATHR_VLAN_TABLE_SETTING(phyUnit) (athrPhyInfo[phyUnit].VLANTableSetting)


#define ATHR_IS_ETHUNIT(phyUnit, ethUnit) \
            (ATHR_IS_ENET_PORT(phyUnit) &&        \
            ATHR_ETHUNIT(phyUnit) == (ethUnit))

#define ATHR_IS_WAN_PORT(phyUnit) (!(ATHR_ETHUNIT(phyUnit)==ENET_UNIT_LAN))
            
/* Forward references */
BOOL athrs16_phy_is_link_alive(int phyUnit);
static uint32_t athrs16_reg_read(uint32_t reg_addr);
static void athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val);
static void phy_mode_setup(void);

static void phy_mode_setup(void) 
{
#ifdef ATHRS16_VER_1_0
    printk("phy_mode_setup\n");

    /*work around for phy4 rgmii mode*/
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 18);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x480c);    

    /*rx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 0);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x824e);  

    /*tx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 5);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x3d47);    
#endif
}
void athrs16_force_100M(int phy ,int duplex)
{
    phy_reg_write(ATHR_PHYBASE(phy),ATHR_PHYADDR(phy),
                     ATHR_PHY_CONTROL,(0xa000|(duplex << 8)));
}
void athrs16_force_10M(int phy,int duplex)
{
    phy_reg_write(ATHR_PHYBASE(phy),ATHR_PHYADDR(phy),
                     ATHR_PHY_CONTROL,(0x8000 |(duplex << 8)));
}


void athrs16_reg_init()
{
    /* if using header for register configuration, we have to     */
    /* configure s16 register after frame transmission is enabled */
    if (athr16_init_flag)
        return;

    /*Power on strip mode setup*/
#if CFG_BOARD_PB45
    athrs16_reg_write(0x208, 0x2fd0001);  /*tx delay*/   
    athrs16_reg_write(0x108, 0x2be0001);  /*mac0 rgmii mode*/ 
#elif CFG_BOARD_AP96
    //athrs16_reg_write(0x8, 0x012e1bea);
    athrs16_reg_write(0x8, 0x01261be2);
#endif
    
#if 0 // pb92 #if defined(V54_BSP)
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: mask ctrl = 0x%x\n", __func__, athrs16_reg_read(S16_MASK_CRTL_REG)));

    /* mac0 */
    athrs16_reg_write(S16_OP_MODE_REG0, 0x07400000);
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: op mode 0 = 0x%x\n", __func__, athrs16_reg_read(S16_OP_MODE_REG0)));

    /* mac6 */
    athrs16_reg_write(S16_OP_MODE_REG2, 0x07a00000);
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: op mode 2 = 0x%x\n", __func__, athrs16_reg_read(S16_OP_MODE_REG2)));

    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: pwr on strap = 0x%x\n", __func__, athrs16_reg_read(S16_PWR_ON_STRAP_REG)));

    /* max frame size=1518+8=0x05f6 */
    athrs16_reg_write(S16_GLOBAL_CTRL_REG, ((athrs16_reg_read(S16_GLOBAL_CTRL_REG) & 0x3fff) | 0x05f6));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: global ctrl = 0x%x\n", __func__, athrs16_reg_read(S16_GLOBAL_CTRL_REG)));

    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_0)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_1)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_2)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_3)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_4)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_5)));
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("%s: port-based vlan 2 = 0x%x\n", __func__, athrs16_reg_read(S16_PORT_BASE_VLAN_REG2_6)));

    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_0, 0x005e0000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_1, 0x001d0000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_2, 0x001b0000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_3, 0x00170000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_4, 0x000f0000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_5, 0x00000000);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REG2_6, 0x00010000);
#endif

    athrs16_reg_write(S16_PORT_STATUS_REGISTER0, 0x7e);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER1, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER2, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER3, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER4, 0x200);
#if CFG_BOARD_PB45
    athrs16_reg_write(0x600, 0x200);
    printk("CFG Board PB45\n");
#elif CFG_BOARD_AP96
    //athrs16_reg_write(0x600, 0x0);
    printk("CFG board AP96\n");
    athrs16_reg_write(0x600, 0x200);
#endif

#if 0 // pb92 #if defined(V54_BSP)
    athrs16_reg_write(S16_PORT_STATUS_REGISTER5, 0);    // mac5 disabled
    athrs16_reg_write(S16_PORT_STATUS_REGISTER6, 0x7e); // mac6: AR8021
#endif

#if 1 // pb92 #if !defined(V54_BSP)
    athrs16_reg_write(S16_FLD_MASK_REG,0x003f003f);
#else
    athrs16_reg_write(S16_FLD_MASK_REG, 0xfe7f007f);
    athrs16_reg_write(S16_ARL_TBL_CTRL_REG,
        (athrs16_reg_read(S16_ARL_TBL_CTRL_REG) | S16_LEARN_CHANGE_EN));
#endif

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)        
#ifdef HEADER_EN        
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x6804);
#else
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x6004);
#endif

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4, 0x6004);    
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5, 0x6004);    
#else
#ifdef HEADER_EN        
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x4804);
#else
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x4004);
#endif
#endif

#ifdef FULL_FEATURE
	hsl_dev_init(0, 2);
#endif
#if 0
   /* Enable ARP packets to CPU port */
    athrs16_reg_write(S16_ARL_TBL_CTRL_REG,(athrs16_reg_read(S16_ARL_TBL_CTRL_REG) | 0x100000));

   /* Enable Broadcast packets to CPU port */
    athrs16_reg_write(S16_FLD_MASK_REG,(athrs16_reg_read(S16_FLD_MASK_REG) | S16_ENABLE_CPU_BROADCAST ));
#endif
#if 0

    /* Connect Port0 to CPU */
    athrs16_reg_write(S16_CPU_PORT_REGISTER,0x000001f0);

    /* Recognize tag packet from CPU */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER0,0xc03e0001);
#endif

#if 0 /*ndef CONFIG_S16_SWITCH_ONLY_MODE*/

    /* Insert PVID 1 to LAN ports */
#ifndef CONFIG_S16_SWAP_WAN
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x001b0001);
#else
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x00010001);
#endif
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER1,0x001d0001);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER3,0x00170001);
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER4,0x000f0001);

   /* Insert PVID 2 to WAN port */
#ifndef CONFIG_S16_SWAP_WAN
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x00010002);
#else
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x001b0002);
#endif

   /* Egress tag packet to CPU and untagged packet to LAN port */

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0,0x00006204);

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5,0x00006104);

#ifndef CONFIG_S16_SWAP_WAN
  /* Group port - 0,1,2,3,4 to VID 1 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x0000081f);
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0001000a);

  /*  port - 0 and 5  to VID 2 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x00000821);
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0002000a);
#else
  /* Group port - 0,1,3,4,5 to VID 1 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x0000083B);
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0001000a);

  /*  port - 0 and 2  to VID 2 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x00000805);
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0002000a);
#endif

#endif
    athr16_init_flag = 1;
#if !defined(V54_BSP)
    printk("athrs16_reg_init complete*\n");
#else
    DRV_PRINT(DRV_DEBUG_PHYSETUP,
              ("athrs16_reg_init complete*\n"));
#endif
}

/******************************************************************************
*
* athrs16_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
athrs16_phy_is_link_alive(int phyUnit)
{
    uint16_t phyHwStatus;
    uint32_t phyBase;
    uint32_t phyAddr;

    phyBase = ATHR_PHYBASE(phyUnit);
    phyAddr = ATHR_PHYADDR(phyUnit);

    phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

    if (phyHwStatus & ATHR_STATUS_LINK_PASS)
        return TRUE;

    return FALSE;
}

/******************************************************************************
*
* athrs16_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
athrs16_phy_setup(int ethUnit)
{
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint16_t  timeout;
    int       liveLinks = 0;
    uint32_t  phyBase = 0;
    BOOL      foundPhy = FALSE;
    uint32_t  phyAddr = 0;
    

    /* See if there's any configuration data for this enet */
    /* start auto negogiation on each phy */
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        foundPhy = TRUE;
        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        
        phy_reg_write(phyBase, phyAddr, ATHR_AUTONEG_ADVERT,
                      ATHR_ADVERTISE_ALL);

#if 1 // pb92 #if !defined(V54_BSP)   /* ar8228: 4 10/100mbps lan ports  */
        phy_reg_write(phyBase, phyAddr, ATHR_1000BASET_CONTROL,
                      ATHR_ADVERTISE_1000FULL);
#endif

        /* Reset PHYs*/
        phy_reg_write(phyBase, phyAddr, ATHR_PHY_CONTROL,
                      ATHR_CTRL_AUTONEGOTIATION_ENABLE 
                      | ATHR_CTRL_SOFTWARE_RESET);

    }

    if (!foundPhy) {
        return FALSE; /* No PHY's configured for this ethUnit */
    }

    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    mdelay(1000);


    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    for (phyUnit=0; (phyUnit < ATHR_PHY_MAX) /*&& (timeout > 0) */; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        timeout=20;
        for (;;) {
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_CONTROL);

            if (ATHR_RESET_DONE(phyHwStatus)) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Neg Success\n", phyUnit));
                break;
            }
            if (timeout == 0) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Negogiation timeout\n", phyUnit));
                break;
            }
            if (--timeout == 0) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Negogiation timeout\n", phyUnit));
                break;
            }

            mdelay(150);
        }
    }

    /*
     * All PHYs have had adequate time to autonegotiate.
     * Now initialize software status.
     *
     * It's possible that some ports may take a bit longer
     * to autonegotiate; but we can't wait forever.  They'll
     * get noticed by mv_phyCheckStatusChange during regular
     * polling activities.
     */
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }
#if 0
	/* Enable RGMII */
	phy_reg_write(0,phyUnit,0x1d,0x12);
	phy_reg_write(0,phyUnit,0x1e,0x8);
	/* Tx delay on PHY */
	phy_reg_write(0,phyUnit,0x1d,0x5);
	phy_reg_write(0,phyUnit,0x1e,0x100);
        
	/* Rx delay on PHY */
	phy_reg_write(0,phyUnit,0x1d,0x0);
	phy_reg_write(0,phyUnit,0x1e,0x8000);
#endif
        if (athrs16_phy_is_link_alive(phyUnit)) {
            liveLinks++;
            ATHR_IS_PHY_ALIVE(phyUnit) = TRUE;
        } else {
            ATHR_IS_PHY_ALIVE(phyUnit) = FALSE;
        }

        DRV_PRINT(DRV_DEBUG_PHYSETUP,
            ("eth%d: Phy Specific Status=%4.4x\n",
            ethUnit, 
            phy_reg_read(ATHR_PHYBASE(phyUnit),
                         ATHR_PHYADDR(phyUnit),
                         ATHR_PHY_SPEC_STATUS)));
    }
    phy_mode_setup();    

#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
    /* ar8021 */
    athr_phy_setup(ethUnit);
#endif

    return (liveLinks > 0);
}

/******************************************************************************
*
* athrs16_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1 --> FULL
*    0 --> HALF
*/
int
athrs16_phy_is_fdx(int ethUnit)
{
    int       phyUnit;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    uint16_t  phyHwStatus;
#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
    int       fdx=0;
    static int  prevFdx;
#else
    int       ii = 200;

    if (ethUnit == ENET_UNIT_LAN)
        return TRUE;
#endif

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (athrs16_phy_is_link_alive(phyUnit)) {

            phyBase = ATHR_PHYBASE(phyUnit);
            phyAddr = ATHR_PHYADDR(phyUnit);

#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
            phyHwStatus = phy_reg_read (phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);
            if ((phyHwStatus & ATHR_STATUS_RESOVLED) && (phyHwStatus & ATHER_STATUS_FULL_DUPLEX)) {
                fdx |= (1 << phyUnit);
            }
#else
            do {
                phyHwStatus = phy_reg_read (phyBase, phyAddr, 
                                               ATHR_PHY_SPEC_STATUS);
        		if(phyHwStatus & ATHR_STATUS_RESOVLED)
			    break;
                mdelay(10);
            } while(--ii);
            
            if (phyHwStatus & ATHER_STATUS_FULL_DUPLEX) {
                return TRUE;
            }
#endif
        }
    }
#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
    if (athr_phy_is_fdx(0)) {
        fdx |= 0x10;
        if (!prevFdx) {
            athrs16_reg_write(S16_PORT_STATUS_REGISTER6,
                (athrs16_reg_read(S16_PORT_STATUS_REGISTER6) | PORT_STATUS_DUPLEX_MASK));
            prevFdx = 1;
        }
    } else {
        if (prevFdx) {
            athrs16_reg_write(S16_PORT_STATUS_REGISTER6,
                (athrs16_reg_read(S16_PORT_STATUS_REGISTER6) & ~PORT_STATUS_DUPLEX_MASK));
            prevFdx = 0;
        }
    }
    return fdx;
#else
    return FALSE;
#endif
}

#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
/******************************************************************************
*
* athrs16_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7240_PHY_SPEED_10T, AG7240_PHY_SPEED_100TX;
*               AG7240_PHY_SPEED_1000T;
*/

int
athrs16_phy_speed(int ethUnit)
{
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
#if !defined(V54_BSP)
    int       ii = 200;
    ag7240_phy_speed_t phySpeed = 0;
#else
    int       ii = 1;
    int       phySpeed = 0;
    static int  prevSpeed;
    int         newSpeed;
#endif

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
#if !defined(V54_BSP)
        phySpeed = AG7100_PHY_SPEED_10T;
#endif

        if (athrs16_phy_is_link_alive(phyUnit)) {

            do {
                phyHwStatus = phy_reg_read(phyBase, phyAddr, 
                                              ATHR_PHY_SPEC_STATUS);
		        if(phyHwStatus & ATHR_STATUS_RESOVLED)
			        break;
#if !defined(V54_BSP)
                mdelay(10);
#endif
            }while(--ii);
            
            phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
                           ATHER_STATUS_LINK_SHIFT);

#if !defined(V54_BSP)
            switch(phyHwStatus) {
            case 0:
                phySpeed = AG7240_PHY_SPEED_10T;
                break;
            case 1:
                phySpeed = AG7240_PHY_SPEED_100TX;
                break;
            case 2:
                phySpeed = AG7240_PHY_SPEED_1000T;
                break;
            default:
                printk("Unkown speed read!\n");
            }
#else
            switch(phyHwStatus) {
            case 0:
                phySpeed |= (AG7100_PHY_SPEED_10T << (phyUnit << 1));
                break;
            case 1:
                phySpeed |= (AG7100_PHY_SPEED_100TX << (phyUnit << 1));
                break;
            case 2:
                phySpeed |= (AG7100_PHY_SPEED_1000T << (phyUnit << 1));
                break;
            default:
                printk("Unkown speed read!\n");
            }
#endif
        } 

#if !defined(V54_BSP)
        phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_ADDRESS, 0x18);
        
        if(phySpeed == AG7240_PHY_SPEED_100TX) {
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0xba8);
        } else {            
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0x2ea);
        }
#endif
    }

#if !defined(V54_BSP)
    if (ethUnit == ENET_UNIT_LAN)
         phySpeed = AG7240_PHY_SPEED_1000T;
#else
    newSpeed = athr_phy_speed(0);
    phySpeed |= ((newSpeed & 3) << 8);
    if (newSpeed != prevSpeed) {
        athrs16_reg_write(S16_PORT_STATUS_REGISTER6,
            ((athrs16_reg_read(S16_PORT_STATUS_REGISTER6) & ~PORT_STATUS_SPEED_MASK) | newSpeed));
        prevSpeed = newSpeed;
    }
#endif

    return phySpeed;
}
#else
/******************************************************************************
*
* athrs16_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7100_PHY_SPEED_1000T;
*/

int
athrs16_phy_speed(int ethUnit)
{
    return AG7100_PHY_SPEED_1000T;
}
#endif

/*****************************************************************************
*
* athr_phy_is_up -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
athrs16_phy_is_up(int ethUnit)
{
    int           phyUnit;
    uint16_t      phyHwStatus, phyHwControl;
    athrPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t      phyBase;
    uint32_t      phyAddr;
#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
    int           phyUp=0;
    static BOOL   lastPortAlive;
#endif

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);

        lastStatus = &athrPhyInfo[phyUnit];

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

            /* See if we've lost link */
            if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
                linkCount++;
#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
                phyUp |= (1 << phyUnit);
#endif
            } else {
                lostLinks++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d down\n",
                                               ethUnit, phyUnit));
                printk("\nport%d down\n", phyUnit);
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */
            /* Check for reset complete */
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_STATUS);
            if (!ATHR_RESET_DONE(phyHwStatus))
                continue;

            phyHwControl = phy_reg_read(phyBase, phyAddr, ATHR_PHY_CONTROL);
            /* Check for AutoNegotiation complete */            
            if ((!(phyHwControl & ATHR_CTRL_AUTONEGOTIATION_ENABLE)) 
                 || ATHR_AUTONEG_DONE(phyHwStatus)) {
                phyHwStatus = phy_reg_read(phyBase, phyAddr, 
                                           ATHR_PHY_SPEC_STATUS);

                if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
                gainedLinks++;
                linkCount++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d up\n",
                                               ethUnit, phyUnit));
                printk("\nport%d up\n", phyUnit);
                lastStatus->isPhyAlive = TRUE;
#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
                phyUp |= (1 << phyUnit);
#endif
                }
            }
        }
    }

#if defined(V54_BSP) && defined(CONFIG_VENETDEV)
    /* ar8021 */
    if (athr_phy_is_up(ethUnit)) {
        phyUp |= 0x10;
        if (!lastPortAlive) {
            printk("\nport%d up\n", 4);
        }
        lastPortAlive = 1;
    } else {
        if (lastPortAlive) {
            printk("\nport%d down\n", 4);
        }
        lastPortAlive = 0;
    }
    return (phyUp);
#else
   return (linkCount);
#endif

}

static uint32_t
athrs16_reg_read(uint32_t reg_addr)
{
    uint32_t reg_word_addr;
    uint32_t phy_addr, tmp_val, reg_val;
    uint16_t phy_val;
    uint8_t phy_reg;

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (uint16_t) ((reg_word_addr >> 8) & 0x1ff);  /* bit16-8 of reg address */
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* For some registers such as MIBs, since it is read/clear, we should */
    /* read the lower 16-bit register then the higher one */

    /* read register in lower address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    reg_val = (uint32_t) phy_reg_read(0, phy_addr, phy_reg);

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    tmp_val = (uint32_t) phy_reg_read(0, phy_addr, phy_reg);
    reg_val |= (tmp_val << 16);

    return reg_val;   
}

static void
athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val)
{
    uint32_t reg_word_addr;
    uint32_t phy_addr;
    uint16_t phy_val;
    uint8_t phy_reg;

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (uint16_t) ((reg_word_addr >> 8) & 0x1ff);  /* bit16-8 of reg address */
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* For some registers such as ARL and VLAN, since they include BUSY bit */
    /* in lower address, we should write the higher 16-bit register then the */
    /* lower one */

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val = (uint16_t) ((reg_val >> 16) & 0xffff);
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* write register in lower address */
    reg_word_addr--;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val = (uint16_t) (reg_val & 0xffff);
    phy_reg_write(0, phy_addr, phy_reg, phy_val); 
}

#if !defined(V54_BSP)
int
athrs16_ioctl(uint32_t *args, int cmd)
{
    struct eth_diag *etd =(struct eth_diag *) args;

    if(cmd  == ATHR_RD_PHY) {
        if(etd->ed_u.portnum != 0xf)
            etd->val = phy_reg_read(0,etd->ed_u.portnum,etd->phy_reg);
        else
            etd->val = athrs16_reg_read(etd->phy_reg);
    }
    else if(cmd  == ATHR_WR_PHY) {
        if(etd->ed_u.portnum != 0xf)
            phy_reg_write(0,etd->ed_u.portnum,etd->phy_reg,etd->val);
        else
            athrs16_reg_write(etd->phy_reg,etd->val);
    }
    else if(cmd == ATHR_FORCE_PHY) {
         if(etd->phy_reg < ATHR_PHY_MAX) {
            if(etd->val == 10) {
	       printk("Forcing 10Mbps %s on port:%d \n",
                         dup_str[etd->ed_u.duplex],(etd->phy_reg));
               athrs16_force_10M(etd->phy_reg,etd->ed_u.duplex);
            }
            else if(etd->val == 100) {
	       printk("Forcing 100Mbps %s on port:%d \n",
                         dup_str[etd->ed_u.duplex],(etd->phy_reg));
               athrs16_force_100M(etd->phy_reg,etd->ed_u.duplex);
            }
	    else if(etd->val == 0) {
	       printk("Enabling Auto Neg on port:%d \n",(etd->phy_reg));
               phy_reg_write(0,etd->phy_reg,ATHR_PHY_CONTROL,0x9000);
            }
            else
               return -EINVAL;
            ag7242_check_link(ag7240_macs[0]);
        }
        else {
            return -EINVAL;
        }
    }
    else
        return -EINVAL;

    return 0;
}
#endif


