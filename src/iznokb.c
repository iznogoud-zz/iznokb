/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include "hid.h"
#include "cdcacm.h"

static uint32_t report_index = 0;
static struct composite_report packet_buffer[1024 / sizeof(struct composite_report)] = {0};

/* Define this to include the DFU APP interface. */
// #define INCLUDE_DFU_INTERFACE

#ifdef INCLUDE_DFU_INTERFACE
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dfu.h>
#endif

// static usbd_device *usbd_dev;



// #ifdef INCLUDE_DFU_INTERFACE
// const struct usb_dfu_descriptor dfu_function = {
// 	.bLength = sizeof(struct usb_dfu_descriptor),
// 	.bDescriptorType = DFU_FUNCTIONAL,
// 	.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
// 	.wDetachTimeout = 255,
// 	.wTransferSize = 1024,
// 	.bcdDFUVersion = 0x011A,
// };

// const struct usb_interface_descriptor dfu_iface = {
// 	.bLength = USB_DT_INTERFACE_SIZE,
// 	.bDescriptorType = USB_DT_INTERFACE,
// 	.bInterfaceNumber = 1,
// 	.bAlternateSetting = 0,
// 	.bNumEndpoints = 0,
// 	.bInterfaceClass = 0xFE,
// 	.bInterfaceSubClass = 1,
// 	.bInterfaceProtocol = 1,
// 	.iInterface = 0,

// 	.extra = &dfu_function,
// 	.extralen = sizeof(dfu_function),
// };
// #endif

// const struct usb_interface ifaces[] = {{
// // 	.num_altsetting = 1,
// // 	.altsetting = &usb_mouse_iface,
// // }, {
// 	.num_altsetting = 1,
// 	.altsetting = &usb_keyboard_iface,
// #ifdef INCLUDE_DFU_INTERFACE
// }, {
// 	.num_altsetting = 1,
// 	.altsetting = &dfu_iface,
// #endif
// }};

// const struct usb_config_descriptor config = {
// 	.bLength = USB_DT_CONFIGURATION_SIZE,
// 	.bDescriptorType = USB_DT_CONFIGURATION,
// 	.wTotalLength = 0,
// #ifdef INCLUDE_DFU_INTERFACE
// 	.bNumInterfaces = 3,
// #else
// 	.bNumInterfaces = 1,
// #endif
// 	.bConfigurationValue = 1,
// 	.iConfiguration = 0,
// 	.bmAttributes = 0xC0,
// 	.bMaxPower = 0x32,

// 	.interface = ifaces,
// };

// static const char *usb_strings[] = {
// 	"izno inc",
// 	"iznokb",
// 	"izno",
// };

// /* Buffer to be used for control requests. */
// uint8_t usbd_control_buffer[128];



// #ifdef INCLUDE_DFU_INTERFACE
// static void dfu_detach_complete(usbd_device *dev, struct usb_setup_data *req)
// {
// 	(void)req;
// 	(void)dev;

// 	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
// 		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO10);
// 	gpio_set(GPIOA, GPIO10);
// 	scb_reset_core();
// }

// static enum usbd_request_return_codes dfu_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
// 			void (**complete)(usbd_device *, struct usb_setup_data *))
// {
// 	(void)buf;
// 	(void)len;
// 	(void)dev;

// 	if ((req->bmRequestType != 0x21) || (req->bRequest != DFU_DETACH))
// 		return USBD_REQ_NOTSUPP; /* Only accept class request. */

// 	*complete = dfu_detach_complete;

// 	return USBD_REQ_HANDLED;
// }
// #endif

// static void hid_set_config(usbd_device *dev, uint16_t wValue)
// {
// 	(void)wValue;
// 	(void)dev;

// 	usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);

// 	// usbd_register_control_callback(
// 	// 			dev,
// 	// 			USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
// 	// 			USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
// 	// 			usb_mouse_control_request);

// 	usbd_register_control_callback(
// 				dev,
// 				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
// 				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
// 				usb_keyboard_control_request);
// #ifdef INCLUDE_DFU_INTERFACE
// 	usbd_register_control_callback(
// 				dev,
// 				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
// 				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
// 				dfu_control_request);
// #endif

// 	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
// 	/* SysTick interrupt every N clock pulses: set reload to N-1 */
// 	systick_set_reload(999999);
// 	systick_interrupt_enable();
// 	systick_counter_enable();
// }	

static usbd_device *usbd_dev;

const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x05ac,
	.idProduct = 0x2227,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};


const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}, {
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc,
	.altsetting = uart_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = uart_data_iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = sizeof(ifaces)/sizeof(ifaces[0]),
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"izno",
	"iznokb",
	"iznokb uart",
};

