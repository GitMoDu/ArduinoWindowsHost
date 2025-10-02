#pragma once

#include "MainPage.g.h"

#include <winrt/Windows.UI.Xaml.Media.h>

#include <ArduinoWindowsHost.h>
#include "Host/ArduinoHost.hpp"

namespace winrt::ArduinoBlink::implementation
{
	static constexpr uint32_t SerialLedDuration = 50000;
	static constexpr uint32_t SerialOutputPollPeriod = 50000;

	struct MainPage : MainPageT<MainPage>
	{
		/// <summary>
		/// HostManager for managing the ArduinoHost.
		/// </summary>
		ArduinoWindowsHost::TemplateHostManager<ArduinoBlinkHost::ArduinoHost> HostManager{};

		/// <summary>
		/// Token for the rendering event registration.
		/// </summary>
		winrt::event_token g_renderingToken{};

		/// <summary>
		/// Dispatcher for updating the serial text box in an Arduino Windows host application at a specified polling interval.
		/// </summary>
		ArduinoWindowsHost::SerialOutputAdapter<SerialOutputPollPeriod> SerialTxAdapter;

		MainPage()
			: SerialTxAdapter(Serial)
		{
		}

		void updateArduinoHostState(const bool enabled);

		void onRendering(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Media::RenderingEventArgs const& e);

		void resetIoLeds();

		void runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void resetButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void serialInputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void clearOutputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void autoScrollCheckBox_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::ArduinoBlink::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}
