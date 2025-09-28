#pragma once


#define INTEGER_WORLD_FRUSTUM_DEBUG // Enable engine frustum visualization in scene.
#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level status measuring.

#define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable material component toggles in the lights shader.

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
		IntegerWorld::DirectXSurface<ScreenWidth, ScreenHeight> DirectxDrawSurface;

	protected:
		IntegerWorld::EngineRenderTask<maxObjectCount, maxOrderedPrimitives, batchSize> EngineRenderer;

	public:
		HostAddonIntegerWorld()
			: BaseHost()
			, DirectxDrawSurface()
			, EngineRenderer(SchedulerBase, DirectxDrawSurface)
		{
		}

		void StartEngine(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& swapChainPanel)
		{
			DirectxDrawSurface.SetSwapChainPanel(swapChainPanel);

			EngineRenderer.Start();
		}

	protected:
		virtual void OnDestroy() override
		{
			BaseHost::OnDestroy();
			DirectxDrawSurface.StopSurface();
		}
	};

	using IntegerWorldBaseHost = HostAddonScheduler<LoopHost>;
}
