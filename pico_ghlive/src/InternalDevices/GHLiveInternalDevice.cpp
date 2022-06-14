#include "GHLiveInternalDevice.h"

// Pin refers to the GPIO pin number
// Not the numbered pin on the board
// ie physical pin 1 is GP0

void GHLiveInternalDevice::initPinInput(int pin){
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
	gpio_pull_up(pin);
}

void GHLiveInternalDevice::initPinOutput(int pin){
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_OUT);
}

void GHLiveInternalDevice::initPinADC(int pin){
	adc_gpio_init(pin);
}

uint16_t GHLiveInternalDevice::readInputADC(int input){
	adc_select_input(input);
	uint16_t ret = adc_read() << 4;
	return ret;
}

bool GHLiveInternalDevice::readButton(int pin){
	return gpio_get(pin) ? 0 : 1;
}


GHLiveInternalDevice::GHLiveInternalDevice(){

	initPinInput(GP_STRUM_U);  // Up   (Strum)
	initPinInput(GP_STRUM_D);  // Down (Strum)

	initPinOutput(GP_LED_1); // LED Ch1
	initPinOutput(GP_LED_2); // LED Ch2
	initPinOutput(GP_LED_3); // LED Ch3
	initPinOutput(GP_LED_4); // LED Ch4

	// VCC Used with LEDs. We set this high and pull Ch low to power them
	initPinOutput(GP_LED_VCC);

	initPinInput(GP_START); // Start
	initPinInput(GP_SELECT); // Select

	initPinInput(GP_STICK_R); // Right (Stick)
	initPinInput(GP_STICK_L); // Left  (Stick)
	initPinInput(GP_STICK_D); // Down  (Stick)
	initPinInput(GP_STICK_U); // Up    (Stick)
	initPinInput(GP_GH_KEY); // GH_Key

	// On the board, Menu is labelled as VB
	// It's used as the sync button and connected to power in a typical controller
	// However for this project, VCC for this board is connected to GND and the button
	// works the same as the rest of them (low = pressed)
	initPinInput(GP_MENU); // Menu

	initPinInput(GP_FRET_B_1); // B1
	initPinInput(GP_FRET_B_2); // B2
	initPinInput(GP_FRET_B_3); // B3
	initPinInput(GP_FRET_W_1); // W1
	initPinInput(GP_FRET_W_2); // W2
	initPinInput(GP_FRET_W_3); // W3

	// Mode (Connects to the neck but goes nowhere inside the neck)
	// Maybe used if different necks are connected?? but none that I know of
	// The port for the neck has 4 pins that don't even connect to it so ??
	initPinInput(GP_FRET_MODE);

	// For this project the tilt sensor is a binary high/low but it's connected to
	// ADC just in case I change my mind in future and replace the sensor
	initPinADC(GP_TILT); // Tilt   (ADC1)
	initPinADC(GP_WHAMMY); // Whammy (ADC2)

	gpio_put(GP_LED_VCC, 1);

	gpio_put(GP_LED_1, 1);
	gpio_put(GP_LED_2, 1);
	gpio_put(GP_LED_3, 0);
	gpio_put(GP_LED_4, 1);

}

GamepadInputState GHLiveInternalDevice::getState(){
	GamepadInputState s{
		.BUTTON_A = readButton(GP_FRET_B_1),
		.BUTTON_B = readButton(GP_FRET_B_2),
		.BUTTON_X = readButton(GP_FRET_W_1),
		.BUTTON_Y = readButton(GP_FRET_B_3),

		.BUTTON_START = readButton(GP_START),
		.BUTTON_BACK  = readButton(GP_SELECT),

		.DPAD_UP   = readButton(GP_STICK_U) || readButton(GP_STRUM_U),
		.DPAD_DOWN = readButton(GP_STICK_D) || readButton(GP_STRUM_D),
		.DPAD_LEFT  = readButton(GP_STICK_L),
		.DPAD_RIGHT = readButton(GP_STICK_R),

		.BUTTON_LB = readButton(GP_FRET_W_2),
		.BUTTON_RB = readButton(GP_FRET_W_3),

		.BUTTON_LS = readButton(GP_GH_KEY),
		.BUTTON_RS = false,

		.BUTTON_XBOX = readButton(GP_MENU),

		.STICK_LEFT_X = (1 << 7),
		.STICK_LEFT_Y = (1 << 7),
		.STICK_RIGHT_X = 0x7fff - (readInputADC(ADC_TILT) >> 1),
		.STICK_RIGHT_Y = readInputADC(ADC_WHAMMY) >> 1,

		.TRIGGER_LEFT  = (1 << 7),
		.TRIGGER_RIGHT = (1 << 7),


	};
	return s;
};
