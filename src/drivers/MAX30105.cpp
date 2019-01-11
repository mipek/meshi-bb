#include "MAX30105.h"
#include "../plat_compat.h"

static const uint8_t WAIT_FOR_CLEAR_TIMEOUT = 100;

static int interpret_as_int24(uint8_t *p)
{
	return (p[0] << 16) | (p[1] << 8) | p[2];
}

bool MAX30105::check_connection()
{
	uint8_t tmp = 0xFF;
	return bus_.is_valid() && (bus_.read_bytes(&tmp, sizeof(uint8_t)) >= 1) && tmp != 0xFF;
}

void MAX30105::setup(
	adc_range adc, bool fifo_rollover_enable,
	sample_averaging avg, sample_rate sps,
	led_mode mode, uint8_t led_amplitude, led_pulse_width pulse_width)
{
	reset();

	set_led_pulse_width(pulse_width);
	set_led_mode(mode, led_amplitude);

	set_sample_rate(sps);

	set_adc_range(adc);

	set_fifo_averaging(avg);
	set_fifo_rollover(fifo_rollover_enable);

	// fifo clear
	clear_fifo();
}

bool MAX30105::reset()
{
	bit_mask(0x09, 0xBF, 0x40);
	return wait_for_clear(0x09, 0x40);
}

void MAX30105::bit_mask(uint8_t reg, uint8_t mask, uint8_t val)
{
	uint8_t original = bus_.read_register<uint8_t>(reg);
	original &= mask;
	bus_.write_register(reg, original | val);
}

bool MAX30105::wait_for_clear(uint8_t reg, uint8_t check)
{
	uint64_t current = millis();
	while (millis() - current < WAIT_FOR_CLEAR_TIMEOUT)
	{
		uint8_t response = bus_.read_register<uint8_t>(reg);
		if ((response & check) == 0)
			return true;
		sleep_ms(10);
	}
	return false;
}

void MAX30105::set_led_mode(led_mode mode, uint8_t amplitude)
{
	bit_mask(0x09, 0b11111000, (uint8_t)mode);

	mode_ = mode;
	switch (mode) {
	case led_mode::red_green_ir:
		bus_.write_register(0x0E, amplitude);
		bus_.write_register(0x11, 0b00100001);
		bus_.write_register(0x12, 0b00000011);
	case led_mode::red_ir:
		bus_.write_register(0x0D, amplitude);
	case led_mode::red:
		bus_.write_register(0x0C, amplitude);
		break;
	}

	// always set proximity mode amplitude
	bus_.write_register(0x10, amplitude);
}

void MAX30105::set_led_pulse_width(led_pulse_width width)
{
	bit_mask(0x0A, 0b11111100, (uint8_t)width);
}

void MAX30105::set_fifo_averaging(sample_averaging avg)
{
	bit_mask(0x08, 0b11100000, (uint8_t)avg);
}

void MAX30105::set_fifo_rollover(bool enable_rollover)
{
	bit_mask(0x08, 0b11101111, enable_rollover ? 0x10 : 0);
}

void MAX30105::set_sample_rate(sample_rate sps)
{
	bit_mask(0x0A, 0b11100011, (uint8_t)sps);
}

void MAX30105::set_adc_range(adc_range range)
{
	bit_mask(0x0A, 0b10011111, (uint8_t)range);
}

uint8_t MAX30105::get_read_pointer()
{
	return bus_.read_register<uint8_t>(0x06);
}

uint8_t MAX30105::get_write_pointer()
{
	return bus_.read_register<uint8_t>(0x04);
}

void MAX30105::clear_fifo()
{
	bus_.write_register(0x04, 0);
	bus_.write_register(0x05, 0);
	bus_.write_register(0x06, 0);
}

// TODO: we should put the data into a buffer and only provide access to measurements via the buffer
// TODO: interrupts instead of polling?
void MAX30105::update()
{
	uint8_t read = get_read_pointer();
	uint8_t write = get_write_pointer();

	int delta = write - read;
	if (delta != 0)
	{
		if (delta < 0) {
			delta += 32;
		}
		
		uint8_t single_read_size = get_count_active_leds() * 3;

		// if we are lagging behind we'll try to catch up
		while (delta > 0)
		{
			uint8_t reg = 0x07;
			bus_.write_bytes(&reg, 1);
			
			uint8_t bytes[3 * 3] = {0};
			bus_.read_bytes(bytes, single_read_size);
		
			int val = interpret_as_int24(bytes);
			
			measurement values;
			values.red = interpret_as_int24(bytes);
			values.ir = interpret_as_int24(bytes + 3);
			values.green = interpret_as_int24(bytes + 6);
			
			delta -= single_read_size;
		}
	}
}

bool MAX30105::read_sensor(measurement &values, uint8_t position)
{
	// TODO: refine this
	bus_.write_register(0x06, position);
	
	uint8_t reg = 0x07;
	bus_.write_bytes(&reg, 1);

	uint8_t bytes[3 * 3] = {0};
	bus_.read_bytes(&bytes, get_count_active_leds() * 3);

	// one measurement is 3-byte, we put it into a uint32_t for simplicity.
	values.red = interpret_as_int24(bytes);
	values.ir = interpret_as_int24(bytes + 3);
	values.green = interpret_as_int24(bytes + 6);
	return true;
}

float MAX30105::read_temperature()
{
	bus_.write_register(0x21, 0x01);
	if (wait_for_clear(0x21, 0x01)) {
		int tempInt = bus_.read_register<uint8_t>(0x1F);
		int tempFrac = bus_.read_register<uint8_t>(0x20);
		return tempInt + (tempFrac * 0.0625f);
	}
	return -999.0f;
}

uint8_t MAX30105::get_count_active_leds()
{
	uint8_t count_active_leds = 0;
	switch (mode_) {
	case led_mode::red_green_ir:
		count_active_leds = 3;
		break;
	case led_mode::red_ir:
		count_active_leds = 2;
		break;
	case led_mode::red:
		count_active_leds = 1;
		break;
	}
	return count_active_leds;
}

#if defined(STANDALONE_TEST)
#include <cstdio>
int main(int argc, char *argv[])
{
	MAX30105 sensor(1, 0x57);
	if (!sensor.check_connection()) {
		printf("MAX30105 is not properly connected\n");
		return 1;
	}
	
	sensor.setup();
	
	for (;;) {
		//sensor.update();
		
		MAX30105::measurement msrmnt = {0};
		float temp = sensor.read_temperature();

		sensor.read_sensor(msrmnt, 0);
		temp = sensor.read_temperature();
		

		printf("R: %d, IR: %d, G: %d, Temp: %f\n",
			msrmnt.red, msrmnt.ir, msrmnt.green, temp);

		sleep_ms(500);
	}
	return 0;
}
#endif //STANDALONE_TEST
