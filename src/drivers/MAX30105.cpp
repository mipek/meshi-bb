#include "MAX30105.h"

#if PLAT == PLAT_WINDOWS
#	include <Windows.h>
#else
#	include <unistd.h>
#	include <sys/time.h>
#endif

static const uint8_t WAIT_FOR_CLEAR_TIMEOUT = 100;

// Returns current milliseconds. Start of the counter is not specified.
static uint64_t millis()
{
#if PLAT == PLAT_WINDOWS
	return GetTickCount64();
#else
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
#endif
}

static void sleep_ms(int sleepMs)
{
#if PLAT == PLAT_WINDOWS
	Sleep(sleepMs);
#else
	usleep(sleepMs * 1000);
#endif
}

void MAX30105::setup(adc_range adc, bool fifo_rollover_enable, sample_averaging avg, sample_rate sps, led_mode mode, uint8_t led_amplitude, led_pulse_width pulse_width)
{
	reset();
	set_fifo_averaging(avg);
	set_fifo_rollover(fifo_rollover_enable);
	set_led_mode(mode, led_amplitude);
	set_adc_range(adc);
	set_sample_rate(sps);
}

void MAX30105::reset()
{
	bit_mask(0x09, 0xBF, 0x40);
	wait_for_clear(0x09, 0x40);
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

bool MAX30105::read_sensor(measurement &values, uint8_t position)
{
	// TODO: refine this
	bus_.write_register(0x06, position);
	int count_active_leds = 0;
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

	if (count_active_leds > 0) {
		uint8_t buffer[2] = { 0x07, count_active_leds };
		bus_.write_bytes(buffer, 2);
		
		uint8_t bytes[3 * 3] = { 0 };
		bus_.read_bytes(&bytes, count_active_leds);
		
		// one measurement is 3-byte, we put it into a uint32_t for simplicity.
		values.red = *reinterpret_cast<uint32_t*>(bytes) & 0xFFFFFF;
		values.ir = *reinterpret_cast<uint32_t*>(bytes + 3) & 0xFFFFFF;
		values.green = *reinterpret_cast<uint32_t*>(bytes + 6) & 0xFFFFFF;
	}
	return false;
	//uint8_t bytes = bus_.read_register<uint8_t>(0x07, )
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

#if defined(STANDALONE_TEST)
#include <cstdio>
int main(int argc, char *argv[])
{
	MAX30105 sensor(1, 0x57);
	for (;;) {
		MAX30105::measurement msrmnt;
		float temp = sensor.read_temperature();

		sensor.read_sensor(msrmnt, 0);
		temp = sensor.read_temperature();

		printf("R: %d, IR: %d, G: %d, Temp: %f\n",
			msrmnt.red, msrmnt.ir, msrmnt.green, temp);

		sleep_ms(500);
	}
}
#endif //STANDALONE_TEST