#pragma once


#include <stdint.h>

namespace ArduinoWindowsHost
{
	namespace Hal
	{
		typedef enum WiringPinMode : uint8_t
		{
			OUTPUT,
			OUTPUT_OPEN_DRAIN,
			INPUT,
			INPUT_ANALOG,
			INPUT_PULLUP,
			INPUT_PULLDOWN
		} WiringPinMode;

		typedef enum WiringState : uint8_t
		{
			LOW,
			HIGH
		} WiringState;

		template<uint8_t IO_COUNT = 32>
		struct ArduinoIo
		{
			struct PinState
			{
				uint8_t Mode = INPUT; // Default to input.
				uint8_t State = LOW;  // Default to low.
			};

			PinState State[IO_COUNT]{};

			void digitalWrite(const uint8_t pin, const WiringState state)
			{
				if (pin < IO_COUNT && State[pin].Mode == OUTPUT)
					State[pin].State = state != LOW;
			}

			uint8_t digitalRead(const uint8_t pin) const
			{
				if (pin < IO_COUNT && State[pin].Mode == INPUT)
					return State[pin].State * 1;
				else
					return 0;
			}

			void pinMode(const uint8_t pin, const WiringPinMode mode)
			{
				if (pin < IO_COUNT)
					State[pin].Mode = (mode == OUTPUT) * OUTPUT;
			}

			void reset()
			{
				for (uint8_t i = 0; i < IO_COUNT; ++i)
				{
					State[i].Mode = INPUT;
					State[i].State = LOW;
				}
			}
		};
	}
}