static void setup_clock(void) {
	rcc_clock_setup_in_hsi_out_48mhz();
	
	rcc_periph_clock_enable(RCC_GPIOC);

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	
	/* SysTick interrupt every N clock pulses: set reload to N-1
	 * Period: N / (72 MHz / 8 )
	 * */
	//systick_set_reload(899999); // 100 ms
	//systick_set_reload(89999); // 10 ms
	systick_set_reload(300000-1); // 0.05s
	systick_interrupt_enable();
	systick_counter_enable();
}

static void setup_gpio(void) {
	// Built-in LED on blue pill board, PC13
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	gpio_set(GPIOC, GPIO13);
}

static void usb_set_config(usbd_device *dev, uint16_t wValue)
{
	hid_set_config(dev, wValue);
	cdcacm_set_config(dev, wValue);
}

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

int main(void)
{
	
	setup_clock();
	setup_gpio();
	/*
	 * This is a somewhat common cheap hack to trigger device re-enumeration
	 * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
	 * setting the pin to output, and driving it explicitly low effectively
	 * "removes" the pullup.  The subsequent USB init will "take over" the
	 * pin, and it will appear as a proper pullup to the host.
	 * The magic delay is somewhat arbitrary, no guarantees on USBIF
	 * compliance here, but "it works" in most places.
	 */
	// gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
	// 	GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	// gpio_clear(GPIOA, GPIO12);

	// for (unsigned i = 0; i < 800000; i++) {
	// 	__asm__("nop");
	// }

	// systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	// /* SysTick interrupt every N clock pulses: set reload to N-1 */
	// systick_set_reload(2999);
	// systick_interrupt_enable();
	// systick_counter_enable();

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings,
		sizeof(usb_strings)/sizeof(char *),
		usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config);

	

	// usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &usb_device, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	// usbd_register_set_config_callback(usbd_dev, hid_set_config);
	
	while (1)
	{
		usbd_poll(usbd_dev);
	}
}


void add_mouse_jiggler(int width)
{       
        int j = report_index;
        for (int i = 0; i < width; ++i) {
                packet_buffer[j].report_id = REPORT_ID_MOUSE;
                packet_buffer[j].mouse.buttons = 0;
                packet_buffer[j].mouse.x = 1;
                packet_buffer[j].mouse.y = 0;
                packet_buffer[j].mouse.wheel = 0;
                ++j;
        }
        
        for (int i = 0; i < width; ++i) {
                packet_buffer[j].report_id = REPORT_ID_MOUSE;
                packet_buffer[j].mouse.buttons = 0;
                packet_buffer[j].mouse.x = -1;
                packet_buffer[j].mouse.y = 0;
                packet_buffer[j].mouse.wheel = 0;
                ++j;
        }
        
        packet_buffer[j].report_id = REPORT_ID_END;
        
        report_index = j;
}

void add_keyboard_spammer(int scancode)
{       
        int j = report_index;
        
        packet_buffer[j].report_id = REPORT_ID_KEYBOARD;
        packet_buffer[j].keyboard.modifiers = 0;
        packet_buffer[j].keyboard.reserved = 0;
        packet_buffer[j].keyboard.keys_down[0] = scancode;
        packet_buffer[j].keyboard.keys_down[1] = 0;
        packet_buffer[j].keyboard.keys_down[2] = 0;
        packet_buffer[j].keyboard.keys_down[3] = 0;
        packet_buffer[j].keyboard.keys_down[4] = 0;
        packet_buffer[j].keyboard.keys_down[5] = 0;
        packet_buffer[j].keyboard.leds = 0;
        ++j;

        packet_buffer[j].report_id = REPORT_ID_END;

        report_index = j;
}

void sys_tick_handler(void)
{
	
	// struct composite_report report = packet_buffer[report_index];
	// uint16_t len = 0;
	// uint8_t id = report.report_id;
	// uart_print("1|", 2, usbd_dev);

	// if (id == REPORT_ID_KEYBOARD) {
	// 	len = 9;
	// } else if (id == REPORT_ID_MOUSE) {
	// 	len = 5;
	// } else {
	// 	report_index = 0;
	// }

	// uart_print("2|", 2, usbd_dev);

	// if (len > 0)
	// {
	// 	uint16_t bytes_written = 0;
	// 	do {
	// 		bytes_written = usbd_ep_write_packet(usbd_dev, 0x81, &report, len);
	// 	} while (bytes_written == 0);

	// 	++report_index;
	// }

	// uart_print("3|", 2, usbd_dev);
	gpio_toggle(GPIOC, GPIO13);

	
}
