#pragma once

#include <stdint.h>

#include <chrono>

#include "ArduinoIo.hpp"
#include "ArduinoSerialPort.hpp"

namespace ArduinoWindowsHost
{
	namespace Hal
	{
		namespace State
		{
			volatile static uint32_t BootMillis = 0;
			volatile static uint32_t BootMicros = 0;
		}

		static uint32_t millis()
		{
			using namespace std::chrono;

			return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()) - State::BootMillis;
		}

		static uint32_t micros()
		{
			using namespace std::chrono;

			return static_cast<uint32_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count()) - State::BootMicros;
		}

		static void delay(const uint32_t duration)
		{
			using namespace std::chrono;

			std::this_thread::sleep_for(std::chrono::milliseconds(duration));
		}

		static uint32_t random(const uint32_t range)
		{
			return std::rand() % range;
		}

		static void delayMicroseconds(const uint32_t duration)
		{
			using namespace std::chrono;

			std::this_thread::sleep_for(std::chrono::microseconds(duration));
		}
	}

	namespace Hal
	{
		namespace Config
		{
			static constexpr size_t SerialLineCapacity = 2048;
		}

		static ArduinoSerialPort Serial(0, Config::SerialLineCapacity);
		static ArduinoSerialPort Serial1(1, Config::SerialLineCapacity);
		static ArduinoSerialPort Serial2(1, Config::SerialLineCapacity);
	}

	namespace Hal
	{
		static constexpr uint8_t LED_BUILTIN = 13;

		namespace Config
		{
			static constexpr uint8_t IoCount = 32;
		}

		namespace State
		{
			static ArduinoIo<Config::IoCount> IoHal{};
		}

		static void digitalWrite(const uint8_t pin, const WiringState state)
		{
			State::IoHal.digitalWrite(pin, state);
		}

		static uint8_t digitalRead(const uint8_t pin)
		{
			return State::IoHal.digitalRead(pin);
		}

		static void pinMode(const uint8_t pin, const WiringPinMode mode)
		{
			State::IoHal.pinMode(pin, mode);
		}
	}

	namespace Hal
	{
		static void reset()
		{
			using namespace std::chrono;

			State::BootMillis = static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
			State::BootMicros = static_cast<uint32_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());

			State::IoHal.reset();
			Serial.flushRx();
			Serial.flushTx();
			Serial1.flushRx();
			Serial1.flushTx();
			Serial2.flushRx();
			Serial2.flushTx();
		}
	}
}

using namespace ArduinoWindowsHost::Hal;

#define PROGMEM
#define F(a) (a)
