#pragma once

#define _TASK_OO_CALLBACKS
#define _TASK_DO_NOT_YIELD
#define _TASK_TICKLESS
#define _TASK_INLINE

#include <Arduino.h>
#include <TScheduler.hpp>

#include "LoopHost.hpp"

namespace ArduinoWindowsHost
{
	template<typename BaseHost>
	class HostAddonScheduler : public BaseHost
	{
	protected:
		TS::Scheduler SchedulerBase{};

	public:
		HostAddonScheduler() : BaseHost()
		{
		}

	protected:
		void loop() override
		{
			SchedulerBase.execute();

			const uint32_t idleTime = SchedulerBase.getNextRun();
			if (idleTime > 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else if (idleTime == 1)
			{
				std::this_thread::yield();
			}
		}
	};

	using SchedulerHost = HostAddonScheduler<LoopHost>;
}
