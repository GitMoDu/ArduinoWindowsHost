#pragma once

#if __has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#include <ArduinoWindowsHost.h>

#include <winrt/Windows.UI.Core.h> 
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>

namespace ArduinoWindowsHost
{
	/// <summary>
	/// Periodically transfers buffered serial TX output (collected from an
	/// ArduinoSerialPort) into a XAML TextBlock, optionally
	/// keeping an associated ScrollViewer scrolled to the bottom.
	/// 
	/// Intended use-case: implement a lightweight “Serial Monitor” view in a
	/// WinRT / UWP XAML UI without wiring explicit events from the serial
	/// backend. A background ThreadPoolTimer polls at a fixed interval
	/// (see <typeparamref name="PollPeriodMicros"/>) and, when the port’s
	/// transmission state id changes, marshals a UI update via
	/// CoreDispatcher::TryRunAsync.
	/// </summary>
	/// <typeparam name="PollPeriodMicros">
	/// Poll period in microseconds (default 50,000 = 50 ms). Lower values increase
	/// UI refresh responsiveness at the cost of more wake-ups.
	/// </typeparam>
	template<const uint32_t PollPeriodMicros = 50000>
	class SerialOutputAdapter
	{
	private:
		// Periodic timer used for polling serial TX state.
		winrt::Windows::System::Threading::ThreadPoolTimer PollTimer{ nullptr };

	private:
		// Reference to the serial instance providing buffered lines and a TX state id.
		ArduinoWindowsHost::Hal::ArduinoSerialPort& SerialInstance;

	private:
		// UI targets (weak interface references - set to nullptr on Stop()).
		winrt::Windows::UI::Xaml::Controls::ITextBlock OutputTextBlock{ nullptr };
		winrt::Windows::UI::Xaml::Controls::ScrollViewer OutputScroller{ nullptr };
		winrt::Windows::UI::Core::CoreDispatcher Dispatcher{ nullptr };

		// Cached last seen TX state id (invalid initial value forces first refresh).
		uint32_t StateId = UINT32_MAX;

		// Auto-scroll flag controlling whether scroller jumps to bottom.
		bool AutoScrollEnabled = true;

	public:
		SerialOutputAdapter(ArduinoWindowsHost::Hal::ArduinoSerialPort& serialInstance)
			: SerialInstance(serialInstance)
		{
		}

		// Returns current auto-scroll preference.
		bool AutoScroll() const
		{
			return AutoScrollEnabled;
		}

		// Enables / disables automatic scroll - to - bottom after updates.
		void AutoScroll(bool value)
		{
			AutoScrollEnabled = value;
		}

		/// <summary>
		/// Begin polling and binding to provided UI elements. 
		/// Must be called on the UI thread(captures CoreDispatcher).
		/// </summary>
		/// <param name="outputTextBlock"></param>
		/// <param name="outputScroller"></param>
		void Start(winrt::Windows::UI::Xaml::Controls::TextBlock& outputTextBlock,
			winrt::Windows::UI::Xaml::Controls::ScrollViewer& outputScroller)
		{
			Dispatcher = outputTextBlock.Dispatcher();
			OutputTextBlock = outputTextBlock;
			OutputScroller = outputScroller;

			using winrt::Windows::System::Threading::ThreadPoolTimer;

			StateId = UINT32_MAX;
			PollTimer = ThreadPoolTimer::CreatePeriodicTimer([this](ThreadPoolTimer const&)
				{
					// Poll for TX data changes.
					const uint32_t stateId = SerialInstance.GetTxId();

					if (StateId != stateId)
					{
						// New data available.
						StateId = stateId;

						std::vector<std::string> txData = SerialInstance.getBufferedLines();

						if (Dispatcher != nullptr)
						{
							// Marshal UI update.
							Dispatcher.TryRunAsync(
								winrt::Windows::UI::Core::CoreDispatcherPriority::Low,
								winrt::Windows::UI::Core::DispatchedHandler([this, txData]
									{
										if (OutputTextBlock != nullptr)
										{
											std::string allText;
											allText.reserve(
												txData.size() * 2 + 64); // light heuristic to reduce reallocs

											for (const auto& line : txData)
											{
												allText.append(line);
											}
											OutputTextBlock.Text(winrt::to_hstring(allText));
											if (AutoScrollEnabled && OutputScroller != nullptr)
											{
												OutputScroller.UpdateLayout();
												double bottom = OutputScroller.ScrollableHeight();
												OutputScroller.ChangeView(nullptr, bottom, nullptr, true);
											}
										}
									})
							);
						}
					}
					else
					{
						// No new data; still honor auto-scroll if enabled (optional).
						if (AutoScrollEnabled && OutputScroller != nullptr)
						{
							OutputScroller.UpdateLayout();
							double bottom = OutputScroller.ScrollableHeight();
							if (bottom > 0)
								OutputScroller.ChangeView(nullptr, bottom, nullptr, true);
						}
					}
				}, std::chrono::microseconds(PollPeriodMicros));
		}

		/// <summary>
		/// Stops polling and releases references to UI objects.
		/// </summary>
		void Stop()
		{
			if (PollTimer)
			{
				PollTimer.Cancel();
				PollTimer = nullptr;
			}
			OutputTextBlock = nullptr;
			OutputScroller = nullptr;
			Dispatcher = nullptr;
		}
	};
}
#endif
