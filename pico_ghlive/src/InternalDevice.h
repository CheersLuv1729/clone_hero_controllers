#pragma once

#include "gamepad.h"

class InternalDevice{

public:

	virtual GamepadInputState getState() = 0;

};