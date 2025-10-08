#pragma once


//#define INTEGER_WORLD_FRUSTUM_DEBUG // Enable engine frustum visualization in scene.
//#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level status measuring.S
//
//#define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable material component toggles in the lights shader.

//#define INTEGER_WORLD_MOCK_OUTPUT // Use mock output surface for rendering (for testing without DirectX).

#include "LoopHost.hpp"
#include "HostAddonScheduler.hpp"

#include <IntegerWorld.h>

#include <IntegerWorldOutputs.h>
#include <IntegerWorldWindows.h>
#include <IntegerWorldExperimental.h>

namespace ArduinoWindowsHost
{
	template<typename BaseHost,
		int16_t screenWidth, int16_t screenHeight, uint16_t maxObjectCount = 255, uint16_t maxOrderedPrimitives = 65535, uint16_t batchSize = 1024>
	class HostAddonIntegerWorld : public BaseHost
	{
	public:
		static constexpr int16_t ScreenWidth = screenWidth;
		static constexpr int16_t ScreenHeight = screenHeight;

	protected:
		using BaseHost::SchedulerBase;

	protected:
#if defined(INTEGER_WORLD_MOCK_OUTPUT)
		IntegerWorld::MockOutput::OutputSurface<ScreenWidth, ScreenHeight> DirectxDrawSurface;
#else
		IntegerWorld::DirectXSurface<ScreenWidth, ScreenHeight> DirectxDrawSurface;
#endif

	protected:
		IntegerWorld::EngineRenderTask<maxObjectCount, maxOrderedPrimitives, batchSize> EngineRenderer;

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
		IntegerWorld::PerformanceLogTask<1000> IntegerWorldEngineLog;
#endif
	public:
		HostAddonIntegerWorld()
			: BaseHost()
			, DirectxDrawSurface()
			, EngineRenderer(SchedulerBase, DirectxDrawSurface)
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
			, IntegerWorldEngineLog(SchedulerBase, EngineRenderer)
#endif
		{
		}

		void StartEngine(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& swapChainPanel)
		{
#if !defined(INTEGER_WORLD_MOCK_OUTPUT)
			DirectxDrawSurface.SetSwapChainPanel(swapChainPanel);
#endif
			EngineRenderer.Start();

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
			IntegerWorldEngineLog.enable();
#endif
		}

	protected:
		virtual void setdown() override
		{
			BaseHost::setdown();
			EngineRenderer.Stop();
			DirectxDrawSurface.StopSurface();
		}
	};

	using IntegerWorldBaseHost = HostAddonScheduler<LoopHost>;
}
