

#ifndef _GENERIC_USB_H_
#define _GENERIC_USB_H_

#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define QCA9377_1_0_DEVICE_ID   (0x0042)

struct generic_usb {
	struct usb_device *udev;
	struct device *dev;
	struct usb_interface *interface;
	const struct usb_device_id *usb_device_id;

	int nr_in_eps;
	int nr_out_eps;
	unsigned int in_ep[4];
	unsigned int out_ep[4];
};

static int generic_usb_dev_init(struct generic_usb *ar_usb);
static int generic_usb_probe(struct usb_interface *interface,
			     const struct usb_device_id *id);
static void generic_usb_disconnect(struct usb_interface *interface);

static int generic_usb_init(void);
static void generic_usb_exit(void);

#endif /* GENERIC_USB_H */
