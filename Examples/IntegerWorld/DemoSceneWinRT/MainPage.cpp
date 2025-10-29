#include "pch.h"

#include "MainPage.h"
#include "MainPage.g.cpp"


using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::DemoSceneWinRT::implementation
{
	MainPage::MainPage()
		: SerialTxAdapter(Serial)
	{
#ifndef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		// Runtime toggles for ambient/emissive/diffuse/specular are not compiled in —
		// disable the UI controls so they don't appear active.
		if (checkAmbient())  checkAmbient().IsEnabled(false);
		if (checkEmissive()) checkEmissive().IsEnabled(false);
		if (checkDiffuse())  checkDiffuse().IsEnabled(false);
		if (checkSpecular()) checkSpecular().IsEnabled(false);
#endif

		// Ensure XAML holds a ref to the same VM instance
		DataContext(ViewModel());

		m_vmPropertyChangedToken = ViewModel().PropertyChanged(
			winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler{
				[this](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::Data::PropertyChangedEventArgs const& e)
				{
					if (e.PropertyName() == L"IsRunning")
					{
						// Reflect VM state into UI
						updateArduinoHostState(ViewModel().IsRunning());
					}
				}
			});
	}

	MainPage::~MainPage()
	{
		// Break XAML bindings and release our strong ref early
		DataContext(nullptr);
		m_viewModel = nullptr;

		if (m_vmPropertyChangedToken.value != 0)
		{
			ViewModel().PropertyChanged(m_vmPropertyChangedToken);
			m_vmPropertyChangedToken = {};
		}
	}

	void MainPage::runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		// Immediately disable the button to prevent multiple clicks.
		runButtonControl().IsEnabled(false);

		// Toggle the running state.
		ViewModel().IsRunning(!ViewModel().IsRunning());
	}

	void MainPage::updateArduinoHostState(const bool enabled)
	{
		if (enabled)
		{
			runButtonControl().Content(winrt::box_value(L"Stop"));

			// Get the SwapChainPanel for rendering
			auto panel = swapChainPanel();

			auto host = ViewModel().HostManager().Host;
			if (host != nullptr)
			{
				host->StartDemo(panel);
			}

			initializeGamepadSupport();

			SerialTxAdapter.Start(serialOutputTextBlock(), serialOutputScroll());
			SerialTxAdapter.AutoScroll(autoScrollCheckBox().IsChecked().GetBoolean());
			enginePanel().IsEnabled(true);

			// Fragment shader defaults: Lights
			if (fragmentShaderLights()) fragmentShaderLights().IsChecked(true);
			// Lights shader group defaults: Light Source
			if (lightsShaderLightSource()) lightsShaderLightSource().IsChecked(true);
			// Ensure lights panel enabled (redundant if Checked event ran)
			if (lightsShaderPanel()) lightsShaderPanel().IsEnabled(true);

			// Shading mix defaults
			if (checkAmbient())  checkAmbient().IsChecked(true);
			if (checkEmissive()) checkEmissive().IsChecked(true);
			if (checkDiffuse())  checkDiffuse().IsChecked(true);
			if (checkSpecular()) checkSpecular().IsChecked(true);

			// Light source defaults
			if (checkRedLight())    checkRedLight().IsChecked(true);
			if (checkGreenLight())  checkGreenLight().IsChecked(true);
			if (checkGlobalLight()) checkGlobalLight().IsChecked(true);

			// Animation default
			if (checkAnimation()) checkAnimation().IsChecked(true);

			// Field-of-View default
			if (parameter1Slider()) parameter1Slider().Value(30);
		}
		else
		{
			uninitializeGamepadSupport();
			SerialTxAdapter.Stop();
			runButtonControl().Content(winrt::box_value(L"Start"));
			enginePanel().IsEnabled(false);
		}

		// Re-enable the run button now that the state update is complete.
		runButtonControl().IsEnabled(true);
	}

	void MainPage::initializeGamepadSupport()
	{
		using winrt::Windows::Gaming::Input::Gamepad;
		using winrt::Windows::System::Threading::ThreadPoolTimer;

		// Register for gamepad added event
		m_gamepadAddedToken = Gamepad::GamepadAdded([this](IInspectable const&, Gamepad const& gamepad)
			{
				m_gamepads.push_back(gamepad);
			});

		// Register for gamepad removed event
		m_gamepadRemovedToken = Gamepad::GamepadRemoved([this](IInspectable const&, Gamepad const& gamepad)
			{
				auto it = std::remove(m_gamepads.begin(), m_gamepads.end(), gamepad);
				m_gamepads.erase(it, m_gamepads.end());
			});

		// Populate initial gamepads
		for (auto const& gamepad : Gamepad::Gamepads())
		{
			m_gamepads.push_back(gamepad);
		}

		// Start polling timer.
		m_gamepadPollTimer = ThreadPoolTimer::CreatePeriodicTimer([this](ThreadPoolTimer const&)
			{
				for (auto const& gamepad : m_gamepads)
				{
					auto reading = gamepad.GetCurrentReading();
					if (ViewModel().IsRunning())
					{
						ViewModel().HostManager().Host->OnGamepadInput(reading);
					}
				}
			}, std::chrono::milliseconds(10));
	}

	void MainPage::uninitializeGamepadSupport()
	{
		using winrt::Windows::Gaming::Input::Gamepad;

		Gamepad::GamepadAdded(m_gamepadAddedToken);
		Gamepad::GamepadRemoved(m_gamepadRemovedToken);
		m_gamepadPollTimer = nullptr;
		m_gamepads.clear();
	}


	void MainPage::fragmentShaderGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::RadioButton;

		auto radio = sender.try_as<RadioButton>();
		if (!radio) return;

		if (ViewModel().IsRunning())
		{
			if (radio == fragmentShaderZ())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::FragmentShaderZ);
				lightsShaderPanel().IsEnabled(false);
			}
			else if (radio == fragmentShaderWireframe())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::FragmentShaderWireframe);
				lightsShaderPanel().IsEnabled(true);
			}
			else if (radio == fragmentShaderLights())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::FragmentShaderLights);
				lightsShaderPanel().IsEnabled(true);
			}
		}
	}

	void MainPage::lightsShaderGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::RadioButton;
		auto radio = sender.try_as<RadioButton>();
		if (!radio) return;
		if (ViewModel().IsRunning())
		{
			if (radio == lightsShaderNone())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightsShaderNone);
			}
			else if (radio == lightsShaderNormal())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightsShaderNormal);
			}
			else if (radio == lightsShaderLightSource())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightsShaderLightSource);
			}
		}
	}

	void MainPage::lightsShaderMixGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (ViewModel().IsRunning())
		{
			if (checkBox == checkAmbient())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::AmbientShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkEmissive())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::EmissiveShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkDiffuse())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::DiffuseShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkSpecular())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::SpecularShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
	}

	void MainPage::lightsShaderMixGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		lightsShaderMixGroup_Checked(sender, e);
	}

	void MainPage::lightsShaderSourceGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (ViewModel().IsRunning())
		{
			if (checkBox == checkRedLight())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightRedEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkGreenLight())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightGreenEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkGlobalLight())
			{
				ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::LightGlobalEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
	}

	void MainPage::lightsShaderSourceGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		lightsShaderSourceGroup_Checked(sender, e);
	}

	bool MainPage::isLightsOrWireframeEnabled() const
	{
		return true;
	}

	bool MainPage::isLightsOrWireframeEnabled(winrt::Windows::Foundation::IReference<bool> const& wireframe, winrt::Windows::Foundation::IReference<bool> const& lights) const
	{
		const bool wf = wireframe && wireframe.Value();
		const bool li = lights && lights.Value();
		return wf || li;
	}

	void MainPage::checkAnimation_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (ViewModel().IsRunning())
		{
			ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::AnimationEnabled, 1);
		}
	}

	void MainPage::checkAnimation_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (ViewModel().IsRunning())
		{
			ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::AnimationEnabled, 0);
		}
	}

	void MainPage::parameter1Slider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
	{
		double newValue = e.NewValue();

		if (newValue >= parameter1Slider().Minimum())
		{
			if (newValue <= parameter1Slider().Maximum())
			{
				parameter1Slider().Value(newValue);
			}
			else
			{
				parameter1Slider().Value(parameter1Slider().Maximum());
			}
		}
		else
		{
			parameter1Slider().Value(parameter1Slider().Minimum());
		}

		if (ViewModel().IsRunning())
		{
			ViewModel().HostManager().Host->OnParameterInput(::DemoSceneWinRT::IntegerWorldInterface::FoV, (uint8_t)parameter1Slider().Value());
		}
	}

	void MainPage::clearOutputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (ViewModel().IsRunning())
		{
			Serial.flushTx();
		}
		else
		{
			serialOutputTextBlock().Text(L"");
		}
	}

	void MainPage::autoScrollCheckBox_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (ViewModel().IsRunning())
		{
			SerialTxAdapter.AutoScroll(autoScrollCheckBox().IsChecked().GetBoolean());
		}
	}
}


