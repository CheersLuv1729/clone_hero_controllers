#include "XInputDevice.h"


uint8_t XInputDevice::endpoint_in = 0;
uint8_t XInputDevice::endpoint_out = 0;

void XInputDevice::sendInputState(const GamepadInputState& state) {

	xinput_report report = gamepad_state_to_report(state);

	// Remote wakeup
	if (tud_suspended()) {
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	}
	if ((tud_ready()) && ((endpoint_in != 0)) && (!usbd_edpt_busy(0, endpoint_in))){
		usbd_edpt_claim(0, endpoint_in);
		usbd_edpt_xfer(0, endpoint_in, (uint8_t*)&report, sizeof(xinput_report));
		usbd_edpt_release(0, endpoint_in);
	}
}

void XInputDevice::xinput_init(void) {

}

void XInputDevice::xinput_reset(uint8_t __unused rhport) {

}

uint16_t XInputDevice::xinput_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
//+16 is for the unknown descriptor
	uint16_t const drv_len = sizeof(tusb_desc_interface_t) + itf_desc->bNumEndpoints*sizeof(tusb_desc_endpoint_t) + 16;
	TU_VERIFY(max_len >= drv_len, 0);

	uint8_t const * p_desc = tu_desc_next(itf_desc);
	uint8_t found_endpoints = 0;
	while ( (found_endpoints < itf_desc->bNumEndpoints) && (drv_len <= max_len)  )
	{
		tusb_desc_endpoint_t const * desc_ep = (tusb_desc_endpoint_t const *) p_desc;
		if ( TUSB_DESC_ENDPOINT == tu_desc_type(desc_ep) )
		{
			TU_ASSERT(usbd_edpt_open(rhport, desc_ep));

			if ( tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN )
			{
				endpoint_in = desc_ep->bEndpointAddress;
			}else
			{
				endpoint_out = desc_ep->bEndpointAddress;
			}
			found_endpoints += 1;
		}
		p_desc = tu_desc_next(p_desc);
	}
	return drv_len;
}

bool XInputDevice::xinput_device_control_request(uint8_t __unused rhport, tusb_control_request_t const *request) {
	return true;
}

bool XInputDevice::xinput_control_complete_cb(uint8_t __unused rhport, tusb_control_request_t __unused const *request) {
	return true;
}
//callback after xfer_transfer
bool XInputDevice::xinput_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes) {
	return true;
}

usbd_class_driver_t XInputDevice::xinput_driver = {
	.init             = XInputDevice::xinput_init,
	.reset            = XInputDevice::xinput_reset,
	.open             = XInputDevice:: xinput_open,
	.control_request  = XInputDevice::xinput_device_control_request,
	.control_complete = XInputDevice::xinput_control_complete_cb,
	.xfer_cb          = XInputDevice::xinput_xfer_cb,
	.sof              = NULL
};

// Implement callback to add our custom driver
usbd_class_driver_t const* XInputDevice::usbd_app_driver_get_cb(uint8_t *driver_count) {
		*driver_count = 1;
		return &xinput_driver;
}


tusb_desc_device_t const XInputDevice::xinputDeviceDescriptor = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF,
	.bDeviceSubClass = 0xFF,
	.bDeviceProtocol = 0xFF,
	.bMaxPacketSize0 =
			CFG_TUD_ENDPOINT0_SIZE,

	.idVendor = 0x045E,
	.idProduct = 0x028E,
	.bcdDevice = 0x0572,

	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,

	.bNumConfigurations = 0x01
};


