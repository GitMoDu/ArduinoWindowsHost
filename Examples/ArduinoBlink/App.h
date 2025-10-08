#pragma once
#include "App.xaml.g.h"

namespace winrt::ArduinoBlink::implementation
{
    struct App : AppT<App>
    {
        App();
        void OnLaunched(winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
        void OnSuspending(IInspectable const&, winrt::Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnNavigationFailed(IInspectable const&, winrt::Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
    };
}
