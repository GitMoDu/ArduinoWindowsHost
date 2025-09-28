#include "pch.h"

#include "MainPage.h"
#include "MainPage.g.cpp"


using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::DemoSceneWinRT::implementation
{
	MainPage::MainPage()
	{
		InitializeComponent();

#ifndef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		// Runtime toggles for ambient/emissive/diffuse/specular are not compiled in —
		// disable the UI controls so they don't appear active.
		if (checkAmbient())  checkAmbient().IsEnabled(false);
		if (checkEmissive()) checkEmissive().IsEnabled(false);
		if (checkDiffuse())  checkDiffuse().IsEnabled(false);
		if (checkSpecular()) checkSpecular().IsEnabled(false);
#endif
	}

	MainPage::~MainPage()
	{
	}


	void MainPage::runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (HostManager.isRunning())
		{
			updateArduinoHostState(false);
			runButtonControl().Content(winrt::box_value(L"Start"));
		}
		else
		{
			updateArduinoHostState(true);
			runButtonControl().Content(winrt::box_value(L"Stop"));
		}
	}

	void MainPage::updateArduinoHostState(const bool enabled)
	{
		if (enabled)
		{
			const auto panel = swapChainPanel();
			HostManager.Start();
			HostManager.Host->StartDemo(panel);
			initializeGamepadSupport();
		}
		else
		{
			uninitializeGamepadSupport();
			HostManager.Stop();
		}
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
					if (HostManager.isRunning())
					{
						HostManager.Host->OnGamepadInput(reading);
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

	void MainPage::shaderGroupChecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::RadioButton;

		auto radio = sender.try_as<RadioButton>();
		if (!radio) return;

		// Notify host of pipeline change (if running)
		if (HostManager.isRunning())
		{
			if (radio == shaderZ())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::ZShaderPipeline);
			}
			else if (radio == shaderNormal())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::NormalShaderPipeline);
			}
			else if (radio == shaderPixel())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::PixelShaderPipeline);
			}
		}

		// Enable/disable scene shading controls based on selected pipeline
		bool pixelPipelineSelected = (radio == shaderPixel());

		if (checkScene()) checkScene().IsEnabled(pixelPipelineSelected);

#ifdef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		bool siblingsEnabled = false;
		if (pixelPipelineSelected && checkScene())
		{
			auto isCheckedRef = checkScene().IsChecked();
			if (isCheckedRef && isCheckedRef.Value())
			{
				siblingsEnabled = true;
			}
		}

		if (checkAmbient())  checkAmbient().IsEnabled(siblingsEnabled);
		if (checkEmissive()) checkEmissive().IsEnabled(siblingsEnabled);
		if (checkDiffuse())  checkDiffuse().IsEnabled(siblingsEnabled);
		if (checkSpecular()) checkSpecular().IsEnabled(siblingsEnabled);
#endif
	}

	void MainPage::checkScene_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
#ifdef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		// Only enable group if master is enabled (i.e., Pixel pipeline selected)
		const bool allow = (checkScene() && checkScene().IsEnabled());
		if (checkAmbient())  checkAmbient().IsEnabled(allow);
		if (checkEmissive()) checkEmissive().IsEnabled(allow);
		if (checkDiffuse())  checkDiffuse().IsEnabled(allow);
		if (checkSpecular()) checkSpecular().IsEnabled(allow);
#endif

		if (HostManager.isRunning())
		{
			HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::SceneShaderEnabled, 1);
		}
	}

	void MainPage::checkScene_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
#ifdef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		// Disable the group UI regardless of pipeline
		if (checkAmbient())  checkAmbient().IsEnabled(false);
		if (checkEmissive()) checkEmissive().IsEnabled(false);
		if (checkDiffuse())  checkDiffuse().IsEnabled(false);
		if (checkSpecular()) checkSpecular().IsEnabled(false);
#endif

		if (HostManager.isRunning())
		{
			HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::SceneShaderEnabled, 0);
		}
	}

	void MainPage::sceneGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
#ifdef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (HostManager.isRunning())
		{
			if (checkBox == checkAmbient())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::AmbientShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkEmissive())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::EmissiveShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkDiffuse())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::DiffuseShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkSpecular())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::SpecularShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
#endif
	}

	void MainPage::sceneGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
#ifdef INTEGER_WORLD_LIGHTS_SHADER_DEBUG
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (HostManager.isRunning())
		{
			if (checkBox == checkAmbient())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::AmbientShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkEmissive())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::EmissiveShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkDiffuse())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::DiffuseShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkSpecular())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::SpecularShadeEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
#endif
	}

	void MainPage::lightsGroup_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (HostManager.isRunning())
		{
			if (checkBox == checkLight1())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::Light1Enabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkLight2())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::Light2Enabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkLightGlobal())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::LightGlobalEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
	}

	void MainPage::lightsGroup_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::CheckBox;

		auto checkBox = sender.try_as<CheckBox>();
		if (!checkBox) return;

		if (HostManager.isRunning())
		{
			if (checkBox == checkLight1())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::Light1Enabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkLight2())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::Light2Enabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
			else if (checkBox == checkLightGlobal())
			{
				HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::LightGlobalEnabled, checkBox.IsChecked().Value() ? 1 : 0);
			}
		}
	}

	void MainPage::checkAnimation_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (HostManager.isRunning())
		{
			HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::AnimationEnabled, 1);
		}
	}

	void MainPage::checkAnimation_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (HostManager.isRunning())
		{
			HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::AnimationEnabled, 0);
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

		if (HostManager.isRunning())
		{
			HostManager.Host->OnParameterInput(DemoSceneWindows::IntegerWorldInterface::FoV, (uint8_t)parameter1Slider().Value());
		}
	}
}
