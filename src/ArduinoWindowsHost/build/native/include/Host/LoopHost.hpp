#pragma once

#include <stdint.h>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <condition_variable>

#include "../HAL/Arduino.h"

namespace ArduinoWindowsHost
{
	// LoopHost
	// - Provides an Arduino-like lifecycle: setup -> loop (repeated) -> setdown.
	// - Hosts a single-threaded "loop" and a dispatch queue to marshal work onto that loop.
	// - Thread-safe start/stop flags guarded by an internal mutex.
	class LoopHost
	{
	protected:
		// Tracks RX state changes to fire serialEvent when Serial input changes.
		volatile uint32_t SerialStateId = UINT32_MAX;

	private:
		// Lifetime flags (guarded by mutex).
		volatile bool cancelled = false;
		volatile bool running = false;

		// Guards access to running/cancelled flags.
		std::mutex mutex{};

	private:
		// Dispatch queue to run work on the host loop thread.
		std::mutex dispatchMutex{};
		std::queue<std::function<void()>> dispatchQueue{};
		std::condition_variable dispatchCv{};
		std::thread::id loopThreadId{};

	protected:
		// Arduino-style setup method, runs once at start.
		virtual void setup() {}

		// Arduino-style loop method, runs repeatedly while not cancelled.
		// Default yields the thread; override to provide user logic.
		virtual void loop()
		{
			std::this_thread::yield();
		}

		// Arduino-style serial event method, runs when serial data is received.
		virtual void serialEvent() {}

		// Opposite of setup, runs once at stop.
		virtual void setdown() {}

	public:
		LoopHost() = default;

	public:
		// Returns true while the host is running.
		bool isRunning()
		{
			std::lock_guard<std::mutex> lock(mutex);
			return running;
		}

		// Returns true if cancellation has been requested or the host is not running.
		bool isCancelled()
		{
			std::lock_guard<std::mutex> lock(mutex);
			return cancelled || !running;
		}

		// Main loop entry point. Sets up, runs until cancelled, then tears down.
		void OnRun()
		{
			setRunning(true);

			try
			{
				loopThreadId = std::this_thread::get_id();

				Hal::reset();

				SerialStateId = Serial.GetRxId();

				setup();

				while (!isCancelled())
				{
					// Check for serial events.
					if (Serial)
					{
						const uint32_t serialStateId = Serial.GetRxId();
						if (serialStateId != SerialStateId)
						{
							SerialStateId = serialStateId;
							serialEvent();
						}
					}

					// Run the main loop.
					loop();

					// Run externally posted work.
					drainDispatchQueue();
				}

				// Opposite of setup.
				setdown();
			}
			catch (...)
			{
				Serial.println("Exception!");
			}

			setRunning(false);
		}

		// Marks the host as started (clears cancellation).
		virtual void OnStart()
		{
			std::lock_guard<std::mutex> lock(mutex);
			running = true;
			cancelled = false;
		}

		// Requests cancellation and wakes any waiters.
		virtual void OnStop()
		{
			{
				std::lock_guard<std::mutex> lock(mutex);
				cancelled = true;
			}
			dispatchCv.notify_all();
		}

		// Post a callable to run on the host loop thread.
		// If called from the loop thread, the callable runs inline.
		template<typename F>
		void Post(F&& f)
		{
			// If already on loop thread, run inline.
			if (std::this_thread::get_id() == loopThreadId)
			{
				std::forward<F>(f)();
				return;
			}

			using Fn = typename std::decay<F>::type;
			auto fn = std::make_shared<Fn>(std::forward<F>(f)); // make copyable
			{
				std::lock_guard<std::mutex> lk(dispatchMutex);
				dispatchQueue.emplace([fn]() { (*fn)(); });
			}
			dispatchCv.notify_one();
		}

		// Post a callable to the loop thread and wait for completion.
		// If called from the loop thread, the callable runs inline.
		template<typename F>
		void PostAndWait(F&& f)
		{
			// If already on loop thread, run inline.
			if (std::this_thread::get_id() == loopThreadId)
			{
				std::forward<F>(f)();
				return;
			}

			using Fn = typename std::decay<F>::type;
			auto fn = std::make_shared<Fn>(std::forward<F>(f)); // copyable handle
			auto done = std::make_shared<std::promise<void>>(); // copyable handle
			auto fut = done->get_future();

			{
				std::lock_guard<std::mutex> lk(dispatchMutex);
				dispatchQueue.emplace([fn, done]() {
					(*fn)();
					done->set_value();
					});
			}
			dispatchCv.notify_one();
			fut.wait();
		}

	private:
		// Drains all pending work from the dispatch queue and runs it on the loop thread.
		void drainDispatchQueue()
		{
			std::queue<std::function<void()>> local;
			{
				std::lock_guard<std::mutex> lk(dispatchMutex);
				if (dispatchQueue.empty())
					return;
				local.swap(dispatchQueue);
			}
			while (!local.empty())
			{
				auto work = std::move(local.front());
				local.pop();
				if (work) work();
			}
		}

		// Internal helper to set the running flag under lock.
		void setRunning(const bool state)
		{
			std::lock_guard<std::mutex> lock(mutex);
			running = state;
		}
	};
}