const uint8_t XInputDevice::xinputConfigurationDescriptor[] = {
//Configuration Descriptor:
0x09,	//bLength
0x02,	//bDescriptorType
0x30,0x00, 	//wTotalLength   (48 bytes)
0x01,	//bNumInterfaces
0x01,	//bConfigurationValue
0x00,	//iConfiguration
0x80,	//bmAttributes   (Bus-powered Device)
0xFA,	//bMaxPower      (500 mA)

//Interface Descriptor:
0x09,	//bLength
0x04,	//bDescriptorType
0x00,	//bInterfaceNumber
0x00,	//bAlternateSetting
0x02,	//bNumEndPoints
0xFF,	//bInterfaceClass      (Vendor specific)
0x5D,	//bInterfaceSubClass
0x01,	//bInterfaceProtocol
0x00,	//iInterface

//Unknown Descriptor:
0x10,
0x21,
0x10,
0x01,
0x01,
0x24,
0x81,
0x14,
0x03,
0x00,
0x03,
0x13,
0x02,
0x00,
0x03,
0x00,

//Endpoint Descriptor:
0x07,	//bLength
0x05,	//bDescriptorType
0x81,	//bEndpointAddress  (IN endpoint 1)
0x03,	//bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x20,0x00, 	//wMaxPacketSize    (1 x 32 bytes)
0x04,	//bInterval         (4 frames)

//Endpoint Descriptor:

0x07,	//bLength
0x05,	//bDescriptorType
0x02,	//bEndpointAddress  (OUT endpoint 2)
0x03,	//bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x20,0x00, 	//wMaxPacketSize    (1 x 32 bytes)
0x08,	//bInterval         (8 frames)
};


// string descriptor table
char const * XInputDevice::string_desc_arr_xinput[] = {
		(const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
		"GENERIC",                   // 1: Manufacturer
		"XINPUT CONTROLLER",         // 2: Product
		"1.0"                       // 3: Serials
};




// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * XInputDevice::tud_descriptor_device_cb(void) {
		return (uint8_t const *)&xinputDeviceDescriptor;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * XInputDevice::tud_hid_descriptor_report_cb(void) {
	return 0;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+



// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * XInputDevice::tud_descriptor_configuration_cb(uint8_t index) {
	(void)index; // for multiple configurations
		return xinputConfigurationDescriptor;
	return 0;
}
//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const * XInputDevice::tud_descriptor_string_cb(uint8_t index, uint16_t langid) {

	(void)langid;

	uint8_t chr_count;

	if (index == 0) {
		memcpy(&_desc_str[1], string_desc_arr_xinput[0], 2);
		chr_count = 1;
	} else {
		// Convert ASCII string into UTF-16

		if (!(index < sizeof(string_desc_arr_xinput) / sizeof(string_desc_arr_xinput[0])))
			return NULL;

		const char *str = string_desc_arr_xinput[index];

		// Cap at max char
		chr_count = strlen(str);
		if (chr_count > 31)
			chr_count = 31;

		for (uint8_t i = 0; i < chr_count; i++) {
			_desc_str[1 + i] = str[i];
		}
	}

	// first byte is length (including header), second byte is string type
	_desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

	return _desc_str;
}


uint16_t XInputDevice::_desc_str[32] = {};

xinput_report gamepad_state_to_report(const GamepadInputState& state){
	xinput_report report {
		.rid = 0,
		.rsize = 20,
		.digital_buttons_1 =
			(state.DPAD_UP      << 0) |
			(state.DPAD_DOWN    << 1) |
			(state.DPAD_LEFT    << 2) |
			(state.DPAD_RIGHT   << 3) |
			(state.BUTTON_START << 4) |
			(state.BUTTON_BACK  << 5) |
			(state.BUTTON_LS    << 6) |
			(state.BUTTON_RS    << 7),
		.digital_buttons_2 =
			state.BUTTON_LB    << 0 |
			state.BUTTON_RB    << 1 |
			state.BUTTON_XBOX  << 2 |

			state.BUTTON_A     << 4 |
			state.BUTTON_B     << 5 |
			state.BUTTON_X     << 6 |
			state.BUTTON_Y     << 7,
		.lt = state.TRIGGER_LEFT,
		.rt = state.TRIGGER_RIGHT,
		.l_x = state.STICK_LEFT_X,
		.l_y = state.STICK_LEFT_Y,
		.r_x = state.STICK_RIGHT_X,
		.r_y = state.STICK_RIGHT_Y,
		.reserved_1 = { },
	};
	return report;
}