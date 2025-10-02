#pragma once

#include "pch.h"

#include <ArduinoWindowsHost.h>

namespace ArduinoBlinkHost
{
	using namespace ArduinoWindowsHost;

	/// <summary>
	/// Implements an Arduino host that blinks the built-in LED and handles serial events.
	/// </summary>
	class ArduinoHost : public LoopHost
	{
	private:
		bool TickTock = false;

	public:
		ArduinoHost() : LoopHost()
		{
		}

	protected:
		void setup() override
		{
			Serial.begin(115200);
			while (!Serial) {}

			Serial.println(F("ArduinoBlink Setup"));

			pinMode(LED_BUILTIN, OUTPUT);

			digitalWrite(LED_BUILTIN, HIGH);

			Serial.println(F("ArduinoBlink Started!"));
		}


		void serialEvent() override
		{
			int32_t count = static_cast<int32_t>(Serial.available());
			Serial.print(F("!Serial Event: "));
			Serial.print(count);
			Serial.println(F(" characters"));

			// Discard all event characters.
			while (count > 0)
			{
				Serial.read();
				count--;
			}
		}

		void loop() override
		{
			digitalWrite(LED_BUILTIN, HIGH);
			delay(500);

			if (TickTock)
				Serial.println(F("\tTock"));
			else
				Serial.println(F("\tTick"));

			TickTock = !TickTock;

			digitalWrite(LED_BUILTIN, LOW);
			delay(500);
		}
	};
}