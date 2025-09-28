#pragma once

#include <mutex>
#include <thread>

namespace ArduinoWindowsHost
{
	template<typename BaseType>
	class HostAddonParameter : public BaseType
	{
	protected:
		using BaseType::cancelled;
		using BaseType::running;
		using BaseType::mutex;

	protected:
		virtual void OnParameterChange(const int parameter, const uint8_t value) {}

	public:
		HostAddonParameter() : BaseType() {}

	public:
		void OnParameterInput(const int parameter, const uint8_t value = 0)
		{
			std::lock_guard<std::mutex> lock(BaseType::mutex);
			if (!cancelled && running)
			{
				OnParameterChange(parameter, value);
			}
		}
	};
}
