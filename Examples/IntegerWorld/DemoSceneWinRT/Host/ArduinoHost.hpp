#pragma once

#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.

#include "pch.h"

#include <ArduinoWindowsHost.h>
#include <ArduinoIntegerWorldWindows.h>

#include "../Scene/AnimatedDemoScene.h"

namespace DemoSceneWindows
{
	using namespace ArduinoWindowsHost;

	enum IntegerWorldInterface
	{
		FoV,
		AnimationEnabled,
		PixelShaderPipeline,
		SceneShaderEnabled,
		ZShaderPipeline,
		NormalShaderPipeline,
		AmbientShadeEnabled,
		EmissiveShadeEnabled,
		DiffuseShadeEnabled,
		SpecularShadeEnabled,
		Light1Enabled,
		Light2Enabled,
		LightGlobalEnabled
	};

	// Add VirtualPad support and Parameter handling to the base host.
	using IntegerWorldBase = HostAddonParameter<HostAddonVirtualPad<IntegerWorldBaseHost>>;

	// Includes:
	// - Animated demo scene with multiple objects and lights.
	// - Free camera control with gamepad.

	/// <summary>
	/// Integer World host with demo scene. Includes:
	/// - Animated demo scene with multiple objects and lights.
	/// - Engine and scene parameters external control interface.
	/// - Free camera control with gamepad.
	/// - Performance logging support.
	/// </summary>
	/// <typeparam name="screenWidth">The width of the rendering screen in pixels.</typeparam>
	/// <typeparam name="screenHeight">The height of the rendering screen in pixels.</typeparam>
	/// <typeparam name="maxObjectCount">The maximum number of objects that can exist in the world (default: 255).</typeparam>
	/// <typeparam name="maxOrderedPrimitives">The maximum number of ordered primitives for rendering (default: 65535).</typeparam>
	/// <typeparam name="batchSize">The batch size for rendering operations (default: 1024).</typeparam>
	template<int16_t screenWidth, int16_t screenHeight, uint16_t maxObjectCount = 255, uint16_t maxOrderedPrimitives = 65535, uint16_t batchSize = 1024>
	class ArduinoHost : public HostAddonIntegerWorld<IntegerWorldBase, screenWidth, screenHeight, maxObjectCount, maxOrderedPrimitives, batchSize>
	{
	private:
		using Base = HostAddonIntegerWorld<IntegerWorldBase, screenWidth, screenHeight, maxObjectCount, maxOrderedPrimitives, batchSize>;

	private:
		// Objects animator for the world.
		AnimatedDemoScene DemoScene;

		// Free camera controller.
		Assets::Cameras::FreeCameraTask<> CameraUpdater;

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
		PerformanceLogTask<4000> IntegerWorldEngineLog;
#endif

	private: // Virtual pad buttons parsers.
		VirtualPad::ButtonParser::Action AButton{};
		VirtualPad::ButtonParser::Action R3Button{};
		VirtualPad::ButtonParser::Action L3Button{};
		VirtualPad::ButtonParser::Action R1Button{};
		VirtualPad::ButtonParser::Action L1Button{};

	public:
		ArduinoHost()
			: Base()
			, DemoScene(SchedulerBase)
			, CameraUpdater(SchedulerBase, EngineRenderer.GetCameraControls())
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
			, IntegerWorldEngineLog(SchedulerBase, EngineRenderer)
#endif
		{
		}

		void StartDemo(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& swapChainPanel)
		{
			// Start the 3D engine with panel for output.
			Base::StartEngine(swapChainPanel);

			// Setup and start the demo scene.
			if (!DemoScene.Start(EngineRenderer, screenWidth, screenHeight))
			{
				halt();
			}

			// Reset the camera position and filters.
			CameraUpdater.ResetFilters();

			Serial.println(F("Integer World DirectX Engine"));
		}

	private:
		void halt()
		{
			Serial.println(F("Setup Failed."));
			while (true)
				;
		}

	protected:
		virtual void OnParameterChange(const int parameter, const uint8_t value)
		{
			switch (parameter)
			{
			case IntegerWorldInterface::FoV:
				EngineRenderer.SetFov((uint32_t(UFRACTION16_1X) * (100 - value)) / 100);
				break;
			case IntegerWorldInterface::AnimationEnabled:
				DemoScene.SetAnimationEnabled(value > 0);
				break;
			case IntegerWorldInterface::AmbientShadeEnabled:
				DemoScene.SetAmbientShadeEnabled(value > 0);
				break;
			case IntegerWorldInterface::EmissiveShadeEnabled:
				DemoScene.SetEmissiveShadeEnabled(value > 0);
				break;
			case IntegerWorldInterface::DiffuseShadeEnabled:
				DemoScene.SetDiffuseShadeEnabled(value > 0);
				break;
			case IntegerWorldInterface::SpecularShadeEnabled:
				DemoScene.SetSpecularShadeEnabled(value > 0);
				break;
			case IntegerWorldInterface::SceneShaderEnabled:
				DemoScene.SetSceneShader(value > 0);
				break;
			case IntegerWorldInterface::PixelShaderPipeline:
				DemoScene.SetPixelShader();
				break;
			case IntegerWorldInterface::ZShaderPipeline:
				DemoScene.SetZShader();
				break;
			case IntegerWorldInterface::NormalShaderPipeline:
				DemoScene.SetNormalShader();
				break;
			case IntegerWorldInterface::Light1Enabled:
				DemoScene.SetLight1Enabled(value > 0);
				break;
			case IntegerWorldInterface::Light2Enabled:
				DemoScene.SetLight2Enabled(value > 0);
				break;
			case IntegerWorldInterface::LightGlobalEnabled:
				DemoScene.SetLightGlobalEnabled(value > 0);
				break;
			default:
				break;
			}
		}

		void OnVirtualPadUpdate(VirtualPad::WindowsPad::VirtualPadType& pad) final
		{
			if (pad.Connected())
			{
				AButton.Parse(pad.A());
				R3Button.Parse(pad.R3());
				L3Button.Parse(pad.L3());

				R1Button.Parse(pad.R1());
				L1Button.Parse(pad.L1());

				if (pad.R3() && pad.L3())
				{
					CameraUpdater.Set(0, 0, 0, 0, 0);
				}
				else if (pad.R3())
				{
					CameraUpdater.Set(pad.Joy1X(), pad.Joy1Y(), ((int32_t)pad.L2() - pad.R2()) / 4, 0, 0);
				}
				else if (pad.L3())
				{
					CameraUpdater.Set(0, 0, 0, pad.Joy2X(), pad.Joy2Y());
				}
				else
				{
					CameraUpdater.Set(pad.Joy1X(), pad.Joy1Y(), ((int32_t)pad.L2() - pad.R2()) / 2, pad.Joy2X(), pad.Joy2Y());
				}

				if (AButton.ActionDown())
				{
					DemoScene.CaptureViewFrustum();
				}

				if (R1Button.ActionDown() && !L1Button.ActionDown())
				{
					CameraUpdater.Roll += 1024;
				}

				if (L1Button.ActionDown() && !R1Button.ActionDown())
				{
					CameraUpdater.Roll -= 1024;
				}

				if (R3Button.ActionDown())
				{
					CameraUpdater.ResetCamera();
				}

				if (L3Button.ActionDown())
				{
					CameraUpdater.ResetPosition();
				}
			}
			else
			{
				CameraUpdater.ResetCamera();
				CameraUpdater.ResetPosition();
				CameraUpdater.Roll = 0;
			}
		}
	};
}

