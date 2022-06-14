#pragma once
#include "InternalDevice.h"
#include "hardware/adc.h"

class GHLiveInternalDevice : public InternalDevice{


	// Pin refers to the GPIO pin number
	// Not the numbered pin on the board
	// ie physical pin 1 is GP0

	void initPinInput(int pin);
	void initPinOutput(int pin);
	void initPinADC(int pin);
	uint16_t readInputADC(int input);
	bool readButton(int pin);

	static constexpr int GP_STRUM_U = 0;
	static constexpr int GP_STRUM_D = 1;

	static constexpr int GP_LED_1 = 5;
	static constexpr int GP_LED_2 = 4;
	static constexpr int GP_LED_3 = 3;
	static constexpr int GP_LED_4 = 2;

	static constexpr int GP_LED_VCC = 6;

	static constexpr int GP_START  = 7;
	static constexpr int GP_SELECT = 8;

	static constexpr int GP_STICK_R = 10;
	static constexpr int GP_STICK_L = 11;
	static constexpr int GP_STICK_D = 12;
	static constexpr int GP_STICK_U = 13;
	static constexpr int GP_GH_KEY  = 14;
	static constexpr int GP_MENU    = 15;

	static constexpr int GP_FRET_B_1 = 18;
	static constexpr int GP_FRET_B_2 = 17;
	static constexpr int GP_FRET_B_3 = 16;
	static constexpr int GP_FRET_W_1 = 21;
	static constexpr int GP_FRET_W_2 = 20;
	static constexpr int GP_FRET_W_3 = 19;

	static constexpr int GP_FRET_MODE = 22;

	static constexpr int GP_TILT   = 27;
	static constexpr int GP_WHAMMY = 28;

	static constexpr int ADC_TILT   = 1;
	static constexpr int ADC_WHAMMY = 2;

public:


	GHLiveInternalDevice();

	GamepadInputState getState();

};