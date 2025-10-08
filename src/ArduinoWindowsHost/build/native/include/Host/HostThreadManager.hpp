#pragma once

#include <stdint.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include "LoopHost.hpp"

namespace ArduinoWindowsHost
{
	class HostThread
	{
	private:
		std::thread* ExecutionThread = nullptr;

	public:
		HostThread() {}

		~HostThread()
		{
			if (ExecutionThread != nullptr)
			{
				delete ExecutionThread;
				ExecutionThread = nullptr;
			}
		}

		void Start(LoopHost& host)
		{
			host.OnStart();
			ExecutionThread = new std::thread(&LoopHost::OnRun, &host);
		}

		void Stop(LoopHost& host)
		{
			if (ExecutionThread != nullptr)
			{
				// Signal the host to stop
				host.PostAndWait([&host]() {
					host.OnStop();
					});

				// Wait for the thread to finish
				if (ExecutionThread->joinable())
				{
					ExecutionThread->join();

					while (host.isRunning())
					{
						std::this_thread::yield();
					}
				}

				delete ExecutionThread;
				ExecutionThread = nullptr;
			}
		}
	};

	template<typename HostType>
	class TemplateHostManager
	{
	private:
		HostThread ThreadManager{};

	public:
		HostType* Host = nullptr;

	public:
		TemplateHostManager() {}

		bool isRunning()
		{
			return (Host != nullptr && Host->isRunning());
		}

		void Start()
		{
			if (Host != nullptr)
			{
				ThreadManager.Stop(*Host);
				delete Host;
				Host = nullptr;
			}
			Host = new HostType();
			ThreadManager.Start(*Host);
		}

		void Stop()
		{
			if (Host)
			{
				ThreadManager.Stop(*Host);
				delete Host;
				Host = nullptr;
			}
		}
	};
}