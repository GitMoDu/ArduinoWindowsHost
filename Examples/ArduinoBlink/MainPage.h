#pragma once

#include "MainPage.g.h"

#include <winrt/Windows.UI.Xaml.Media.h>

#include <ArduinoWindowsHost.h>
#include "Host/ArduinoHost.hpp"

namespace winrt::ArduinoBlink::implementation
{
	struct MainPage : MainPageT<MainPage>
	{
		static constexpr uint32_t SerialLedDuration = 50000;
		static constexpr uint32_t SerialOutputPollPeriod = 50000;

		/// <summary>
		/// ArduinoHostViewModel type for the specific ArduinoHost used in this application.
		/// </summary>
		using HostViewModelType = ArduinoWindowsHost::TemplateHostViewModel<typename ArduinoBlinkHost::ArduinoHost>;

		winrt::com_ptr<HostViewModelType> m_viewModel{ winrt::make_self<HostViewModelType>() };
		HostViewModelType& ViewModel() noexcept { return *m_viewModel; }
		HostViewModelType const& ViewModel() const noexcept { return *m_viewModel; }


		/// <summary>
		/// Token used to manage the registration of a property changed event handler.
		/// </summary>
		winrt::event_token m_vmPropertyChangedToken{};

		/// <summary>
		/// Token for the rendering event registration.
		/// </summary>
		winrt::event_token g_renderingToken{};

		/// <summary>
		/// Dispatcher for updating the serial text box in an Arduino Windows host application at a specified polling interval.
		/// </summary>
		ArduinoWindowsHost::SerialOutputAdapter<SerialOutputPollPeriod> SerialTxAdapter;

		MainPage();

		~MainPage();

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
