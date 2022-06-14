#pragma once
#include "pico/stdlib.h"


struct GamepadInputState {
	bool BUTTON_A;
	bool BUTTON_B;
	bool BUTTON_X;
	bool BUTTON_Y;

	bool BUTTON_START;
	bool BUTTON_BACK;

	bool DPAD_UP;
	bool DPAD_DOWN;
	bool DPAD_LEFT;
	bool DPAD_RIGHT;

	bool BUTTON_LB;
	bool BUTTON_RB;

	bool BUTTON_LS;
	bool BUTTON_RS;

	bool BUTTON_XBOX;

	int16_t STICK_LEFT_X;
	int16_t STICK_LEFT_Y;
	int16_t STICK_RIGHT_X;
	int16_t STICK_RIGHT_Y;
	uint8_t TRIGGER_LEFT;
	uint8_t TRIGGER_RIGHT;
};