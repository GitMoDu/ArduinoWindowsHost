#pragma once

#include "MainPage.g.h"

#include <ArduinoWindowsHost.h>


#include "Host/ArduinoHost.hpp"

#include <vector>
#include <algorithm>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.Threading.h>

namespace winrt::DemoSceneWinRT::implementation
{
	using namespace ArduinoWindowsHost;

	static constexpr uint8_t ScreenScale = 6;
	static constexpr uint16_t ScreenWidth = 96 * ScreenScale;
	static constexpr uint16_t ScreenHeight = 64 * ScreenScale;

	static constexpr uint8_t UnitShift = GetBitShifts(VERTEX16_UNIT);

	static constexpr auto bareShift = GetBitShifts(MaxValue<uint16_t>(ScreenWidth, ScreenHeight) / 2);
	static constexpr auto value = UnitShift - bareShift;

	using ArduinoHostType = DemoSceneWindows::ArduinoHost<ScreenWidth, ScreenHeight>;

	struct MainPage : MainPageT<MainPage>
	{
		TemplateHostManager<ArduinoHostType> HostManager{};

		// Gamepad support members
		std::vector<winrt::Windows::Gaming::Input::Gamepad> m_gamepads;
		winrt::event_token m_gamepadAddedToken;
		winrt::event_token m_gamepadRemovedToken;
		winrt::Windows::System::Threading::ThreadPoolTimer m_gamepadPollTimer{ nullptr };

		MainPage();

		~MainPage();


		void updateArduinoHostState(const bool enabled);

		void runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);


		// Gamepad support methods
		void initializeGamepadSupport();
		void uninitializeGamepadSupport();


		void shaderGroupChecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void checkScene_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void checkScene_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void sceneGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void sceneGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void lightsGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void lightsGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void checkAnimation_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void checkAnimation_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		void parameter1Slider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
	};
}

namespace winrt::DemoSceneWinRT::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}
