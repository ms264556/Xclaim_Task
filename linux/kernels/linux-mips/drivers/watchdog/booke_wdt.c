/*
 * Watchdog timer for PowerPC Book-E systems
 *
 * Author: Matthew McClintock
 * Maintainer: Kumar Gala <galak@kernel.crashing.org>
 *
 * Copyright 2005, 2008 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/smp.h>
#include <linux/miscdevice.h>
#include <linux/notifier.h>
#include <linux/watchdog.h>
#include <linux/uaccess.h>

#include <asm/reg_booke.h>
#include <asm/system.h>
#include <asm/time.h>
#include <asm/div64.h>
#include <linux/reboot.h>

/* If the kernel parameter wdt=1, the watchdog will be enabled at boot.
 * Also, the wdt_period sets the watchdog timer period timeout.
 * For E500 cpus the wdt_period sets which bit changing from 0->1 will
 * trigger a watchog timeout. This watchdog timeout will occur 3 times, the
 * first time nothing will happen, the second time a watchdog exception will
 * occur, and the final time the board will reset.
 */

#define WATCHDOG_DEBUG 0

#define P10XX_WATCHDOG_DEFAULT_S 10   /* seconds */
#define MAX_WD_TIMER_SECS 0xffffffff

u32 booke_wdt_enabled;
u32 booke_wdt_period;

static int expect_close = 0;


static unsigned int margin = P10XX_WATCHDOG_DEFAULT_S; /* WDT timeout value in seconds */
module_param(margin,int,0);
MODULE_PARM_DESC(margin, "Watchdog timeout value");

#ifdef CONFIG_WATCHDOG_NOWAYOUT
static int nowayout = 1;
#else
static int nowayout = 0;
#endif
module_param(nowayout,int,0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");

#ifdef	CONFIG_FSL_BOOKE
#define WDTP(x)		((((x)&0x3)<<30)|(((x)&0x3c)<<15))
#define WDTP_MASK	(WDTP(0x3f))
#else
#define WDTP(x)		(TCR_WP(x))
#define WDTP_MASK	(TCR_WP_MASK)
#endif

static DEFINE_SPINLOCK(booke_wdt_lock);

static int num_open = 0;	/* number of in progress open calls */

/* For the specified period, determine the number of seconds
 * corresponding to the reset time.  There will be a watchdog
 * exception at approximately 3/5 of this time.
 *
 * The formula to calculate this is given by:
 * 2.5 * (2^(63-period+1)) / timebase_freq
 *
 * In order to simplify things, we assume that period is
 * at least 1.  This will still result in a very long timeout.
 */
static unsigned long long period_to_sec(unsigned int period)
{
	unsigned long long tmp = 1ULL << (64 - period);
	unsigned long tmp2 = ppc_tb_freq;

	/* tmp may be a very large number and we don't want to overflow,
	 * so divide the timebase freq instead of multiplying tmp
	 */
	tmp2 = tmp2 / 5 * 2;

	do_div(tmp, tmp2);
	return tmp;
}

/*
 * This procedure will find the highest period which will give a timeout
 * greater than the one required. e.g. for a bus speed of 66666666 and
 * and a parameter of 2 secs, then this procedure will return a value of 38.
 */
static unsigned int sec_to_period(unsigned int secs)
{
#ifdef	CONFIG_FSL_BOOKE
	unsigned int period;
	for (period = 63; period > 0; period--) {
		if (period_to_sec(period) >= secs)
			return period;
	}
	return 0;
#else
    return secs;
#endif
}

static void __booke_wdt_ping(void *data)
{
	mtspr(SPRN_TSR, TSR_ENW|TSR_WIS);
}

static void booke_wdt_ping(void)
{
	on_each_cpu(__booke_wdt_ping, NULL, 0);
}

/*
 * Notify the watchdog that we're alive.
 * Reset the watchdog timer.
 */
void
watchdog_notify_alive(void)
{
    __booke_wdt_ping(NULL);
    touch_hw_wdt_ts();          // update timestamp to track last hw wd update
}
EXPORT_SYMBOL(watchdog_notify_alive);

static void __booke_wdt_enable(void *data)
{
	u32 val;

	/* clear status before enabling watchdog */
	__booke_wdt_ping(NULL);
	val = mfspr(SPRN_TCR);
	val &= ~WDTP_MASK;
	val |= (TCR_WIE|TCR_WRC(WRC_CHIP)|WDTP(booke_wdt_period));

	mtspr(SPRN_TCR, val);

    touch_hw_wdt_ts();
    touch_wdt_timestamp();      /* kernel softdog timestamp */
    _soft_wdt_stop(0);          /* mark the soft watchdog as started */
}

static void __booke_wdt_disable(void *data)
{
	u32 val;

	val = mfspr(SPRN_TCR);
	val &= ~(TCR_WIE | TCR_WRC_MASK | WDTP_MASK);
	mtspr(SPRN_TCR, val);

    printk("*** Hardware Watchdog stopped ***\n");
    _soft_wdt_stop(1);
}

static ssize_t
booke_wdt_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	char mybuf[512];
	char * bp = mybuf;

#if 0 /* _WATCHDOG_HISTO */
	len = _watchdog_dump_histo(&watchdog_notify_histo, bp, len);
#endif
	len += sprintf(bp+len, "\nWatchdog timeout value = %llu milliseconds\n",
                            period_to_sec(booke_wdt_period) * 1000
                            );
	len += sprintf(bp+len, "/dev/watchdog in use count = %d\n", num_open);

	copy_to_user(buf, bp,len);
	if ( *ppos ==  0 ) {
		*ppos += len;
		return len;
	} else {
		return 0;
	}
}

