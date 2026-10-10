// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright(c) 2025  Realtek Corporation
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/usb.h>
#include "rtw8851b.h"
#include "reg.h"
#include "usb.h"

static const struct rtw89_usb_info rtw8851b_usb_info = {
	.usb_host_request_2		= R_AX_USB_HOST_REQUEST_2,
	.usb_wlan0_1			= R_AX_USB_WLAN0_1,
	.hci_func_en			= R_AX_HCI_FUNC_EN,
	.usb3_mac_npi_config_intf_0	= R_AX_USB3_MAC_NPI_CONFIG_INTF_0,
	.usb_endpoint_0			= R_AX_USB_ENDPOINT_0,
	.usb_endpoint_2			= R_AX_USB_ENDPOINT_2,
	.rx_agg_alignment		= 8,
	.bulkout_id = {
		[RTW89_DMA_ACH0] = 3,
		[RTW89_DMA_ACH1] = 4,
		[RTW89_DMA_ACH2] = 5,
		[RTW89_DMA_ACH3] = 6,
		[RTW89_DMA_B0MG] = 0,
		[RTW89_DMA_B0HI] = 1,
		[RTW89_DMA_H2C] = 2,
	},
};

/*
 * The 8851b rfkill configuration samples the WL_DISABLE input on the
 * chip's GPIO9 pad, a wiring that exists on mini-PCIe 8851B cards, whose
 * RFKILL button feeds that pad. 8851BU USB adapters (all of them, see the
 * ID table) leave the pad unconnected, so the driver would arm a pinmux on
 * a floating input and, once the radio stops running, poll it at 1 Hz; a
 * low sample then reports a spurious hardware rfkill that shuts the radio
 * down and only clears on the next probe. No rfkill switch exists on these
 * adapters, so use a private copy of the 8851b chip info with the rfkill
 * configuration removed. Software rfkill (wiphy soft state) is unaffected.
 */
static struct rtw89_chip_info rtw8851bu_chip_info = rtw8851b_chip_info;

static const struct rtw89_driver_info rtw89_8851bu_info = {
	.chip = &rtw8851bu_chip_info,
	.variant = NULL,
	.board = NULL,
	.quirks = NULL,
	.dev_id_quirks = 0,
	.bus = {
		.usb = &rtw8851b_usb_info,
	}
};

static const struct usb_device_id rtw_8851bu_id_table[] = {
	{ USB_DEVICE_AND_INTERFACE_INFO(0x056e, 0x4027, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x056e, 0x4028, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x0bda, 0xb831, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x0bda, 0xb851, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	/* D-Link AX9U rev. A1 */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x2001, 0x332a, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	/* Mercusys MA60XNB */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x2c4e, 0x0128, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	/* TP-Link Archer TX10UB Nano */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x3625, 0x010b, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	/* Edimax EW-7611UXB */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x7392, 0xe611, 0xff, 0xff, 0xff),
	  .driver_info = (kernel_ulong_t)&rtw89_8851bu_info },
	{},
};
MODULE_DEVICE_TABLE(usb, rtw_8851bu_id_table);

static int rtw8851bu_probe(struct usb_interface *intf,
			   const struct usb_device_id *id)
{
	return rtw89_usb_probe(intf, id, &rtw89_8851bu_info);
}

static struct usb_driver rtw_8851bu_driver = {
	.name = KBUILD_MODNAME,
	.id_table = rtw_8851bu_id_table,
	.probe = rtw8851bu_probe,
	.disconnect = rtw89_usb_disconnect,
};

static int __init rtw8851bu_module_init(void)
{
	int ret;

	/* Disarm the chip's polled rfkill input (see rtw8851bu_chip_info
	 * above) before any interface can probe.
	 */
	rtw8851bu_chip_info.rfkill_init = NULL;
	memset(&rtw8851bu_chip_info.rfkill_get, 0,
	       sizeof(rtW8851bu_chip_info.rfkill_get));

	ret = usb_register(&rtw_8851bu_driver);
	if (ret)
		pr_err("rtw89_8851bu: usb_register failed (%pe)\n",
		       ERR_PTR(ret));

	return ret;
}

static void __exit rtw8851bu_module_exit(void)
{
	usb_deregister(&rtw_8851bu_driver);
}

module_init(rtw8851bu_module_init);
module_exit(rtw8851bu_module_exit);

MODULE_AUTHOR("Bitterblue Smith <rtl8821cerfe2@gmail.com>");
MODULE_DESCRIPTION("Realtek 802.11ax wireless 8851BU driver");
MODULE_LICENSE("Dual BSD/GPL");
