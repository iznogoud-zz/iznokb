#ifndef __USB_KEYBOARD_DEVICE__
#define __USB_KEYBOARD_DEVICE__

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

static const uint8_t hid_keyboard_report[] = {
    // 78 bytes
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x75, 0x01,        //   Report Size (1)
0x95, 0x08,        //   Report Count (8)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x03,        //   Report Count (3)
0x75, 0x08,        //   Report Size (8)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x64,        //   Logical Maximum (100)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x29, 0x65,        //   Usage Maximum (0x65)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
0x05, 0x0C,        // Usage Page (Consumer)
0x09, 0x01,        // Usage (Consumer Control)
0xA1, 0x01,        // Collection (Application)
0x85, 0x02,        //   Report ID (2)
0x05, 0x0C,        //   Usage Page (Consumer)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x07,        //   Report Count (7)
0x09, 0xB5,        //   Usage (Scan Next Track)
0x09, 0xB6,        //   Usage (Scan Previous Track)
0x09, 0xB7,        //   Usage (Stop)
0x09, 0xB8,        //   Usage (Eject)
0x09, 0xCD,        //   Usage (Play/Pause)
0x09, 0xE2,        //   Usage (Mute)
0x09, 0xE9,        //   Usage (Volume Increment)
0x09, 0xEA,        //   Usage (Volume Decrement)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) usb_keyboard_function = {
	.hid_descriptor = {
		.bLength = sizeof(usb_keyboard_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_keyboard_report),
	}
};

const struct usb_interface_descriptor usb_keyboard_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 1, /* boot */
	.bInterfaceProtocol = 1, /* keyboard */
	.iInterface = 0,

	.endpoint = &usb_endpoint,

	.extra = &usb_keyboard_function,
	.extralen = sizeof(usb_keyboard_function),
};

static const uint8_t USBD_HID_Desc[] = {
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  0x21, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  65,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

static enum usbd_request_return_codes usb_keyboard_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *, struct usb_setup_data *))
{
	(void)complete;
	(void)dev;

	if((req->bmRequestType == 0x81) &&
	   (req->bRequest == USB_REQ_GET_DESCRIPTOR))
       {
           if (req->wValue == 0x2100)
           {
               /* Handle the HID report descriptor. */
                *buf = (uint8_t *)hid_keyboard_report;
                *len = sizeof(hid_keyboard_report);

                return USBD_REQ_HANDLED;
           }else
           {
               if(req->wValue == 0x2200)
               {
                   /* Handle the HID report descriptor. */
                    *buf = (uint8_t *)USBD_HID_Desc;
                    *len = sizeof(USBD_HID_Desc);

                    return USBD_REQ_HANDLED;

               }else
               {
                    return USBD_REQ_NOTSUPP;
               }
           }
       }else
       {
           return USBD_REQ_NOTSUPP;
       }
}



#endif //__USB_KEYBOARD_DEVICE__