static int booke_wdt_release(struct inode *inode, struct file *file)
{
	spin_lock(&booke_wdt_lock);
	num_open--;
	if ((num_open == 0) && booke_wdt_enabled) {
	    if (expect_close) {
		    on_each_cpu(__booke_wdt_disable, NULL, 0);
		    booke_wdt_enabled = 0;
		    printk(KERN_INFO "PowerPC Book-E Watchdog Timer Disabled\n");
		} else {
		    printk("WDT device closed unexpectedly. WDT will not stop!\n");
		}
	}
	spin_unlock(&booke_wdt_lock);
}

static ssize_t booke_wdt_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
    if (count == 0) {
	    printk("%s: error: count=0\n", __FUNCTION__);
	    return -EIO;
	}

	if (!nowayout) {
		size_t i;
        int zcount = 0;
		/* In case it was set long ago */
		expect_close = 0;

		for (i = 0; i < count; i++) {
			char c;
			if (get_user(c, buf + i))
				return -EFAULT;
            switch (c) {
            case 'Z':
                zcount++;
                if ( zcount < 2 ) break;
				expect_close = 1;
#if WATCHDOG_DEBUG
                printk(KERN_DEBUG "%s: expect_close = %d\n",
                        __FUNCTION__, expect_close);
#endif
                break;
            case '2':
                { // set to MAX timer_value
                margin = MAX_WD_TIMER_SECS;
                booke_wdt_period = sec_to_period(margin);
                printk("%s: set timeout value to %llu\n", __FUNCTION__
                        , period_to_sec(booke_wdt_period) * 1000
                        );
                mtspr(SPRN_TCR, (mfspr(SPRN_TCR) & ~WDTP_MASK) |
						WDTP(booke_wdt_period));
                }
                break;
            case '6':
                margin = P10XX_WATCHDOG_DEFAULT_S;
                booke_wdt_period = sec_to_period(margin);
                mtspr(SPRN_TCR, (mfspr(SPRN_TCR) & ~WDTP_MASK) |
						WDTP(booke_wdt_period));
                printk("%s: reset timeout value to %d\n", __FUNCTION__, margin);
                break;
			case '0':
//				WATCHDOG_INIT_HISTO();
				break;
            default:
				break;
            }
			if ( zcount <= 0 ) {
				// only look at 1st character except ZZ
				break;
			}
		}
	}

    touch_wdt_timestamp();      /* kernel softdog timestamp */
	booke_wdt_ping();
	touch_hw_wdt_ts();
	return count;
}

static struct watchdog_info ident = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = "PowerPC Book-E Watchdog",
};

