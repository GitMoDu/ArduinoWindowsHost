#pragma once

#include <stdint.h>

#include <ArduinoGraphicsCore.h>
#include "ISurfaceSink.h"

namespace Egfx
{
	template<typename FramebufferType>
	class HostScreenDriver : public IScreenDriver
	{
	private:
		static constexpr pixel_t ScreenWidth = FramebufferType::FrameWidth;
		static constexpr pixel_t ScreenHeight = FramebufferType::FrameHeight;

	private:
		Egfx::ISurfaceSink^ SurfaceSink;
		size_t ColorDepth;
		bool Monochrome;

	public:
		HostScreenDriver(ISurfaceSink^ sink, const uint8_t colorDepth, const bool monochrome)
			: IScreenDriver()
			, SurfaceSink(sink)
			, ColorDepth(colorDepth)
			, Monochrome(monochrome)
		{
		}

	public:
		bool CanPushBuffer() final { return true; }

		void StartBuffer() final {}

		uint32_t PushBuffer(const uint8_t* frameBuffer) final
		{
			if (SurfaceSink != nullptr)
			{
				if (ColorDepth == 24
					|| ColorDepth == 32)
				{
					SurfaceSink->updateSurfaceColor32(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
				else if (ColorDepth == 16 && !Monochrome)
				{
					SurfaceSink->updateSurfaceColor16(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
				else if (ColorDepth == 16 && Monochrome)
				{
					SurfaceSink->updateSurfaceGrayscale16(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
				else if (ColorDepth == 8 && Monochrome)
				{
					SurfaceSink->updateSurfaceGrayscale8(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
				else if (ColorDepth == 8 && !Monochrome)
				{
					SurfaceSink->updateSurfaceColor8(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
				else
				{
					SurfaceSink->updateSurfaceMonochrome(
						(uint8_t*)frameBuffer,
						ScreenWidth,
						ScreenHeight);
				}
			}

			return 0;
		}

		bool PushingBuffer(const uint8_t* frameBuffer) final
		{
			return false;
		}

		virtual void EndBuffer() final {}

		virtual pixel_t GetScreenWidth() const final { return ScreenWidth; }
		virtual pixel_t GetScreenHeight() const final { return ScreenHeight; }

		virtual bool Start() { return true; }

		virtual void Stop() {}

		virtual void SetBrightness(const uint8_t brightness) {}

		virtual void SetBufferTaskCallback(void (*taskCallback)(void* parameter)) {}

		virtual void BufferTaskCallback(void* parameter) {}
	};
}

