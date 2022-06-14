#pragma once

#include <stdlib.h>
#include "tusb.h"
#include "device/usbd_pvt.h"
#include "gamepad.h"

struct usb_report
{
		void* data;
		size_t size;
};

class USBDevice {

public:
	virtual void sendInputState(const GamepadInputState& state) = 0;

	virtual uint8_t const *tud_descriptor_device_cb(void) = 0;
	virtual uint8_t const *tud_hid_descriptor_report_cb(void) = 0;
	virtual uint8_t const *tud_descriptor_configuration_cb(uint8_t index) = 0;
	virtual uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) = 0;
	virtual usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t *driver_count) = 0;

	static USBDevice* usb_device;

};

