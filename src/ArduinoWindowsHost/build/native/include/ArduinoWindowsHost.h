#pragma once

// Task scheduler configuration compiler flags.
#define _TASK_OO_CALLBACKS
#define _TASK_DO_NOT_YIELD
#define _TASK_TICKLESS
#define _TASK_INLINE


// Main Arduino HAL include.
#include "Arduino.h"

// Base host with loop support.
#include "Host/LoopHost.hpp"

// Abstract host thread manager.
#include "Host/HostThreadManager.hpp"

// Host addons.
#include "Host/HostAddonParameter.hpp" 

// Only include the scheduler addon if TaskScheduler is available.
#if defined(__has_include)
#if __has_include(<TScheduler.hpp>)
#include "Host/HostAddonScheduler.hpp" // Depends on TaskScheduler (https://github.com/arkhipenko/TaskScheduler)
#endif
#endif

// Only include the virtual pad addon if the platform headers and VirtualPad are available.
// Check for WinRT Gaming.Input and the VirtualPad header used by the VirtualPad project.
#if defined(__has_include)
#if __has_include(<winrt/Windows.Gaming.Input.h>) && __has_include(<VirtualPads.h>)
#include "Host/HostAddonVirtualPad.hpp" // Depends on Windows.Gaming.Input and VirtualPad (https://github.com/GitMoDu/VirtualPad)
#endif
#endif

// Only include the XAML serial output adapter if the platform headers are available.
#if defined(__has_include)
#if __has_include(<winrt/Windows.UI.Xaml.Controls.h>)
#include "Bridge/XAML/SerialOutputAdapter.hpp"
#endif
#endif