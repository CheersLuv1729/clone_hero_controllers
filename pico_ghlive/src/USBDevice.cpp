#include "USBDevice.h"


USBDevice* USBDevice::usb_device = nullptr;

uint8_t const *tud_descriptor_device_cb(void) {
	return USBDevice::usb_device->tud_descriptor_device_cb();
}

uint8_t const *tud_hid_descriptor_report_cb(void) {
	return USBDevice::usb_device->tud_hid_descriptor_report_cb();
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
	return USBDevice::usb_device->tud_descriptor_configuration_cb(index);
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	return USBDevice::usb_device->tud_descriptor_string_cb(index, langid);
}

usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t *driver_count) {
		return USBDevice::usb_device->usbd_app_driver_get_cb(driver_count);
}