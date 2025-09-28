#pragma once

// Task scheduler configuration compiler flags.
#define _TASK_OO_CALLBACKS
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
#include "Host/HostAddonScheduler.hpp" // Depends on TaskScheduler (https://github.com/arkhipenko/TaskScheduler)
#include "Host/HostAddonVirtualPad.hpp" // Depends on Windows.Gaming.Input and VirtualPad (https://github.com/GitMoDu/VirtualPad)

namespace ArduinoWindowsHost
{
	using SchedulerHost = HostAddonScheduler<LoopHost>;
}
