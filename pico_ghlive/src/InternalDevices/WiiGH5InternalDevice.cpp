#include "WiiGH5InternalDevice.h"

uint8_t WiiGH5InternalDevice::mapTouchBoard(uint8_t input)
{
	// Taken from WiiBrew
	// https://wiibrew.org/wiki/Wiimote/Extension_Controllers/Guitar_Hero_(Wii)_Guitars
	// All in-between values are just guessed

	// An error value could possible be returned instead if the input doesn't match that
	// of WiiBrew's??

	constexpr uint8_t touchbar_table[] = {
		G  , // 0x00
		G  , // 0x01
		G  , // 0x02
		G  , // 0x03
		G  , // 0x04  Green
		G|R, // 0x05
		G|R, // 0x06
		G|R, // 0x07  Green and red
		R|R, // 0x08
		R  , // 0x09
		R  , // 0x0A  Red
		R  , // 0x0B
		R|Y, // 0x0C  Red and yellow
		R|Y, // 0x0D  Red and yellow
		R|Y, // 0x0E
		0  , // 0x0F  Not touching
		Y  , // 0x10
		Y  , // 0x11
		Y  , // 0x12  Yellow
		Y  , // 0x13  Yellow
		Y|B, // 0x14  Yellow and blue
		Y|B, // 0x15  Yellow and blue
		Y|B, // 0x16
		B  , // 0x17  Blue
		B  , // 0x18  Blue
		B  , // 0x19
		B|O, // 0x1A  Blue and orange
		B|O, // 0x1B
		B|O, // 0x1C
		O  , // 0x1D
		O  , // 0x1E
		O  , // 0x1F  Orange
	};

	return touchbar_table[input & 0x1F];
}



WiiGH5InternalDevice::WiiGH5InternalDevice(){
	i2c_init(i2c1, 100*1000);
	gpio_set_function(26, GPIO_FUNC_I2C);
	gpio_set_function(27, GPIO_FUNC_I2C);

	gpio_pull_up(26);
	gpio_pull_up(27);


	{
		// Disable encryption
		std::array<uint8_t, 2> a = {0xF0, 0x55};
		std::array<uint8_t, 2> b = {0xFB, 0x00};
		i2c_write_blocking(i2c1, 0x52, a.data(), 2, false);
		i2c_write_blocking(i2c1, 0x52, b.data(), 2, false);

		//sleep_ms(100);

		// Read ident
		uint8_t c = 0xFA;
		i2c_write_blocking(i2c1, 0x52, &c, 1, true);
		i2c_read_blocking(i2c1, 0x52, ident.data(), 6, false);
		sleep_ms(100);

		// Poll inputs once to "flush" out any state
		// Useful for if buttons were held down for config mode
		c = 0x00;
		i2c_write_blocking(i2c1, 0x52, &c, 1, true);

		std::array<uint8_t, 6> data;
		i2c_read_blocking(i2c1, 0x52, data.data(), 6, false);
	}

	// MPU-6050 (tilt sensor) code

	{
		std::array<uint8_t, 2> data = {0x6B, 0x00};
		i2c_write_blocking(i2c1, 0x68, data.data(), 2, false);
	}

}

bool WiiGH5InternalDevice::validateInput(const std::array<uint8_t, 6>& data){
	return (
		(data[2] & 0xE0) == 0x00 &&
		(data[3] & 0xE0) == 0x00 &&
		(data[4] & 0xAB) == 0xAB &&
		(data[5] & 0x06) == 0x06
	);
}

GamepadInputState WiiGH5InternalDevice::getState(){

	std::array<uint8_t, 6> data;
	std::array<uint8_t, 6> accel_data;

	do
	{
		uint8_t val = 0x00;
		i2c_write_blocking(i2c1, 0x52, &val, 1, true);
		i2c_read_blocking(i2c1, 0x52, data.data(), 6, false);
	} while (!validateInput(data));

	{
		uint8_t val = 0x3B;
		i2c_write_blocking(i2c1, 0x68, &val, 1, false);
		i2c_read_blocking(i2c1, 0x68, accel_data.data(), 6, false);
	}

	auto x = ((accel_data[0] << 8) | (accel_data[1]));

	float accel_x = ((int16_t)((accel_data[0] << 8) | (accel_data[1]))) / 16384.0;
	float accel_y = ((int16_t)((accel_data[2] << 8) | (accel_data[3]))) / 16384.0;
	float accel_z = ((int16_t)((accel_data[4] << 8) | (accel_data[5]))) / 16384.0;

	float angle_x = asin(clamp<float>(accel_x, -1, 1));
	float angle_y = asin(clamp<float>(accel_y, -1, 1));
	float angle_z = asin(clamp<float>(accel_z, -1, 1));

	const float DAMP_FACTOR = 0.99;

	static float smooth_x = 0;
	smooth_x = smooth_x * DAMP_FACTOR + angle_x * (1 - DAMP_FACTOR);

	uint8_t touchbar = mapTouchBoard(data[2]) & 0x00;

	// Whammy pressed = 0
	// Whammy released = 0x0f ??

	// TODO
	// Fix whammy mapping

	GamepadInputState s{
		.BUTTON_A = ((data[5] & (1 << 4)) == 0) || (touchbar & G),
		.BUTTON_B = ((data[5] & (1 << 6)) == 0) || (touchbar & R),
		.BUTTON_X = ((data[5] & (1 << 5)) == 0) || (touchbar & B),
		.BUTTON_Y = ((data[5] & (1 << 3)) == 0) || (touchbar & Y),

		.BUTTON_START = (data[4] & (1 << 2)) == 0,
		.BUTTON_BACK  = (data[4] & (1 << 4)) == 0,

		.DPAD_UP   = (data[5] & (1 << 0)) == 0,
		.DPAD_DOWN = (data[4] & (1 << 6)) == 0,
		.DPAD_LEFT  = 0,
		.DPAD_RIGHT = 0,

		.BUTTON_LB = ((data[5] & (1 << 7)) == 0) || (touchbar & O),
		.BUTTON_RB = 0,

		.BUTTON_LS = 0,
		.BUTTON_RS = 0,

		.BUTTON_XBOX = 0,

		.STICK_LEFT_X = ((int)(data[0] & 0x3f) - 32) << 10,
		.STICK_LEFT_Y = ((int)(data[1] & 0x3f) - 32) << 10,
		.STICK_RIGHT_X = (-smooth_x > 0.8) ? 32767 : 0, // TILT AXIS
		.STICK_RIGHT_Y = (clamp<int8_t>((data[3]) - 0x10,0, 0x0f)) << 11,

		.TRIGGER_LEFT  = 0,
		.TRIGGER_RIGHT = 0,
	};

	return s;
};