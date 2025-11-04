#pragma once


#include "pch.h"

#include <ArduinoWindowsHost.h>
#include <ArduinoIntegerWorldWindows.h>

#include <IntegerWorldExperimental.h>

namespace DemoSceneWinRT
{
	using namespace ArduinoWindowsHost;

	enum IntegerWorldInterface
	{
		FoV,
		AnimationEnabled,
		FragmentShaderZ,
		FragmentShaderWireframe,
		FragmentShaderLights,

		LightsShaderLightSource,
		LightsShaderNormal,
		LightsShaderNone,

		AmbientShadeEnabled,
		EmissiveShadeEnabled,
		DiffuseShadeEnabled,
		SpecularShadeEnabled,

		LightRedEnabled,
		LightGreenEnabled,
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

	protected:
		using Base::SchedulerBase;

	private:
		// Objects animator for the world.
		AnimatedDemoScene DemoScene;

		// Free camera controller.
		Assets::Cameras::FreeCameraTask<> CameraUpdater;

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
		{
		}

		void StartDemo(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& swapChainPanel)
		{
			LoopHost::PostAndWait([this, &swapChainPanel]()
				{
					// Start the 3D engine with panel for output.
					Base::StartEngine(swapChainPanel);

					// Start the demo scene.
					if (!DemoScene.Start(EngineRenderer, screenWidth, screenHeight))
					{
						halt();
					}

					// Reset the camera position and filters.
					CameraUpdater.ResetFilters();

					FrustumLock = false;

					Serial.println(F("Integer World DirectX Engine"));
				});
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
				EngineRenderer.SetFov((uint32_t(UFRACTION16_1X) * value) / 100);
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
			case IntegerWorldInterface::FragmentShaderZ:
				DemoScene.SetFragmentShaderZ();
				break;
			case IntegerWorldInterface::FragmentShaderWireframe:
				DemoScene.SetFragmentShaderWireframe();
				break;
			case IntegerWorldInterface::FragmentShaderLights:
				DemoScene.SetFragmentShaderLights();
				break;
			case IntegerWorldInterface::LightsShaderLightSource:
				DemoScene.SetLightsShaderLightSource();
				break;
			case IntegerWorldInterface::LightsShaderNormal:
				DemoScene.SetLightsShaderNormal();
				break;
			case IntegerWorldInterface::LightsShaderNone:
				DemoScene.SetLightsShaderNone();
				break;
			case IntegerWorldInterface::LightRedEnabled:
				DemoScene.SetLightRedEnabled(value > 0);
				break;
			case IntegerWorldInterface::LightGreenEnabled:
				DemoScene.SetLightGreenEnabled(value > 0);
				break;
			case IntegerWorldInterface::LightGlobalEnabled:
				DemoScene.SetLightGlobalEnabled(value > 0);
				break;
			default:
				break;
			}
		}


		bool FrustumLock = false;

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

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
				if (AButton.ActionDown())
				{
					DemoScene.CaptureViewFrustum();
					FrustumLock = !FrustumLock;
					EngineRenderer.SetFrustumLock(FrustumLock);
				}
#endif

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

