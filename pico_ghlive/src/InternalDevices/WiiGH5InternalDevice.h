#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <algorithm>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "gamepad.h"
#include <math.h>

#include "InternalDevice.h"


template<class T>
constexpr const T clamp(const T& v, const T& l, const T& h)
{
	return (v < l) ? l : (v > h) ? h : v;
}

class WiiGH5InternalDevice : public InternalDevice{

	std::array<uint8_t, 6> ident;

	static constexpr uint8_t G = 1 << 0;
	static constexpr uint8_t R = 1 << 1;
	static constexpr uint8_t Y = 1 << 2;
	static constexpr uint8_t B = 1 << 3;
	static constexpr uint8_t O = 1 << 4;

	static bool validateInput(const std::array<uint8_t, 6>& data);
	static uint8_t mapTouchBoard(uint8_t input);

public:

  	WiiGH5InternalDevice();

	GamepadInputState getState();
};