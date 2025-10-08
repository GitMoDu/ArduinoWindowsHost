#pragma once

#include <mutex>
#include <thread>

namespace ArduinoWindowsHost
{
	template<typename BaseType>
	class HostAddonParameter : public BaseType
	{
	protected:
		virtual void OnParameterChange(const int parameter, const uint8_t value) {}

	public:
		HostAddonParameter() : BaseType() {}

	public:
		void OnParameterInput(const int parameter, const uint8_t value = 0)
		{
			LoopHost::Post([this, parameter, value]()
				{
					OnParameterChange(parameter, value);
				});
		}
	};
}
