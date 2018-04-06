#ifndef __IZNOKB_USB_SERIAL_H__
#define __IZNOKB_USB_SERIAL_H__

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <stddef.h>

extern const struct usb_interface_descriptor uart_comm_iface[];

extern const struct usb_interface_descriptor uart_data_iface[];

extern const struct usb_iface_assoc_descriptor uart_assoc;

void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue);

void USBPrint(const char *msg, usbd_device *dev);

#endif //__IZNOKB_USB_SERIAL_H__