#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;

namespace winrt::ArduinoBlink::implementation
{
	void MainPage::runButtonControl_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (HostManager.isRunning())
		{
			updateArduinoHostState(false);
		}
		else
		{
			updateArduinoHostState(true);
		}
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
		resetIoLeds();

		if (enabled)
		{
			HostManager.Start();
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
			HostManager.Stop();

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
	}

	void MainPage::resetButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		resetButtonHolder().Visibility(Visibility::Collapsed);
		serialInputButton().IsEnabled(false);
		updateArduinoHostState(false);
		updateArduinoHostState(true);
	}

	void MainPage::serialInputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
	{
		if (HostManager.isRunning())
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
		if (HostManager.isRunning())
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
		if (HostManager.isRunning())
		{
			SerialTxAdapter.AutoScroll(autoScrollCheckBox().IsChecked().GetBoolean());
		}
	}
}

