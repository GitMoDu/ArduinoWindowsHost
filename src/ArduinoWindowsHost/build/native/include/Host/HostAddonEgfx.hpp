#pragma once



#include "HostScreenDriver.hpp"

namespace ArduinoWindowsHost
{
	template<typename BaseType, typename FramebufferType>
	class HostAddonEgfx : public BaseType
	{
	public:
		static constexpr pixel_t ScreenWidth = FramebufferType::FrameWidth;
		static constexpr pixel_t ScreenHeight = FramebufferType::FrameHeight;

	private:
		using FullLayout = LayoutElement<0, 0, ScreenWidth, ScreenHeight>;

	protected:
		FramebufferType Framebuffer;
		GraphicsEngineTask GraphicsEngine;

	private:
		uint8_t Buffer[FramebufferType::BufferSize]{};

		HostScreenDriver<typename FramebufferType> ScreenDriver;

	protected:
#if defined(GRAPHICS_ENGINE_MEASURE)
		EngineLogTask<1000> EngineLog;
#endif

	public:
		HostAddonEgfx() : BaseType(),
			, Framebuffer(Buffer)
			, ScreenDriver(sink, Framebuffer.GetColorDepth(), Framebuffer.IsMonochrome())
			, GraphicsEngine(&SchedulerBase, &Framebuffer, &ScreenDriver)
#if defined(GRAPHICS_ENGINE_MEASURE)
			, EngineLog(SchedulerBase, GraphicsEngine)
#endif
		{}

	protected:
		void setup() override
		{
			setupLog();

			// Optional callback for RTOS driver variants.
			//GraphicsEngine.SetBufferTaskCallback(BufferTaskCallback);

			// Set the Display Sync Type.
			GraphicsEngine.SetSyncType(DisplaySyncType::Vrr);

			if (!GraphicsEngine.Start())
			{
				halt();
			}
		}

	protected:
		virtual void setupLog()
		{
			Serial.println(F("Graphics Engine"));
			Serial.print(Framebuffer.GetColorDepth());
			if (Framebuffer.IsMonochrome())
			{
				Serial.println(F(" bit monochrome screen."));
			}
			else
			{
				Serial.println(F(" bit color screen."));
			}
			Serial.print(F("EGFX_PLATFORM_BIG "));
#if defined(EGFX_PLATFORM_BIG)
			Serial.println(1);
#else
			Serial.println(0);
#endif

			Serial.print(F("EGFX_PLATFORM_HDR "));
#if defined(EGFX_PLATFORM_HDR)
			Serial.println(1);
#else
			Serial.println(0);
#endif
		}
	};
}
