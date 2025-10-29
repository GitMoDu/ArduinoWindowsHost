#include "pch.h"

#include "App.h"
#include "MainPage.h"

//using namespace winrt;
//using namespace winrt::Windows::ApplicationModel;
//using namespace winrt::Windows::ApplicationModel::Activation;
//using namespace winrt::Windows::Foundation;
//using namespace winrt::Windows::UI::Xaml;
//using namespace winrt::Windows::UI::Xaml::Controls;
//using namespace winrt::Windows::UI::Xaml::Navigation;
//using namespace winrt::DemoSceneWinRT;
//using namespace winrt::DemoSceneWinRT::implementation;

namespace winrt::DemoSceneWinRT::implementation
{
	/// <summary>
	/// Creates the singleton application object.  This is the first line of authored code
	/// executed, and as such is the logical equivalent of main() or WinMain().
	/// </summary>
	App::App()
	{
		Suspending({ this, &App::OnSuspending });

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
		UnhandledException([this](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::UnhandledExceptionEventArgs const& e)
			{
				if (IsDebuggerPresent())
				{
					auto errorMessage = e.Message();
					__debugbreak();
				}
			});
#endif
	}

	/// <summary>
	/// Invoked when the application is launched normally by the end user.  Other entry points
	/// will be used such as when the application is launched to open a specific file.
	/// </summary>
	/// <param name="e">Details about the launch request and process.</param>
	void App::OnLaunched(winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const& e)
	{
		using winrt::Windows::UI::Xaml::Controls::Frame;
		using winrt::Windows::UI::Xaml::Window;
		using winrt::Windows::ApplicationModel::Activation::ApplicationExecutionState;

		Frame rootFrame{ nullptr };
		auto content = Window::Current().Content();
		if (content)
		{
			rootFrame = content.try_as<Frame>();
		}

		// Do not repeat app initialization when the Window already has content,
		// just ensure that the window is active
		if (rootFrame == nullptr)
		{
			// Create a Frame to act as the navigation context and associate it with
			// a SuspensionManager key
			rootFrame = Frame();

			rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });

			if (e.PreviousExecutionState() == ApplicationExecutionState::Terminated)
			{
				// Restore the saved session state only when appropriate, scheduling the
				// final launch steps after the restore is complete
			}

			if (e.PrelaunchActivated() == false)
			{
				if (rootFrame.Content() == nullptr)
				{
					// When the navigation stack isn't restored navigate to the first page,
					// configuring the new page by passing required information as a navigation
					// parameter
					rootFrame.Navigate(xaml_typename<DemoSceneWinRT::MainPage>(), box_value(e.Arguments()));
				}
				// Place the frame in the current Window
				Window::Current().Content(rootFrame);
				// Ensure the current window is active
				Window::Current().Activate();
			}
		}
		else
		{
			if (e.PrelaunchActivated() == false)
			{
				if (rootFrame.Content() == nullptr)
				{
					// When the navigation stack isn't restored navigate to the first page,
					// configuring the new page by passing required information as a navigation
					// parameter
					rootFrame.Navigate(xaml_typename<DemoSceneWinRT::MainPage>(), box_value(e.Arguments()));
				}
				// Ensure the current window is active
				Window::Current().Activate();
			}
		}
	}

	void App::OnSuspending([[maybe_unused]] winrt::Windows::Foundation::IInspectable const& sender,
		[[maybe_unused]] winrt::Windows::ApplicationModel::SuspendingEventArgs const& e)
	{
		// Save application state and stop any background activity
	}

	void App::OnNavigationFailed(winrt::Windows::Foundation::IInspectable const&,
		winrt::Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const& e)
	{
		throw winrt::hresult_error(E_FAIL, winrt::hstring(L"Failed to load Page "));
	}
}