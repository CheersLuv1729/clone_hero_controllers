#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>

#include "pico/time.h"

#include "hardware/i2c.h"

#include "gamepad.h"
#include "pico/bootrom.h"

#include "USBDevice.h"
#include "USBDevices/XInputDevice.h"

#include "InternalDevice.h"
#include "InternalDevices/GHLiveInternalDevice.h"
#include "InternalDevices/WiiGH5InternalDevice.h"



static inline uint32_t board_millis(void) {
	return to_ms_since_boot(get_absolute_time());
}

int main(void)
{
	stdio_init_all();
	adc_init();

	USBDevice::usb_device = new XInputDevice();
	InternalDevice* internal_device = new WiiGH5InternalDevice();
	//InternalDevice* internal_device = new GHLiveInternalDevice();

	tusb_init(); // Call this AFTER USBDevice::usb_device has been set
	while (true)
	{
		GamepadInputState state = internal_device->getState();
		if(state.BUTTON_A && state.BUTTON_B && state.BUTTON_Y && state.BUTTON_BACK && state.BUTTON_START){
			reset_usb_boot(0, 0);
		}
		USBDevice::usb_device->sendInputState(state);
		tud_task();
		sleep_ms(1);
	}
}