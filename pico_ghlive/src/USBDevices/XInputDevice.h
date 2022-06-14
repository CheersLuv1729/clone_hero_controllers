#pragma once

#include "USBDevice.h"

struct __attribute((packed, aligned(1))) xinput_report{
	uint8_t rid;
	uint8_t rsize;
	uint8_t digital_buttons_1;
	uint8_t digital_buttons_2;
	uint8_t lt;
	uint8_t rt;
	int16_t l_x;
	int16_t l_y;
	int16_t r_x;
	int16_t r_y;
	uint8_t reserved_1[6];
};

xinput_report gamepad_state_to_report(const GamepadInputState& state);

class XInputDevice : public USBDevice {

public:

	virtual void sendInputState(const GamepadInputState& state);

	virtual uint8_t const *tud_descriptor_device_cb(void);
	virtual uint8_t const *tud_hid_descriptor_report_cb(void);
	virtual uint8_t const *tud_descriptor_configuration_cb(uint8_t index);
	virtual uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);
	virtual usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t *driver_count);

private:

	static uint8_t endpoint_in;//=0;
	static uint8_t endpoint_out;//=0;
	static void xinput_init(void);

	static void xinput_reset(uint8_t __unused rhport);

	static uint16_t xinput_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len);

	static bool xinput_device_control_request(uint8_t __unused rhport, tusb_control_request_t const *request);

	static bool xinput_control_complete_cb(uint8_t __unused rhport, tusb_control_request_t __unused const *request);

	static bool xinput_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes);


	static usbd_class_driver_t xinput_driver;

	static tusb_desc_device_t const xinputDeviceDescriptor;

	static const uint8_t xinputConfigurationDescriptor[];

	static char const *string_desc_arr_xinput[];

	static uint16_t _desc_str[32];

};

#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))