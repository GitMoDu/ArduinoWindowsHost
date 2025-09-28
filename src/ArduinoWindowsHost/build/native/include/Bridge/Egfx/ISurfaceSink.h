#pragma once

#include <stdint.h>

namespace Egfx
{
	public interface class ISurfaceSink
	{
		void updateSurfaceMonochrome(uint8_t* frameBuffer, int16_t width, int16_t height);
		void updateSurfaceGrayscale8(uint8_t* frameBuffer, int16_t width, int16_t height) = 0;
		void updateSurfaceGrayscale16(uint8_t* frameBuffer, int16_t width, int16_t height) = 0;
		void updateSurfaceColor8(uint8_t* frameBuffer, int16_t width, int16_t height) = 0;
		void updateSurfaceColor16(uint8_t* frameBuffer, int16_t width, int16_t height) = 0;
		void updateSurfaceColor32(uint8_t* frameBuffer, int16_t width, int16_t height) = 0;
	};
}