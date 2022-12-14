/*
 * drivers/misc/tdm_test.c
 *
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * TDM Test Module.
 * This TDM test module is a small test module which registers with the
 * TDM framework and sets up a TDM Voice path between Port 0
 * and Port 1 for each slic.
 *
 * Author:Hemant Agrawal <hemant@freescale.com>
 * 	Rajesh Gumasta <rajesh.gumasta@freescale.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/tdm.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/tdm.h>

#define DRV_DESC "Test Module for Freescale Platforms with TDM support"
#define DRV_NAME "tdm_test"

#define TDM_FRAME_LENGTH NUM_OF_FRAMES
#define TDM_E_OK 0
#define FRAME_SIZE 0x500

static struct task_struct *tdm_thread_task = NULL;

/** \brief      Driver's license
 ** \details    GPL
 ** \ingroup    Linux_module
 **/
MODULE_LICENSE("GPL");
/** \brief      Module author
 ** \ingroup    Linux_module
 **/

MODULE_AUTHOR("Hemant Agrawal <hemant@freescale.com> and "
	"Rajesh Gumasta <rajesh.gumasta@freescale.com>");
/** \brief      Module description
 ** \ingroup    Linux_module
 **/
MODULE_DESCRIPTION(DRV_DESC);

static struct tdm_driver test_tdmdev_driver;
struct tdm_port *tdmport;
int tdm_thread_state;

static int tdm_thread(void *ptr)
{
	int err = TDM_E_OK;
	int poll = 0, poll_count = 5000;
	void *h_port1, *h_port2, *h_port3, *h_port4;
	unsigned short p_data1[TDM_FRAME_LENGTH];
	unsigned short p_data2[TDM_FRAME_LENGTH];
	uint16_t size = TDM_FRAME_LENGTH;
	int ch1_id = 0, ch2_id = 1, ch3_id = 2, ch4_id = 3;
	tdm_thread_state = 1;

	err = tdm_port_open(&test_tdmdev_driver, ch1_id, &h_port1);
	pr_debug("%s tdm_port_open err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_open(%d)- err %x\n", ch1_id, err);
		return -ENXIO;
	}
	if (h_port1 == NULL)
		pr_debug("%s tdm_port_open err in hport1 = %d \n",
				__func__, err);

	err = tdm_port_open(&test_tdmdev_driver, ch2_id, &h_port2);
	pr_debug("%s tdm_port_open err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_open(%d)- err %x\n", ch2_id, err);
		return -ENXIO;
	}
	if (h_port2 == NULL)
		pr_debug("%s tdm_port_open err in hport2 = %d \n",
				__func__, err);

	err = tdm_port_open(&test_tdmdev_driver, ch3_id, &h_port3);
	pr_debug("%s tdm_port_open err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_open(%d)- err %x\n", ch3_id, err);
		return -ENXIO;
	}
	if (h_port3 == NULL)
		pr_debug("%s tdm_port_open err in hport3 = %d \n",
				__func__, err);

	err = tdm_port_open(&test_tdmdev_driver, ch4_id, &h_port4);
	pr_debug("%s tdm_port_open err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_open(%d)- err %x\n", ch4_id, err);
		return -ENXIO;
	}
	if (h_port4 == NULL)
		pr_debug("%s tdm_port_open err in hport5 = %d \n",
				__func__, err);

	while ((poll < poll_count) && !kthread_should_stop()) {

		poll++;
		while (tdm_port_poll(h_port1, 10) != TDM_E_OK);

		err = tdm_port_read(h_port1, p_data1, &size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_read\n");

		err = tdm_port_write(h_port2, p_data1, size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_write\n");

		err = tdm_port_read(h_port2, p_data1, &size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_read\n");

		err = tdm_port_write(h_port1, p_data1, size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_write\n");

		err = tdm_port_read(h_port3, p_data2, &size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_read\n");

		err = tdm_port_write(h_port4, p_data2, size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_write\n");

		err = tdm_port_read(h_port4, p_data2, &size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_read\n");

		err = tdm_port_write(h_port3, p_data2, size);
		if (err != TDM_E_OK)
			pr_info("Error in tdm_port_write\n");
	}
	pr_debug("\n CLOSING THE TDM TEST\n");
	err = tdm_port_close(h_port1);
	pr_debug("%s tdm_port_close err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_close(%d)- err %x\n", ch1_id, err);
		return -ENXIO;
	}

	err = tdm_port_close(h_port2);
	pr_debug("%s tdm_port_close err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_close(%d)- err %x\n", ch2_id, err);
		return -ENXIO;
	}

	err = tdm_port_close(h_port3);
	pr_debug("%s tdm_port_close err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_close(%d)- err %x\n", ch3_id, err);
		return -ENXIO;
	}

	err = tdm_port_close(h_port4);
	pr_debug("%s tdm_port_close err = %d \n", __func__, err);
	if (err != TDM_E_OK) {
		pr_err("Error in tdm_port_close(%d)- err %x\n", ch4_id, err);
		return -ENXIO;
	}

	tdm_thread_state = 0;

	return 0;
}

static int test_attach_adapter(struct tdm_adapter *adap)
{
	pr_debug("tdm-dev: adapter [%s] registered as minor %d\n",
		 adap->name, adap->id);
	tdm_thread_state = 0;
	tdm_thread_task = kthread_run(tdm_thread, NULL, "tdm_thread");

	return 0;
}

static int test_detach_adapter(struct tdm_adapter *adap)
{
	if (tdm_thread_state)
		kthread_stop(tdm_thread_task);

	pr_debug("tdm-dev: adapter [%s] unregistered\n", adap->name);

	return 0;
}

static const struct tdm_device_id test_starlite_id[] = {
	{ "fsl_starlite", 0 },
	{ }
};

static struct tdm_driver test_tdmdev_driver = {
	.attach_adapter	= test_attach_adapter,
	.detach_adapter	= test_detach_adapter,
	.id_table = test_starlite_id,
};

static int __init tdm_test_init(void)
{
	int ret;
	pr_info("TDM_TEST: " DRV_DESC "\n");

	test_tdmdev_driver.id = 1;
	/* create a binding with TDM driver */
	ret = tdm_add_driver(&test_tdmdev_driver);
	if (ret == 0)
		pr_info("TDM_TEST module installed\n");
	else
		pr_err("%s tdm_port_init failed\n", __func__);

	return ret;

}

static void __exit tdm_test_exit(void)
{
	tdm_del_driver(&test_tdmdev_driver);
	pr_info("TDM_TEST module un-installed\n");
}

module_init(tdm_test_init);
module_exit(tdm_test_exit);
