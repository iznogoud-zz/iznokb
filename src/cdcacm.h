// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

#ifndef __CDCACM_IZNOKB__
#define __CDCACM_IZNOKB__

#include <libopencm3/usb/cdc.h>

extern const struct usb_interface_descriptor uart_comm_iface[];
extern const struct usb_interface_descriptor uart_data_iface[];
extern const struct usb_iface_assoc_descriptor uart_assoc;

void uart_print(char *msg, uint8_t len, usbd_device *dev);

void cdcacm_set_config(usbd_device *dev, uint16_t wValue);


void usbuart_send_buf(usbd_device *dev, void *buf, unsigned int size);

#endif //__CDCACM_IZNOKB__