#pragma once

#include "MainPage.g.h"

#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level status measuring.

//#define INTEGER_WORLD_MOCK_OUTPUT // Use mock output surface for rendering (for testing without DirectX).

#define INTEGER_WORLD_FRUSTUM_DEBUG // Enable engine frustum visualization in scene.
#define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable light component toggles in the scene lights shader.
#define INTEGER_WORLD_TEXTURED_CUBE_DEMO // Use textured cube object in the demo scene instead of colored cube.
#define INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY // Textured cube object with perspective correct and accurate texture mapping.


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

	static constexpr uint8_t ScreenScale = 4;
	static constexpr uint16_t Width = 96;
	static constexpr uint16_t Height = 64;
	static constexpr uint16_t ScreenWidth = Width * ScreenScale;
	static constexpr uint16_t ScreenHeight = Height * ScreenScale;

	static constexpr uint32_t SerialOutputPollPeriod = 100000;

	using ArduinoHostType = ::DemoSceneWinRT::ArduinoHost<ScreenWidth, ScreenHeight>;

	struct MainPage : MainPageT<MainPage>
	{

		/// <summary>
		/// ArduinoHostViewModel type for the specific ArduinoHost used in this application.
		/// </summary>
		using HostViewModelType = ArduinoWindowsHost::TemplateHostViewModel<typename ArduinoHostType>;

		winrt::com_ptr<HostViewModelType> m_viewModel{ winrt::make_self<HostViewModelType>() };
		HostViewModelType& ViewModel() noexcept { return *m_viewModel; }
		HostViewModelType const& ViewModel() const noexcept { return *m_viewModel; }

		/// <summary>
		/// Token used to manage the registration of a property changed event handler.
		/// </summary>
		winrt::event_token m_vmPropertyChangedToken{};

		///// <summary>
		///// HostManager for managing the ArduinoHost.
		///// </summary>
		//TemplateHostManager<ArduinoHostType> HostManager{};

		/// <summary>
		/// Dispatcher for updating the serial text box in an Arduino Windows host application at a specified polling interval.
		/// </summary>
		ArduinoWindowsHost::SerialOutputAdapter<SerialOutputPollPeriod> SerialTxAdapter;

		// Gamepad support members
		std::vector<winrt::Windows::Gaming::Input::Gamepad> m_gamepads;
		winrt::event_token m_gamepadAddedToken;
		winrt::event_token m_gamepadRemovedToken;
		winrt::Windows::System::Threading::ThreadPoolTimer m_gamepadPollTimer{ nullptr };

		MainPage();
		~MainPage();

		bool isLightsOrWireframeEnabled(winrt::Windows::Foundation::IReference<bool> const& wireframe, winrt::Windows::Foundation::IReference<bool> const& lights) const;

		bool isLightsOrWireframeEnabled() const;

		void updateArduinoHostState(const bool enabled);

		void runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);


		// Gamepad support methods
		void initializeGamepadSupport();
		void uninitializeGamepadSupport();



		void fragmentShaderGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void lightsShaderGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		void lightsShaderMixGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void lightsShaderMixGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		void lightsShaderSourceGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void lightsShaderSourceGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		void checkAnimation_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void checkAnimation_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		void parameter1Slider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
		void clearOutputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
		void autoScrollCheckBox_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::DemoSceneWinRT::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}
