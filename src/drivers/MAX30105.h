#ifndef _include_drivers_max30105_
#define _include_drivers_max30105_

#include <cstdint>
#include "../blackbox.h"
#include "../i2cmaster.h"

enum class led_mode
{
	red = 0x02,
	red_ir = 0x03,
	red_green_ir = 0x07
};

enum class led_pulse_width
{
	pw69 = 0x00, // µs
	pw118 = 0x01,
	pw215 = 0x02,
	pw411 = 0x03
};

enum class sample_averaging
{
	avg1 = 0x00,
	avg2 = 0x20,
	avg4 = 0x40,
	avg8 = 0x60,
	avg16 = 0x80,
	avg32 = 0xa0
};

enum class sample_rate
{
	sps50 = 0x00,
	sps100 = 0x04,
	sps200 = 0x08,
	sps400 = 0x0c,
	sps800 = 0x10,
	sps1000 = 0x14,
	sps1600 = 0x18,
	sps3200 = 0x1c
};

enum class adc_range
{
	adc2048 = 0,
	adc4096 = 0x20,
	adc8192 = 0x40,
	adc16384 = 0x60
};

/**
 * Simple interface to the MAX30105 particle sensing IC
 */
class MAX30105
{
	i2c_master bus_;
	led_mode mode_;

public:
	struct measurement
	{
		uint32_t red, ir, green;
	};

	MAX30105(int bus, uint8_t address): bus_(bus)
	{
		bus_.set_slave_address(address);
	}
	void setup(adc_range adc = adc_range::adc2048, bool fifo_rollover_enable = true,
		sample_averaging avg = sample_averaging::avg1, sample_rate sps = sample_rate::sps50,
		led_mode mode = led_mode::red, uint8_t led_amplitude = 0x1F, led_pulse_width pulse_width = led_pulse_width::pw69);
	void reset();

public:
	void set_fifo_averaging(sample_averaging avg);
	void set_fifo_rollover(bool enable_rollover);

	// Amplitude range: 0x00 -> 0xFF (0.0 -> 50.0 mA, see DS page 21)
	void set_led_mode(led_mode mode, uint8_t amplitude);
	void set_led_pulse_width(led_pulse_width width);
	void set_sample_rate(sample_rate sps);
	void set_adc_range(adc_range range);

public:
	bool read_sensor(measurement &values, uint8_t position);
	float read_temperature();
	
private:
	bool wait_for_clear(uint8_t reg, uint8_t check);
	void bit_mask(uint8_t reg, uint8_t mask, uint8_t val);
};

#endif //_include_drivers_max30105_