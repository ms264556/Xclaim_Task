/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc.
 * 	Dave Liu <daveliu@freescale.com>
 *
 * The cpufreq driver is for Freescale 85xx processor,
 * based on arch/powerpc/platforms/cell/cbe_cpufreq.c
 * (C) Copyright IBM Deutschland Entwicklung GmbH 2005-2007
 *	Christian Krafft <krafft@de.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/cpufreq.h>
#include <linux/of_platform.h>

#include <asm/prom.h>
#include <asm/time.h>
#include <asm/reg.h>
#include <asm/io.h>

#include <sysdev/fsl_soc.h>

static DEFINE_MUTEX(mpc85xx_switch_mutex);

static __be32 __iomem *porpllsr;
static __be32 __iomem *pmjcr;
static __be32 __iomem *powmgtcsr;

#define PORPLLSR	0xe0000
#define PMJCR		0xe007c
#define POWMGTCSR	0xe0080

/* e500 core frequence is ratio * system bus freq (CCB_CLK)
static struct core_ratio {
	ulong pll;
	char *ratio;
} e500_ratio[] = {
	{2, "1:1"},
	{3, "3:2"},
	{4, "2:1"},
	{5, "5:2"},
	{6, "3:1"},
	{7, "7:2"},
	{8, "4:1"},
};
*/
static struct cpufreq_frequency_table mpc85xx_freqs[] = {
	{2,	0},
	{3,	0},
	{4,	0},
	{5,	0},
	{6,	0},
	{7,	0},
	{8,	0},
	{0,	CPUFREQ_TABLE_END},
};

/*
 * hardware specific functions
 */
static int get_pll(int cpu)
{
	int ret, shift;
	u32 pll = in_be32(porpllsr);
	shift = (cpu == 1) ? 24 : 16;
	ret = (pll >> shift) & 0x3f;

	return ret;
}

static void set_pll(unsigned int pll, int cpu)
{
	int shift;
	u32 busfreq, corefreq, val;
	u32 core_spd, mask, tmp;

	tmp = in_be32(pmjcr);
	shift = (cpu == 1) ? 24 : 16;
	busfreq = fsl_get_sys_freq();
	val = (pll & 0x3f) << shift;

	corefreq = ((busfreq * pll) >> 1);
	/* must set the bit[18/19] if the requested core freq > 533 MHz */
	core_spd = (cpu == 1) ? 0x00002000 : 0x00001000;
	if (corefreq > 533000000)
		val |= core_spd;

	mask = (cpu == 1) ? 0x3f002000 : 0x003f1000;
	tmp &= ~mask;
	tmp |= val;
	out_be32(pmjcr, tmp);
	val = in_be32(pmjcr);
	out_be32(powmgtcsr, 0x00600000);
	printk("PMJCR request %08x at CPU %d\n", tmp, cpu);
}

static void verify_pll(int cpu)
{
	int shift;
	u32 busfreq, pll, corefreq;

	shift = (cpu == 1) ? 24 : 16;
	busfreq = fsl_get_sys_freq();
	pll = (in_be32(porpllsr) >> shift) & 0x3f;

	corefreq = (busfreq * pll) >> 1;
	corefreq /= 1000000;
	printk("PORPLLSR core freq %dMHz at CPU %d\n", corefreq, cpu);
}

/*
 * cpufreq functions
 */

static int mpc85xx_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	u32 busfreq = fsl_get_sys_freq();
	int i, cur_pll;

	/* we need the freq unit with kHz */
	busfreq /= 1000;

	/* initialize frequency table */
	for (i = 0; mpc85xx_freqs[i].frequency != CPUFREQ_TABLE_END; i++) {
		mpc85xx_freqs[i].frequency = (busfreq * mpc85xx_freqs[i].index) >> 1;
		printk("%d: %dkHz\n", i, mpc85xx_freqs[i].frequency);
	}

	/* the latency of a transition, the unit is ns */
	policy->cpuinfo.transition_latency = 2000;

	cur_pll = get_pll(policy->cpu);
	pr_debug("current pll is at %d\n", cur_pll);

	for (i = 0; mpc85xx_freqs[i].frequency != CPUFREQ_TABLE_END; i++) {
		if (mpc85xx_freqs[i].index == cur_pll)
			policy->cur = mpc85xx_freqs[i].frequency;
	}
	pr_debug("current core freq is %d\n", policy->cur);

	cpufreq_frequency_table_get_attr(mpc85xx_freqs, policy->cpu);

	/* this ensures that policy->cpuinfo_min
	 * and policy->cpuinfo_max are set correctly */
	return cpufreq_frequency_table_cpuinfo(policy, mpc85xx_freqs);
}

static int mpc85xx_cpufreq_cpu_exit(struct cpufreq_policy *policy)
{
	cpufreq_frequency_table_put_attr(policy->cpu);
	return 0;
}

static int mpc85xx_cpufreq_verify(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, mpc85xx_freqs);
}

static int mpc85xx_cpufreq_target(struct cpufreq_policy *policy,
			      unsigned int target_freq,
			      unsigned int relation)
{
	struct cpufreq_freqs freqs;
	unsigned int new;

	cpufreq_frequency_table_target(policy,
				       mpc85xx_freqs,
				       target_freq,
				       relation,
				       &new);

	freqs.old = policy->cur;
	freqs.new = mpc85xx_freqs[new].frequency;
	freqs.cpu = policy->cpu;

	mutex_lock(&mpc85xx_switch_mutex);
	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	printk("setting frequency for cpu %d to %d kHz, " \
		 "PLL ratio is %d/2\n",
		 policy->cpu,
		 mpc85xx_freqs[new].frequency,
		 mpc85xx_freqs[new].index);

	set_pll(mpc85xx_freqs[new].index, policy->cpu);

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	mutex_unlock(&mpc85xx_switch_mutex);

	ppc_proc_freq = freqs.new * 1000ul;

	verify_pll(policy->cpu);

	return 0;
}

static struct cpufreq_driver mpc85xx_cpufreq_driver = {
	.verify		= mpc85xx_cpufreq_verify,
	.target		= mpc85xx_cpufreq_target,
	.init		= mpc85xx_cpufreq_cpu_init,
	.exit		= mpc85xx_cpufreq_cpu_exit,
	.name		= "mpc85xx-cpufreq",
	.owner		= THIS_MODULE,
	.flags		= CPUFREQ_CONST_LOOPS,
};

/*
 * module init and destoy
 */

static int __init mpc85xx_cpufreq_init(void)
{
	if (!machine_is_compatible("fsl,P1022DS"))
		return -ENODEV;

	porpllsr = ioremap(get_immrbase() + PORPLLSR, 4);
	pmjcr = ioremap(get_immrbase() + PMJCR, 4);
	powmgtcsr = ioremap(get_immrbase() + POWMGTCSR, 4);

	return cpufreq_register_driver(&mpc85xx_cpufreq_driver);
}

static void __exit mpc85xx_cpufreq_exit(void)
{
	iounmap(porpllsr);
	iounmap(pmjcr);
	iounmap(powmgtcsr);

	cpufreq_unregister_driver(&mpc85xx_cpufreq_driver);
}

module_init(mpc85xx_cpufreq_init);
module_exit(mpc85xx_cpufreq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dave Liu <daveliu@freescale.com>");
