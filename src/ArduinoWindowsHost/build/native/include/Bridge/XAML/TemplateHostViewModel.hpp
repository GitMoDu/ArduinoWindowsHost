#pragma once

#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/base.h>

#include <ArduinoWindowsHost.h>

namespace ArduinoWindowsHost
{
	template<typename HostType>
	struct TemplateHostViewModel
		: winrt::implements<TemplateHostViewModel<HostType>, winrt::Windows::UI::Xaml::Data::INotifyPropertyChanged>
	{
	public:
		/// <summary>
		/// HostManager for managing the ArduinoHost.
		/// </summary>
		TemplateHostManager<HostType> m_hostManager{};

	private:
		// INotifyPropertyChanged backing event
		winrt::event<winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged{};

		void RaisePropertyChanged(winrt::hstring const& name)
		{
			m_propertyChanged(*this, winrt::Windows::UI::Xaml::Data::PropertyChangedEventArgs{ name });
		}

	public:
		TemplateHostViewModel() = default;

		// Must be noexcept to satisfy winrt::implements' virtual dtor
		~TemplateHostViewModel() noexcept
		{
			try { m_hostManager.Stop(); }
			catch (...) { /* never throw from dtor */ }
		}

		// INotifyPropertyChanged
		winrt::event_token PropertyChanged(winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
		{
			return m_propertyChanged.add(handler);
		}

		void PropertyChanged(winrt::event_token const& token) noexcept
		{
			m_propertyChanged.remove(token);
		}

		// Properties
		TemplateHostManager<HostType>& HostManager() noexcept
		{
			return m_hostManager;
		}

		TemplateHostManager<HostType> const& HostManager() const noexcept
		{
			return m_hostManager;
		}

		bool IsRunning() noexcept
		{
			return m_hostManager.isRunning();
		}

		void IsRunning(bool value)
		{
			if (m_hostManager.isRunning() != value)
			{
				if (value) { m_hostManager.Start(); }
				else { m_hostManager.Stop(); }

				RaisePropertyChanged(L"IsRunning");
			}
		}
	};
}