#pragma once


#include <stdint.h>
#include <algorithm>
#include <thread>
#include <mutex>

#include "../HAL/Arduino.h"

namespace ArduinoWindowsHost
{
	class LoopHost
	{
	protected:
		virtual void setup() {}

		virtual void loop()
		{
			std::this_thread::yield();
		}

		virtual void serialEvent() {};

	protected:
		std::mutex mutex{};

	protected:
		volatile uint32_t SerialStateId = UINT32_MAX;

		volatile bool cancelled = false;
		volatile bool running = false;

	public:
		LoopHost()
		{
		}

	protected:
		virtual void OnDestroy() {}

	public:
		bool isRunning()
		{
			std::lock_guard<std::mutex> lock(mutex);
			return running;
		}

		bool isCancelled()
		{
			std::lock_guard<std::mutex> lock(mutex);
			return cancelled || !running;
		}

		void OnRun()
		{
			try
			{
				Hal::reset();

				SerialStateId = Serial.GetRxId();

				setup();

				setRunning(true);
				bool wasCancelled = false;
				while (!isCancelled())
				{
					if (Serial)
					{
						const uint32_t serialStateId = Serial.GetRxId();
						if (serialStateId != SerialStateId)
						{
							SerialStateId = serialStateId;
							serialEvent();
						}
					}
					loop();
				}

				OnDestroy();
			}
			catch (...)
			{
				Hal::Serial.println("Exception!");
			}

			setRunning(false);
		}

		virtual void OnStart()
		{
			std::lock_guard<std::mutex> lock(mutex);
			running = true;
			cancelled = false;
		}

		virtual void OnStop()
		{
			std::lock_guard<std::mutex> lock(mutex);
			cancelled = true;
		}

	private:
		void setRunning(const bool state)
		{
			std::lock_guard<std::mutex> lock(mutex);
			running = state;
		}
	};
}