static long booke_wdt_ioctl(struct file *file,
				unsigned int cmd, unsigned long arg)
{
	u32 tmp = 0;
	u32 __user *p = (u32 __user *)arg;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		if (copy_to_user((void *)arg, &ident, sizeof(ident)))
			return -EFAULT;
	case WDIOC_GETSTATUS:
		return put_user(ident.options, p);
	case WDIOC_GETBOOTSTATUS:
		/* XXX: something is clearing TSR */
		tmp = mfspr(SPRN_TSR) & TSR_WRS(3);
		/* returns 1 if last reset was caused by the WDT */
		return (tmp ? 1 : 0);
	case WDIOC_SETOPTIONS:
		if (get_user(tmp, p))
			return -EINVAL;
		if (tmp == WDIOS_ENABLECARD) {
			booke_wdt_ping();
			break;
		} else
			return -EINVAL;
		return 0;
	case WDIOC_KEEPALIVE:
		booke_wdt_ping();
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(tmp, p))
			return -EFAULT;
#ifdef	CONFIG_FSL_BOOKE
		/* period of 1 gives the largest possible timeout */
		if (tmp > period_to_sec(1))
			return -EINVAL;
#endif
        margin = tmp;
		booke_wdt_period = sec_to_period(margin);
		mtspr(SPRN_TCR, (mfspr(SPRN_TCR) & ~WDTP_MASK) |
						WDTP(booke_wdt_period));
		return 0;
	case WDIOC_GETTIMEOUT:
		return put_user(booke_wdt_period, p);
	default:
		return -ENOTTY;
	}

	return 0;
}

static int booke_wdt_open(struct inode *inode, struct file *file)
{

	spin_lock(&booke_wdt_lock);
	num_open++;
	if (booke_wdt_enabled == 0) {
		booke_wdt_enabled = 1;
		booke_wdt_period = sec_to_period(margin);
		on_each_cpu(__booke_wdt_enable, NULL, 0);
		printk(KERN_INFO
		      "booke_wdt_open: PowerPC Book-E Watchdog Timer Enabled (wdt_period=%d, period_to_sec=%llu)\n",
				booke_wdt_period, period_to_sec(booke_wdt_period));
	}
	spin_unlock(&booke_wdt_lock);

	return nonseekable_open(inode, file);
}

/*
 *	Notifier for system down
 */
static int
booke_wdt_notify_sys(struct notifier_block *this, unsigned long code,
	void *unused)
{
        // include SYS_POWER_OFF in the check. halt is now mapped to power_off
	if (code == SYS_DOWN || code == SYS_HALT || code == SYS_POWER_OFF) {
		/* Turn the WDT off */
		on_each_cpu(__booke_wdt_disable, NULL, 0);
	}
	return NOTIFY_DONE;
}

static const struct file_operations booke_wdt_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read  = booke_wdt_read,
	.write = booke_wdt_write,
	.unlocked_ioctl = booke_wdt_ioctl,
	.open = booke_wdt_open,
	.release = booke_wdt_release,
};

static struct miscdevice booke_wdt_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &booke_wdt_fops,
};

/*
 *	The WDT needs to learn about soft shutdowns in order to
 *	turn the watchdog registers off.
 */
static struct notifier_block booke_wdt_notifier = {
	booke_wdt_notify_sys,
	NULL,
	0
};

static void __exit booke_wdt_exit(void)
{
	misc_deregister(&booke_wdt_miscdev);
	unregister_reboot_notifier(&booke_wdt_notifier);
}

static int __init booke_wdt_init(void)
{
	int ret = 0;

	printk(KERN_INFO "PowerPC Book-E Watchdog Timer Loaded\n");
	ident.firmware_version = cur_cpu_spec->pvr_value;

	ret = misc_register(&booke_wdt_miscdev);
	if (ret) {
		printk(KERN_CRIT "Cannot register miscdev on minor=%d: %d\n",
				WATCHDOG_MINOR, ret);
		return ret;
	}

	register_reboot_notifier(&booke_wdt_notifier);

    booke_wdt_period = sec_to_period(margin);
	spin_lock(&booke_wdt_lock);
	if (booke_wdt_enabled == 1) {
		printk(KERN_INFO
		      "PowerPC Book-E Watchdog Timer Enabled (wdt_period=%d, period_to_sec=%llu)\n",
				booke_wdt_period, period_to_sec(booke_wdt_period));
		on_each_cpu(__booke_wdt_enable, NULL, 0);
	}
	spin_unlock(&booke_wdt_lock);

	return ret;
}
device_initcall(booke_wdt_init);
