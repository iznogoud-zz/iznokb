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
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <string.h>

//#include "usb_serial.h"
#include "cdcacm.h"
#include "hid.h"
#include "iznokb.h"

bool usb_ready = false;

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
	.idVendor = 0x05ac,
	.idProduct = 0x2227,
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
},
{
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc,
	.altsetting = uart_comm_iface,
},
{
	.num_altsetting = 1,
	.altsetting = uart_data_iface,
}
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
	cdcacm_set_config(dev, wValue);

	hid_set_config(dev, wValue);
}

static void setup_clock(void) {
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOB);

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

	gpio_set_mode(rows[0].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[0].pin); // row 1
	gpio_set_mode(rows[1].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[1].pin); // row 1
	gpio_set_mode(rows[2].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[2].pin); // row 2
	gpio_set_mode(rows[3].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[3].pin); // row 3
	gpio_set_mode(rows[4].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[4].pin);  // row 4
	gpio_set_mode(rows[5].port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, rows[5].pin);  // row 5

	gpio_set_mode(cols[0].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[0].pin); // col 0
	gpio_set_mode(cols[1].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[1].pin); // col 1
	gpio_set_mode(cols[2].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[2].pin);  // col 2
	gpio_set_mode(cols[3].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[3].pin);  // col 3
	gpio_set_mode(cols[4].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[4].pin);  // col 4
	gpio_set_mode(cols[5].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[5].pin);  // col 5
	gpio_set_mode(cols[6].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[6].pin);  // col 6
	gpio_set_mode(cols[7].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[7].pin);  // col 7
	gpio_set_mode(cols[8].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[8].pin);  // col 8
	gpio_set_mode(cols[9].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[9].pin);  // col 9
	gpio_set_mode(cols[10].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[10].pin);  // col 10
	gpio_set_mode(cols[11].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[11].pin);  // col 11
	gpio_set_mode(cols[12].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[12].pin); // col 12
	gpio_set_mode(cols[13].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[13].pin); // col 13
	gpio_set_mode(cols[14].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[14].pin);  // col 14
	gpio_set_mode(cols[15].port, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, cols[15].pin);  // col 15
}

static uint16_t check_cols(void)
{
	uint16_t columns = 0;

	for(int i=0; i<NUM_COLS; i++)
	{
		columns |= (gpio_get(cols[i].port, cols[i].pin) << i);
	}

	return columns;
}

static int dir = 1;
static bool jiggler = true;
static unsigned int tick_counter = 0;

static char uart_debug[128] = {0};
static uint16_t column_val = 0;

void sys_tick_handler(void)
{
	if(usb_ready && tick_counter % 100 == 0)
		gpio_toggle(GPIOC, GPIO13);

	tick_counter += 1;

	if(usb_ready && jiggler)
	{
		static int x = 0;
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

		uint8_t report[9] = {0};
		report[0] = 1; // keyboard
		report[1] = 0; // no modifiers down
		report[2] = 0;
		report[3] = 0x05; // 'A'
		usbd_ep_write_packet(usbd_dev, 0x81, report, sizeof(report));
	}

	if(usb_ready && tick_counter % 100 == 0)
	{
		column_val = 0;
		gpio_set(rows[0].port, rows[0].pin);

		column_val = check_cols();

		// memcpy(uart_debug, &column_val, sizeof(uint16_t));
		uart_debug[0] = '>';
		uart_debug[1] = column_val & 0xff;
		uart_debug[2] = (column_val >> 8) & 0xff;
		uart_debug[3] = '|';
		uart_debug[4] = '\n';

		usbuart_send_buf(usbd_dev, uart_debug, 5);

		gpio_clear(rows[0].port, rows[0].pin);
	}
}

int main(void)
{
	setup_clock();

	setup_gpio();

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, sizeof(usb_strings)/sizeof(char *),
	 usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config);

	for (int i = 0; i < 0x800000; i++)
		__asm__("nop");

	usb_ready = true;

	gpio_set(GPIOC, GPIO13);

	

	while (1)
	{
		usbd_poll(usbd_dev);
	}
}
