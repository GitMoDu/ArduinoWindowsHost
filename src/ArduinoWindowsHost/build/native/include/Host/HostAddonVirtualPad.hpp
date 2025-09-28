#pragma once

#include <stdint.h>
#include <algorithm>
#include <thread>
#include <mutex>

#include <VirtualPads.h>
#include <winrt/Windows.Gaming.Input.h>

namespace ArduinoWindowsHost
{
	/// <summary>
	/// Extends a host to provide virtual gamepad input handling using a VirtualPad instance.
	/// </summary>
	template<typename BaseType>
	class HostAddonVirtualPad : public BaseType
	{
	protected:
		VirtualPad::WindowsPad::VirtualPadUpdater PadInstance{};

	protected:
		virtual void OnVirtualPadUpdate(VirtualPad::WindowsPad::VirtualPadType& pad) {}

	public:
		HostAddonVirtualPad() : BaseType() {}

		void OnGamepadInput(const winrt::Windows::Gaming::Input::GamepadReading& reading)
		{
			PadInstance.OnGamepadInput(reading);

			std::lock_guard<std::mutex> lock(mutex);
			if (!cancelled && running)
			{
				OnVirtualPadUpdate(PadInstance);
			}
		}

	protected:
		VirtualPad::WindowsPad::VirtualPadType& GetVirtualPad()
		{
			return PadInstance;
		}
	};
}
