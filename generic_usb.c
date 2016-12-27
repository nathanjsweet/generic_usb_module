/*
 * Copyright (c) 2016 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "generic_usb.h"


static struct usb_device_id generic_usb_ids[] = {
	{ USB_DEVICE(0x13b1, 0x0042),
	  .driver_info = QCA9377_1_0_DEVICE_ID }, /* Linksys */

	{0},
};


static int generic_usb_dev_init(struct generic_usb *usb)
{
	struct usb_interface_descriptor *interface_desc;
	struct usb_host_interface *host_interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_interface *interface = usb->interface;
	
	int i, endpoints;
	u8 dir, xtype, num;
	int out_idx = 0;
	int in_idx = 0;
	int ret = 0;

	host_interface = &interface->altsetting[0];
	interface_desc = &host_interface->desc;
	endpoints = interface_desc->bNumEndpoints;

	for (i = 0; i < endpoints; i++) {
		endpoint = &host_interface->endpoint[i].desc;

		dir = endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK;
		num = usb_endpoint_num(endpoint);
		xtype = usb_endpoint_type(endpoint);
		printk(KERN_INFO
			   "%s: endpoint: dir %02x, # %02x, type %02x\n",
			   __func__, dir, num, xtype);

		if (usb_endpoint_dir_in(endpoint)) {
			if (usb_endpoint_xfer_bulk(endpoint))
			    usb->in_ep[in_idx++] = usb_rcvbulkpipe(usb->udev, num);
		}

		if (usb_endpoint_dir_out(endpoint)) {
			if (usb_endpoint_xfer_bulk(endpoint))
				usb->out_ep[out_idx++] = usb_sndbulkpipe(usb->udev, num);
		}
	}
	usb->nr_in_eps = in_idx;
	usb->nr_out_eps = out_idx;
	
	return ret;
}

static int generic_usb_probe(struct usb_interface *interface,
			       const struct usb_device_id *id)
{
	int ret = 0;
	struct usb_device *udev = interface_to_usbdev(interface);
	struct generic_usb *usb = kzalloc(sizeof(struct generic_usb), GFP_KERNEL);
	if (!usb)
		return -ENOMEM;
	
	switch (id->driver_info) {
	case QCA9377_1_0_DEVICE_ID:
		break;
	default:
		WARN_ON(1);
		return -ENOTSUPP;
	}

	printk(KERN_INFO "usb probe: %04x:%04x:%s:%s:%s\n",
		   id->idVendor, id->idProduct, udev->manufacturer,
		   udev->product, udev->serial);

	usb->udev = udev;
	usb->dev = &udev->dev;
	usb->interface = interface;
	usb->usb_device_id = id;
	
	usb_get_dev(udev);
	
	usb_set_intfdata(interface, usb);

	ret = generic_usb_dev_init(usb);
	if (ret)
		goto exit;

	return 0;

exit:
	usb_set_intfdata(interface, NULL);
	usb_put_dev(udev);

	return ret;
}
static void generic_usb_disconnect(struct usb_interface *interface)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct generic_usb *usb = usb_get_intfdata(interface);

	usb_set_intfdata(interface, NULL);

	kfree(usb);
	printk(KERN_INFO "USB layer deinitialized\n");
	usb_put_dev(udev);
}

MODULE_DEVICE_TABLE(usb, generic_usb_ids);

static struct usb_driver generic_usb_driver = {
	.name = KBUILD_MODNAME,
	.probe = generic_usb_probe,
	.disconnect = generic_usb_disconnect,
	.id_table = generic_usb_ids,
};

static int __init generic_usb_init(void)
{
	return usb_register(&generic_usb_driver);
}
module_init(generic_usb_init);

static void __exit generic_usb_exit(void)
{
	usb_deregister(&generic_usb_driver);
}

module_exit(generic_usb_exit);

MODULE_LICENSE("GPL");
