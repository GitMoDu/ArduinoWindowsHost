#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;

namespace winrt::ArduinoBlink::implementation
{
	MainPage::MainPage()
		: SerialTxAdapter(Serial)
	{
		// Ensure XAML holds a ref to the same VM instance
		DataContext(ViewModel());

		m_vmPropertyChangedToken = ViewModel().PropertyChanged(
			Windows::UI::Xaml::Data::PropertyChangedEventHandler{
				[this](winrt::Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Data::PropertyChangedEventArgs const& e)
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

	// Do per-frame work here (UI updates or enqueue work on UI thread).
	void MainPage::onRendering(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Media::RenderingEventArgs const& e)
	{
		if (ArduinoWindowsHost::Hal::State::IoHal.State[uint8_t(LED_BUILTIN)].State != 0)
		{
			if (IoLedBuiltIn().Visibility() != Visibility::Visible)
				IoLedBuiltIn().Visibility(Visibility::Visible);
		}
		else
		{
			if (IoLedBuiltIn().Visibility() != Visibility::Collapsed)
				IoLedBuiltIn().Visibility(Visibility::Collapsed);
		}

		if (Serial.ElapsedTx() <= SerialLedDuration)
		{
			if (IoLedTx().Visibility() != Visibility::Visible)
				IoLedTx().Visibility(Visibility::Visible);
		}
		else
		{
			if (IoLedTx().Visibility() != Visibility::Collapsed)
				IoLedTx().Visibility(Visibility::Collapsed);
		}

		if (Serial.ElapsedRx() <= SerialLedDuration)
		{
			if (IoLedRx().Visibility() != Visibility::Visible)
				IoLedRx().Visibility(Visibility::Visible);
		}
		else
		{
			if (IoLedRx().Visibility() != Visibility::Collapsed)
				IoLedRx().Visibility(Visibility::Collapsed);
		}

	}
	void MainPage::resetIoLeds()
	{
		IoLedPower().Visibility(Visibility::Collapsed);
		IoLedBuiltIn().Visibility(Visibility::Collapsed);
		IoLedRx().Visibility(Visibility::Collapsed);
		IoLedTx().Visibility(Visibility::Collapsed);
	}

	void MainPage::updateArduinoHostState(const bool enabled)
	{
		// Reset all IO LEDs to off state.
		resetIoLeds();

		if (enabled)
		{
			runButtonControl().Content(winrt::box_value(L"Stop"));
			IoLedPower().Visibility(Visibility::Visible);
			resetButtonHolder().Visibility(Visibility::Visible);
			serialInputButton().IsEnabled(true);

			SerialTxAdapter.Start(serialOutputTextBlock(), serialOutputScroll());
			SerialTxAdapter.AutoScroll(autoScrollCheckBox().IsChecked().GetBoolean());

			g_renderingToken = CompositionTarget::Rendering(
				[this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args)
				{
					this->onRendering(sender, *reinterpret_cast<winrt::Windows::UI::Xaml::Media::RenderingEventArgs const*>(&args));
				});
		}
		else
		{
			// unsubscribe the handler if registered
			if (g_renderingToken.value != 0)
			{
				CompositionTarget::Rendering(g_renderingToken);
				g_renderingToken = {};
			}

			SerialTxAdapter.Stop();
			resetButtonHolder().Visibility(Visibility::Collapsed);
			runButtonControl().Content(winrt::box_value(L"Start"));
			serialInputButton().IsEnabled(false);
		}

		// Re-enable the run button now that the state update is complete.
		runButtonControl().IsEnabled(true);
	}

	void MainPage::resetButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		runButtonControl().IsEnabled(false);
		ViewModel().IsRunning(false);
		runButtonControl().IsEnabled(false);
		ViewModel().IsRunning(true);
	}

	void MainPage::serialInputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (ViewModel().IsRunning())
		{
			auto serialText = winrt::to_string(serialInputTextBox().Text());

			if (newlineCheckBox().IsChecked().GetBoolean())
			{
				if (serialText.empty())
				{
					// Send newline if empty.
					serialText = "\n";
				}
				else
				{
					// Append newline if not present.
					if (serialText.back() != '\n')
						serialText += '\n';
				}
			}
			Serial.Rx(serialText);
		}

		if (clearInputOnSendCheckBox().IsChecked().GetBoolean())
			serialInputTextBox().Text(L"");
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

