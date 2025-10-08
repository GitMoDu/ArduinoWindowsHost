#pragma once

// Task scheduler configuration compiler flags.
#define _TASK_INLINE
#define _TASK_OO_CALLBACKS
#define _TASK_DO_NOT_YIELD
#define _TASK_HEADER_AND_CPP
#define _TASK_TICKLESS
#define _TASK_NON_ARDUINO
//#define _TASK_SLEEP_ON_IDLE_RUN
//#define _TASK_THREAD_SAFE
//#define _TASK_ISR_SUPPORT


#include <Arduino.h>
#include <TScheduler.hpp>
#include "LoopHost.hpp"

namespace TS
{
#if defined(_TASK_NON_ARDUINO)
	static unsigned long _task_millis()
	{
		return millis();
	}
#endif

#if !defined(_TASK_DO_NOT_YIELD)
	static void _task_yield()
	{
		yield();
	}
#endif
}

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

#if defined(_TASK_TICKLESS)
			const uint32_t idleTime = SchedulerBase.getNextRun();
			if (idleTime > 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else if (idleTime == 1)
#endif
			{
#if defined(_TASK_DO_NOT_YIELD)
				std::this_thread::yield();
#endif
			}
		}
	};

	using SchedulerHost = HostAddonScheduler<LoopHost>;
}
