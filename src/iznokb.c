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
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "usb_serial.h"
#include "hid.h"

usbd_device *usbd_dev;

static const char *usb_strings[] = {
	"izno tech",
	"iznokb",
	"izno",
	"izno uart",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usb_interface ifaces[] = {
{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}/*,
{
	.num_altsetting = 1,
	.altsetting = comm_iface,
},
{
	.num_altsetting = 1,
	.altsetting = data_iface,
}*/
};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = sizeof(ifaces)/sizeof(ifaces[0]),
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static void usb_set_config(usbd_device *dev, uint16_t wValue)
{
	// cdcacm_set_config(dev, wValue);

	hid_set_config(dev, wValue);
}

static void setup_clock(void) {
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOC);

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* SysTick interrupt every N clock pulses: set reload to N-1
	 * Period: N / (48 MHz / 8 )
	 * */
	// systick_set_reload(2999999); // 0.5s
	// systick_set_reload(59999); // 10ms
	systick_set_reload(5999); // 1ms
	// systick_set_reload(599); // 100us
	systick_interrupt_enable();
	systick_counter_enable();
}

static void setup_gpio(void)
{
	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

	gpio_clear(GPIOC, GPIO13);
}

static int dir = 1;
static bool jiggler = true;
static bool odd = true;
void sys_tick_handler(void)
{
	gpio_toggle(GPIOC, GPIO13);

	static int x = 0;

	if(odd)
	{
		if (jiggler) {
				x += dir;
				if (x > 2500)
						dir = -dir;
				if (x < -2500)
						dir = -dir;
		}

		uint8_t report_m[5] = {0};
		report_m[0] = 2; // mouse
		report_m[1] = 0; // no modifiers down
		report_m[2] = dir;
		report_m[3] = 0; // 'A'
		usbd_ep_write_packet(usbd_dev, 0x81, report_m, sizeof(report_m));

	// 	odd = false;
	// }else{

		uint8_t report[9] = {0};
		report[0] = 1; // keyboard
		report[1] = 0; // no modifiers down
		report[2] = 0;
		report[3] = 0x04; // 'A'
		usbd_ep_write_packet(usbd_dev, 0x81, report, sizeof(report));
		odd = true;
	}


	
}



int main(void)
{
	setup_clock();

	setup_gpio();

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config);

	for (int i = 0; i < 0x800000; i++)
		__asm__("nop");

	gpio_set(GPIOC, GPIO13);

	while (1)
		usbd_poll(usbd_dev);
}
